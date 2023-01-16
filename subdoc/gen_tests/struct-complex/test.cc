// This tests static and non-static members. They are intermixed and will
// come out grouped and in alphabetical order.

/// Comment headline S
struct S {
  /// Comment headline void_method
  void void_method() const& {}
  /// Comment headline static_int_method
  static int static_int_method() {}
  /// Comment headline int_method
  int int_method() {}
  /// Comment headline static_bool_method
  static bool static_bool_method() {}

  // Overload should be grouped with the other int_method().
  void int_method() volatile {}

  /// Comment headline integer_field
  const int integer_field;
  /// Comment headline static_int_member
  static volatile int static_int_member;
  /// Comment headline bool_field
  bool bool_field;
  /// Comment headline static_bool_member
  static bool static_bool_member;
};
