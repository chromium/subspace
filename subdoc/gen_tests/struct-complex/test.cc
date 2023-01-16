// This tests static and non-static data members. They are intermixed and will
// come out grouped and in alphabetical order.

/// Comment headline S
struct S {
  /// Comment headline integer_field
  const int integer_field;
  /// Comment headline static_int_member
  static volatile int static_int_member;
  /// Comment headline bool_field
  bool bool_field;
  /// Comment headline static_bool_member
  static bool static_bool_member;
};
