/// Outer namespace
///
/// Outer details.
namespace outer_namespace {

/// Outer function
void outer();

/// Inner namespace
///
/// Inner details.
namespace inner_namespace {}

/// Empty namespace
namespace empty {}

}  // namespace outer_namespace

namespace outer_namespace::inner_namespace {

/// Inner function
void inner();

}  // namespace outer_namespace::inner_namespace
