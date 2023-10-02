namespace other::subother {
struct S {
  struct FirstNested {
    struct Nested {};
  };
};

template <class T>
concept C = true;

void subother_func() {}
/// #[doc.overloads=hasanoverload]
void subother_func_with_overload() {}

enum class A {
  AValue,
};

int var;

}  // namespace other::subother

namespace n {

/// Forwarding alias of [`S`]($other::subother::S).
using other::subother::S;
/// Forwarding alias of [`C`]($other::subother::C).
using other::subother::C;
/// Forwarding alias of [`subother_func`]($other::subother::subother_func).
using other::subother::subother_func;
/// Forwarding alias of [`subother_func`](
/// $other::subother::subother_func_with_overload!hasanoverload) can't link to
/// the named overload, needs a doc attribute or something?
using other::subother::subother_func_with_overload;
/// Forwarding alias of a value from [`A`]($other::subother::A).
using other::subother::A::AValue;
/// Forwarding alias of [`var`]($other::subother::var).
using other::subother::var;

namespace __private {
  struct PrivateS {};
  template <class T> concept PrivateC = true;
  void private_func() {}
  enum class PrivateA {
    PrivateAValue,
  };
  template <class T> struct TemplateClass {};
  int private_var;
}

/// Alias of `PrivateS` which is not in the database.
using __private::PrivateS;
/// Alias of `PrivateC` which is not in the database.
using __private::PrivateC;
/// Alias of `private_func` which is not in the database.
using __private::private_func;
/// Alias of `PrivateAValue` which is not in the database.
using __private::PrivateA::PrivateAValue;
/// Alias of `private_var` which is not in the database.
using __private::private_var;

template<>
struct __private::TemplateClass<int> {
  // Ignored since it's in a hidden namespace.
  using Thing = ::other::subother::S;
  using ::other::subother::A::AValue;
};

/// Renamed alias of [`S`]($other::subother::S).
using RenamedUsingS = other::subother::S;
/// Renamed alias of [`S`]($other::subother::S).
///
/// A reference to [RenamedUsingS]($n::RenamedUsingS).
typedef other::subother::S RenamedTypedefS;

/// Should show `S` as the return type, not the full path.
other::subother::S return_s();

/// Should show `Nested` as the return type, not the full path.
///
/// A reference to [var]($other::subother::var).
other::subother::S::FirstNested::Nested return_nested();

/// Should show `S` as the paramter type, not the full path.
void pass_s(other::subother::S);

struct HoldS {
  /// Should show `S` as the field type, not the full path, and link to
  /// `other::subother::S`.
  other::subother::S s;
  /// Should show `Nested` as the field type, not the full path, and link to
  /// `other::subother::S::FirstNested::Nested`.
  other::subother::S::FirstNested::Nested nested;
};

struct FunctionParams {
  using AliasInStruct = other::subother::S;

  /// A const lvalue ref.
  static auto const_ref(const other::subother::S& s) -> const other::subother::S&;
  /// A mut lvalue ref.
  static auto mut_ref(other::subother::S& s) -> other::subother::S&;
  /// A mut rvalue ref.
  static auto rvalue_ref(other::subother::S&& s) -> other::subother::S&&;
  /// A const rvalue ref.
  static auto const_rvalue_ref(other::subother::S const&& s) -> other::subother::S const&&;
  /// A mut pointer `s` to a mut S.
  static auto pointer(other::subother::S* s) -> other::subother::S*;
  /// A const pointer `s` to a mut S.
  static auto pointer_const(other::subother::S* const s) -> other::subother::S*;
  /// A mut pointer `s` to a const S.
  static auto const_pointer(const other::subother::S* s) -> const other::subother::S*;
  /// A const pointer `s` to a const S.
  static auto const_pointer_const(const other::subother::S* const s) -> const other::subother::S*;
  /// A const pointer `s` to a volatile pointer to a const S.
  static auto multi_pointer(other::subother::S const* volatile* const s)
      -> other::subother::S const* volatile*;
  /// A reference to a const pointer `s` to a const S.
  static auto const_ref_pointer(const other::subother::S* const& s)
      -> const other::subother::S* const&;
  /// A reference to a mut pointer `s` to a const S.
  static auto mut_ref_pointer(const other::subother::S*& s) -> const other::subother::S*&;
};

}  // namespace n
