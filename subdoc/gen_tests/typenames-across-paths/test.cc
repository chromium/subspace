namespace other {
struct S {
  struct Nested {};
};
}  // namespace other

namespace n {

// Should show `S` as the return type, not the full path.
other::S return_s();

// Should show `Nested` as the return type, not the full path.
other::S::Nested return_nested();

// Should show `S` as the paramter type, not the full path.
void pass_s(other::S);

struct HoldS {
  // Should show `S` as the field type, not the full path, and link to
  // `other::S`.
  other::S s;
  // Should show `Nested` as the field type, not the full path, and link to
  // `other::S::Nested`.
  other::S::Nested nested;
};

struct FunctionParams {
  /// A const lvalue ref.
  static auto const_ref(const other::S& s) -> const other::S&;
  /// A mut lvalue ref.
  static auto mut_ref(other::S& s) -> other::S&;
  /// A mut rvalue ref.
  static auto rvalue_ref(other::S&& s) -> other::S&&;
  /// A const rvalue ref.
  static auto const_rvalue_ref(other::S const&& s) -> other::S const&&;
  /// A mut pointer `s` to a mut S.
  static auto pointer(other::S* s) -> other::S*;
  /// A const pointer `s` to a mut S.
  static auto pointer_const(other::S* const s) -> other::S*;
  /// A mut pointer `s` to a const S.
  static auto const_pointer(const other::S* s) -> const other::S*;
  /// A const pointer `s` to a const S.
  static auto const_pointer_const(const other::S* const s) -> const other::S*;
  /// A const pointer `s` to a volatile pointer to a const S.
  static auto multi_pointer(other::S const* volatile* const s)
      -> other::S const* volatile*;
  /// A reference to a const pointer `s` to a const S.
  static auto const_ref_pointer(const other::S* const& s)
      -> const other::S* const&;
  /// A reference to a mut pointer `s` to a const S.
  static auto mut_ref_pointer(const other::S*& s) -> const other::S*&;
};

}  // namespace n
