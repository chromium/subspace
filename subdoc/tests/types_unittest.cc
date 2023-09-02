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
#include "subdoc/tests/subdoc_test.h"
#include "sus/fn/fn.h"
#include "sus/prelude.h"

namespace {

using subdoc::Qualifier;

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

    EXPECT_EQ(t.name, "int");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
    EXPECT_EQ(t.pointers.len(), 0u);
    EXPECT_EQ(t.array_dims.len(), 0u);
    EXPECT_EQ(t.template_params.len(), 0u);
    EXPECT_EQ(t.record_path.len(), 0u);
    EXPECT_EQ(t.namespace_path.len(), 0u);
  });
}

TEST_F(SubDocTypeTest, Nullptr) {
  const char test[] = R"(
    auto f() { return nullptr; };
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::FunctionDecl&> fdecl = find_function("f", cx);
    subdoc::Type t = subdoc::build_local_type(
        fdecl->getReturnType(), cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.name, "nullptr_t");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
    EXPECT_EQ(t.pointers.len(), 0u);
    EXPECT_EQ(t.array_dims.len(), 0u);
    EXPECT_EQ(t.template_params.len(), 0u);
    EXPECT_EQ(t.record_path.len(), 0u);
    EXPECT_EQ(t.namespace_path.len(), 0u);
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

    EXPECT_EQ(t.name, "bool");
    EXPECT_EQ(t.qualifier.is_const, true);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
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

    EXPECT_EQ(t.name, "bool");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, true);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
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

    EXPECT_EQ(t.name, "bool");
    EXPECT_EQ(t.qualifier.is_const, true);
    EXPECT_EQ(t.qualifier.is_volatile, true);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
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

    EXPECT_EQ(t.name, "int");
    EXPECT_EQ(t.qualifier.is_const, true);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::LValueRef);
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

    EXPECT_EQ(t.name, "int");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::LValueRef);
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

    EXPECT_EQ(t.name, "int");
    EXPECT_EQ(t.qualifier.is_const, true);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::RValueRef);
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

    EXPECT_EQ(t.name, "int");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::RValueRef);
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

    EXPECT_EQ(t.name, "int");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
    EXPECT_EQ(t.pointers, sus::vec(Qualifier(false, false)));
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

    EXPECT_EQ(t.name, "int");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::LValueRef);
    EXPECT_EQ(t.pointers, sus::vec(Qualifier(false, false)));
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

    EXPECT_EQ(t.name, "int");
    EXPECT_EQ(t.qualifier.is_const, true);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::LValueRef);
    EXPECT_EQ(t.pointers, sus::vec(Qualifier(false, false)));
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

    EXPECT_EQ(t.name, "int");
    EXPECT_EQ(t.qualifier.is_const, true);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::LValueRef);
    EXPECT_EQ(t.pointers, sus::vec(Qualifier(true, false)));
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

    EXPECT_EQ(t.name, "int");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
    EXPECT_EQ(t.pointers,
              sus::vec(Qualifier(true, false), Qualifier(false, false),
                       Qualifier(true, true), Qualifier(false, false),
                       Qualifier(false, true)));
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

    EXPECT_EQ(t.name, "int");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
    EXPECT_EQ(t.array_dims, sus::vec("5"));
    EXPECT_EQ(t.pointers, sus::vec());
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

    EXPECT_EQ(t.name, "int");
    EXPECT_EQ(t.qualifier.is_const, true);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
    EXPECT_EQ(t.array_dims, sus::vec("5"));
    EXPECT_EQ(t.pointers, sus::vec());
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

    EXPECT_EQ(t.name, "int");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
    EXPECT_EQ(t.array_dims, sus::vec("5", "4", "3", "2", "1"));
    EXPECT_EQ(t.pointers, sus::vec());
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

    EXPECT_EQ(t.name, "int");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
    EXPECT_EQ(t.array_dims, sus::vec(""));
    EXPECT_EQ(t.pointers, sus::vec());
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

    EXPECT_EQ(t.name, "int");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
    EXPECT_EQ(t.array_dims, sus::vec("", "3"));
    EXPECT_EQ(t.pointers, sus::vec());
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

    EXPECT_EQ(t.name, "int");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
    EXPECT_EQ(t.array_dims, sus::vec("", "N", "3"));
    EXPECT_EQ(t.pointers, sus::vec());
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

    EXPECT_EQ(t.name, "int");
    EXPECT_EQ(t.qualifier.is_const, true);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::LValueRef);
    EXPECT_EQ(t.array_dims, sus::vec("3"));
    EXPECT_EQ(t.pointers, sus::vec());
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

    EXPECT_EQ(t.name, "S");
    EXPECT_EQ(t.qualifier.is_const, true);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::LValueRef);
    EXPECT_EQ(t.record_path, sus::vec());
    EXPECT_EQ(t.namespace_path, sus::Vec<std::string>("c", "b", "a"));
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

    EXPECT_EQ(t.name, "S2");
    EXPECT_EQ(t.record_path, sus::vec());
    EXPECT_EQ(t.namespace_path, sus::Vec<std::string>("b", "a"));
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

    EXPECT_EQ(t.name, "S2");
    EXPECT_EQ(t.record_path, sus::vec());
    EXPECT_EQ(t.namespace_path, sus::Vec<std::string>("b", "a"));
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

    EXPECT_EQ(t.name, "auto");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
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

    EXPECT_EQ(t.name, "auto");
    EXPECT_EQ(t.qualifier.is_const, true);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::LValueRef);
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

    EXPECT_EQ(t.name, "auto");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
    EXPECT_EQ(t.pointers, sus::vec(Qualifier(false, false)));
  });
}

TEST_F(SubDocTypeTest, Concept) {
  const char test[] = R"(
    template <class T> concept C = true;
    void f(C auto, C auto);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.name, "auto");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);

    sus::Option<clang::QualType> qual2 =
        find_function("f", cx).map([](clang::FunctionDecl& fdecl) {
          auto it = fdecl.parameters().begin();
          it++;
          return (*it)->getType();
        });
    subdoc::Type t2 =
        subdoc::build_local_type(*qual2, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t2.name, "auto");
    EXPECT_EQ(t2.qualifier.is_const, false);
    EXPECT_EQ(t2.qualifier.is_volatile, false);
    EXPECT_EQ(t2.refs, subdoc::Refs::None);
  });
}

TEST_F(SubDocTypeTest, ConceptWithParam) {
  const char test[] = R"(
    template <class T, unsigned> concept C = true;
    void f(C<5> auto);
  )";
  run_test(test, [](clang::ASTContext& cx, clang::Preprocessor& preprocessor) {
    sus::Option<clang::QualType> qual = find_function_parm("f", cx);
    subdoc::Type t =
        subdoc::build_local_type(*qual, cx.getSourceManager(), preprocessor);

    EXPECT_EQ(t.name, "auto");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::None);
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

    EXPECT_EQ(t.name, "A");
    ASSERT_EQ(t.template_params.len(), 1u);
    EXPECT_EQ(t.template_params[0u].choice,
              subdoc::TypeOrValueTag::DependentType);
    EXPECT_EQ(t.template_params[0u]
                  .choice.as<subdoc::TypeOrValueTag::DependentType>(),
              "T");
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

    EXPECT_EQ(t.name, "A");
    EXPECT_EQ(t.record_path, sus::vec("S"));
    EXPECT_EQ(t.namespace_path, sus::vec("b", "a"));

    ASSERT_EQ(t.template_params.len(), 1u);
    EXPECT_EQ(t.template_params[0u].choice, subdoc::TypeOrValueTag::Type);
    const subdoc::Type& p1 =
        t.template_params[0u].choice.as<subdoc::TypeOrValueTag::Type>();
    EXPECT_EQ(p1.name, "int");
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

    EXPECT_EQ(t.name, "S");
    ASSERT_EQ(t.template_params.len(), 1u);
    EXPECT_EQ(t.template_params[0u].choice,
              subdoc::TypeOrValueTag::DependentType);
    EXPECT_EQ(t.template_params[0u]
                  .choice.as<subdoc::TypeOrValueTag::DependentType>(),
              "T");
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

    EXPECT_EQ(t.name, "C");
    EXPECT_EQ(t.record_path, sus::vec("B", "A"));
    EXPECT_EQ(t.namespace_path, sus::vec("b", "a"));
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

    EXPECT_EQ(t.name, "T_");  // `T` IS WRONG CUZ IT WILL POINT TO `struct T`.
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::LValueRef);
    EXPECT_EQ(t.template_params.len(), 0u);
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

    EXPECT_EQ(t.name, "T_");  // `T` IS WRONG CUZ IT WILL POINT TO `struct T`.
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::LValueRef);
    EXPECT_EQ(t.template_params.len(), 0u);
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

    EXPECT_EQ(t.name, "T_");  // `T` IS WRONG CUZ IT WILL POINT TO `struct T`.
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::LValueRef);
    EXPECT_EQ(t.template_params.len(), 0u);
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

    EXPECT_EQ(t.name, "T_");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::LValueRef);
    EXPECT_EQ(t.template_params.len(), 0u);
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

    EXPECT_EQ(t.name, "T_");
    EXPECT_EQ(t.qualifier.is_const, false);
    EXPECT_EQ(t.qualifier.is_volatile, false);
    EXPECT_EQ(t.refs, subdoc::Refs::LValueRef);
    EXPECT_EQ(t.template_params.len(), 0u);
  });
}

}  // namespace
