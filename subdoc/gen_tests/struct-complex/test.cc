// This tests static and non-static members. They are intermixed and will
// come out grouped and in alphabetical order.

struct OtherType {};

/// Comment headline S
struct S {
  /// Comment headline void_method
  void void_method() const& {}
  /// Comment headline static_type_method
  static OtherType static_type_method() {}
  /// Comment headline type_method
  OtherType type_method() {}
  /// Comment headline static_bool_method
  static bool static_bool_method() {}

  // Overload should be grouped with the other int_method().
  void int_method() volatile {}

  /// Comment headline type_field
  const OtherType type_field;
  /// Comment headline static_type_member
  static volatile OtherType static_type_member;
  /// Comment headline bool_field
  bool bool_field;
  /// Comment headline static_bool_member
  static bool static_bool_member;
};
