// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

namespace sus {

/// Collection types.
///
/// The Subspace library provides implementations of common general purpose
/// programming data structures, with rich APIs that allow for interactions with
/// [Iterators]($sus::iter), and with APIs that provide safe defaults.
///
/// The collections offer similar functionality to the [C++ standard containers
/// library](https://en.cppreference.com/w/cpp/container) but differ in some
/// key ways.
/// * Introduce compiler errors for common mistakes instead of runtime failures
///   and Undefined Behaviour which leads to miscompiles. Subspace relies on
///   the latest C++ standards to achieve this, and does not provide less-safe
///   APIs compatible with old C++ standards.
/// * Providing safe defaults. All API methods will do what is asked of them, or
///   fail to compile. And in some cases, will perform runtime checks and
///   terminate in the case of a software bug, which is represented in the
///   method documentation.
/// * No uninitialized memory through default initialization.
/// * Indexing operations with negative signed values no longer compile.
///   Containers that index on `size_t` (as in the standard library), instead of
///   `usize`, will silently accept memory safety bugs with negative indices.
/// * Providing explicit unsafe backdoors. Occasionally runtime checks can't be
///   elided by the compiler and they are in hot code that has visible
///   performance impact. Explicit unsafe backdoors allow individual callsites
///   to opt out of runtime checks as needed, with this choice being fully
///   visible in the syntax of the code. This allows them to be properly
///   scrutinized in code review or checked for with tooling.
/// * Providing fallible APIs for element access that hook into the rich,
///   composable APIs of [`Option`]($sus::option::Option) in order to clearly
///   and easily write error handling instead of Undefined Behaviour or
///   crashes.
/// * No accidental copies. Subspace collections (that are not view types) do
///   not satisfy the [`Copy`]($sus::mem::Copy) concept, and instead must be
///   explicitly cloned via `sus::clone(x)` to make a copy.
///   This allows them to be passed by value without introducing a copy at a
///   caller that was expecting it to be received by reference.
/// * Catch iterator invalidation. By default Subspace containers are built with
///   runtime protection against iterator invalidation. Iterators produced by
///   collections are tracked and if the collection is mutated while an iterator
///   still exists, the collection will panic and terminate the program.
///
/// Subspace's collections can be grouped into four major categories:
/// * Sequences: [`Vec`]($sus::collections::Vec), [`Array`]($sus::collections::Array)
///   (TODO: VecDeque, LinkedList,
///   [Hive](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p0447r16.html))
/// * Maps: (TODO: HashMap, BTreeMap, FlatMap)
/// * Sets: (TODO: HashSet, BTreeSet, FlatSet)
/// * Misc: (TODO: BinaryHeap)
///
/// # When Should You Use Which Collection
/// These are fairly high-level and quick break-downs of when each collection
/// should be considered. Detailed discussions of strengths and weaknesses of
/// individual collections can be found on their own documentation pages.
///
/// ## Use a Vec when:
/// * You want to collect items up to be processed or sent elsewhere later, and
///   don't care about any properties of the actual values being stored.
/// * You want a sequence of elements in a particular order, and will only be
///   appending to (or near) the end.
/// * You want a stack.
/// * You want a resizable array.
/// * You want a heap-allocated array.
///
/// ## Use an Array when:
/// * You want a fixed-size array of items that are all constructed up front
///   and share a single lifetime.
/// * You want to store a sequence of compile-time constants.
/// * You want the sequence to live on the stack.
///
/// TODO: More collections here as they exist!
///
/// TODO: Performance info/comparisons when there's more types.
///
/// # Slices vs spans
/// [`Slice`]($sus::collections::Slice) and [`SliceMut`]($sus::collections::SliceMut)
/// are how the Subspace library exposes views of
/// contiguous sequences of elements with O(1) random access. They provide const
/// and mutable access to the underlying objects, respectively.
///
/// [`Slice<T>`]($sus::collections::Slice) is similar to
/// [`std::span<const T>`](https://en.cppreference.com/w/cpp/container/span)
/// and [`SliceMut<T>`]($sus::collections::SliceMut) is similar
/// to [`std::span<T>`](https://en.cppreference.com/w/cpp/container/span).
/// All contiguous sequence collections in this library can
/// implicitly convert to a [`Slice`]($sus::collections::Slice) (always) or
/// to a [`SliceMut`]($sus::collections::SliceMut) (if the collection is
/// mutable).
///
/// Slices and owning contiguous collections like
/// [`Vec`]($sus::collections::Vec) share most of the same API surface,
/// including methods such as
/// [`sort`]($sus::collections::SliceMut::sort),
/// [`chunks`]($sus::collections::Slice::chunks),
/// [`iter`]($sus::collections::Slice::iter), and
/// [`concat`]($sus::collections::Slice::concat).
///
/// # Capacity Management
/// Many collections provide several constructors and methods that refer to
/// "capacity". These collections are generally built on top of an array.
/// Optimally, this array would be exactly the right size to fit only the
/// elements stored in the collection, but for the collection to do this would
/// be very inefficient. If the backing array was exactly the right size at all
/// times, then every time an element is inserted, the collection would have to
/// grow the array to fit it. Due to the way memory is allocated and managed on
/// most computers, this would almost surely require allocating an entirely new
/// array and copying every single element from the old one into the new one.
/// Hopefully you can see that this wouldn't be very efficient to do on every
/// operation.
///
/// Most collections therefore use an amortized allocation strategy. They
/// generally let themselves have a fair amount of unoccupied space so that they
/// only have to grow on occasion. When they do grow, they allocate a
/// substantially larger array to move the elements into so that it will take a
/// while for another grow to be required. While this strategy is great in
/// general, it would be even better if the collection never had to resize its
/// backing array. Unfortunately, the collection itself doesn't have enough
/// information to do this itself. Therefore, it is up to us programmers to give
/// it hints.
///
/// Any `with_capacity()` constructor will instruct the collection to allocate
/// enough space for the specified number of elements. Ideally this will be for
/// exactly that many elements, but some implementation details may prevent
/// this. See collection-specific documentation for details. In general, use
/// `with_capacity()` when you know exactly how many elements will be inserted,
/// or at least have a reasonable upper-bound on that number.
///
/// When anticipating a large influx of elements, the reserve family of methods
/// can be used to hint to the collection how much room it should make for the
/// coming items. As with with_capacity, the precise behavior of these methods
/// will be specific to the collection of interest.
///
/// For optimal performance, collections will generally avoid shrinking
/// themselves. If you believe that a collection will not soon contain any more
/// elements, or just really need the memory, the `shrink_to_fit()` method
/// prompts the collection to shrink the backing array to the minimum size
/// capable of holding its elements.
///
/// Finally, if ever you're interested in what the actual capacity of the
/// collection is, most collections provide a `capacity()` method to query this
/// information on demand. This can be useful for debugging purposes, or for use
/// with the `reserve()` methods.
///
/// # Iterators
/// [Iterators]($sus::iter) are a powerful and robust mechanism used throughout the Subspace
/// C++ library. [Iterators]($sus::iter) provide a sequence of values in a generic, safe,
/// efficient and convenient way. The contents of an iterator are usually lazily
/// evaluated, so that only the values that are actually needed are ever
/// actually produced, and no allocation need be done to temporarily store them.
/// Iterators are primarily consumed using a for loop, although many functions
/// also take iterators where a collection or sequence of values is desired.
///
/// All of the collections in Subspace provide several iterators for performing
/// bulk manipulation of their contents. The three primary iterators almost
/// every collection should provide are `iter()`, `iter_mut()`, and
/// `into_iter()`. Some of these may not be provided on collections where it
/// would not be reasonable to provide them.
///
/// `iter()` provides an iterator of immutable references to all the contents of
/// a collection in the most “natural” order. For sequence collections like Vec,
/// this means the items will be yielded in increasing order of index starting
/// at 0. For ordered collections like BTreeMap, this means that the items will
/// be yielded in sorted order. For unordered collections like HashMap, the
/// items will be yielded in whatever order the internal representation made
/// most convenient. This is great for reading through all the contents of the
/// collection.
///
/// ```
/// const auto vec = sus::Vec<i32>(1, 2, 3, 4);
/// for (const auto& x: vec.iter()) {
///    fmt::println("vec contained {}", x);
/// }
/// ```
///
/// `iter_mut()` provides an iterator of mutable references in the same order as
/// iter. This is great for mutating all the contents of the collection.
///
/// ```
/// auto vec = sus::Vec<i32>(1, 2, 3, 4);
/// for (auto& x: vec.iter_mut()) {
///    x += 1;
/// }
/// ```
///
/// `into_iter()` transforms the actual collection into an iterator over its
/// contents by-value. This is great when the collection itself is no longer
/// needed, and the values are needed elsewhere. Using `extend()` with
/// `into_iter()` is the main way that contents of one collection are moved into
/// another. `extend()` automatically calls `into_iter()`, and takes any `T`
/// that satisfies `IntoIterator`. Calling `collect()` on an iterator itself is
/// also a great way to convert one collection into another. Both of these
/// methods should internally use the capacity management tools discussed in the
/// previous section to do this as efficiently as possible.
///
/// ```
/// auto vec1 = sus::Vec<i32>(1, 2, 3, 4);
/// auto vec2 = sus::Vec<i32>(10, 20, 30, 40);
/// vec1.extend(sus::move(vec2));
/// ```
///
/// ```
/// #include "sus/collections/vec_deque.h"
///
/// auto vec = sus::Vec<i32>(1, 2, 3, 4);
/// auto deque = sus::move(vec).into_iter().collect<sus::VecDeque<i32>>();
/// ```
///
/// Iterators also provide a series of adapter methods for performing common
/// threads to sequences. Among the adapters are functional favorites like map,
/// fold, skip and take. Of particular interest to collections is the rev
/// adapter, which reverses any iterator that supports this operation. Most
/// collections provide reversible iterators as the way to iterate over them in
/// reverse order.
///
/// ```
/// auto vec = sus::Vec<i32>(1, 2, 3, 4);
/// for (const auto& x: vec.iter().rev()) {
///    fmt::println("vec contained {}", x);
/// }
/// ```
///
/// Several other collection methods also return iterators to yield a sequence
/// of results but avoid allocating an entire collection to store the result in.
/// This provides maximum flexibility as collect or extend can be called to
/// “pipe” the sequence into any collection if desired. Otherwise, the sequence
/// can be looped over with a for loop. The iterator can also be discarded after
/// partial use, preventing the computation of the unused items.
///
/// # Ranges
/// The collections in the Subspace C++ library can be used with standard ranges
/// by calling the [`range()`]($sus::iter::IteratorBase::range) adaptor on any
/// Subspace iterator. It will return an object that satisfies
/// [`std::ranges::input_range`](https://en.cppreference.com/w/cpp/ranges/input_range)
/// and
/// [`std::ranges::viewable_range`](https://en.cppreference.com/w/cpp/ranges/viewable_range),
/// such as with `vec.iter().range()`.
///
/// Iterators over value types or mutable references when converted by `range()`
/// will also satisfy
/// [`std::ranges::output_range`](https://en.cppreference.com/w/cpp/ranges/input_range),
/// such as with `vec.iter_mut().range()`.
///
/// Conversely, standard ranges, such as the types in the standard containers
/// library, can be used to construct a Subspace iterator through
/// [`sus::iter::from_range`]($sus::iter::from_range).
///
/// # Familiarity with Rust APIs
/// These collections were inspired by porting Rust
/// [std::collections](https://doc.rust-lang.org/stable/std/collections/index.html)
/// and traits/concepts into C++. Additional changes are minimized to aid with
/// familiarity and working across languages, but some are necessary for use in
/// C++ or interop with the standard library. Subspace also provides additional
/// containers specific to C++ when needed, such as `Array` (and TODO: FlatMap).
namespace collections {}
}  // namespace sus
