# The Subspace C++ Library

The Subspace C++ Library provides a concept-centered abstraction on top of
the bare-metal C++ standard library. It provides the tools to build
stable applications quickly, and to make your application performant through
explicitly leveraging compiler optimizations without accidentally tripping
over Undefined Behaviour and miscompilation. Stop spending time debugging
tricky heisenbugs and start relying on the compiler to produce the program
you wrote.

Find Subspace on Github here:
[https://github.com/chromium/subspace](https://github.com/chromium/subspace)

## Prelude

The `sus/prelude.h` header imports the most commonly used types into the
global namespace. All types in the [`prelude`]($sus::prelude) namespace are
included:
* [`Option`]($sus::option::Option)
* [`Vec`]($sus::collections::Vec)
* [`i32`]($sus::num::i32) and other integers
* [`f32`]($sus::num::f32) and other floating point types
* [`unsafe_fn`]($sus::marker::unsafe_fn) marker

### Example
```
#include "sus/prelude.h"

int main() {
    auto v = Vec<i32>(1, 2, 3, 4, 5);
    return sus::move(v).into_iter().sum();
}
```
