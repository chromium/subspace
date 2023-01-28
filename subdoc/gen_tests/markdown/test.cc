// This tests a struct in the global namespace, and a struct in a private
// namespace. The former is generated and the latter is not.

template <class T>
struct S {};

/// Returns template
S<int> return_template();
