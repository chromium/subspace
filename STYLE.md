# Style

A collection of style rules that we collect as we design the library, with a
focus on providing clear APIs that can't easily be used wrong, and avoid
footguns, crashes, bugs, and UB.

1. All methods are `constexpr` unless they must call a non-`constexpr` function,
   or they expose floating point NaNs (since constexpr NaNs change their bit
   values).
    * Consider `panic()`/`check()` as constexpr for these purposes, they will
      correctly prevent compile if the condition fails.
1. If you override on `const&`, then explicity provide or delete the `&&`
   override.
    * If the function should only act on lvalues, provide `const&`- and
      `&`-qualified overrides, and delete the `&&`-qualified override.
    * If the function should act on lvalues or rvalues, omit qualifying the
      method or provide all three.
    * If the function should act on rvalues, provide a `&&`-qualified override,
      and omit the rest.
1. Use concepts or `requires` to prevent compilation errors inside a function.
1. Use `sus` concepts where possible.
    * If you `requires` a concept, then use that concept to do the intended
   action instead of rolling it yourself. For example if `T` was required to be
   `MakeDefault` then construct `T` via `make_default()`.
1. Use `()` around the `requires` expression always, since clang-format does bad
   things without it.
1. Use `usize`, `u32`, `i32`, etc in all public APIs, except:
    * Use `size_t` for template parameters instead of `usize`. Since it may
      require forward declarations (sometimes from places that also must
      forward-declare the `usize` type), and template args are always statically
      determined, `usize` doesn't provide the same value in this context, and
      can get in the way.
