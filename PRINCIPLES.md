# Principles Document

See the [README](README.md) for how this library is meant to be used.

## High Level

1. Defaults matter. The right, and safe, thing should always require less typing
   than the maybe-wrong thing.
1. Memory safety should be the default. You don't pay (in bugs and CVEs) for
   premature optimization you didn't need.
    * Leaving this path should require more typing, and a clear signal what is
      happening. We will use either a naming convention, or a marker argument
      type, to do so in place of [a language keyword](
      https://docs.microsoft.com/en-us/dotnet/csharp/language-reference/keywords/unsafe).
1. KISS Principle
    * "most systems work best if they are kept simple rather than made
      complicated; therefore, simplicity should be a key goal in design, and
      unnecessary complexity should be avoided" - most systems work best if they
      are kept simple rather than made complicated; therefore, simplicity should
      be a key goal in design, and unnecessary complexity should be avoided
    * Consider inherent vs accidental complexity and avoid the latter.
1. Principle of Least Astonishment
    * A component of a system should behave in a way that most users will expect
      it to behave. The behavior should not astonish or surprise users.
1. Single-choice
    * "Whenever a software system must support a set of alternatives, one and
      only one module in the system should know their exhaustive list." -
      Bertrand Meyer: Object-Oriented Software Construction
1. Persistence-Closure
    * "Whenever a storage mechanism stores an object, it must store with it the
      dependents of that object. Whenever a retrieval mechanism retrieves a
      previously stored object, it must also retrieve any dependent of that
      object that has not yet been retrieved." - Bertrand Meyer: Object-Oriented
      Software Construction
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

1. No public constructors. All construction is through (named) static methods.
    * Would like to extend this to Copy and Move constructors too, through Clone
      and Move methods. Implementation details required.
1. No use-after-move. All types abort if they are used after a move.
1. No implicit conversions, ever.
1. Bounds are always checked unless you explicitly ask for them to not be.
1. Lifetimes are always checked unless you explicitly ask for them to not be.
1. Small headers. C++ compilation speed is proportional to the amount of input.
    * We will split types into separate headers as much as possible. tokens.
    * We may provide headers which group together smaller headers for simplicity
      in smaller projects.
    * We will move implementations out of headers, and out of templates, to
      reduce header sizes where possible and where it does not have strong
      performance implications.
1. We will use doxygen to document all types, functions, and public methods.
    * Documentation will include examples where use is non-trivial.
    * All functions that can abort() will document under what conditions they do
      so, with the exception of: use-after-move.
1. No null pointers. Smart pointers have no null state. Functions never return
    null pointers.
1. No raw pointers by default. Raw pointers throw away all lifetime information,
    can produce memory safety bugs. And raw pointers can be null, which is
    undesirable.
1. No expensive and implicit copies.
    * Copies should generally be explicit. If we opt into implicit copies, they
      should be cheap - and [trivial](https://en.cppreference.com/w/cpp/types/is_trivially_copyable).
1. No surprising heap allocations. Types should live on the stack whenever
    possible. They only use internal heap allocations when strictly required:
    for instance, because they have a dynamic size. Instead, the user can choose
    what lives on the heap through the use of heap-based smart pointers.
1. Deep constness. Const methods do not mutate state in visible ways.
    * No const methods return non-const pointers or references.
    * No const methods call non-const methods through pointers or references.
    * No const_cast usage.
    * State in members marked `mutable` does not escape the class from const
      methods.
    * No mutable pointers or references constructed from fields, or reachable
      from fields, in const methods.
    * Pointer and reference fields accessed through a const class pointer are
      only used as const pointers and references.
    * We will use tooling to enforce this for the library, and it will be
      made available for user code.
    * TODO: Maybe there's a more concise way to say this.
1. No static mutable state.
    * Static mutable state introduces potential thread races, and the complexity
      of global knowledge. A `static` variable declaration must also be `const`,
      and if it's a pointer, then "deep constness" applies.
1. No exceptions. Return results instead.
    * Every function and method is `noexcept`.
    * No use of `throw`.
    * Calls to functions in another library that can throw exceptions are
      banned.
    * We will provide vocabulary types for returning a success-or-failure and a
      value-or-none.
1. Traits, SFINAE and type tags to define behaviour on data. No inheritance.
    * Inheritance is [no longer
      needed](https://en.cppreference.com/w/cpp/language/ebo).
    * We will provide a public set of traits with defined behaviour and tools or
      guidance to ensure consistent behaviour in implementations of traits.
1. Tools to use SFINAE for common patterns, such as traits, without having to
   understand all of the stdlib type_traits.
1. Use constexpr everywhere it is correct to.
    * Functions or method that can ever perform non-constexpr actions may not be
      marked constexpr, with the exception of: abort().
    * Any other function or method should be marked constexpr.
1. Common C++ language behaviours are part of the class' public API. We will
   document _and test_ for being trivially copyable, movable, destructible,
   among others.

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
1. We will provide a required set of clang warnings to get expected behaviour
   from the library when working with user-provided types.
    * We will consider different "levels" of grouped warnings if needed.
1. We will provide clang static analysis tools and/or compiler plugins as needed
   to ensure expected behaviour from the library when it acts on user-provided
   types.

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
