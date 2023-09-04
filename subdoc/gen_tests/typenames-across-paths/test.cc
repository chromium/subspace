namespace other::subother {
struct S {
  struct FirstNested {
    struct Nested {};
  };
};
}  // namespace other

namespace n {

// Should show `S` as the return type, not the full path.
other::subother::S return_s();

// Should show `Nested` as the return type, not the full path.
other::subother::S::FirstNested::Nested return_nested();

// Should show `S` as the paramter type, not the full path.
void pass_s(other::subother::S);

struct HoldS {
  // Should show `S` as the field type, not the full path, and link to
  // `other::subother::S`.
  other::subother::S s;
  // Should show `Nested` as the field type, not the full path, and link to
  // `other::subother::S::FirstNested::Nested`.
  other::subother::S::FirstNested::Nested nested;
};

struct FunctionParams {
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
