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

// IWYU pragma: private, include "sus/iter/iterator.h"
// IWYU pragma: friend "sus/.*"
#pragma once

#include <compare>

#include "sus/construct/default.h"
#include "sus/construct/into.h"
#include "sus/fn/fn.h"
#include "sus/iter/__private/is_generator.h"
#include "sus/iter/__private/iter_compare.h"
#include "sus/iter/__private/iterator_end.h"
#include "sus/iter/__private/iterator_loop.h"
#include "sus/iter/extend.h"
#include "sus/iter/from_iterator.h"
#include "sus/iter/into_iterator.h"
#include "sus/iter/product.h"
#include "sus/iter/sum.h"
#include "sus/lib/__private/forward_decl.h"
#include "sus/macros/__private/compiler_bugs.h"
#include "sus/mem/addressof.h"
#include "sus/mem/move.h"
#include "sus/mem/size_of.h"
#include "sus/num/unsigned_integer.h"
#include "sus/ops/eq.h"
#include "sus/ops/try.h"
#include "sus/option/option.h"
#include "sus/tuple/tuple.h"

namespace sus::iter {

using ::sus::construct::Into;
using ::sus::option::Option;

/// The base class for all Iterator types.
///
/// The `sus::iter::Iterator` concept requires that a type subclasses from
/// IteratorBase and is `final` in order to be considered an Iterator. No
/// code should refer to `IteratorBase` except for defining the base class
/// of an iterator, and it should be treated as an implementation detail only.
template <class Iter, class ItemT>
class IteratorBase {
 protected:
  constexpr IteratorBase() noexcept {
    // These are checked in the constructor as the Iter type may be incomplete
    // outside of it.
    static_assert(std::same_as<ItemT, typename Iter::Item>);
    static_assert(std::is_final_v<Iter>,
                  "Iterator implementations must be `final`, as the provided "
                  "methods must know the complete type.");
  }

  constexpr inline const Iter& as_subclass() const {
    return static_cast<const Iter&>(*this);
  }
  constexpr inline Iter& as_subclass_mut() { return static_cast<Iter&>(*this); }

 public:
  using Item = ItemT;

  /// Adaptor for use in ranged for loops.
  constexpr auto begin() & noexcept {
    return __private::IteratorLoop<Iter&>(as_subclass_mut());
  }
  /// Adaptor for use in ranged for loops.
  constexpr auto end() & noexcept { return __private::IteratorEnd(); }

  /// An Iterator also satisfies IntoIterator, which simply returns itself.
  ///
  /// sus::iter::IntoIterator trait implementation.
  constexpr Iter&& into_iter() && noexcept {
    return static_cast<Iter&&>(*this);
  }

  // Provided overridable methods.

  /// Tests whether all elements of the iterator match a predicate.
  ///
  /// If the predicate returns `true` for all elements in the iterator, this
  /// functions returns `true`, otherwise `false`. The function is
  /// short-circuiting; it stops iterating on the first `false` returned
  /// from the predicate.
  ///
  /// Returns `true` if the iterator is empty.
  constexpr bool all(::sus::fn::FnMut<bool(Item)> auto f) noexcept;

  /// Tests whether any elements of the iterator match a predicate.
  ///
  /// If the predicate returns `true` for any elements in the iterator, this
  /// functions returns `true`, otherwise `false`. The function is
  /// short-circuiting; it stops iterating on the first `true` returned from
  /// the predicate.
  ///
  /// Returns `false` if the iterator is empty.
  constexpr bool any(::sus::fn::FnMut<bool(Item)> auto f) noexcept;

  /// Returns an iterator that refers to this iterator, and for which operations
  /// on it will also be applied to this iterator.
  ///
  /// This is useful to allow applying iterator adapters while still retaining
  /// ownership of the original iterator.
  constexpr Iterator<Item> auto by_ref() & noexcept;

  // Provided final methods.

  /// Takes two iterators and creates a new iterator over both in sequence.
  ///
  /// `chain()` will return a new iterator which will first iterate over values
  /// from the first iterator and then over values from the second iterator.
  ///
  /// In other words, it links two iterators together, in a chain. 🔗
  ///
  /// `sus::iter::Once` is commonly used to adapt a single value into a chain of
  /// other kinds of iteration.
  // TODO: Shouldn't chain() allow any iterator of *convertible to T* instead
  // just iterator of *T*?
  template <IntoIterator<ItemT> Other>
  constexpr Iterator<Item> auto chain(Other&& other) && noexcept;

  /// Creates an iterator which clones all of its elements.
  ///
  /// This is useful when you have an iterator over `&T`, but you need an
  /// iterator over `T`.
  ///
  /// There is no guarantee whatsoever about the clone method actually being
  /// called or optimized away. So code should not depend on either.
  constexpr Iterator<std::remove_cvref_t<Item>> auto cloned() && noexcept
    requires(::sus::mem::Clone<Item>);

  /// [Lexicographically]($sus::ops::Ord#how-can-i-implement-ord?)
  /// compares the elements of this `Iterator` with those of another.
  template <IntoIteratorAny Other, int&...,
            class OtherItem = typename IntoIteratorOutputType<Other>::Item>
    requires(::sus::ops::Ord<ItemT, OtherItem>)
  constexpr std::weak_ordering cmp(Other&& other) && noexcept;

  /// [Lexicographically]($sus::ops::Ord#how-can-i-implement-ord?)
  /// compares the elements of this `Iterator` with those of another with
  /// respect to the specified comparison function.
  template <IntoIteratorAny Other, int&...,
            class OtherItem = typename IntoIteratorOutputType<Other>::Item>
  constexpr std::weak_ordering cmp_by(
      Other&& other,
      ::sus::fn::FnMut<std::weak_ordering(
          const std::remove_reference_t<Item>&,
          const std::remove_reference_t<OtherItem>&)> auto cmp) && noexcept;

  /// Creates an iterator which copies all of its elements.
  ///
  /// This is useful when you have an iterator over &T, but you need an iterator
  /// over T.
  constexpr Iterator<std::remove_cvref_t<Item>> auto copied() && noexcept
    requires(::sus::mem::Copy<Item>);

  /// Consumes the iterator, and returns the number of elements that were in
  /// it.
  ///
  /// The function walks the iterator until it sees an Option holding #None.
  ///
  /// # Panics
  ///
  /// If the iterator has more than `usize::MAX` elements in it the `usize` will
  /// catch overflow and panic. To avoid panic, you may use a fold over
  /// `OverflowInteger<usize>` that increments the count each iteration.
  constexpr ::sus::num::usize count() && noexcept;

  /// Repeats an iterator endlessly.
  ///
  /// ```
  /// asdaa
  /// ```
  /// Instead of stopping at `None`, the iterator will instead start again, from
  /// the beginning. After iterating again, it will start at the beginning
  /// again. And again. And again. Forever. Note that in case the original
  /// iterator is empty, the resulting iterator will also be empty.
  ///
  /// The iterator must be `Clone` as it will be cloned in order to be
  /// repeatedly iterated.
  constexpr Iterator<Item> auto cycle() && noexcept
    requires(::sus::mem::Clone<Iter>);

  /// Creates an iterator which gives the current iteration count as well as the
  /// next value.
  ///
  /// The iterator returned yields pairs `(i, val)`, where `i` is the current
  /// index of iteration and `val` is the value returned by the iterator.
  ///
  /// `enumerate()` keeps its count as a `usize`. If you want to count by a
  /// different sized integer, the `zip()` function provides similar
  /// functionality.
  ///
  /// # Overflow Behavior
  /// The method does no guarding against overflows, so enumerating more than
  /// `usize::MAX` elements either produces the wrong result or panics depending
  /// on your build configuration. If debug assertions are enabled, a panic is
  /// guaranteed.
  ///
  /// # Panics
  /// The returned iterator might panic if the to-be-returned index would
  /// overflow a `usize`.
  constexpr auto enumerate() && noexcept;

  /// Determines if the elements of this `Iterator` are equal to those of
  /// another.
  template <IntoIteratorAny Other, int&...,
            class OtherItem = typename IntoIteratorOutputType<Other>::Item>
    requires(::sus::ops::Eq<ItemT, OtherItem>)
  constexpr bool eq(Other&& other) && noexcept;

  /// Determines if the elements of this `Iterator` are equal to those of
  /// another with respect to the specified equality function.
  template <IntoIteratorAny Other, int&...,
            class OtherItem = typename IntoIteratorOutputType<Other>::Item>
  constexpr bool eq_by(
      Other&& other,
      ::sus::fn::FnMut<bool(const std::remove_reference_t<Item>&,
                            const std::remove_reference_t<OtherItem>&)> auto
          eq_fn) && noexcept;

  /// Creates an iterator which uses a closure to determine if an element should
  /// be yielded.
  ///
  /// Given an element the closure must return true or false. The returned
  /// iterator will yield only the elements for which the closure returns true.
  constexpr Iterator<Item> auto filter(
      ::sus::fn::FnMut<bool(const std::remove_reference_t<Item>&)> auto
          pred) && noexcept;

  template <
      ::sus::fn::FnMut<::sus::fn::NonVoid(ItemT&&)> MapFn, int&...,
      class R = std::invoke_result_t<MapFn&, ItemT&&>,
      class InnerR = ::sus::option::__private::IsOptionType<R>::inner_type>
    requires(::sus::option::__private::IsOptionType<R>::value)
  constexpr Iterator<InnerR> auto filter_map(MapFn f) && noexcept;

  /// Searches for an element of an iterator that satisfies a predicate.
  ///
  /// `find()` takes a closure that returns `true` or `false`. It applies this
  /// predicate to each element of the iterator, and if any of them return true,
  /// then `find()` returns `Some(element)`. If they all return `false`, it
  /// returns `None`.
  ///
  /// `find()` is short-circuiting; in other words, it will stop processing as
  /// soon as the predicate returns `true`.
  ///
  /// If you need the index of the element, see [`position()`]().
  constexpr Option<Item> find(
      ::sus::fn::FnMut<bool(const std::remove_reference_t<Item>&)> auto
          pred) noexcept;

  /// Applies function to the elements of iterator and returns the first
  /// non-none result.
  ///
  /// `sus::move(iter).find_map(f)` is equivalent to
  /// `sus::move(iter).filter_map(f).next()`.
  template <
      ::sus::fn::FnMut<::sus::fn::NonVoid(ItemT&&)> FindFn, int&...,
      class R = std::invoke_result_t<FindFn&, ItemT&&>,
      class InnerR = ::sus::option::__private::IsOptionType<R>::inner_type>
    requires(::sus::option::__private::IsOptionType<R>::value)
  constexpr Option<InnerR> find_map(FindFn f) noexcept;

  /// Creates an iterator that works like map, but flattens nested structure.
  ///
  /// The `map()` adapter is very useful, but only when the closure argument
  /// produces values. If it produces an iterator instead, there’s an extra
  /// layer of indirection. `flat_map()` will remove this extra layer on its
  /// own.
  ///
  /// You can think of `flat_map(f)` as the semantic equivalent of mapping, and
  /// then flattening as in `map(f).flatten()`.
  ///
  /// Another way of thinking about `flat_map()`: `map()`'s closure returns one
  /// item for each element, and `flat_map()`'s closure returns an iterator for
  /// each element.
  template <::sus::fn::FnMut<::sus::fn::NonVoid(ItemT&&)> F, int&...,
            class R = std::invoke_result_t<F&, ItemT&&>,
            class InnerR = IntoIteratorOutputType<R>::Item>
    requires(IntoIteratorAny<R>)
  constexpr Iterator<InnerR> auto flat_map(F f) && noexcept;

  /// Creates an iterator that flattens nested structure.
  ///
  /// This is useful when you have an iterator of iterators or an iterator of
  /// things that can be turned into iterators and you want to remove one level
  /// of indirection.
  ///
  /// In other words, this type maps `Iterator[Iterable[T]]` into `Iterator[T]`.
  constexpr auto flatten() && noexcept
    requires(IntoIteratorAny<Item>);

  /// Folds every element into an accumulator by applying an operation,
  /// returning the final result.
  ///
  /// `fold()` takes two arguments: an initial value, and a closure with two
  /// arguments: an "accumulator", and an element. The closure returns the value
  /// that the accumulator should have for the next iteration.
  ///
  /// The initial value is the value the accumulator will have on the first
  /// call.
  ///
  /// After applying this closure to every element of the iterator, `fold()`
  /// returns the accumulator.
  ///
  /// This operation is sometimes called "reduce" or "inject".
  ///
  /// Folding is useful whenever you have a collection of something, and want to
  /// produce a single value from it.
  ///
  /// Note: `fold()`, and similar methods that traverse the entire iterator,
  /// might not terminate for infinite iterators, even on traits for which a
  /// result is determinable in finite time.
  ///
  /// Note: `reduce()` can be used to use the first element as the initial
  /// value, if the accumulator type and item type is the same.
  ///
  /// Note: `fold()` combines elements in a left-associative fashion. For
  /// associative operators like `+`, the order the elements are combined in is
  /// not important, but for non-associative operators like `-` the order will
  /// affect the final result. For a right-associative version of `fold()`, see
  /// `rfold()` if the `Iterator` also satisfies `DoubleEndedIterator`.
  ///
  /// # Folding over References
  ///
  /// The initial value type for fold will decay to a value (non-reference) by
  /// default, due to the way C++ templates resolve types. In order to fold over
  /// reference types, pass the type of the initial value explicitly as in the
  /// following example, which will return the last reference in the
  /// `Iterator<i32&>`.
  /// ```cpp
  /// auto v = sus::Vec<i32>(1, 2, 3);
  /// i32 init;
  /// i32& out = v.iter_mut().fold<i32&>(
  ///     init, [](i32&, i32& v) -> i32& { return v; });
  /// ::sus::check(&out == &v.last().unwrap());
  /// ```
  template <class B, ::sus::fn::FnMut<::sus::fn::NonVoid(B, ItemT)> F>
    requires(std::convertible_to<std::invoke_result_t<F&, B &&, ItemT &&>, B> &&
             (!std::is_reference_v<B> ||
              std::is_reference_v<std::invoke_result_t<F&, B &&, ItemT &&>>))
  constexpr B fold(B init, F f) && noexcept;

  /// Calls a closure on each element of an iterator.
  ///
  /// This is equivalent to using a for loop on the iterator, although break and
  /// continue are not possible from a closure. It’s generally more idiomatic to
  /// use a for loop, but for_each may be more legible when processing items at
  /// the end of longer iterator chains. In some cases for_each may also be
  /// faster than a loop, because it avoids constructing a proxy type for the
  /// loop to consume.
  template <::sus::fn::FnMut<void(ItemT&&)> F>
  constexpr void for_each(F f) && noexcept;

  /// Creates an iterator which ends after the first None.
  ///
  /// After an iterator returns `None`, future calls may or may not yield
  /// `Some(T)` again. `fuse()` adapts an iterator, ensuring that after a `None`
  /// is given, it will always return None forever.
  ///
  /// This is useful for cases where the iterator may continue to be polled
  /// after it has returned None.
  ///
  /// TODO: Implement a FusedIterator concept though a tag of some sort, so that
  /// fuse() can be a no-op in that case?
  constexpr Iterator<Item> auto fuse() && noexcept;

  /// Creates an iterator from a generator function that consumes the current
  /// iterator.
  ///
  /// Coroutines can not be constexpr, so this function is not constexpr to
  /// avoid deeper compiler errors.
  template <::sus::fn::FnOnce<::sus::fn::NonVoid(Iter&&)> GenFn, int&...,
            class R = std::invoke_result_t<GenFn&&, Iter&&>,
            class GenR = __private::IsGenerator<R>::type>
    requires(__private::IsGenerator<R>::value)
  Iterator<GenR> auto generate(GenFn generator_fn) && noexcept;

  /// Determines if the elements of this Iterator are
  /// [lexicographically]($sus::ops::Ord#how-can-i-implement-ord?)
  /// greater than or equal to those of another.
  template <IntoIteratorAny Other, int&...,
            class OtherItem = typename IntoIteratorOutputType<Other>::Item>
    requires(::sus::ops::PartialOrd<ItemT, OtherItem>)
  constexpr bool ge(Other&& other) && noexcept;

  /// Determines if the elements of this Iterator are
  /// [lexicographically]($sus::ops::Ord#how-can-i-implement-ord?)
  /// greater than those of another.
  template <IntoIteratorAny Other, int&...,
            class OtherItem = typename IntoIteratorOutputType<Other>::Item>
    requires(::sus::ops::PartialOrd<ItemT, OtherItem>)
  constexpr bool gt(Other&& other) && noexcept;

  /// Does something with each element of an iterator, passing the value on.
  ///
  /// When using iterators, you’ll often chain several of them together. While
  /// working on such code, you might want to check out what’s happening at
  /// various parts in the pipeline. To do that, insert a call to `inspect()`.
  ///
  /// It’s more common for `inspect()` to be used as a debugging tool than to
  /// exist in your final code, but applications may find it useful in certain
  /// situations when errors need to be logged before being discarded.
  constexpr Iterator<Item> auto inspect(
      ::sus::fn::FnMut<void(const std::remove_reference_t<Item>&)> auto
          fn) && noexcept;

  /// Checks if the elements of this iterator are sorted.
  ///
  /// That is, it returns true if for each consecutive element `a` and `b`,
  /// `a <= b` is true. If the iterator yields exactly zero or one element, true
  /// is returned.
  constexpr bool is_sorted() noexcept
    requires(::sus::ops::Ord<Item>);

  /// Checks if the elements of this iterator are sorted using the given
  /// comparator function.
  ///
  /// Returns true if for each consecutive element `a` and `b`, `a <= b` is
  /// true. If the iterator yields exactly zero or one element, true is
  /// returned.
  constexpr bool is_sorted_by(
      ::sus::fn::FnMut<std::weak_ordering(
          const std::remove_reference_t<Item>&,
          const std::remove_reference_t<Item>&)> auto compare) noexcept;

  /// Determines if the elements of this Iterator are
  /// [lexicographically]($sus::ops::Ord#how-can-i-implement-ord?)
  /// less than or equal to those of another.
  template <IntoIteratorAny Other, int&...,
            class OtherItem = typename IntoIteratorOutputType<Other>::Item>
    requires(::sus::ops::PartialOrd<ItemT, OtherItem>)
  constexpr bool le(Other&& other) && noexcept;

  /// Determines if the elements of this Iterator are
  /// [lexicographically]($sus::ops::Ord#how-can-i-implement-ord?)
  /// less than those of another.
  template <IntoIteratorAny Other, int&...,
            class OtherItem = typename IntoIteratorOutputType<Other>::Item>
    requires(::sus::ops::PartialOrd<ItemT, OtherItem>)
  constexpr bool lt(Other&& other) && noexcept;

  /// Consumes the iterator, returning the last element.
  ///
  /// This method will evaluate the iterator until it returns `None`. While
  /// doing so, it keeps track of the current element. After `None` is returned,
  /// `last()` will then return the last element it saw.
  constexpr Option<Item> last() && noexcept;

  /// Creates an iterator which uses a closure to map each element to another
  /// type.
  ///
  /// The returned iterator's type is whatever is returned by the closure.
  template <::sus::fn::FnMut<::sus::fn::NonVoid(ItemT&&)> MapFn, int&...,
            class R = std::invoke_result_t<MapFn&, ItemT&&>>
  constexpr Iterator<R> auto map(MapFn fn) && noexcept;

  /// Creates an iterator that both yields elements based on a predicate and
  /// maps.
  ///
  /// `map_while()` takes a closure as an argument that returns Options. It will
  /// call this closure on each element of the iterator, and yield elements
  /// while it returns an Option with a value in it.
  template <
      ::sus::fn::FnMut<::sus::fn::NonVoid(ItemT&&)> MapFn, int&...,
      class R = std::invoke_result_t<MapFn&, ItemT&&>,
      class InnerR = ::sus::option::__private::IsOptionType<R>::inner_type>
    requires(::sus::option::__private::IsOptionType<R>::value)
  constexpr Iterator<InnerR> auto map_while(MapFn fn) && noexcept;

  /// Returns the maximum element of an iterator.
  ///
  /// If several elements are equally maximum, the last element is returned. If
  /// the iterator is empty, None is returned.
  ///
  /// Note that `f32`/`f64` doesn’t implement `StrongOrd` due to NaN being
  /// incomparable. You can work around this by using [`Iterator::reduce`](
  /// sus::iter::IteratorBase::reduce):
  ///
  /// ```cpp
  /// sus::check(
  ///     sus::Array<f32, 3>(2.4, f32::NAN, 1.3)
  ///         .into_iter()
  ///         .reduce(&f32::max)
  ///         .unwrap() ==
  ///     2.4
  /// );
  /// ```
  constexpr Option<Item> max() && noexcept
    requires(::sus::ops::Ord<Item>);

  /// Returns the element that gives the maximum value with respect to the
  /// specified comparison function.
  ///
  /// If several elements are equally maximum, the last element is returned. If
  /// the iterator is empty, None is returned.
  constexpr Option<Item> max_by(
      sus::fn::FnMut<std::weak_ordering(
          const std::remove_reference_t<Item>&,
          const std::remove_reference_t<Item>&)> auto compare) && noexcept;

  /// Returns the element that gives the maximum value from the specified
  /// function.
  ///
  /// If several elements are equally maximum, the last element is returned. If
  /// the iterator is empty, `None` is returned.
  template <::sus::fn::FnMut<
                ::sus::fn::NonVoid(const std::remove_reference_t<ItemT>&)>
                KeyFn,
            int&...,
            class Key = std::invoke_result_t<
                KeyFn&, const std::remove_reference_t<ItemT>&>>
    requires(::sus::ops::Ord<Key> &&  //
             !std::is_reference_v<Key>)
  constexpr Option<Item> max_by_key(KeyFn fn) && noexcept;

  /// Returns the minimum element of an iterator.
  ///
  /// If several elements are equally minimum, the first element is returned. If
  /// the iterator is empty, None is returned.
  ///
  /// Note that `f32`/`f64` doesn’t implement `StrongOrd` due to NaN being
  /// incomparable. You can work around this by using [`Iterator::reduce`](
  /// sus::iter::IteratorBase::reduce):
  ///
  /// ```cpp
  /// sus::check(
  ///     sus::Array<f32, 3>(2.4, f32::NAN, 1.3)
  ///         .into_iter()
  ///         .reduce(&f32::min)
  ///         .unwrap() ==
  ///     2.4
  /// );
  /// ```
  constexpr Option<Item> min() && noexcept
    requires(::sus::ops::Ord<Item>);

  /// Returns the element that gives the minimum value with respect to the
  /// specified comparison function.
  ///
  /// If several elements are equally minimum, the first element is returned. If
  /// the iterator is empty, None is returned.
  constexpr Option<Item> min_by(
      sus::fn::FnMut<std::weak_ordering(
          const std::remove_reference_t<Item>&,
          const std::remove_reference_t<Item>&)> auto compare) && noexcept;

  /// Returns the element that gives the minimum value from the specified
  /// function.
  ///
  /// If several elements are equally minimum, the first element is returned. If
  /// the iterator is empty, `None` is returned.
  template <::sus::fn::FnMut<
                ::sus::fn::NonVoid(const std::remove_reference_t<ItemT>&)>
                KeyFn,
            int&...,
            class Key = std::invoke_result_t<
                KeyFn&, const std::remove_reference_t<ItemT>&>>
    requires(::sus::ops::Ord<Key> &&  //
             !std::is_reference_v<Key>)
  constexpr Option<Item> min_by_key(KeyFn fn) && noexcept;

  /// Determines if the elements of this `Iterator` are not equal to those of
  /// another.
  template <IntoIteratorAny Other, int&...,
            class OtherItem = typename IntoIteratorOutputType<Other>::Item>
    requires(::sus::ops::Eq<ItemT, OtherItem>)
  constexpr bool ne(Other&& other) && noexcept;

  /// Returns the nth element of the iterator.
  ///
  /// Like most indexing operations, the count starts from zero, so `nth(0u)`
  /// returns the first value, `nth(1u)` the second, and so on.
  ///
  /// Note that all preceding elements, as well as the returned element, will be
  /// consumed from the iterator. That means that the preceding elements will be
  /// discarded, and also that calling `nth(0u)` multiple times on the same
  /// iterator will return different elements.
  ///
  /// `nth()` will return `None` if `n` is greater than or equal to the length
  /// of the iterator. It will stop at the first `None` encountered in the
  /// iterator and return `None`.
  constexpr Option<Item> nth(usize n) noexcept;

  /// Returns the nth element from the end of the iterator.
  ///
  /// This is essentially the reversed version of Iterator::nth(). Although like
  /// most indexing operations, the count starts from zero, so nth_back(0)
  /// returns the first value from the end, nth_back(1) the second, and so on.
  ///
  /// Note that all elements between the end and the returned element will be
  /// consumed, including the returned element. This also means that calling
  /// nth_back(0) multiple times on the same iterator will return different
  /// elements.
  ///
  /// nth_back() will return None if n is greater than or equal to the length of
  /// the iterator.  It will stop at the first `None` encountered in the
  /// iterator and return `None`.
  constexpr Option<Item> nth_back(usize n) noexcept
    requires(DoubleEndedIterator<Iter, Item>);

  /// [Lexicographically]($sus::ops::Ord#how-can-i-implement-ord?)
  /// compares the elements of this `Iterator` with those of another.
  ///
  /// The comparison works like short-circuit evaluation, returning a result
  /// without comparing the remaining elements. As soon as an order can be
  /// determined, the evaluation stops and a result is returned.
  ///
  /// For floating-point numbers, NaN does not have a total order and will
  /// result in `std::partial_ordering::unordered` when compared.
  template <IntoIteratorAny Other, int&...,
            class OtherItem = typename IntoIteratorOutputType<Other>::Item>
    requires(::sus::ops::PartialOrd<ItemT, OtherItem>)
  constexpr std::partial_ordering partial_cmp(Other&& other) && noexcept;

  /// [Lexicographically]($sus::ops::Ord#how-can-i-implement-ord?)
  /// compares the elements of this `Iterator` with those of another with
  /// respect to the specified comparison function.
  template <IntoIteratorAny Other, int&...,
            class OtherItem = typename IntoIteratorOutputType<Other>::Item>
  constexpr std::partial_ordering partial_cmp_by(
      Other&& other,
      ::sus::fn::FnMut<std::partial_ordering(
          const std::remove_reference_t<Item>&,
          const std::remove_reference_t<OtherItem>&)> auto cmp) && noexcept;

  /// Consumes an iterator, creating two disjoint collections from it.
  ///
  /// The predicate passed to `partition()` can return `true` or `false`.
  /// `partition()` returns a pair, all of the elements for which the predicate
  /// returned `true`, and all of the elements for which it returned `false`.
  template <class B>
    requires(::sus::construct::Default<B> &&  //
             Extend<B, ItemT>)
  constexpr sus::Tuple<B, B> partition(
      ::sus::fn::FnMut<bool(const std::remove_reference_t<Item>&)> auto
          pred) && noexcept;

  /// Creates an iterator which can use the `peek()` and `peek_mut()` methods to
  /// look at the next element of the iterator without consuming it. See their
  /// documentation for more information.
  ///
  /// A peekable iterator also supports conditionally pulling the next item out
  /// of the iterator, which is built on top of `peek()`.
  ///
  /// Note that the underlying iterator is still advanced when `peek()` or
  /// `peek_mut()` are called for the first time: In order to retrieve the next
  /// element, `next()` is called on the underlying iterator, hence any side
  /// effects (i.e. anything other than fetching the next value) of the `next()`
  /// method will occur.
  constexpr Iterator<Item> auto peekable() && noexcept;

  /// Searches for an element in an iterator, returning its index.
  ///
  /// `position()` takes a closure that returns `true` or `false`. It applies
  /// this closure to each element of the iterator, and if one of them
  /// returns `true`, then `position()` returns `Some(index)`. If all of
  /// them return `false`, it returns `None`.
  ///
  /// `position()` is short-circuiting; in other words, it will stop
  /// processing as soon as it finds a `true`.
  ///
  /// If position is called multiple times on the same iterator, the second
  /// search starts where the first left off, but always considers the first
  /// element seen to be at position 0.
  ///
  /// # Panics
  ///
  /// The method does no guarding against overflows, so if there are more
  /// than [`usize::MAX`] non-matching elements, it will panic.
  constexpr Option<usize> position(
      ::sus::fn::FnMut<bool(Item&&)> auto pred) noexcept;

  /// Iterates over the entire iterator, multiplying all the elements.
  ///
  /// An empty iterator returns the "one" value of the type.
  ///
  /// `product()` can be used to multiply any type implementing `Product`,
  /// including `Option` and `Result`.
  ///
  /// # Panics
  ////
  /// When calling `product()` and a primitive integer type is being returned,
  /// method will panic if the computation overflows.
  ///
  /// Using `product<OverflowInteger<T>>()` will allow the caller to handle
  /// overflow without a panic.
  template <class P = ItemT>
    requires(Product<P, ItemT>)
  constexpr P product() && noexcept;

  /// Converts the iterator into a `std::ranges::range` for use with the std
  /// ranges library.
  ///
  /// This provides stdlib compatibility for iterators in libraries that
  /// expect stdlib types.
  ///
  /// The `sus/iter/compat_ranges.h` header must be included separately
  /// to use this method, to avoid pulling in large stdlib headers by
  /// default.
  constexpr IteratorRange<Iter> range() && noexcept;

  /// Reduces the elements to a single one, by repeatedly applying a reducing
  /// operation.
  ///
  /// If the iterator is empty, returns `None`; otherwise, returns the
  /// result of the reduction.
  ///
  /// The reducing function is a closure with two arguments: an 'accumulator',
  /// and an element. For iterators with at least one element, this is the same
  /// as `fold()` with the first element of the iterator as the initial
  /// accumulator value, folding every subsequent element into it.
  ///
  /// # Reducing References
  ///
  /// If the iterator is over references, the `reduce()` function will be
  /// limited to returning a reference; in most cases to one of the members in
  /// the iterator.
  ///
  /// To reduce and produce a new value, first apply `.copied()` or `.cloned()`
  /// and then `reduce()` that, such as `it.copied().reduce(...)` which will
  /// ensure the `reduce()` function is able to work with values instead of
  /// references.
  ///
  /// This example uses `copied()` to copy each `i32` and sum them.
  /// ```cpp
  /// auto a = sus::Array<i32, 3>(2, 3, 4);
  /// auto out = a.iter().copied().reduce(
  ///     [](i32 acc, i32 v) { return acc + v; });
  /// sus::check(out.as_value() == 2 + 3 + 4);
  /// ```
  template <::sus::fn::FnMut<ItemT(ItemT, ItemT)> F, int&...,
            class R = std::invoke_result_t<F&, ItemT&&, ItemT&&>>
    requires(!std::is_reference_v<ItemT> ||  //
             std::is_reference_v<R>)
  constexpr Option<Item> reduce(F f) && noexcept;

  /// Reverses an iterator's direction.
  ///
  /// Usually, iterators iterate from front to back. After using `rev()`, an
  /// iterator will instead iterate from back to front.
  ///
  /// This is only possible if the iterator has an end, so `rev()` only works on
  /// `DoubleEndedIterator`s.
  constexpr Iterator<Item> auto rev() && noexcept
    requires(DoubleEndedIterator<Iter, Item>);

  /// Searches for an element of an iterator from the back that satisfies a
  /// predicate.
  ///
  /// `rfind()` takes a closure that returns `true` or `false`. It applies this
  /// closure to each element of the iterator, starting at the end, and if any
  /// of them return `true`, then `rfind()` returns `Some(element)`. If they all
  /// return `false`, it returns `None`.
  ///
  /// `rfind()` is short-circuiting; in other words, it will stop processing as
  /// soon as the closure returns `true`.
  constexpr Option<Item> rfind(
      ::sus::fn::FnMut<bool(const std::remove_reference_t<Item>&)> auto
          pred) noexcept
    requires(DoubleEndedIterator<Iter, Item>);

  /// An iterator method that reduces the iterator’s elements to a single, final
  /// value, starting from the back.
  ///
  /// This is the reverse version of
  /// [`Iterator::fold()`]($sus::iter::IteratorBase::fold): it takes
  /// elements starting from the back of the iterator.
  ///
  /// `rfold()` takes two arguments: an initial value, and a closure with two
  /// arguments: an "accumulator", and an element. The closure returns the value
  /// that the accumulator should have for the next iteration.
  ///
  /// The initial value is the value the accumulator will have on the first
  /// call.
  ///
  /// After applying this closure to every element of the iterator, `rfold()`
  /// returns the accumulator.
  ///
  /// This operation is sometimes called "reduce" or "inject".
  ///
  /// Folding is useful whenever you have a collection of something, and want to
  /// produce a single value from it.
  ///
  /// Note: `rfold()` combines elements in a right-associative fashion. For
  /// associative operators like `+`, the order the elements are combined in is
  /// not important, but for non-associative operators like `-` the order will
  /// affect the final result. For a left-associative version of `rfold()`, see
  /// [`Iterator::fold()`]($sus::iter::IteratorBase::fold).
  template <class B, ::sus::fn::FnMut<::sus::fn::NonVoid(B, ItemT)> F>
    requires(DoubleEndedIterator<Iter, ItemT> &&
             std::convertible_to<std::invoke_result_t<F&, B &&, ItemT &&>, B> &&
             (!std::is_reference_v<B> ||
              std::is_reference_v<std::invoke_result_t<F&, B &&, ItemT &&>>))
  constexpr B rfold(B init, F f) && noexcept;

  /// Searches for an element in an iterator from the right, returning its
  /// index.
  ///
  /// `rposition()` takes a closure that returns `true` or `false`. It applies
  /// this closure to each element of the iterator, starting from the end,
  /// and if one of them returns `true`, then `rposition()` returns
  /// `Some(index)`. If all of them return `false`, it returns `None`.
  ///
  /// `rposition()` is short-circuiting; in other words, it will stop
  /// processing as soon as it finds a `true`.
  ///
  /// Because this requires the iterator to satisfy ExactSizeIterator, which
  /// means its length can be represented in a `usize`, this function can not
  /// overflow and will not panic.
  constexpr Option<usize> rposition(
      ::sus::fn::FnMut<bool(Item&&)> auto pred) noexcept
    requires(DoubleEndedIterator<Iter, Item> &&  //
             ExactSizeIterator<Iter, Item>);

  /// An iterator adapter which, like `fold()`, holds internal state, but unlike
  /// `fold()`, produces a new iterator.
  ///
  /// To write a function with internal state that receives the current iterator
  /// as input and yields items in arbitrary ways, see `generate()`. `scan()` is
  /// a less general tool where the given function is executed for each item in
  /// the iterator in order. However `scan()` is constexpr while generator
  /// coroutiunes can not be.
  ///
  /// `scan()` takes two arguments: an initial value which seeds the internal
  /// state, and a closure with two arguments, the first being a mutable
  /// reference to the internal state and the second an iterator element. The
  /// closure can mutate the internal state to share state between iterations.
  ///
  /// On iteration, the closure will be applied to each element of the iterator
  /// and the return value from the closure, an `Option`, is returned by the
  /// next method. Thus the closure can return `Some(value)` to yield value, or
  /// `None` to end the iteration.
  template <
      class State, ::sus::fn::FnMut<::sus::fn::NonVoid(State&, ItemT&&)> F,
      int&..., class R = std::invoke_result_t<F&, State&, ItemT&&>,
      class InnerR = ::sus::option::__private::IsOptionType<R>::inner_type>
    requires(::sus::option::__private::IsOptionType<R>::value &&  //
             !std::is_reference_v<State>)
  constexpr Iterator<InnerR> auto scan(State initial_state, F f) && noexcept;

  /// Creates an iterator that skips the first `n` elements.
  ///
  /// `skip(n)` skips elements until `n` elements are skipped or the end of the
  /// iterator is reached (whichever happens first). After that, all the
  /// remaining elements are yielded. In particular, if the original iterator is
  /// too short, then the returned iterator is empty.
  constexpr Iterator<Item> auto skip(usize n) && noexcept;

  /// Creates an iterator that skips elements based on a predicate.
  ///
  /// skip_while() takes a closure as an argument. It will call this closure on
  /// each element of the iterator, and ignore elements until it returns false.
  ///
  /// After false is returned, the closure is not called again, and the
  /// remaining elements are all yielded.
  constexpr Iterator<Item> auto skip_while(
      ::sus::fn::FnMut<bool(const std::remove_reference_t<Item>&)> auto
          pred) && noexcept;

  /// Creates an iterator starting at the same point, but stepping by the given
  /// amount at each iteration.
  ///
  /// The first element of the iterator will always be returned, regardless of
  /// the step given. After that, skipped elements will be lazily walked
  /// over as needed.
  ///
  /// `step_by()` behaves like the sequence `next()`, `nth(step-1)`,
  /// `self.nth(step-1)`, ...
  ///
  /// # Panics
  ///
  /// The `step` must be greater than 0, or the function will panic. A step size
  /// of 1 returns every element.
  constexpr Iterator<Item> auto step_by(usize step) && noexcept;

  /// [Lexicographically]($sus::ops::Ord#how-can-i-implement-ord?)
  /// compares the elements of this `Iterator` with those of another.
  ///
  /// Strong ordering requires each item being compared that compares equal to
  /// share the same identity (be replaceable). Typically `Ord` is
  /// sufficient, which is required for `cmp()` and `cmp_by()`, where items that
  /// compare equivalent may still have different internal state.
  ///
  /// The comparison works like short-circuit evaluation, returning a result
  /// without comparing the remaining elements. As soon as an order can be
  /// determined, the evaluation stops and a result is returned.
  template <IntoIteratorAny Other, int&...,
            class OtherItem = typename IntoIteratorOutputType<Other>::Item>
    requires(::sus::ops::StrongOrd<ItemT, OtherItem>)
  constexpr std::strong_ordering strong_cmp(Other&& other) && noexcept;

  /// [Lexicographically]($sus::ops::Ord#how-can-i-implement-ord?)
  /// compares the elements of this `Iterator` with those of another with
  /// respect to the specified comparison function.
  template <IntoIteratorAny Other, int&...,
            class OtherItem = typename IntoIteratorOutputType<Other>::Item>
  constexpr std::strong_ordering strong_cmp_by(
      Other&& other,
      ::sus::fn::FnMut<std::strong_ordering(
          const std::remove_reference_t<Item>&,
          const std::remove_reference_t<OtherItem>&)> auto cmp) && noexcept;

  /// Sums the elements of an iterator.
  ///
  /// Takes each element, adds them together, and returns the result.
  ///
  /// An empty iterator returns the zero value of the type.
  ///
  /// `sum()` can be used to sum any type implementing `Sum`, including `Option`
  /// and `Result`.
  ///
  /// # Panics
  ////
  /// When calling `sum()` and a primitive integer type is being returned,
  /// method will panic if the computation overflows.
  ///
  /// Using `sum<OverflowInteger<T>>()` will allow the caller to handle overflow
  /// without a panic.
  template <class P = ItemT>
    requires(Sum<P, ItemT>)
  constexpr P sum() && noexcept;

  /// Creates an iterator that yields the first `n` elements, or fewer if the
  /// underlying iterator ends sooner.
  ///
  /// `take(n)` yields elements until `n` elements are yielded or the end of the
  /// iterator is reached (whichever happens first). The returned iterator is a
  /// prefix of length `n` if the original iterator contains at least `n`
  /// elements, otherwise it contains all of the (fewer than `n`) elements of
  /// the original iterator.
  constexpr Iterator<Item> auto take(usize n) && noexcept;

  /// Creates an iterator that yields elements based on a predicate.
  ///
  /// `take_while()` takes a closure as an argument. It will call this closure
  /// on each element of the iterator, and yield elements while it returns
  /// `true`.
  ///
  /// After false is returned, the closure is not called again, and the
  /// remaining elements will not be yielded.
  constexpr Iterator<Item> auto take_while(
      ::sus::fn::FnMut<bool(const std::remove_reference_t<Item>&)> auto
          pred) && noexcept;

  /// Fallibly transforms an iterator into a collection, short circuiting if a
  /// failure is encountered.
  ///
  /// `try_collect()` is a variation of `collect()` that allows fallible
  /// conversions during collection. Its main use case is simplifying
  /// conversions from iterators yielding `Option<T>` into
  /// `Option<Collection<T>>`, or similarly for other Try types (e.g. `Result`
  /// or `std::optional`).
  ///
  /// Importantly, `try_collect()` doesn’t require that the outer `Try` type
  /// also implements FromIterator; only the `Try` type's `Output` type must
  /// implement it. Concretely, this means that collecting into
  /// `TryThing<Vec<i32>, _>` can be valid because `Vec<i32>` implements
  /// FromIterator, even if `TryThing` doesn’t.
  ///
  /// Also, if a failure is encountered during `try_collect()`, the iterator is
  /// still valid and may continue to be used, in which case it will continue
  /// iterating starting after the element that triggered the failure. See the
  /// last example below for an example of how this works.
  ///
  /// # Examples
  /// Successfully collecting an iterator of `Option<i32>` into
  /// `Option<Vec<i32>>`:
  /// ```
  /// auto u = Vec<Option<i32>>(some(1), some(2), some(3));
  /// auto v = sus::move(u).into_iter().try_collect<Vec<i32>>();
  /// sus::check(v == some(Vec<i32>(1, 2, 3 )));
  /// ```
  /// Failing to collect in the same way:
  /// ```
  /// auto u = Vec<Option<i32>>(some(1), some(2), none(), some(3));
  /// auto v = sus::move(u).into_iter().try_collect<Vec<i32>>();
  /// sus::check(v == none());
  /// ```
  /// A similar example, but with [`Result`]($sus::result::Result):
  /// ```
  /// enum Error { ERROR };
  /// auto u = Vec<Result<i32, Error>>(ok(1), ok(2), ok(3));
  /// auto v = sus::move(u).into_iter().try_collect<Vec<i32>>();
  /// sus::check(v == ok(Vec<i32>(1, 2, 3)));
  /// auto w = Vec<Result<i32, Error>>(ok(1), ok(2), err(ERROR), ok(3));
  /// auto x = sus::move(w).into_iter().try_collect<Vec<i32>>();
  /// sus::check(x == err(ERROR));
  /// ```
  template <class C>
    requires(::sus::ops::Try<ItemT> &&  //
             FromIterator<C, ::sus::ops::TryOutputType<ItemT>> &&
             // Void can not be collected from.
             !std::is_void_v<::sus::ops::TryOutputType<ItemT>>)
  constexpr auto try_collect() noexcept;

  /// This function acts like `fold()` but the closure returns a type that
  /// satisfies `sus::ops::Try` and which converts to the accumulator type on
  /// success through the Try concept. If the closure ever returns failure, the
  /// fold operation immediately stops and returns the failure
  /// (short-circuiting).
  ///
  /// See `fold()` for more on how to use this function.
  ///
  /// Unlike `fold()` this function may be used on an iterator without fully
  /// consuming it, since it can stop iterating early.
  ///
  /// Also unlike `fold()` the `sus::ops::Try` concept limits the accumulator
  /// value to not being a reference.
  template <class B, ::sus::fn::FnMut<::sus::fn::NonVoid(B, ItemT)> F, int&...,
            class R = std::invoke_result_t<F&, B&&, ItemT&&>>
    requires(::sus::ops::Try<R> &&
             std::convertible_to<typename ::sus::ops::TryImpl<R>::Output, B>)
  constexpr R try_fold(B init, F f) noexcept;

  /// This is the reverse version of
  /// [`Iterator::try_fold()`]($sus::iter::IteratorBase::try_fold): it
  /// takes elements starting from the back of the iterator.
  template <class B, ::sus::fn::FnMut<::sus::fn::NonVoid(B, ItemT)> F, int&...,
            class R = std::invoke_result_t<F&, B&&, ItemT&&>>
    requires(DoubleEndedIterator<Iter, ItemT> &&  //
             ::sus::ops::Try<R> &&                //
             std::convertible_to<typename ::sus::ops::TryImpl<R>::Output, B>)
  constexpr R try_rfold(B init, F f) noexcept;

  /// An iterator method that applies a fallible function to each item in the
  /// iterator, stopping at the first error and returning that error.
  ///
  /// This can also be thought of as the fallible form of `for_each()` or as the
  /// stateless version of `try_fold()`.
  ///
  /// The closure must return a type that satisfies `sus::ops::Try`Default. For
  /// each success value returned, the iteration will continue. When a failure
  /// is returned from the closure, iteration will stop and the failure will be
  /// returned from `for_each()`. On success, the default success value of the
  /// `TryDefault` is returned.
  ///
  /// Unlike `for_each()` this function may be used on an iterator without fully
  /// consuming it, since it can stop iterating early.
  template <::sus::fn::FnMut<::sus::fn::NonVoid(ItemT)> F, int&...,
            class R = std::invoke_result_t<F&, ItemT&&>>
    requires(::sus::ops::TryDefault<R>)
  constexpr R try_for_each(F f) noexcept;

  /// Converts an iterator of pairs into a pair of collections.
  ///
  /// `unzip()` consumes an entire iterator of pairs, producing two collections:
  /// one from the left elements of the pairs, and one from the right elements.
  ///
  /// This function is, in some sense, the opposite of `zip()`.
  template <class CollectionA, class CollectionB, int&...,
            class ItemA =
                ::sus::option::__private::IsTupleOfSizeTwo<ItemT>::first_type,
            class ItemB =
                ::sus::option::__private::IsTupleOfSizeTwo<ItemT>::second_type>
    requires(::sus::option::__private::IsTupleOfSizeTwo<ItemT>::value &&
             ::sus::construct::Default<CollectionA> &&  //
             ::sus::construct::Default<CollectionB> &&  //
             Extend<CollectionA, ItemA> &&              //
             Extend<CollectionB, ItemB>)
  constexpr sus::Tuple<CollectionA, CollectionB> unzip() && noexcept;

  /// "Zips up" two iterators into a single iterator of pairs.
  ///
  /// `zip()` returns a new iterator that will iterate over two other iterators,
  /// returning a tuple where the first element comes from the first iterator,
  /// and the second element comes from the second iterator.
  ///
  /// In other words, it zips two iterators together, into a single one.
  ///
  /// If either iterator returns `None`, `next()` from the zipped iterator will
  /// return `None`. If the zipped iterator has returned `None`, further calls
  /// to `next()` will try advance both iterators, and if either returns `None`
  /// the zipped iterator will continue to return `None`. The zipped iterator is
  /// not fused if both iterators are not fused, and both resume returning
  /// values.
  ///
  /// To "undo" the result of zipping up two iterators, see unzip.
  template <IntoIteratorAny Other, int&...,
            class OtherItem = typename IntoIteratorOutputType<Other>::Item>
  constexpr Iterator<sus::Tuple<ItemT, OtherItem>> auto zip(
      Other&& other) && noexcept;

  /// Transforms an iterator into a collection.
  ///
  /// collect() can turn anything iterable into a relevant collection. If this
  /// is used anything like in Rust, it would be one of the more powerful
  /// methods in the subspace library, used in a variety of contexts.
  ///
  /// The most basic pattern in which collect() is used is to turn one
  /// collection into another. You take a collection, call iter on it, do a
  /// bunch of transformations, and then collect() at the end.
  ///
  /// collect() can also create instances of types that are not typical
  /// collections. For example, a string can be built from chars, and an
  /// iterator of Result<T, E> items can be collected into Result<Collection<T>,
  /// E>. Or an iterator of Option<T> can be collected into
  /// Option<Collection<T>>.
  ///
  /// Because collect() is so general, and C++ lacks strong type inference,
  /// collect() doesn't know the type of collection that you want to produce, so
  /// you will always need to pass it a type argument, such as:
  /// ```cpp
  /// sus::move(iter).collect<MyCollection<i32>>()
  /// ```
  template <FromIterator<ItemT> C>
  constexpr C collect() && noexcept;

  /// Transforms an iterator into a Vec.
  ///
  /// This function is a shorthand for `it.collect<Vec<Item>>()` in order to
  /// avoid the need for specifying a template argument.
  ///
  /// See `collect()` for more details.
  //
  // TODO: If the iterator is over references, collect_vec() could map them to
  // NonNull.
  constexpr ::sus::collections::Vec<ItemT> collect_vec() && noexcept;
};

template <class Iter, class Item>
constexpr bool IteratorBase<Iter, Item>::all(
    ::sus::fn::FnMut<bool(Item)> auto f) noexcept {
  while (true) {
    Option<Item> item = as_subclass_mut().next();
    if (item.is_none()) return true;
    // SAFETY: `item` was checked to hold Some already.
    if (!::sus::fn::call_mut(
            f, ::sus::move(item).unwrap_unchecked(::sus::marker::unsafe_fn)))
      return false;
  }
}

template <class Iter, class Item>
constexpr bool IteratorBase<Iter, Item>::any(
    ::sus::fn::FnMut<bool(Item)> auto f) noexcept {
  while (true) {
    Option<Item> item = as_subclass_mut().next();
    if (item.is_none()) return false;
    // SAFETY: `item` was checked to hold Some already.
    if (::sus::fn::call_mut(
            f, ::sus::move(item).unwrap_unchecked(::sus::marker::unsafe_fn)))
      return true;
  }
}

template <class Iter, class Item>
constexpr Iterator<Item> auto IteratorBase<Iter, Item>::by_ref() & noexcept {
  return ByRef<Iter>(as_subclass_mut());
}

template <class Iter, class Item>
template <IntoIterator<Item> Other>
constexpr Iterator<Item> auto IteratorBase<Iter, Item>::chain(
    Other&& other) && noexcept {
  using Chain = Chain<Iter, IntoIteratorOutputType<Other>>;
  return Chain(static_cast<Iter&&>(*this), ::sus::move(other).into_iter());
}

template <class Iter, class Item>
constexpr Iterator<std::remove_cvref_t<Item>> auto
IteratorBase<Iter, Item>::cloned() && noexcept
  requires(::sus::mem::Clone<Item>)
{
  using Cloned = Cloned<Iter>;
  return Cloned(static_cast<Iter&&>(*this));
}

template <class Iter, class Item>
template <IntoIteratorAny Other, int&..., class OtherItem>
  requires(::sus::ops::Ord<Item, OtherItem>)
constexpr std::weak_ordering IteratorBase<Iter, Item>::cmp(
    Other&& other) && noexcept {
  return static_cast<Iter&&>(*this).cmp_by(
      ::sus::move(other),
      [](const std::remove_reference_t<Item>& x,
         const std::remove_reference_t<OtherItem>& y) { return x <=> y; });
}

template <class Iter, class Item>
template <IntoIteratorAny Other, int&..., class OtherItem>
constexpr std::weak_ordering IteratorBase<Iter, Item>::cmp_by(
    Other&& other,
    ::sus::fn::FnMut<std::weak_ordering(
        const std::remove_reference_t<Item>&,
        const std::remove_reference_t<OtherItem>&)> auto cmp) && noexcept {
  return __private::iter_compare<std::weak_ordering, Item, OtherItem>(
      static_cast<Iter&&>(*this), ::sus::move(other).into_iter(),
      ::sus::move(cmp));
}

template <class Iter, class Item>
Iterator<std::remove_cvref_t<Item>> auto constexpr IteratorBase<
    Iter, Item>::copied() && noexcept
  requires(::sus::mem::Copy<Item>)
{
  using Copied = Copied<Iter>;
  return Copied(static_cast<Iter&&>(*this));
}

template <class Iter, class Item>
constexpr ::sus::num::usize IteratorBase<Iter, Item>::count() && noexcept {
  auto c = 0_usize;
  while (as_subclass_mut().next().is_some()) c += 1_usize;
  return c;
}

template <class Iter, class Item>
constexpr Iterator<Item> auto IteratorBase<Iter, Item>::cycle() && noexcept
  requires(::sus::mem::Clone<Iter>)
{
  using Cycle = Cycle<Iter>;
  return Cycle(static_cast<Iter&&>(*this));
}

template <class Iter, class Item>
constexpr auto IteratorBase<Iter, Item>::enumerate() && noexcept {
  using Enumerate = Enumerate<Iter>;
  return Enumerate(static_cast<Iter&&>(*this));
}

template <class Iter, class Item>
template <IntoIteratorAny Other, int&..., class OtherItem>
  requires(::sus::ops::Eq<Item, OtherItem>)
constexpr bool IteratorBase<Iter, Item>::eq(Other&& other) && noexcept {
  return static_cast<Iter&&>(*this).eq_by(
      ::sus::move(other),
      [](const std::remove_reference_t<Item>& x,
         const std::remove_reference_t<OtherItem>& y) { return x == y; });
}

template <class Iter, class Item>
template <IntoIteratorAny Other, int&..., class OtherItem>
constexpr bool IteratorBase<Iter, Item>::eq_by(
    Other&& other,
    ::sus::fn::FnMut<bool(const std::remove_reference_t<Item>&,
                          const std::remove_reference_t<OtherItem>&)> auto
        eq_fn) && noexcept {
  return __private::iter_compare_eq<Item, OtherItem>(
      static_cast<Iter&&>(*this), ::sus::move(other).into_iter(),
      ::sus::move(eq_fn));
}

template <class Iter, class Item>
constexpr Iterator<Item> auto IteratorBase<Iter, Item>::filter(
    ::sus::fn::FnMut<bool(const std::remove_reference_t<Item>&)> auto
        pred) && noexcept {
  using Filter = Filter<Iter, decltype(pred)>;
  return Filter(::sus::move(pred), static_cast<Iter&&>(*this));
}

template <class Iter, class Item>
template <::sus::fn::FnMut<::sus::fn::NonVoid(Item&&)> MapFn, int&..., class R,
          class InnerR>
  requires(::sus::option::__private::IsOptionType<R>::value)
constexpr Iterator<InnerR> auto IteratorBase<Iter, Item>::filter_map(
    MapFn f) && noexcept {
  using FilterMap = FilterMap<InnerR, Iter, decltype(f)>;
  return FilterMap(::sus::move(f), static_cast<Iter&&>(*this));
}

template <class Iter, class Item>
constexpr Option<Item> IteratorBase<Iter, Item>::find(
    ::sus::fn::FnMut<bool(const std::remove_reference_t<Item>&)> auto
        pred) noexcept {
  while (true) {
    Option<Item> o = as_subclass_mut().next();
    if (o.is_none() || ::sus::fn::call_mut(pred, o.as_value())) return o;
  }
}

template <class Iter, class Item>
template <::sus::fn::FnMut<::sus::fn::NonVoid(Item&&)> FindFn, int&..., class R,
          class InnerR>
  requires(::sus::option::__private::IsOptionType<R>::value)
constexpr Option<InnerR> IteratorBase<Iter, Item>::find_map(FindFn f) noexcept {
  while (true) {
    Option<Option<InnerR>> o = as_subclass_mut().next().map(f);
    if (o.is_none()) return sus::Option<InnerR>();
    if (o.as_value().is_some()) return sus::move(o).flatten();
  }
}

template <class Iter, class Item>
template <::sus::fn::FnMut<::sus::fn::NonVoid(Item&&)> F, int&..., class R,
          class InnerR>
  requires(IntoIteratorAny<R>)
constexpr Iterator<InnerR> auto IteratorBase<Iter, Item>::flat_map(
    F fn) && noexcept {
  using FlatMap = FlatMap<R, Iter, F>;
  return FlatMap(::sus::move(fn), static_cast<Iter&&>(*this));
}

template <class Iter, class Item>
constexpr auto IteratorBase<Iter, Item>::flatten() && noexcept
  requires(IntoIteratorAny<Item>)
{
  using Flatten = Flatten<IntoIteratorOutputType<Item>, Iter>;
  return Flatten(static_cast<Iter&&>(*this));
}

template <class Iter, class Item>
template <class B, ::sus::fn::FnMut<::sus::fn::NonVoid(B, Item)> F>
  requires(std::convertible_to<std::invoke_result_t<F&, B &&, Item &&>, B> &&
           (!std::is_reference_v<B> ||
            std::is_reference_v<std::invoke_result_t<F&, B &&, Item &&>>))
constexpr B IteratorBase<Iter, Item>::fold(B init, F f) && noexcept {
  if constexpr (std::is_reference_v<B>) {
    std::remove_reference_t<B>* out = ::sus::mem::addressof(init);
    while (true) {
      if (Option<Item> o = as_subclass_mut().next(); o.is_none())
        return *out;
      else
        out = ::sus::mem::addressof(
            ::sus::fn::call_mut(f, *out, sus::move(o).unwrap()));
    }
  } else {
    while (true) {
      if (Option<Item> o = as_subclass_mut().next(); o.is_none())
        return init;
      else
        init = ::sus::fn::call_mut(f, ::sus::move(init), sus::move(o).unwrap());
    }
  }
}

template <class Iter, class Item>
template <::sus::fn::FnMut<void(Item&&)> F>
constexpr void IteratorBase<Iter, Item>::for_each(F f) && noexcept {
  // TODO: Implement with fold()? Allow fold to take B=void?
  while (true) {
    if (Option<Item> o = as_subclass_mut().next(); o.is_none())
      break;
    else
      ::sus::fn::call_mut(f, std::move(o).unwrap());
  }
}

template <class Iter, class Item>
constexpr Iterator<Item> auto IteratorBase<Iter, Item>::fuse() && noexcept {
  return Fuse<Iter>(static_cast<Iter&&>(*this));
}

template <class Iter, class Item>
template <::sus::fn::FnOnce<::sus::fn::NonVoid(Iter&&)> GenFn, int&..., class R,
          class GenR>
  requires(__private::IsGenerator<R>::value)
Iterator<GenR> auto IteratorBase<Iter, Item>::generate(
    GenFn generator_fn) && noexcept {
  return ::sus::fn::call_once(::sus::move(generator_fn),
                              static_cast<Iter&&>(*this));
}

template <class Iter, class Item>
template <IntoIteratorAny Other, int&..., class OtherItem>
  requires(::sus::ops::PartialOrd<Item, OtherItem>)
constexpr bool IteratorBase<Iter, Item>::ge(Other&& other) && noexcept {
  return static_cast<Iter&&>(*this).partial_cmp(::sus::move(other)) >= 0;
}

template <class Iter, class Item>
template <IntoIteratorAny Other, int&..., class OtherItem>
  requires(::sus::ops::PartialOrd<Item, OtherItem>)
constexpr bool IteratorBase<Iter, Item>::gt(Other&& other) && noexcept {
  return static_cast<Iter&&>(*this).partial_cmp(::sus::move(other)) > 0;
}

template <class Iter, class Item>
constexpr Iterator<Item> auto IteratorBase<Iter, Item>::inspect(
    ::sus::fn::FnMut<void(const std::remove_reference_t<Item>&)> auto
        fn) && noexcept {
  using Inspect = Inspect<Iter, decltype(fn)>;
  return Inspect(::sus::move(fn), static_cast<Iter&&>(*this));
}

template <class Iter, class Item>
constexpr bool IteratorBase<Iter, Item>::is_sorted() noexcept
  requires(::sus::ops::Ord<Item>)
{
  return is_sorted_by(
      [](const std::remove_reference_t<Item>& a,
         const std::remove_reference_t<Item>& b) { return a <=> b; });
}

template <class Iter, class Item>
constexpr bool IteratorBase<Iter, Item>::is_sorted_by(
    ::sus::fn::FnMut<std::weak_ordering(
        const std::remove_reference_t<Item>&,
        const std::remove_reference_t<Item>&)> auto compare) noexcept {
  Option<Item> o = as_subclass_mut().next();
  if (o.is_none()) return true;
  // We unwrap the `Item`, if it's a reference we need to work with it as a
  // pointer.
  if constexpr (std::is_reference_v<Item>) {
    std::remove_reference_t<Item>* last =
        ::sus::mem::addressof(sus::move(o).unwrap());
    return static_cast<Iter&&>(*this).all([&last, &compare](Item item) -> bool {
      auto ord = ::sus::fn::call_mut(compare, *last, item);
      if (ord > 0) return false;
      last = ::sus::mem::addressof(item);
      return true;
    });
  } else {
    Item last = sus::move(o).unwrap();
    return static_cast<Iter&&>(*this).all([&last, &compare](Item item) -> bool {
      auto ord = ::sus::fn::call_mut(compare, last, item);
      if (ord > 0) return false;
      last = sus::move(item);
      return true;
    });
  }
}

template <class Iter, class Item>
template <IntoIteratorAny Other, int&..., class OtherItem>
  requires(::sus::ops::PartialOrd<Item, OtherItem>)
constexpr bool IteratorBase<Iter, Item>::le(Other&& other) && noexcept {
  return static_cast<Iter&&>(*this).partial_cmp(::sus::move(other)) <= 0;
}

template <class Iter, class Item>
template <IntoIteratorAny Other, int&..., class OtherItem>
  requires(::sus::ops::PartialOrd<Item, OtherItem>)
constexpr bool IteratorBase<Iter, Item>::lt(Other&& other) && noexcept {
  return static_cast<Iter&&>(*this).partial_cmp(::sus::move(other)) < 0;
}

template <class Iter, class Item>
constexpr Option<Item> IteratorBase<Iter, Item>::last() && noexcept {
  return static_cast<Iter&&>(*this).fold(
      Option<Item>(),
      [](Option<Item>&&, Item&& cur) { return Option<Item>(cur); });
}

template <class Iter, class Item>
template <::sus::fn::FnMut<::sus::fn::NonVoid(Item&&)> MapFn, int&..., class R>
constexpr Iterator<R> auto IteratorBase<Iter, Item>::map(MapFn fn) && noexcept {
  using Map = Map<R, Iter, MapFn>;
  return Map(sus::move(fn), static_cast<Iter&&>(*this));
}

template <class Iter, class Item>
template <::sus::fn::FnMut<::sus::fn::NonVoid(Item&&)> MapFn, int&..., class R,
          class InnerR>
  requires(::sus::option::__private::IsOptionType<R>::value)
constexpr Iterator<InnerR> auto IteratorBase<Iter, Item>::map_while(
    MapFn fn) && noexcept {
  using MapWhile = MapWhile<InnerR, Iter, MapFn>;
  return MapWhile(sus::move(fn), static_cast<Iter&&>(*this));
}

template <class Iter, class Item>
constexpr Option<Item> IteratorBase<Iter, Item>::max() && noexcept
  requires(::sus::ops::Ord<Item>)
{
  return static_cast<Iter&&>(*this).max_by(
      [](const std::remove_reference_t<Item>& a,
         const std::remove_reference_t<Item>& b) { return a <=> b; });
}

template <class Iter, class Item>
constexpr Option<Item> IteratorBase<Iter, Item>::max_by(
    sus::fn::FnMut<std::weak_ordering(
        const std::remove_reference_t<Item>&,
        const std::remove_reference_t<Item>&)> auto compare) && noexcept {
  return static_cast<Iter&&>(*this).reduce(
      [&compare](Item acc, Item item) -> Item {
        auto acc_cref = static_cast<const std::remove_reference_t<Item>&>(acc);
        if (::sus::fn::call_mut(compare, item, acc_cref) >= 0)
          return ::sus::forward<Item>(item);
        return ::sus::forward<Item>(acc);
      });
}

template <class Iter, class Item>
template <
    ::sus::fn::FnMut<::sus::fn::NonVoid(const std::remove_reference_t<Item>&)>
        KeyFn,
    int&...,
    class Key>
  requires(::sus::ops::Ord<Key> &&  //
           !std::is_reference_v<Key>)
constexpr Option<Item> IteratorBase<Iter, Item>::max_by_key(
    KeyFn fn) && noexcept {
  auto fold = [&fn](sus::Tuple<Key, Item>&& acc, Item&& item) {
    Key key = ::sus::fn::call_mut(fn, item);
    if (key >= acc.template at<0>())
      return sus::Tuple<Key, Item>(::sus::move(key),
                                   ::sus::forward<Item>(item));
    return ::sus::move(acc);
  };

  // TODO: We could do .map() to make the tuple and use max_by(), and not need
  // the if statement but for that .map() would need to take a reference
  // on/ownership of `fn` and that requires heap allocations for FnMutBox.
  auto first = as_subclass_mut().next();
  if (first.is_none()) return Option<Item>();
  Key first_key = fn(first.as_value());
  return Option<Item>(
      // Run fold() over a Tuple<Key, Item> to find the max Key.
      static_cast<Iter&&>(*this)
          .fold(sus::Tuple<Key, Item>(first_key,
                                      ::sus::move(first).unwrap_unchecked(
                                          ::sus::marker::unsafe_fn)),
                fold)
          // Pull out the Item for the max Key.
          .template into_inner<1>());
}

template <class Iter, class Item>
constexpr Option<Item> IteratorBase<Iter, Item>::min() && noexcept
  requires(::sus::ops::Ord<Item>)
{
  return static_cast<Iter&&>(*this).min_by(
      [](const std::remove_reference_t<Item>& a,
         const std::remove_reference_t<Item>& b) { return a <=> b; });
}

template <class Iter, class Item>
constexpr Option<Item> IteratorBase<Iter, Item>::min_by(
    sus::fn::FnMut<std::weak_ordering(
        const std::remove_reference_t<Item>&,
        const std::remove_reference_t<Item>&)> auto compare) && noexcept {
  return static_cast<Iter&&>(*this).reduce(
      [&compare](Item acc, Item item) -> Item {
        auto acc_cref = static_cast<const std::remove_reference_t<Item>&>(acc);
        if (::sus::fn::call_mut(compare, item, acc_cref) < 0)
          return ::sus::forward<Item>(item);
        return ::sus::forward<Item>(acc);
      });
}

template <class Iter, class Item>
template <
    ::sus::fn::FnMut<::sus::fn::NonVoid(const std::remove_reference_t<Item>&)>
        KeyFn,
    int&...,
    class Key>
  requires(::sus::ops::Ord<Key> &&  //
           !std::is_reference_v<Key>)
constexpr Option<Item> IteratorBase<Iter, Item>::min_by_key(
    KeyFn fn) && noexcept {
  auto fold = [&fn](sus::Tuple<Key, Item>&& acc, Item&& item) {
    Key key = ::sus::fn::call_mut(fn, item);
    if (key < acc.template at<0>())
      return sus::Tuple<Key, Item>(::sus::move(key),
                                   ::sus::forward<Item>(item));
    return ::sus::move(acc);
  };

  // TODO: We could do .map() to make the tuple and use min_by(), and not need
  // the if statement but for that .map() would need to take a reference
  // on/ownership of `fn` and that requires heap allocations for FnMutBox.
  auto first = as_subclass_mut().next();
  if (first.is_none()) return Option<Item>();
  Key first_key = fn(first.as_value());
  return Option<Item>(
      // Run fold() over a Tuple<Key, Item> to find the min Key.
      static_cast<Iter&&>(*this)
          .fold(sus::Tuple<Key, Item>(first_key,
                                      ::sus::move(first).unwrap_unchecked(
                                          ::sus::marker::unsafe_fn)),
                fold)
          // Pull out the Item for the min Key.
          .template into_inner<1>());
}

template <class Iter, class Item>
template <IntoIteratorAny Other, int&..., class OtherItem>
  requires(::sus::ops::Eq<Item, OtherItem>)
constexpr bool IteratorBase<Iter, Item>::ne(Other&& other) && noexcept {
  return !static_cast<Iter&&>(*this).eq(::sus::move(other));
}

template <class Iter, class Item>
constexpr Option<Item> IteratorBase<Iter, Item>::nth(usize n) noexcept {
  while (true) {
    if (n == 0u) return as_subclass_mut().next();
    if (as_subclass_mut().next().is_none()) return Option<Item>();
    n -= 1u;
  }
}

template <class Iter, class Item>
constexpr Option<Item> IteratorBase<Iter, Item>::nth_back(usize n) noexcept
  requires(DoubleEndedIterator<Iter, Item>)
{
  while (true) {
    if (n == 0u) return as_subclass_mut().next_back();
    if (as_subclass_mut().next_back().is_none()) return Option<Item>();
    n -= 1u;
  }
}

template <class Iter, class Item>
template <IntoIteratorAny Other, int&..., class OtherItem>
  requires(::sus::ops::PartialOrd<Item, OtherItem>)
constexpr std::partial_ordering IteratorBase<Iter, Item>::partial_cmp(
    Other&& other) && noexcept {
  return static_cast<Iter&&>(*this).partial_cmp_by(
      ::sus::move(other),
      [](const std::remove_reference_t<Item>& x,
         const std::remove_reference_t<OtherItem>& y) -> std::partial_ordering {
        return x <=> y;
      });
}

template <class Iter, class Item>
template <IntoIteratorAny Other, int&..., class OtherItem>
constexpr std::partial_ordering IteratorBase<Iter, Item>::partial_cmp_by(
    Other&& other,
    ::sus::fn::FnMut<std::partial_ordering(
        const std::remove_reference_t<Item>&,
        const std::remove_reference_t<OtherItem>&)> auto cmp) && noexcept {
  return __private::iter_compare<std::partial_ordering, Item, OtherItem>(
      static_cast<Iter&&>(*this), ::sus::move(other).into_iter(),
      ::sus::move(cmp));
}

template <class Iter, class Item>
template <class B>
  requires(::sus::construct::Default<B> &&  //
           Extend<B, Item>)
constexpr sus::Tuple<B, B> IteratorBase<Iter, Item>::partition(
    ::sus::fn::FnMut<bool(const std::remove_reference_t<Item>&)> auto
        pred) && noexcept {
  B left;
  B right;

  auto extend = [&pred, &left, &right](Item&& i) mutable {
    if (::sus::fn::call_mut(
            pred, static_cast<const std::remove_reference_t<Item>&>(i))) {
      // TODO: Consider adding extend_one() to the Extend concept, which can
      // take Item instead of an Option<Item>.
      left.extend(::sus::Option<Item>(::sus::forward<Item>(i)));
    } else {
      right.extend(::sus::Option<Item>(::sus::forward<Item>(i)));
    }
  };

  static_cast<Iter&&>(*this).for_each(extend);
  return sus::tuple(sus::move(left), sus::move(right));
}

template <class Iter, class Item>
constexpr Iterator<Item> auto IteratorBase<Iter, Item>::peekable() && noexcept {
  using Peekable = Peekable<Iter>;
  return Peekable(static_cast<Iter&&>(*this));
}

template <class Iter, class Item>
constexpr Option<usize> IteratorBase<Iter, Item>::position(
    ::sus::fn::FnMut<bool(Item&&)> auto pred) noexcept {
  usize pos;
  while (true) {
    Option<Item> o = as_subclass_mut().next();
    if (o.is_none()) return Option<usize>();
    if (::sus::fn::call_mut(
            pred, ::sus::move(o).unwrap_unchecked(::sus::marker::unsafe_fn)))
      return Option<usize>(pos);
    pos += 1u;
  }
}

template <class Iter, class Item>
template <class P>
  requires(Product<P, Item>)
constexpr P IteratorBase<Iter, Item>::product() && noexcept {
  return P::from_product(static_cast<Iter&&>(*this));
}

template <class Iter, class Item>
constexpr IteratorRange<Iter> IteratorBase<Iter, Item>::range() && noexcept {
  return IteratorRange<Iter>(static_cast<Iter&&>(*this));
}

template <class Iter, class Item>
template <::sus::fn::FnMut<Item(Item, Item)> F, int&..., class R>
  requires(!std::is_reference_v<Item> ||  //
           std::is_reference_v<R>)
constexpr Option<Item> IteratorBase<Iter, Item>::reduce(F f) && noexcept {
  Option<Item> first = as_subclass_mut().next();
  if (first.is_some()) {
    first = Option<Item>(static_cast<Iter&&>(*this).template fold<Item>(
        ::sus::move(first).unwrap_unchecked(::sus::marker::unsafe_fn),
        ::sus::move(f)));
  }
  return first;
}

template <class Iter, class Item>
constexpr Iterator<Item> auto IteratorBase<Iter, Item>::rev() && noexcept
  requires(DoubleEndedIterator<Iter, Item>)
{
  using Reverse = Reverse<Iter>;
  return Reverse(static_cast<Iter&&>(*this));
}

template <class Iter, class Item>
constexpr Option<Item> IteratorBase<Iter, Item>::rfind(
    ::sus::fn::FnMut<bool(const std::remove_reference_t<Item>&)> auto
        pred) noexcept
  requires(DoubleEndedIterator<Iter, Item>)
{
  while (true) {
    Option<Item> o = as_subclass_mut().next_back();
    if (o.is_none() || ::sus::fn::call_mut(pred, o.as_value())) return o;
  }
}

template <class Iter, class Item>
template <class B, ::sus::fn::FnMut<::sus::fn::NonVoid(B, Item)> F>
  requires(DoubleEndedIterator<Iter, Item> &&
           std::convertible_to<std::invoke_result_t<F&, B &&, Item &&>, B> &&
           (!std::is_reference_v<B> ||
            std::is_reference_v<std::invoke_result_t<F&, B &&, Item &&>>))
constexpr B IteratorBase<Iter, Item>::rfold(B init, F f) && noexcept {
  while (true) {
    if (Option<Item> o = as_subclass_mut().next_back(); o.is_none())
      return init;
    else
      init = ::sus::fn::call_mut(f, ::sus::move(init), sus::move(o).unwrap());
  }
}

template <class Iter, class Item>
constexpr Option<usize> IteratorBase<Iter, Item>::rposition(
    ::sus::fn::FnMut<bool(Item&&)> auto pred) noexcept
  requires(DoubleEndedIterator<Iter, Item> &&  //
           ExactSizeIterator<Iter, Item>)
{
  usize pos = as_subclass().exact_size_hint();
  while (true) {
    Option<Item> o = as_subclass_mut().next_back();
    if (o.is_none()) return Option<usize>();
    // This can't underflow since exact_size_hint() promises we will iterate
    // a given number of times, and that number fits in `usize`.
    pos -= 1u;
    if (::sus::fn::call_mut(
            pred, ::sus::move(o).unwrap_unchecked(::sus::marker::unsafe_fn)))
      return Option<usize>(pos);
  }
}

template <class Iter, class Item>
template <class State, ::sus::fn::FnMut<::sus::fn::NonVoid(State&, Item&&)> F,
          int&..., class R,
          class InnerR>
  requires(::sus::option::__private::IsOptionType<R>::value &&  //
           !std::is_reference_v<State>)
constexpr Iterator<InnerR> auto IteratorBase<Iter, Item>::scan(
    State initial_state, F f) && noexcept {
  using Scan = Scan<InnerR, State, Iter, F>;
  return Scan(::sus::move(initial_state), ::sus::move(f),
              static_cast<Iter&&>(*this));
}

template <class Iter, class Item>
constexpr Iterator<Item> auto IteratorBase<Iter, Item>::skip(
    usize n) && noexcept {
  using Skip = Skip<Iter>;
  return Skip(n, static_cast<Iter&&>(*this));
}

template <class Iter, class Item>
constexpr Iterator<Item> auto IteratorBase<Iter, Item>::skip_while(
    ::sus::fn::FnMut<bool(const std::remove_reference_t<Item>&)> auto
        pred) && noexcept {
  using SkipWhile = SkipWhile<Iter, decltype(pred)>;
  return SkipWhile(::sus::move(pred), static_cast<Iter&&>(*this));
}

template <class Iter, class Item>
constexpr Iterator<Item> auto IteratorBase<Iter, Item>::step_by(
    usize step) && noexcept {
  using StepBy = StepBy<Iter>;
  return StepBy(step, static_cast<Iter&&>(*this));
}

template <class Iter, class Item>
template <IntoIteratorAny Other, int&..., class OtherItem>
  requires(::sus::ops::StrongOrd<Item, OtherItem>)
constexpr std::strong_ordering IteratorBase<Iter, Item>::strong_cmp(
    Other&& other) && noexcept {
  return static_cast<Iter&&>(*this).strong_cmp_by(
      ::sus::move(other),
      [](const std::remove_reference_t<Item>& x,
         const std::remove_reference_t<OtherItem>& y) { return x <=> y; });
}

template <class Iter, class Item>
template <IntoIteratorAny Other, int&..., class OtherItem>
constexpr std::strong_ordering IteratorBase<Iter, Item>::strong_cmp_by(
    Other&& other,
    ::sus::fn::FnMut<std::strong_ordering(
        const std::remove_reference_t<Item>&,
        const std::remove_reference_t<OtherItem>&)> auto cmp) && noexcept {
  return __private::iter_compare<std::strong_ordering, Item, OtherItem>(
      static_cast<Iter&&>(*this), ::sus::move(other).into_iter(),
      ::sus::move(cmp));
}

template <class Iter, class Item>
template <class P>
  requires(Sum<P, Item>)
constexpr P IteratorBase<Iter, Item>::sum() && noexcept {
  return P::from_sum(static_cast<Iter&&>(*this));
}

template <class Iter, class Item>
constexpr Iterator<Item> auto IteratorBase<Iter, Item>::take(
    usize n) && noexcept {
  using Take = Take<Iter>;
  return Take(n, static_cast<Iter&&>(*this));
}

template <class Iter, class Item>
constexpr Iterator<Item> auto IteratorBase<Iter, Item>::take_while(
    ::sus::fn::FnMut<bool(const std::remove_reference_t<Item>&)> auto
        pred) && noexcept {
  using TakeWhile = TakeWhile<Iter, decltype(pred)>;
  return TakeWhile(::sus::move(pred), static_cast<Iter&&>(*this));
}

template <class Iter, class Item>
template <class B, ::sus::fn::FnMut<::sus::fn::NonVoid(B, Item)> F, int&...,
          class R>
  requires(::sus::ops::Try<R> &&
           std::convertible_to<typename ::sus::ops::TryImpl<R>::Output, B>)
constexpr R IteratorBase<Iter, Item>::try_fold(B init, F f) noexcept {
  while (true) {
    if (Option<Item> o = as_subclass_mut().next(); o.is_none())
      return ::sus::ops::try_from_output<R>(::sus::move(init));
    else {
      R out = ::sus::fn::call_mut(f, ::sus::move(init), sus::move(o).unwrap());
      if (!::sus::ops::try_is_success(out)) return out;
      init = ::sus::ops::try_into_output(::sus::move(out));
    }
  }
}

template <class Iter, class Item>
template <class B, ::sus::fn::FnMut<::sus::fn::NonVoid(B, Item)> F, int&...,
          class R>
  requires(DoubleEndedIterator<Iter, Item> &&  //
           ::sus::ops::Try<R> &&               //
           std::convertible_to<typename ::sus::ops::TryImpl<R>::Output, B>)
constexpr R IteratorBase<Iter, Item>::try_rfold(B init, F f) noexcept {
  while (true) {
    if (Option<Item> o = as_subclass_mut().next_back(); o.is_none())
      return ::sus::ops::try_from_output<R>(::sus::move(init));
    else {
      R out = ::sus::fn::call_mut(f, ::sus::move(init), sus::move(o).unwrap());
      if (!::sus::ops::try_is_success(out)) return out;
      init = ::sus::ops::try_into_output(::sus::move(out));
    }
  }
}

template <class Iter, class Item>
template <::sus::fn::FnMut<::sus::fn::NonVoid(Item)> F, int&..., class R>
  requires(::sus::ops::TryDefault<R>)
constexpr R IteratorBase<Iter, Item>::try_for_each(F f) noexcept {
  // TODO: Implement with try_fold()? Allow try_fold to take B=void?
  R out = ::sus::ops::try_from_default<R>();
  while (true) {
    if (Option<Item> o = as_subclass_mut().next(); o.is_none()) {
      break;
    } else {
      R test = ::sus::fn::call_mut(f, std::move(o).unwrap());
      if (!::sus::ops::try_is_success(test)) {
        out = ::sus::move(test);  // Store the failure to be returned.
        break;
      }
    }
  }
  return out;
}

template <class Iter, class Item>
template <class CollectionA, class CollectionB, int&..., class ItemA,
          class ItemB>
  requires(::sus::option::__private::IsTupleOfSizeTwo<Item>::value &&
           ::sus::construct::Default<CollectionA> &&  //
           ::sus::construct::Default<CollectionB> &&  //
           Extend<CollectionA, ItemA> &&              //
           Extend<CollectionB, ItemB>)
sus::Tuple<CollectionA, CollectionB> constexpr IteratorBase<
    Iter, Item>::unzip() && noexcept {
  auto out = sus::Tuple<CollectionA, CollectionB>();
  out.template extend<ItemA, ItemB>(static_cast<Iter&&>(*this));
  return out;
}

template <class Iter, class Item>
template <IntoIteratorAny Other, int&..., class OtherItem>
constexpr Iterator<sus::Tuple<Item, OtherItem>> auto
IteratorBase<Iter, Item>::zip(Other&& other) && noexcept {
  using Zip = Zip<Iter, IntoIteratorOutputType<Other>>;
  return Zip(
      sus::tuple(static_cast<Iter&&>(*this), sus::move(other).into_iter()));
}

template <class Iter, class Item>
template <FromIterator<Item> C>
constexpr C IteratorBase<Iter, Item>::collect() && noexcept {
  return ::sus::iter::from_iter<C>(static_cast<Iter&&>(*this));
}

template <class Iter, class Item>
constexpr ::sus::collections::Vec<Item>
IteratorBase<Iter, Item>::collect_vec() && noexcept {
  return ::sus::iter::from_iter<::sus::collections::Vec<Item>>(
      static_cast<Iter&&>(*this));
}

}  // namespace sus::iter
