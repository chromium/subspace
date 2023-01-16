// This tests a struct in the global namespace, and a struct in a private
// namespace. The former is generated and the latter is not.

/// Comment headline S
struct S {
};

namespace __private {
    /// This struct will be ignored.
    struct P {
    };
}
