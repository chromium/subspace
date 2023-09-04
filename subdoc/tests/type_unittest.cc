// Copyright 2023 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "subdoc/lib/type.h"

#include <sstream>

#include "subdoc/tests/subdoc_test.h"
#include "sus/fn/fn.h"
#include "sus/prelude.h"

namespace {

using subdoc::Qualifier;

std::string make_string(std::string_view var_name, const subdoc::Type& type) {
  std::ostringstream str;
  auto text_fn = [&](std::string_view text) { str << text; };
  auto type_fn = [&](subdoc::TypeToStringQuery q) {
    str << "!" << q.name << "!";
  };
  auto const_fn = [&]() { str << "const"; };
  auto volatile_fn = [&]() { str << "volatile"; };
  auto var_fn = [&]() { str << var_name; };

  subdoc::type_to_string(type, text_fn, type_fn, const_fn, volatile_fn,
                         sus::some(sus::move(var_fn)));
  return sus::move(str).str();
}

sus::Option<clang::FunctionDecl&> find_function(
    std::string_view name, clang::ASTContext& cx) noexcept {
  class Visitor : public clang::RecursiveASTVisitor<Visitor> {
   public:
    Visitor(std::string_view name, sus::Option<clang::FunctionDecl&>& option)
        : name(name), option(option) {}
    bool VisitFunctionDecl(clang::FunctionDecl* decl) noexcept {
      if (std::string_view(decl->getName()) == name) option = sus::some(*decl);
      return true;
    }

    std::string_view name;
    sus::Option<clang::FunctionDecl&>& option;
  };

  sus::Option<clang::FunctionDecl&> option;
  auto v = Visitor(name, option);
  v.TraverseAST(cx);
  return option;
}

sus::Option<clang::QualType> find_function_parm(
    std::string_view name, clang::ASTContext& cx) noexcept {
  return find_function(name, cx).map([](clang::FunctionDecl& fdecl) {
    return (*fdecl.parameters().begin())->getType();
  });
}

struct SubDocTypeTest : public SubDocTest {
  void run_test(std::string code,
                sus::fn::FnMut<void(clang::ASTContext&,
                                    clang::Preprocessor&)> auto body) {
    auto opts = subdoc::RunOptions()           //
                    .set_show_progress(false)  //
                    .set_on_tu_complete(body);
    auto result = run_code_with_options(opts, code);
    ASSERT_TRUE(result.is_ok());
  }
};

TEST_F(SubDocTypeTest, Primitive) {
  const char test[] = R"(
    void f(int);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Type);
    EXPECT_EQ(t.name, "int");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
    EXPECT_EQ(t.pointers.len(), 0u);
    EXPECT_EQ(t.array_dims.len(), 0u);
    EXPECT_EQ(t.template_params.len(), 0u);
    EXPECT_EQ(t.record_path.len(), 0u);
    EXPECT_EQ(t.namespace_path.len(), 0u);

    EXPECT_EQ(make_string("foo", t), "!int! foo");
  });
}

TEST_F(SubDocTypeTest, Bool) {
  const char test[] = R"(
    void f(bool);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Type);
    EXPECT_EQ(t.name, "bool");  // Not "_Bool".

    EXPECT_EQ(make_string("foo", t), "!bool! foo");
  });
}

TEST_F(SubDocTypeTest, Const) {
  const char test[] = R"(
    void f(const bool);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Type);
    EXPECT_EQ(t.name, "bool");
    EXPECT_EQ(t.qualifier.is_const, true);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);

    EXPECT_EQ(make_string("foo", t), "!bool! const foo");
  });
}

TEST_F(SubDocTypeTest, Volatile) {
  const char test[] = R"(
    void f(volatile bool);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Type);
    EXPECT_EQ(t.name, "bool");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, true);
    EXPECT_EQ(t.refs, subdoc::Refs::None);

    EXPECT_EQ(make_string("foo", t), "!bool! volatile foo");
  });
}

TEST_F(SubDocTypeTest, ConstVolatile) {
  const char test[] = R"(
    void f(volatile const bool);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Type);
    EXPECT_EQ(t.name, "bool");
    EXPECT_EQ(t.qualifier.is_const, true);
    EXPECT_EQ(t.qualifier.is_volatile, true);
    EXPECT_EQ(t.refs, subdoc::Refs::None);

    EXPECT_EQ(make_string("foo", t), "!bool! const volatile foo");
  });
}

TEST_F(SubDocTypeTest, ConstRef) {
  const char test[] = R"(
    void f(int const&);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Type);
    EXPECT_EQ(t.name, "int");
    EXPECT_EQ(t.qualifier.is_const, true);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::LValueRef);

    EXPECT_EQ(make_string("foo", t), "!int! const& foo");
  });
}

TEST_F(SubDocTypeTest, MutRef) {
  const char test[] = R"(
    void f(int &);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Type);
    EXPECT_EQ(t.name, "int");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::LValueRef);

    EXPECT_EQ(make_string("foo", t), "!int!& foo");
  });
}

TEST_F(SubDocTypeTest, ConstRRef) {
  const char test[] = R"(
    void f(int const&&);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Type);
    EXPECT_EQ(t.name, "int");
    EXPECT_EQ(t.qualifier.is_const, true);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::RValueRef);

    EXPECT_EQ(make_string("foo", t), "!int! const&& foo");
  });
}

TEST_F(SubDocTypeTest, MutRRef) {
  const char test[] = R"(
    void f(int &&);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Type);
    EXPECT_EQ(t.name, "int");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::RValueRef);

    EXPECT_EQ(make_string("foo", t), "!int!&& foo");
  });
}

TEST_F(SubDocTypeTest, Pointer) {
  const char test[] = R"(
    void f(int*);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Type);
    EXPECT_EQ(t.name, "int");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
    EXPECT_EQ(t.pointers, sus::vec(Qualifier(false, false)));

    EXPECT_EQ(make_string("foo", t), "!int!* foo");
  });
}

TEST_F(SubDocTypeTest, RefToPointer) {
  const char test[] = R"(
    void f(int* &);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Type);
    EXPECT_EQ(t.name, "int");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::LValueRef);
    EXPECT_EQ(t.pointers, sus::vec(Qualifier(false, false)));

    EXPECT_EQ(make_string("foo", t), "!int!*& foo");
  });
}

TEST_F(SubDocTypeTest, ConstRefToPointer) {
  const char test[] = R"(
    void f(int* const&);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Type);
    EXPECT_EQ(t.name, "int");
    EXPECT_EQ(t.qualifier.is_const, true);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::LValueRef);
    EXPECT_EQ(t.pointers, sus::vec(Qualifier(false, false)));

    EXPECT_EQ(make_string("foo", t), "!int!* const& foo");
  });
}

TEST_F(SubDocTypeTest, ConstRefToPointerToConst) {
  const char test[] = R"(
    void f(int const* const&);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Type);
    EXPECT_EQ(t.name, "int");
    EXPECT_EQ(t.qualifier.is_const, true);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::LValueRef);
    EXPECT_EQ(t.pointers, sus::vec(Qualifier(true, false)));

    EXPECT_EQ(make_string("foo", t), "!int! const* const& foo");
  });
}

TEST_F(SubDocTypeTest, PointerQualifiers) {
  const char test[] = R"(
    void f(int const* * const volatile* * volatile*);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Type);
    EXPECT_EQ(t.name, "int");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
    EXPECT_EQ(t.pointers,
              sus::vec(Qualifier(true, false), Qualifier(false, false),
                       Qualifier(true, true), Qualifier(false, false),
                       Qualifier(false, true)));

    EXPECT_EQ(make_string("foo", t),
              "!int! const** const volatile** volatile* foo");
  });
}

TEST_F(SubDocTypeTest, SizedArray) {
  const char test[] = R"(
    void f(int s[5]);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Type);
    EXPECT_EQ(t.name, "int");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
    EXPECT_EQ(t.array_dims, sus::vec("5"));
    EXPECT_EQ(t.pointers, sus::vec());

    EXPECT_EQ(make_string("foo", t), "!int! foo[5]");
  });
}

TEST_F(SubDocTypeTest, QualifiedArray) {
  const char test[] = R"(
    void f(const int s[5]);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Type);
    EXPECT_EQ(t.name, "int");
    EXPECT_EQ(t.qualifier.is_const, true);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
    EXPECT_EQ(t.array_dims, sus::vec("5"));
    EXPECT_EQ(t.pointers, sus::vec());

    EXPECT_EQ(make_string("foo", t), "!int! const foo[5]");
  });
}

TEST_F(SubDocTypeTest, SizedMultiArray) {
  const char test[] = R"(
    void f(int s[5][4][3][2][1]);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Type);
    EXPECT_EQ(t.name, "int");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
    EXPECT_EQ(t.array_dims, sus::vec("5", "4", "3", "2", "1"));
    EXPECT_EQ(t.pointers, sus::vec());

    EXPECT_EQ(make_string("foo", t), "!int! foo[5][4][3][2][1]");
  });
}

TEST_F(SubDocTypeTest, UnsizedArray) {
  const char test[] = R"(
    void f(int s[]);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Type);
    EXPECT_EQ(t.name, "int");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
    EXPECT_EQ(t.array_dims, sus::vec(""));
    EXPECT_EQ(t.pointers, sus::vec());

    EXPECT_EQ(make_string("foo", t), "!int! foo[]");
  });
}

TEST_F(SubDocTypeTest, UnsizedAndSizedArray) {
  const char test[] = R"(
    void f(int s[][3]);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Type);
    EXPECT_EQ(t.name, "int");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
    EXPECT_EQ(t.array_dims, sus::vec("", "3"));
    EXPECT_EQ(t.pointers, sus::vec());

    EXPECT_EQ(make_string("foo", t), "!int! foo[][3]");
  });
}

TEST_F(SubDocTypeTest, DependentArray) {
  const char test[] = R"(
    template <unsigned N>
    void f(int s[][N][3]);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Type);
    EXPECT_EQ(t.name, "int");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
    EXPECT_EQ(t.array_dims, sus::vec("", "N", "3"));
    EXPECT_EQ(t.pointers, sus::vec());

    EXPECT_EQ(make_string("foo", t), "!int! foo[][N][3]");
  });
}

TEST_F(SubDocTypeTest, SizedArrayRef) {
  const char test[] = R"(
    template <unsigned N>
    void f(const int (&s)[3]);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Type);
    EXPECT_EQ(t.name, "int");
    EXPECT_EQ(t.qualifier.is_const, true);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::LValueRef);
    EXPECT_EQ(t.array_dims, sus::vec("3"));
    EXPECT_EQ(t.pointers, sus::vec());

    EXPECT_EQ(make_string("foo", t), "!int! const (&foo)[3]");
  });
}

TEST_F(SubDocTypeTest, SizedArrayRvalueRef) {
  const char test[] = R"(
    template <unsigned N>
    void f(volatile int (&&s)[3]);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Type);
    EXPECT_EQ(t.name, "int");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, true);
    EXPECT_EQ(t.refs, subdoc::Refs::RValueRef);
    EXPECT_EQ(t.array_dims, sus::vec("3"));
    EXPECT_EQ(t.pointers, sus::vec());

    EXPECT_EQ(make_string("foo", t), "!int! volatile (&&foo)[3]");
  });
}

TEST_F(SubDocTypeTest, NamespaceReference) {
  const char test[] = R"(
    namespace a::b::c { struct S {}; }
    void f(const a::b::c::S&);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Type);
    EXPECT_EQ(t.name, "S");
    EXPECT_EQ(t.qualifier.is_const, true);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::LValueRef);
    EXPECT_EQ(t.record_path, sus::vec());
    EXPECT_EQ(t.namespace_path, sus::Vec<std::string>("c", "b", "a"));

    EXPECT_EQ(make_string("foo", t), "!S! const& foo");
  });
}

TEST_F(SubDocTypeTest, NamespaceTypedefReference) {
  const char test[] = R"(
    namespace a::b::c { struct S {}; }
    namespace a::b { typedef c::S S2; }
    void f(a::b::S2);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Type);
    EXPECT_EQ(t.name, "S2");
    EXPECT_EQ(t.record_path, sus::vec());
    EXPECT_EQ(t.namespace_path, sus::Vec<std::string>("b", "a"));

    EXPECT_EQ(make_string("foo", t), "!S2! foo");
  });
}

TEST_F(SubDocTypeTest, NamespaceUsingReference) {
  const char test[] = R"(
    namespace a::b::c { struct S {}; }
    namespace a::b { using S2 = c::S; }
    void f(a::b::S2);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Type);
    EXPECT_EQ(t.name, "S2");
    EXPECT_EQ(t.record_path, sus::vec());
    EXPECT_EQ(t.namespace_path, sus::Vec<std::string>("b", "a"));

    EXPECT_EQ(make_string("foo", t), "!S2! foo");
  });
}

TEST_F(SubDocTypeTest, Auto) {
  const char test[] = R"(
    void f(auto);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::TemplateVariable);
    EXPECT_EQ(t.name, "auto");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);

    EXPECT_EQ(make_string("foo", t), "auto foo");
  });
}

TEST_F(SubDocTypeTest, AutoRef) {
  const char test[] = R"(
    void f(auto const&);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::TemplateVariable);
    EXPECT_EQ(t.name, "auto");
    EXPECT_EQ(t.qualifier.is_const, true);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::LValueRef);

    EXPECT_EQ(make_string("foo", t), "auto const& foo");
  });
}

TEST_F(SubDocTypeTest, AutoPointer) {
  const char test[] = R"(
    void f(auto*);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::TemplateVariable);
    EXPECT_EQ(t.name, "auto");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
    EXPECT_EQ(t.pointers, sus::vec(Qualifier(false, false)));

    EXPECT_EQ(make_string("foo", t), "auto* foo");
  });
}

TEST_F(SubDocTypeTest, Concept) {
  const char test[] = R"(
    namespace a::b { template <class T> concept C = true; }
    void f(a::b::C auto, const a::b::C auto&);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Concept);
    EXPECT_EQ(t.name, "C");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
    EXPECT_EQ(t.namespace_path, sus::vec("b", "a"));

    sus::Option<clang::QualType> qual2 =
        find_function("f", cx).map([](clang::FunctionDecl& fdecl) {
          auto it = fdecl.parameters().begin();
          it++;
          return (*it)->getType();
        });
    subdoc::Type t2 =
        subdoc::build_local_type(*qual2, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Concept);
    EXPECT_EQ(t2.name, "C");
    EXPECT_EQ(t2.qualifier.is_const, true);
    EXPECT_EQ(t2.qualifier.is_volatile, false);
    EXPECT_EQ(t2.refs, subdoc::Refs::LValueRef);
    EXPECT_EQ(t2.namespace_path, sus::vec("b", "a"));

    EXPECT_EQ(make_string("foo", t), "!C! auto foo");
    EXPECT_EQ(make_string("foo", t2), "!C! auto const& foo");
  });
}

TEST_F(SubDocTypeTest, ConceptReturn) {
  const char test[] = R"(
    namespace a::b { template <class T, unsigned> concept C = true; }
    a::b::C<1 + 3> auto f();
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function("f", cx).map(
        [](clang::FunctionDecl& fdecl) { return fdecl.getReturnType(); });
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Concept);
    EXPECT_EQ(t.name, "C");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
    EXPECT_EQ(t.namespace_path, sus::vec("b", "a"));
    const std::string& p1 =
        t.template_params[0u].choice.as<subdoc::TypeOrValueTag::Value>();
    EXPECT_EQ(p1, "1 + 3");

    EXPECT_EQ(make_string("foo", t), "!C!<1 + 3> auto foo");
  });
}

TEST_F(SubDocTypeTest, ConceptReturnWithBody) {
  const char test[] = R"(
    namespace a::b { template <class T, unsigned> concept C = true; }
    a::b::C<1 + 3> auto f() { return 1; }
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function("f", cx).map(
        [](clang::FunctionDecl& fdecl) { return fdecl.getReturnType(); });
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Concept);
    EXPECT_EQ(t.name, "C");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
    EXPECT_EQ(t.namespace_path, sus::vec("b", "a"));
    const std::string& p1 =
        t.template_params[0u].choice.as<subdoc::TypeOrValueTag::Value>();
    EXPECT_EQ(p1, "1 + 3");

    EXPECT_EQ(make_string("foo", t), "!C!<1 + 3> auto foo");
  });
}

TEST_F(SubDocTypeTest, ConceptWithParam) {
  const char test[] = R"(
    namespace a::b { template <class T, unsigned> concept C = true; }
    void f(a::b::C<5 + 2> auto);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Concept);
    EXPECT_EQ(t.name, "C");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
    EXPECT_EQ(t.namespace_path, sus::vec("b", "a"));
    const std::string& p1 =
        t.template_params[0u].choice.as<subdoc::TypeOrValueTag::Value>();
    EXPECT_EQ(p1, "5 + 2");

    EXPECT_EQ(make_string("foo", t), "!C!<5 + 2> auto foo");
  });
}

TEST_F(SubDocTypeTest, ConceptWithDependentTypeParam) {
  const char test[] = R"(
    namespace a::b { template <class T, class U> concept C = true; }
    template <class T>
    void f(a::b::C<T volatile* const&&> auto);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Concept);
    EXPECT_EQ(t.name, "C");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
    EXPECT_EQ(t.namespace_path, sus::vec("b", "a"));
    const subdoc::Type& p1 =
        t.template_params[0u].choice.as<subdoc::TypeOrValueTag::Type>();
    EXPECT_EQ(p1.category, subdoc::TypeCategory::TemplateVariable);
    EXPECT_EQ(p1.name, "T");
    EXPECT_EQ(p1.qualifier.is_const, true);
    EXPECT_EQ(p1.refs, subdoc::Refs::RValueRef);
    EXPECT_EQ(p1.pointers, sus::Vec<Qualifier>(subdoc::Qualifier(false, true)));

    EXPECT_EQ(make_string("foo", t), "!C!<T volatile* const&&> auto foo");
  });
}

TEST_F(SubDocTypeTest, ConceptWithTypeParam) {
  const char test[] = R"(
    namespace a::b { template <class T, class U> concept C = true; }
    namespace c::d { struct E {}; }
    void f(a::b::C<c::d::E volatile* const&&> auto);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Concept);
    EXPECT_EQ(t.name, "C");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
    EXPECT_EQ(t.namespace_path, sus::vec("b", "a"));
    const subdoc::Type& p1 =
        t.template_params[0u].choice.as<subdoc::TypeOrValueTag::Type>();
    EXPECT_EQ(p1.category, subdoc::TypeCategory::Type);
    EXPECT_EQ(p1.name, "E");
    EXPECT_EQ(p1.qualifier.is_const, true);
    EXPECT_EQ(p1.refs, subdoc::Refs::RValueRef);
    EXPECT_EQ(p1.pointers, sus::Vec<Qualifier>(subdoc::Qualifier(false, true)));

    EXPECT_EQ(make_string("foo", t), "!C!<!E! volatile* const&&> auto foo");
  });
}

TEST_F(SubDocTypeTest, ConceptWithPack) {
  const char test[] = R"(
    namespace a::b { template <class... T> concept C = true; }
    template <class... T>
    void f(a::b::C<T...> auto);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Concept);
    EXPECT_EQ(t.name, "C");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
    EXPECT_EQ(t.namespace_path, sus::vec("b", "a"));
    const subdoc::Type& p1 =
        t.template_params[0u].choice.as<subdoc::TypeOrValueTag::Type>();
    EXPECT_EQ(p1.category, subdoc::TypeCategory::TemplateVariable);
    EXPECT_EQ(p1.name, "T...");
    EXPECT_EQ(p1.qualifier.is_const, false);
    EXPECT_EQ(p1.qualifier.is_volatile, false);
    EXPECT_EQ(p1.refs, subdoc::Refs::None);
    EXPECT_EQ(p1.namespace_path, sus::vec());

    EXPECT_EQ(make_string("foo", t), "!C!<T...> auto foo");
  });
}

TEST_F(SubDocTypeTest, AliasTemplate) {
  const char test[] = R"(
    template <class T> struct S {};
    template <class T> using A = S<T>;
    template <class T>
    void f(A<T>);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Type);
    EXPECT_EQ(t.name, "A");
    ASSERT_EQ(t.template_params.len(), 1u);
    EXPECT_EQ(t.template_params[0u].choice, subdoc::TypeOrValueTag::Type);
    EXPECT_EQ(t.template_params[0u]
                  .choice.as<subdoc::TypeOrValueTag::Type>()
                  .category,
              subdoc::TypeCategory::TemplateVariable);
    EXPECT_EQ(
        t.template_params[0u].choice.as<subdoc::TypeOrValueTag::Type>().name,
        "T");

    EXPECT_EQ(make_string("foo", t), "!A!<T> foo");
  });
}

TEST_F(SubDocTypeTest, NestedAliasTemplate) {
  const char test[] = R"(
    namespace a::b {
      template <class T> struct S { template <class U> using A = U; };
    }
    template <class T>
    void f(a::b::S<char>::template A<int>);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Type);
    EXPECT_EQ(t.name, "A");
    EXPECT_EQ(t.record_path, sus::vec("S"));
    EXPECT_EQ(t.namespace_path, sus::vec("b", "a"));

    ASSERT_EQ(t.template_params.len(), 1u);
    EXPECT_EQ(t.template_params[0u].choice, subdoc::TypeOrValueTag::Type);
    const subdoc::Type& p1 =
        t.template_params[0u].choice.as<subdoc::TypeOrValueTag::Type>();
    EXPECT_EQ(p1.name, "int");

    EXPECT_EQ(make_string("foo", t), "!A!<!int!> foo");
  });
}

TEST_F(SubDocTypeTest, DependentTypeInTemplate) {
  const char test[] = R"(
    template <class T> struct S {};
    template <class T>
    void f(S<T>);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Type);
    EXPECT_EQ(t.name, "S");
    ASSERT_EQ(t.template_params.len(), 1u);
    EXPECT_EQ(t.template_params[0u].choice, subdoc::TypeOrValueTag::Type);
    EXPECT_EQ(t.template_params[0u]
                  .choice.as<subdoc::TypeOrValueTag::Type>()
                  .category,
              subdoc::TypeCategory::TemplateVariable);
    EXPECT_EQ(
        t.template_params[0u].choice.as<subdoc::TypeOrValueTag::Type>().name,
        "T");

    EXPECT_EQ(make_string("foo", t), "!S!<T> foo");
  });
}

TEST_F(SubDocTypeTest, DependentTypeQualified) {
  const char test[] = R"(
    template <class T> struct S {};
    template <class T>
    void f(S<const T&>);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Type);
    EXPECT_EQ(t.name, "S");
    ASSERT_EQ(t.template_params.len(), 1u);
    EXPECT_EQ(t.template_params[0u].choice, subdoc::TypeOrValueTag::Type);
    const subdoc::Type& p1 =
        t.template_params[0u].choice.as<subdoc::TypeOrValueTag::Type>();
    EXPECT_EQ(p1.category, subdoc::TypeCategory::TemplateVariable);
    EXPECT_EQ(p1.name, "T");
    EXPECT_EQ(p1.qualifier.is_const, true);
    EXPECT_EQ(p1.refs, subdoc::Refs::LValueRef);

    EXPECT_EQ(make_string("foo", t), "!S!<T const&> foo");
  });
}

TEST_F(SubDocTypeTest, DependentTypePointer) {
  const char test[] = R"(
    template <class T> struct S {};
    template <class T>
    void f(S<T volatile*>);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Type);
    EXPECT_EQ(t.name, "S");
    ASSERT_EQ(t.template_params.len(), 1u);
    const subdoc::Type& p1 =
        t.template_params[0u].choice.as<subdoc::TypeOrValueTag::Type>();
    EXPECT_EQ(p1.category, subdoc::TypeCategory::TemplateVariable);
    EXPECT_EQ(p1.name, "T");
    EXPECT_EQ(p1.qualifier.is_const, false);
    EXPECT_EQ(p1.refs, subdoc::Refs::None);
    EXPECT_EQ(p1.pointers, sus::vec(subdoc::Qualifier(false, true)));

    EXPECT_EQ(make_string("foo", t), "!S!<T volatile*> foo");
  });
}

TEST_F(SubDocTypeTest, NestedClassMultiple) {
  const char test[] = R"(
    namespace a::b { struct A { struct B { struct C {}; }; }; }
    void f(a::b::A::B::C);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Type);
    EXPECT_EQ(t.name, "C");
    EXPECT_EQ(t.record_path, sus::vec("B", "A"));
    EXPECT_EQ(t.namespace_path, sus::vec("b", "a"));

    EXPECT_EQ(make_string("foo", t), "!C! foo");
  });
}

TEST_F(SubDocTypeTest, DependentTypeAsParam) {
  const char test[] = R"(
    struct T {};

    template <class T>
    void f(T&);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::TemplateVariable);
    EXPECT_EQ(t.name, "T");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::LValueRef);
    EXPECT_EQ(t.template_params.len(), 0u);

    EXPECT_EQ(make_string("foo", t), "T& foo");
  });
}

TEST_F(SubDocTypeTest, DependentTypeFromClassAsParam) {
  const char test[] = R"(
    struct T {};

    template <class T>
    struct S {
      static void f(T&);
    };
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::TemplateVariable);
    EXPECT_EQ(t.name, "T");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::LValueRef);
    EXPECT_EQ(t.template_params.len(), 0u);

    EXPECT_EQ(make_string("foo", t), "T& foo");
  });
}

TEST_F(SubDocTypeTest, DependentTypeFromClassAsParamOnTemplateFunction) {
  const char test[] = R"(
    struct T {};

    template <class T>
    struct S {
      template <class U>
      static void f(T&);
    };
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::TemplateVariable);
    EXPECT_EQ(t.name, "T");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::LValueRef);
    EXPECT_EQ(t.template_params.len(), 0u);

    EXPECT_EQ(make_string("foo", t), "T& foo");
  });
}

TEST_F(SubDocTypeTest, DependentTypeAsParamWithRequires) {
  const char test[] = R"(
    struct T {};

    template <class T> concept C = true;
    template <class T>
      requires(C<T>)
    void f(T&);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::TemplateVariable);
    EXPECT_EQ(t.name, "T");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::LValueRef);
    EXPECT_EQ(t.template_params.len(), 0u);

    EXPECT_EQ(make_string("foo", t), "T& foo");
  });
}

TEST_F(SubDocTypeTest, DependentTypeAsParamWithConcept) {
  const char test[] = R"(
    struct T {};

    template <class T> concept C = true;
    template <C T>
    void f(T&);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::TemplateVariable);
    EXPECT_EQ(t.name, "T");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::LValueRef);
    EXPECT_EQ(t.template_params.len(), 0u);

    EXPECT_EQ(make_string("foo", t), "T& foo");
  });
}

TEST_F(SubDocTypeTest, AutoReturn) {
  const char test[] = R"(
    auto f();
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function("f", cx).map(
        [](clang::FunctionDecl& fdecl) { return fdecl.getReturnType(); });
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::TemplateVariable);
    EXPECT_EQ(t.name, "auto");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);

    EXPECT_EQ(make_string("foo", t), "auto foo");
  });
}

TEST_F(SubDocTypeTest, AutoReturnWithBody) {
  const char test[] = R"(
    namespace a::b { struct S{}; }
    auto f() { return a::b::S(); }
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function("f", cx).map(
        [](clang::FunctionDecl& fdecl) { return fdecl.getReturnType(); });
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::TemplateVariable);
    EXPECT_EQ(t.name, "auto");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
    EXPECT_EQ(t.namespace_path, sus::vec());  // Don't use the namespace of `S`.

    EXPECT_EQ(make_string("foo", t), "auto foo");
  });
}

TEST_F(SubDocTypeTest, AutoReturnQualified) {
  const char test[] = R"(
    auto&& f();
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function("f", cx).map(
        [](clang::FunctionDecl& fdecl) { return fdecl.getReturnType(); });
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::TemplateVariable);
    EXPECT_EQ(t.name, "auto");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::RValueRef);

    EXPECT_EQ(make_string("foo", t), "auto&& foo");
  });
}

TEST_F(SubDocTypeTest, AutoReturnPointer) {
  const char test[] = R"(
    auto* f();
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function("f", cx).map(
        [](clang::FunctionDecl& fdecl) { return fdecl.getReturnType(); });
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::TemplateVariable);
    EXPECT_EQ(t.name, "auto");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
    EXPECT_EQ(t.pointers, sus::vec(subdoc::Qualifier(false, false)));

    EXPECT_EQ(make_string("foo", t), "auto* foo");
  });
}

TEST_F(SubDocTypeTest, AutoDecltypeReturn) {
  const char test[] = R"(
    decltype(auto) f();
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function("f", cx).map(
        [](clang::FunctionDecl& fdecl) { return fdecl.getReturnType(); });
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::TemplateVariable);
    EXPECT_EQ(t.name, "decltype(auto)");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);

    EXPECT_EQ(make_string("foo", t), "decltype(auto) foo");
  });
}

TEST_F(SubDocTypeTest, AutoDecltypeReturnWithBody) {
  const char test[] = R"(
    namespace a::b { struct S{}; }
    decltype(auto) f() { return a::b::S(); }
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function("f", cx).map(
        [](clang::FunctionDecl& fdecl) { return fdecl.getReturnType(); });
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::TemplateVariable);
    EXPECT_EQ(t.name, "decltype(auto)");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
    EXPECT_EQ(t.namespace_path, sus::vec());  // Don't use the namespace of `S`.

    EXPECT_EQ(make_string("foo", t), "decltype(auto) foo");
  });
}

TEST_F(SubDocTypeTest, DecltypeParam) {
  const char test[] = R"(
    namespace a::b { struct C {}; }
    void f(decltype(a::b::C()));
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::TemplateVariable);
    EXPECT_EQ(t.name, "decltype(a::b::C())");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
    EXPECT_EQ(t.pointers.len(), 0u);
    EXPECT_EQ(t.array_dims.len(), 0u);
    EXPECT_EQ(t.template_params.len(), 0u);
    EXPECT_EQ(t.record_path.len(), 0u);
    EXPECT_EQ(t.namespace_path.len(), 0u);

    EXPECT_EQ(make_string("foo", t), "decltype(a::b::C()) foo");
  });
}

TEST_F(SubDocTypeTest, DecltypeReturnType) {
  const char test[] = R"(
    namespace a::b { struct C {}; }
    decltype(a::b::C()) f() { return a::b::C(); };
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::FunctionDecl&> fdecl = find_function("f", cx);
    subdoc::Type t = subdoc::build_local_type(
        fdecl->getReturnType(), cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::TemplateVariable);
    EXPECT_EQ(t.name, "decltype(a::b::C())");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
    EXPECT_EQ(t.pointers.len(), 0u);
    EXPECT_EQ(t.array_dims.len(), 0u);
    EXPECT_EQ(t.template_params.len(), 0u);
    EXPECT_EQ(t.record_path.len(), 0u);
    EXPECT_EQ(t.namespace_path.len(), 0u);

    EXPECT_EQ(make_string("foo", t), "decltype(a::b::C()) foo");
  });
}

TEST_F(SubDocTypeTest, ConceptReturnWithTypeParam) {
  const char test[] = R"(
    namespace a::b { template <class T, class... U> concept C = true; }
    namespace c::d { struct E {}; }
    a::b::C<c::d::E> auto f();

    template <class T>
    struct S {
      a::b::C<c::d::E> auto g();
    };
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function("f", cx).map(
        [](clang::FunctionDecl& fdecl) { return fdecl.getReturnType(); });
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Concept);
    EXPECT_EQ(t.name, "C");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
    EXPECT_EQ(t.namespace_path, sus::vec("b", "a"));
    const subdoc::Type& p1 =
        t.template_params[0u].choice.as<subdoc::TypeOrValueTag::Type>();
    EXPECT_EQ(p1.category, subdoc::TypeCategory::Type);
    EXPECT_EQ(p1.name, "E");
    EXPECT_EQ(p1.namespace_path, sus::vec("d", "c"));

    EXPECT_EQ(make_string("foo", t), "!C!<!E!> auto foo");

    sus::Option<clang::QualType> qual2 = find_function("g", cx).map(
        [](clang::FunctionDecl& fdecl) { return fdecl.getReturnType(); });
    subdoc::Type t2 =
        subdoc::build_local_type(*qual2, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t2.category, subdoc::TypeCategory::Concept);
    EXPECT_EQ(t2.name, "C");
    EXPECT_EQ(t2.qualifier.is_const, false);
    EXPECT_EQ(t2.qualifier.is_volatile, false);
    EXPECT_EQ(t2.refs, subdoc::Refs::None);
    EXPECT_EQ(t2.namespace_path, sus::vec("b", "a"));
    const subdoc::Type& p21 =
        t2.template_params[0u].choice.as<subdoc::TypeOrValueTag::Type>();
    EXPECT_EQ(p21.category, subdoc::TypeCategory::Type);
    EXPECT_EQ(p21.name, "E");
    EXPECT_EQ(p21.namespace_path, sus::vec("d", "c"));

    EXPECT_EQ(make_string("foo", t2), "!C!<!E!> auto foo");
  });
}

// The type in the concept is a template, rather than a specialization of a
// template, which is not valid(?) but is accepted by compilers. So we should
// handle it.
TEST_F(SubDocTypeTest, ConceptReturnWithTemplate) {
  const char test[] = R"(
    namespace a::b { template <class T, class... U> concept C = true; }
    namespace c::d { template <class T> struct E {}; }
    a::b::C<c::d::E> auto f();
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function("f", cx).map(
        [](clang::FunctionDecl& fdecl) { return fdecl.getReturnType(); });
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Concept);
    EXPECT_EQ(t.name, "C");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
    EXPECT_EQ(t.namespace_path, sus::vec("b", "a"));
    const std::string& p1 =
        t.template_params[0u].choice.as<subdoc::TypeOrValueTag::Value>();
    EXPECT_EQ(p1, "c::d::E");

    EXPECT_EQ(make_string("foo", t), "!C!<c::d::E> auto foo");
  });
}

TEST_F(SubDocTypeTest, ConceptReturnWithTemplateTemplate) {
  const char test[] = R"(
    namespace a::b { template <class T, class... U> concept C = true; }
    template <template<class> class T> 
    a::b::C<T> auto f();
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function("f", cx).map(
        [](clang::FunctionDecl& fdecl) { return fdecl.getReturnType(); });
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Concept);
    EXPECT_EQ(t.name, "C");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
    EXPECT_EQ(t.namespace_path, sus::vec("b", "a"));
    const std::string& p1 =
        t.template_params[0u].choice.as<subdoc::TypeOrValueTag::Value>();
    EXPECT_EQ(p1, "T");

    EXPECT_EQ(make_string("foo", t), "!C!<T> auto foo");
  });
}

TEST_F(SubDocTypeTest, ConceptReturnWithPack) {
  const char test[] = R"(
    namespace a::b { template <class T, class... U> concept C = true; }
    template <class... T>
    a::b::C<T...> auto f();
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function("f", cx).map(
        [](clang::FunctionDecl& fdecl) { return fdecl.getReturnType(); });
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.category, subdoc::TypeCategory::Concept);
    EXPECT_EQ(t.name, "C");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
    EXPECT_EQ(t.namespace_path, sus::vec("b", "a"));
    const subdoc::Type& p1 =
        t.template_params[0u].choice.as<subdoc::TypeOrValueTag::Type>();
    EXPECT_EQ(p1.category, subdoc::TypeCategory::TemplateVariable);
    EXPECT_EQ(p1.name, "T...");
    EXPECT_EQ(p1.namespace_path, sus::vec());

    EXPECT_EQ(make_string("foo", t), "!C!<T...> auto foo");
  });
}

}  // namespace
