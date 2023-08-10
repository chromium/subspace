/// Outer namespace
namespace outer_namespace {

/// Outer function
void outer();

/// Inner namespace
namespace inner_namespace {}

}  // namespace outer_namespace

namespace outer_namespace::inner_namespace {

/// Inner function
void inner();

}  // namespace outer_namespace::inner_namespace
