# Principles Document

This library is an experiment and not intended for use. See the
[README](README.md).

## High Level

1. Defaults matter. The right, and safe, thing should always require less typing
   than the maybe-wrong thing.
1. Memory safety should be the default. You don't pay (in bugs and CVEs) for
   premature optimization you didn't need.
    * Leaving this path should require more typing, and a clear signal what is
      happening. We will use [a naming convention of "unchecked"](
      https://github.com/chromium/subspace/blob/9d0ae908aa8a98d11f78e0315470be64b97a0dbe/mem/nonnull.h#L31),
      and [a marker argument type of `UnsafeFnMarker`](
      https://github.com/chromium/subspace/blob/main/sus/marker/unsafe.h),
      to do so in place of [a language keyword](
      https://docs.microsoft.com/en-us/dotnet/csharp/language-reference/keywords/unsafe).
1. KISS Principle
    * > most systems work best if they are kept simple rather than made
      > complicated; therefore, simplicity should be a key goal in design, and
      > unnecessary complexity should be avoided
      >  - [Kelly Johnson](https://en.wikipedia.org/wiki/KISS_principle)
    * Consider inherent vs accidental complexity and avoid the latter.
1. Principle of Least Astonishment
    * A component of a system should behave in a way that most users will expect
      it to behave. The behavior should not astonish or surprise users.
1. Single-choice
    * > Whenever a software system must support a set of alternatives, one and
      > only one module in the system should know their exhaustive list.
      >  - Bertrand Meyer: Object-Oriented Software Construction
1. Persistence-Closure
    * > Whenever a storage mechanism stores an object, it must store with it the
      > dependents of that object. Whenever a retrieval mechanism retrieves a
      > previously stored object, it must also retrieve any dependent of that
      > object that has not yet been retrieved.
      >  - Bertrand Meyer: Object-Oriented Software Construction
1. Single-responsibility principle
    * > Every module, class or function in a computer program should have
      > responsibility over a single part of that program's functionality, and it
      > should encapsulate that part. All of that module, class or function's
      > services should be narrowly aligned with that responsibility.
      >  - [Wikipedia](https://en.wikipedia.org/wiki/Single-responsibility_principle)
1. Use the latest C++ language features to provide the simplest APIs possible.
1. Accept incompatibility. No attempt is made for backward compatibility with C,
   earlier versions of C++, compatibility with stdlib, etc.
    * When compatibility or conversion is desirable, it will be done through
      explicit functions.
1. Surprising, and hidden, performance cliffs are bad; we will avoid them.
1. Provide monadic functions to chain together functionality, and types that
   play well with them.
    * Monadic functions allow for functional programming, which results in less
      complex code: fewer program states are possible when functions have no
      side effects.
    * Monadic functions can provide general building blocks that allow for the
      construction of a wide variety of software from a small set of tools.

## Nitty gritty.

1. No public constructors, except for the default constructor and an aggregate
   constructor of all fields. All other construction is through (named) static
   methods.
    * The aggregate constructor acts like the implicit constructor in cases
      where it does not exist. It takes values for and initializes all data
      members in the order they are declared.
    * Non-default constructors are prefixed with `with_()` unless there's a
      clear and better domain-specific name.
    * Avoid writing copy constructors in *non-containers* unless moving can be
      implemented in terms of a copy, thus has the same cost as a copy, and
      it is cheap to pass by value (a size of <= 2 pointers). Otherwise, write
      `Clone()` instead.
      * Containers have a copy constructor if their inner type has one, and
        if not, implement `Clone()` if their inner type does.
    * Write a `::from(x)` constructing method to implement
      `sus::construct::From` when the type is constructed from another type.
    * Exception granted for closure types (Fn, FnMut, FnOnce) because
        1. the construction does not do work, it stores a pointer, or the output
           of sus_bind().
        1. the purpose of the closure types is to type-erase a lambda, for use
           in places where the type can't be propagated, such a virtual methods
           which can not be a template.
        1. since the intent is for use where templates can not be used,
           `Into<FnOnce<...>>` can not be used either.
1. No use-after-move. All types abort if they are used after a move.
1. No implicit conversions, unless it's another representation of the _same
   type_.
    * Provide `::from(x)` instead, for `sus::construct::From` and
      `sus::construct::Into`.
1. No function overloads. Const is the default.
    * When there are const and mutable versions of a method, use the `_mut`
      suffix on the mutable one to distinguish them. The const method gets the
      shorter name.
    * Don't use a `get_` prefix on getters.
    * Do use a prefix like `is_` to differentiate from a verb, like `is_empty()`.
    * Use an appropriate prefix like `set_`, or a clear verb name, to
      distinguish setters.
    * No default parameters, as they are a form of overloading.
    * Constructors (public copy/move, or private constructors) and operators
      (including operator=, operator*, operator->) can not be fully spelt
      (const/mutable) without overloads, so they are the exception.
1. There are two types of Objects (or methods on a mixed-type Object). Owning
   types, which give access to data they own and reference types that share the
   reference they hold.
    * For Owning types:
       * Methods that return a reference to an object owned by the class are
         lvalue-reference-qualified (with `&` or `const&`). If they are
         `const&`-qualified, then `=delete` the `&&` override to avoid rvalues
         returning references to what they own.
          * If the inner type is _always_ owned, the method can be annotated
            with `sus_lifetimebound`, but avoid it when the inner type may be a
            pointer/reference. We rely on deleting the `&&` override to avoid
            references to temporaries.
       * When the inner type is a **single** template variable, disallow const
         on the inner type. The const on the Owning type will apply transitively
         to the owned inner type.
    * For Reference types:
       * Methods that return a `const&` are `const&` qualified _without_ deleting
         the `&&` override. Mutable methods, including those that return a `&`
         are unqualified or `&&-qualified` (non-const, but not `&`-qualified in
         order to allow rvalues).
          * Avoid `sus_lifetimebound` on methods that return references, as it
            ties the lifetime to the method's class, but the class also only
            holds a reference.
       * Annotate constructor and method parameters with `sus_lifetimebound`
         when the reference will be stored in the class.
       * When the inner type is a template variable, allow const on the inner
         type. The const on the Owning type is unable to be transferred to the
         inner type, i.e. a const `ref<T>` can be copied or moved to a non-const
         `ref<T>`.
1. Class members which have tail padding, or can have based on templates, should
   be marked as [[sus_no_unique_address]] when there are other fields that can
   be packed into the tail padding, if it exists and is usable by the compiler
   implementation.
1. Bounds are always checked unless you explicitly ask for them to not be.
1. Small headers. C++ compilation speed is proportional to the amount of input.
    * We will split types into separate headers as much as possible.
    * We may provide headers which group together smaller headers for simplicity
      in smaller projects.
    * We will move implementations out of headers, and out of templates, to
      reduce header sizes where possible and where it does not have strong
      performance implications.
1. We will document all types, functions, and public methods.
    * Documentation will include examples where use is non-trivial.
    * All functions that can `abort()` will document under what conditions they
      do so, with the exception of: use-after-move.
1. No null pointers. Smart pointers have no null state. Functions never return
   null pointers.
1. No raw pointers by default. Raw pointers throw away all lifetime information,
   can produce memory safety bugs. And raw pointers can be null, which is
   undesirable.
1. No expensive and implicit copies.
    * Copies should generally be explicit. If we opt into implicit copies, they
      should be cheap - and
      [trivial](https://en.cppreference.com/w/cpp/types/is_trivially_copyable).
1. No surprising heap allocations. Types should live on the stack whenever
   possible. They only use internal heap allocations when strictly required:
   for instance, because they have a dynamic size. Instead, the user can choose
   what lives on the heap through the use of heap-based smart pointers.
1. No secret sometimes-heap-allocated-sometimes-not optimizations (e.g. the
   small string optimization). This leads to performance cliffs, but worse,
   pointer stability is unpredictable if a type vends a pointer to its storage
   which is sometimes heap allocated (and stable) and sometimes not.
1. No native arrays. Use an Array type instead.
    * Native arrays can't be bounds-checked, and decay to pointers, making them
      a bit invisible. A library type has the same overheads, unless it
      explicitly chooses to provide more.
1. Deep constness. Const methods of owning types do not mutate state in visible ways.
    * No const methods return non-const pointers or references.
    * No const methods call non-const methods through pointers or references.
    * Reference/view types follow the same, or else provide clear const vs mutable
      access. SliceMut allows const methods that act in mutable ways on the
      container because it expresses mutability vs Slice in a clear way in the
      type system. This compromise is required to allow receiving lvalue and rvalue
      refences (as `const&`) without forcing multiple overloads onto library users
      or without receiving by value ([which is more expensive](
      https://danakj.github.io/2023/06/05/not-generating-constructors.html)).
    * No `const_cast` usage.
    * State in members marked `mutable` does not escape the class from const
      methods.
    * No mutable pointers or references constructed from fields, or reachable
      from fields, in const methods.
    * Pointer and reference fields accessed through a const class pointer are
      only used as const pointers and references.
    * We will use tooling to enforce this for the library, and it will be
      made available for user code.
    * Prior art and motivations: https://isocpp.org/files/papers/N4388.html
    * TODO: Maybe there's a more concise way to say this.
1. No static mutable state.
    * Static mutable state introduces potential thread races, and the complexity
      of global knowledge. A `static` variable declaration must also be `const`,
      and if it's a pointer, then "deep constness" applies.
1. No exceptions. Return results instead.
    * Every function and method is `noexcept` and/or `consteval`.
    * No use of `throw` outside of `consteval` functions, where it is used to
      provide compiler errors.
    * Calls to functions in another library that can throw exceptions are
      banned.
    * We will provide vocabulary types for returning a
      [success-or-failure](https://github.com/chromium/subspace/blob/main/sus/result/result.h)
      and a
      [value-or-none](https://github.com/chromium/subspace/blob/main/sus/option/option.h).
1. Traits, SFINAE and type tags to define behaviour on data. No inheritance
   unless from an abstract interface. All other classes are marked `final`.
    * Always use `final` instead of `override`.
    * Inheritance is [no longer
      needed](https://en.cppreference.com/w/cpp/language/ebo) for object sizes.
    * We will provide a public set of traits with defined behaviour and tools or
      guidance to ensure consistent behaviour in implementations of traits.
1. Tools to use SFINAE for common patterns, such as traits, without having to
   understand all of the stdlib type_traits.
1. Use constexpr everywhere it is correct to do so.
    * Functions or method that can be constexpr should be marked constexpr.
    * Use [std::is_constant_evaluated](https://en.cppreference.com/w/cpp/types/is_constant_evaluated)
      to provide a (more expensive) constexpr implementation when needed.
1. Common C++ language behaviours are part of the class' public API. We will
   document _and [test](https://github.com/chromium/subspace/blob/main/sus/test/behaviour_types_unittest.cc)_
   for being trivially copyable, movable, destructible, among others.
1. Everything comes with tests. Tests are not flaky.
1. Avoid compiler-specific things whenever possible. Some exceptions:
    * `[[clang:trivial_abi]]` is a perf benefit, once Clang has C++20 support.
    * Compiler builtins for implementing things, to avoid including large stdlib
      headers.
1. Use method qualifiers extensively for improved safety.
    * Methods that return references should be lvalue-qualified. If const-ref
      qualified, `delete` the rvalue-reference overload.
    * Only omit a qualifier when the method makes sense for both lvalue and
      rvalue uses.

## What's included

1. Safe and predictable integer math, with safe casting and simple names.
    * Starting point: https://github.com/google/integers
    * On float to int conversion: https://blog.m-ou.se/floats/
1. Smart pointers that naturally express ownership, sharing, and threading
   semantics.
    * The smart pointers are not "leaky", and are not (safely) backward
      compatible with raw pointer ownership.
1. Containers with lifetime safety at runtime.
1. Function pointers with additional state bound to them, for purposes of
   currying.
1. Vocabulary types for returning success-or-failure with different embedded
   types, or value-or-none.
1. Tagged unions.
1. We will provide a required set of compiler warnings to get expected behaviour
   from the library when working with user-provided types.
    * We will consider different "levels" of grouped warnings if needed.
1. We will provide clang static analysis tools and/or compiler plugins as needed
   to ensure expected behaviour from the library when it acts on user-provided
   types (once clang supports C++20 and can compile the library).

### Less clear ideas

1. Support for thread models using traits to designate types that can be sent or
   used across threads.
    * Consider also the use of [virtual threads](
      https://chromium.googlesource.com/chromium/src.git/+/HEAD/docs/threading_and_tasks.md).
1. Consider integration with [PartitionAlloc](
  https://chromium.googlesource.com/chromium/src/+/HEAD/base/allocator/partition_allocator/PartitionAlloc.md).
1. Library types to make safe and easy use of coroutines.

## Context

* https://www.atlanticcouncil.org/content-series/buying-down-risk/home/
* https://www.atlanticcouncil.org/content-series/buying-down-risk/memory-safety/
* https://docs.google.com/document/d/e/2PACX-1vRZr-HJcYmf2Y76DhewaiJOhRNpjGHCxliAQTBhFxzv1QTae9o8mhBmDl32CRIuaWZLt5kVeH9e9jXv/pub
