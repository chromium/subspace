# Style

A collection of style rules that we collect as we design the library, with a
focus on providing clear APIs that can't easily be used wrong, and avoid
footguns, crashes, bugs, and UB.

1. All methods are `constexpr` unless they must call a non-`constexpr` function,
   or they expose floating point NaNs (since constexpr NaNs change their bit
   values).
    * Consider `panic()`/`check()` as constexpr for these purposes, they will
      correctly prevent compiling if the condition fails.
1. If you override on `const&`, then explicitly provide or delete the `&&`
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
      action instead of rolling it yourself. For example if `T` was required to
      be `Into<U, T>` then construct `T` via `sus::into()`.
    * On types, it often makes sense to static assert instead to give a nice
      compiler error, since they are not used in overloads like functions.
1. Use `usize`, `u32`, `i32`, etc in all public APIs, except:
    * Use `size_t` for template parameters instead of `usize`. Since it may
      require forward declarations (sometimes from places that also must
      forward-declare the `usize` type), and template args are always statically
      determined, `usize` doesn't provide the same value in this context, and
      can get in the way.
1. Do not `clone()` inside constructor methods. The clone should happen at the
   call site. Only allow Copy to happen inside the library type.
1. Default constructors should not be explicit typically, allow `{}` syntax,
   since we have to write out the type elsewhere anyway, especially with static
   ctor methods. This reduces the amount of noise. e.g. `auto o =
   Option<DefaultCtor>({})` instead of `auto o =
   Option<DefaultCtor>(DefaultCtor())`.
1. Iterator types should be `[[nodiscard]]` always. And trivially relocatable
   (and `[[sus_trivial_abi]]`) if at all possible.
1. Accessor methods and ctor methods can be marked `sus_pure` IF:
  * They don't mutate through a global or parameter mutable reference or pointer.
  * Thus they don't have observable side effects. Calling them on the same input
    values multiple times always produces the same output.
  * Additionally, if the function does not deref a pointer, access through
    a reference, or access any static variables, it may be marked
    `sus_pure_const`.
1. If a type has implicit ctor from T then it should have assignment from T.
   The same is not true for an explicit ctor: no assignment should be present
   for that type.
1. Use [IWYU pragmas](https://github.com/include-what-you-use/include-what-you-use/blob/master/docs/IWYUPragmas.md):
   `// IWYU pragma: ___`
  * Headers that are impl details are marked and point to the public header
    like `private, public "sus/public/header.h"`.
  * Headers in __private are marked `private`.
  * All private headers have `friend "sus/.*"`
  * Headers that just include other headers mark those with `export`, usually
    through a `begin_exports` and `end_exports` section.
  * But don't follow the IWYU rules _inside_ the library. We include the minimal internal
    headers internally to reduce the amount of textual parsing needed to satisfy
    including one public header.
1. All types in requires clauses should be references or pointers.
   * For mutable access always write it as `requires(T& t)` instead of
     `requires (T t)`, as the latter will not match pure virtual types in
     clang-16.
   * Similarly, for const usage, write `requires (const T& t)` instead of
     `requires (const T t)`.
1. When writing `operator==` or `operator<=>`, always provide a non-templated overload
   even if it looks redundant with a templated one. Example
   `operator==(Option, Option)` and `operator==(Option<T>, Option<U>)` look redundant
   but they are not, as the former allows conversions to Option for the rhs to happen
   while the latter does not (it would have to deduce `U` and fails).
  
## Containers that hold references

Container types that hold references require extra care in a number of ways. To
properly build such a container type (e.g. `Option` and `Tuple`):
  * While the external API can work with references, internally it must be stored in a
    pointer so it can be rebound, usually in a wrapper that converts to/from references
    such as `StoragePointer` used for `Option`.
  * Never check concepts such as `Copy`, `Clone`, `Move` or `Default` directly, when
    it can be a reference. Instead, use `CopyOrRef`, `CloneOrRef`, and `MoveOrRef`, and
    treat `Default` as `false`.
  * Hide methods or provide specialized overloads on `std::is_reference_v` where needed
    for correctness. For instance, when holding a reference, returning that reference
    from an rvalue is okay, but when holding a value, giving a reference to it from an
    rvalue is not.
  * Use `static_assert(SafelyConstructibleFromReference<ToType, FromReferenceType&&>)`
    in places that store the reference to ensure a reference to a temporary does not 
    get created due to an implicit conversion. The `FromReferenceType&&` here is should
    be the input type as it's written in the function parameters.
  * If a ctor type deduction guide is provided, the deduction should struct qualifiers
    and references with `std::remove_cvref_t` on the deduced type arguments.
  * Consider providing a construction marker type such as `some() -> SomeMarker` which
    captures the parameters as references and lazily constructs the final type. This 
    allows reference types to be preserved through to the construction of the
    container without requiring the full type defn to be written every time.
    * Notably, this is omitted for `Choice`, which needs to be reasonably used behind
      a type-alias so spelling the full type does not require template arguments.
  * Test all APIs with a reference to `sus::test::NoCopyMove` which ensures the
    references are correctly preserved as copy/move of the underlying value will not
    compile.
