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

}  // namespace n
