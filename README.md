[![CI](https://github.com/chromium/subspace/actions/workflows/ci.yml/badge.svg)](https://github.com/chromium/subspace/actions/workflows/ci.yml)
[![hdoc](https://github.com/chromium/subspace/actions/workflows/hdoc.yml/badge.svg)](https://docs.hdoc.io/danakj/subspace/)
[![sub-doc](https://github.com/chromium/subspace/actions/workflows/subdoc.yml/badge.svg)](https://suslib.cc)
<!---
[![clang-doc](https://github.com/chromium/subspace/actions/workflows/clang-doc.yml/badge.svg)](https://danakj.github.io/subspace-docs/sus/#Namespaces)
-->
# Subspace Library

An [experimental take](https://danakj.github.io/2022/12/31/why-subspace.html)
on a safer, simpler C++ standard library.

Please don't use this library. This is an experiment and we don't yet know where
it will take us. There will be breaking changes without warning, and there is no
stable version.

1. See [BUILD.md](BUILD.md) for instructions on building Subspace and Subdoc
and running their tests.
1. See [USAGE.md](USAGE.md) for instructions on integrating the
Subspace library into your project.
1. See [PRINCIPLES.md](PRINCIPLES.md) for the principles behind design choices in
the Subspace library.
1. See [STYLE.md](STYLE.md) for evolving guidance and best practices for
developers of the Subspace library.

## Subdoc

[Subdoc](subdoc/) is a documentation generator from inline C++ documentation
in the spirit of
[Rustdoc](https://doc.rust-lang.org/rustdoc/what-is-rustdoc.html).

The comments in the Subspace library use markdown syntax with additional Subdoc
attributes, and are designed to be consumed by Subdoc in order to generate an
HTML website.

Subdoc is built on top of Subspace, giving the developers a chance to see the
Subspace library in action and test the ergonomics and features of the library.

**Status:**
Subdoc is still very much a work-in-progress, but it is being used to generate
[Subspace documentation](https://danakj.github.io/subspace-docs/sus/#Namespaces)
on each commit.

## Compiler Support

Subspace is a
[concept-first](https://en.cppreference.com/w/cpp/language/constraints)
library and requires C++20.
Compiler support for C++20 varies, and with active development ongoing, bugs in
their implementations still appear and disappear regularly. When in doubt,
check out which compiler versions are used by the
[CI bots](.github/workflows/ci.yml).

| Compiler   | Version |
|------------|---------|
| **Clang:** | 16 and up |
| **GCC**:   | 13 and up |
| **MSVC**:   | 17.4.2 (not [newer versions](https://github.com/chromium/subspace/issues/267)) |

We attempt to work around bugs when reasonable, to widen compiler version
support. See [compiler_bugs.h](sus/macros/__private/compiler_bugs.h) for
the set of bugs we are aware of and currently work around.
