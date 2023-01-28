namespace other {
struct S {};
}  // namespace other

namespace n {

// Should show `S` as the return type, not the full path.
other::S return_s();

// Should show `S` as the paramter type, not the full path.
void pass_s(other::S);

struct HoldS {
  // Should show `S` as the field type, not the full path.
  other::S s;
};

}  // namespace n
