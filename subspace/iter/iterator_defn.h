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

#include <compare>

#include "subspace/construct/default.h"
#include "subspace/construct/into.h"
#include "subspace/fn/fn.h"
#include "subspace/iter/__private/iter_compare.h"
#include "subspace/iter/__private/iterator_end.h"
#include "subspace/iter/__private/iterator_loop.h"
#include "subspace/iter/boxed_iterator.h"
#include "subspace/iter/extend.h"
#include "subspace/iter/from_iterator.h"
#include "subspace/iter/into_iterator.h"
#include "subspace/iter/product.h"
#include "subspace/iter/sized_iterator.h"
#include "subspace/iter/sum.h"
#include "subspace/macros/__private/compiler_bugs.h"
#include "subspace/mem/addressof.h"
#include "subspace/mem/move.h"
#include "subspace/mem/size_of.h"
#include "subspace/num/unsigned_integer.h"
#include "subspace/ops/eq.h"
#include "subspace/ops/try.h"
#include "subspace/option/option.h"
#include "subspace/tuple/tuple.h"

namespace sus::containers {
template <class T>
class Vec;
}

namespace sus::result {
template <class T, class E>
class Result;
}

namespace sus::iter {

using ::sus::construct::Into;
using ::sus::option::Option;

// TODO: Move forward decls somewhere?
template <class RefIterator>
class ByRef;
template <class InnerSizedIter, class OtherSizedIter>
class Chain;
template <class InnerSizedIter>
class Cloned;
template <class InnerSizedIter>
class Copied;
template <class InnerSizedIter>
class Cycle;
template <class InnerSizedIter>
class Enumerate;
template <class InnerSizedIter>
class Filter;
template <class ToItem, class InnerSizedIter>
class FilterMap;
template <class IntoIterable, class InnerSizedIter>
class FlatMap;
template <class EachIter, class InnerSizedIter>
class Flatten;
template <class InnerIter>
class Fuse;
template <class Item>
class Generator;
template <class InnerSizedIter>
class Inspect;
template <class ToItem, class InnerSizedIter>
class Map;
template <class ToItem, class InnerSizedIter>
class MapWhile;
template <class InnerSizedIter>
class Peekable;
template <class InnerSizedIter>
class Reverse;
template <class OutType, class State, class InnerSizedIter>
  requires(!std::is_reference_v<State>)
class Scan;
template <class InnerIter>
class Skip;
template <class InnerIter>
class SkipWhile;
template <class InnerIter>
class StepBy;
template <class InnerIter>
class Take;
template <class InnerIter>
class TakeWhile;
template <class Iter>
class IteratorRange;

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
  bool all(::sus::fn::FnMutRef<bool(Item)> f) noexcept;

  /// Tests whether any elements of the iterator match a predicate.
  ///
  /// If the predicate returns `true` for any elements in the iterator, this
  /// functions returns `true`, otherwise `false`. The function is
  /// short-circuiting; it stops iterating on the first `true` returned from
  /// the predicate.
  ///
  /// Returns `false` if the iterator is empty.
  bool any(::sus::fn::FnMutRef<bool(Item)> f) noexcept;

  /// Returns an iterator that refers to this iterator, and for which operations
  /// on it will also be applied to this iterator.
  ///
  /// This is useful to allow applying iterator adapters while still retaining
  /// ownership of the original iterator.
  Iterator<Item> auto by_ref() & noexcept;

  // Provided final methods.

  /// Wraps the iterator in a new iterator that is trivially relocatable.
  ///
  /// Being trivially relocatable is required to chain the iterator, though
  /// methods such as `filter()`. This method converts the iterator to be
  /// trivially relocatable by moving the iterator into heap storage, which
  /// implies this does a heap allocation, which is slow compared to working on
  /// the stack.
  ///
  /// When possible, favour making the iterator be trivially relocatable by
  /// having it iterate over types which are themselves trivially relocatable,
  /// instead of using `box()`. This will give much better performance.
  ///
  /// It's only possible to call this in cases where it would do something
  /// useful, that is when the Iterator type is not trivially relocatable.
  Iterator<Item> auto box() && noexcept
    requires(!::sus::mem::relocate_by_memcpy<Iter>);

  /// Takes two iterators and creates a new iterator over both in sequence.
  ///
  /// `chain()` will return a new iterator which will first iterate over values
  /// from the first iterator and then over values from the second iterator.
  ///
  /// In other words, it links two iterators together, in a chain. 🔗
  ///
  /// `sus::iter::Once` is commonly used to adapt a single value into a chain of
  /// other kinds of iteration.
  template <IntoIterator<ItemT> Other>
  Iterator<Item> auto chain(Other&& other) && noexcept
    requires(::sus::mem::relocate_by_memcpy<Iter> &&
             ::sus::mem::relocate_by_memcpy<IntoIteratorOutputType<Other>>);

  /// Creates an iterator which clones all of its elements.
  ///
  /// This is useful when you have an iterator over `&T`, but you need an
  /// iterator over `T`.
  ///
  /// There is no guarantee whatsoever about the clone method actually being
  /// called or optimized away. So code should not depend on either.
  Iterator<std::remove_cvref_t<Item>> auto cloned() && noexcept
    requires(::sus::mem::Clone<Item> && ::sus::mem::relocate_by_memcpy<Iter>);

  /// [Lexicographically](sus::ops::Ord#How-can-I-implement-Ord?) compares
  /// the elements of this `Iterator` with those of another.
  template <IntoIteratorAny Other, int&...,
            class OtherItem = typename IntoIteratorOutputType<Other>::Item>
    requires(::sus::ops::Ord<ItemT, OtherItem>)
  std::strong_ordering cmp(Other&& other) && noexcept;

  /// [Lexicographically](sus::ops::Ord#How-can-I-implement-Ord?) compares
  /// the elements of this `Iterator` with those of another with respect to the
  /// specified comparison function.
  template <IntoIteratorAny Other, int&...,
            class OtherItem = typename IntoIteratorOutputType<Other>::Item>
  std::strong_ordering cmp_by(Other&& other,
                              ::sus::fn::FnMutBox<std::strong_ordering(
                                  const std::remove_reference_t<Item>&,
                                  const std::remove_reference_t<OtherItem>&)>
                                  cmp) && noexcept;

  /// Creates an iterator which copies all of its elements.
  ///
  /// This is useful when you have an iterator over &T, but you need an iterator
  /// over T.
  Iterator<std::remove_cvref_t<Item>> auto copied() && noexcept
    requires(::sus::mem::Copy<Item> && ::sus::mem::relocate_by_memcpy<Iter>);

  /// Consumes the iterator, and returns the number of elements that were in
  /// it.
  ///
  /// The function walks the iterator until it sees an Option holding #None.
  ///
  /// # Safety
  ///
  /// If the `usize` type does not have trapping arithmetic enabled, and the
  /// iterator has more than `usize::MAX` elements in it, the value will wrap
  /// and be incorrect. Otherwise, `usize` will catch overflow and panic.
  ::sus::num::usize count() && noexcept;

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
  Iterator<Item> auto cycle() && noexcept
    requires(::sus::mem::Clone<Iter> && ::sus::mem::relocate_by_memcpy<Iter>);

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
  auto enumerate() && noexcept
    requires(::sus::mem::relocate_by_memcpy<Iter>);

  /// Determines if the elements of this `Iterator` are equal to those of
  /// another.
  template <IntoIteratorAny Other, int&...,
            class OtherItem = typename IntoIteratorOutputType<Other>::Item>
    requires(::sus::ops::Eq<ItemT, OtherItem>)
  bool eq(Other&& other) && noexcept;

  /// Determines if the elements of this `Iterator` are equal to those of
  /// another with respect to the specified equality function.
  template <IntoIteratorAny Other, int&...,
            class OtherItem = typename IntoIteratorOutputType<Other>::Item>
  bool eq_by(
      Other&& other,
      ::sus::fn::FnMutBox<bool(const std::remove_reference_t<Item>&,
                               const std::remove_reference_t<OtherItem>&)>
          eq_fn) && noexcept;

  /// Creates an iterator which uses a closure to determine if an element should
  /// be yielded.
  ///
  /// Given an element the closure must return true or false. The returned
  /// iterator will yield only the elements for which the closure returns true.
  Iterator<Item> auto filter(
      ::sus::fn::FnMutBox<bool(const std::remove_reference_t<Item>&)>
          pred) && noexcept
    requires(::sus::mem::relocate_by_memcpy<Iter>);

  template <
      class F, int&..., class R = std::invoke_result_t<F, Item&&>,
      class InnerR = ::sus::option::__private::IsOptionType<R>::inner_type,
      class B = ::sus::fn::FnMutBox<R(Item&&)>>
    requires(::sus::option::__private::IsOptionType<R>::value && Into<F, B>)
  Iterator<InnerR> auto filter_map(F f) && noexcept
    requires(::sus::mem::relocate_by_memcpy<Iter>);

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
  Option<Item> find(
      ::sus::fn::FnMutRef<bool(const std::remove_reference_t<Item>&)>
          pred) noexcept;

  template <
      ::sus::fn::FnMut<::sus::fn::NonVoid(ItemT&&)> FindFn, int&...,
      class R = std::invoke_result_t<FindFn&, ItemT&&>,
      class InnerR = ::sus::option::__private::IsOptionType<R>::inner_type>
    requires(::sus::option::__private::IsOptionType<R>::value)
  Option<InnerR> find_map(FindFn f) noexcept;

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
  template <class F, int&..., class R = std::invoke_result_t<F, ItemT&&>,
            class B = ::sus::fn::FnMutBox<R(ItemT&&)>>
    requires(Into<F, B>)
  auto flat_map(F f) && noexcept
    requires(::sus::mem::relocate_by_memcpy<Iter>);

  /// Creates an iterator that flattens nested structure.
  ///
  /// This is useful when you have an iterator of iterators or an iterator of
  /// things that can be turned into iterators and you want to remove one level
  /// of indirection.
  ///
  /// In other words, this type maps `Iterator[Iterable[T]]` into `Iterator[T]`.
  auto flatten() && noexcept
    requires(IntoIteratorAny<Item> &&  //
             ::sus::mem::relocate_by_memcpy<Iter>);

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
  /// auto v = sus::Vec<i32>::with(1, 2, 3);
  /// i32 init;
  /// i32& out = v.iter_mut().fold<i32&>(
  ///     init, [](i32&, i32& v) -> i32& { return v; });
  /// ::sus::check(&out == &v.last().unwrap());
  /// ```
  template <class B, ::sus::fn::FnMut<::sus::fn::NonVoid(B, ItemT)> F>
    requires(std::convertible_to<std::invoke_result_t<F&, B &&, ItemT &&>, B> &&
             (!std::is_reference_v<B> ||
              std::is_reference_v<std::invoke_result_t<F&, B&&, ItemT&&>>))
  B fold(B init, F f) && noexcept;

  /// Calls a closure on each element of an iterator.
  ///
  /// This is equivalent to using a for loop on the iterator, although break and
  /// continue are not possible from a closure. It’s generally more idiomatic to
  /// use a for loop, but for_each may be more legible when processing items at
  /// the end of longer iterator chains. In some cases for_each may also be
  /// faster than a loop, because it avoids constructing a proxy type for the
  /// loop to consume.
  template <::sus::fn::FnMut<void(ItemT&&)> F>
  void for_each(F f) && noexcept;

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
  Iterator<Item> auto fuse() && noexcept;

  /// Creates an iterator from a generator function that consumes the current
  /// iterator.
  template <::sus::fn::FnOnce<::sus::iter::Generator<ItemT>(Iter&&)> GenFn>
  Iterator<Item> auto generate(GenFn&& generator_fn) && noexcept;

  /// Determines if the elements of this Iterator are
  /// [lexicographically](sus::ops::Ord#How-can-I-implement-Ord?) greater than
  /// or equal to those of another.
  template <IntoIteratorAny Other, int&...,
            class OtherItem = typename IntoIteratorOutputType<Other>::Item>
    requires(::sus::ops::PartialOrd<ItemT, OtherItem>)
  bool ge(Other&& other) && noexcept;

  /// Determines if the elements of this Iterator are
  /// [lexicographically](sus::ops::Ord#How-can-I-implement-Ord?) greater than
  /// those of another.
  template <IntoIteratorAny Other, int&...,
            class OtherItem = typename IntoIteratorOutputType<Other>::Item>
    requires(::sus::ops::PartialOrd<ItemT, OtherItem>)
  bool gt(Other&& other) && noexcept;

  /// Does something with each element of an iterator, passing the value on.
  ///
  /// When using iterators, you’ll often chain several of them together. While
  /// working on such code, you might want to check out what’s happening at
  /// various parts in the pipeline. To do that, insert a call to `inspect()`.
  ///
  /// It’s more common for `inspect()` to be used as a debugging tool than to
  /// exist in your final code, but applications may find it useful in certain
  /// situations when errors need to be logged before being discarded.
  template <
      class F, int&...,
      class B = ::sus::fn::FnMutBox<void(const std::remove_reference_t<Item>&)>>
    requires(Into<F, B>)
  Iterator<Item> auto inspect(F fn) && noexcept
    requires(::sus::mem::relocate_by_memcpy<Iter>);

  /// Determines if the elements of this Iterator are
  /// [lexicographically](sus::ops::Ord#How-can-I-implement-Ord?) less than or
  /// equal to those of another.
  template <IntoIteratorAny Other, int&...,
            class OtherItem = typename IntoIteratorOutputType<Other>::Item>
    requires(::sus::ops::PartialOrd<ItemT, OtherItem>)
  bool le(Other&& other) && noexcept;

  /// Determines if the elements of this Iterator are
  /// [lexicographically](sus::ops::Ord#How-can-I-implement-Ord?) less than
  /// those of another.
  template <IntoIteratorAny Other, int&...,
            class OtherItem = typename IntoIteratorOutputType<Other>::Item>
    requires(::sus::ops::PartialOrd<ItemT, OtherItem>)
  bool lt(Other&& other) && noexcept;

  /// Consumes the iterator, returning the last element.
  ///
  /// This method will evaluate the iterator until it returns `None`. While
  /// doing so, it keeps track of the current element. After `None` is returned,
  /// `last()` will then return the last element it saw.
  Option<Item> last() && noexcept;

  /// Creates an iterator which uses a closure to map each element to another
  /// type.
  ///
  /// The returned iterator's type is whatever is returned by the closure.
  template <class T, int&..., class R = std::invoke_result_t<T, Item&&>,
            class B = ::sus::fn::FnMutBox<R(Item&&)>>
    requires(!std::is_void_v<R> && Into<T, B>)
  Iterator<R> auto map(T fn) && noexcept
    requires(::sus::mem::relocate_by_memcpy<Iter>);

  template <
      class F, int&..., class R = std::invoke_result_t<F&, Item&&>,
      class InnerR = ::sus::option::__private::IsOptionType<R>::inner_type,
      class B = ::sus::fn::FnMutBox<R(Item&&)>>
    requires(::sus::option::__private::IsOptionType<R>::value && Into<F, B>)
  Iterator<InnerR> auto map_while(F fn) && noexcept
    requires(::sus::mem::relocate_by_memcpy<Iter>);

  /// Returns the maximum element of an iterator.
  ///
  /// If several elements are equally maximum, the last element is returned. If
  /// the iterator is empty, None is returned.
  ///
  /// Note that `f32`/`f64` doesn’t implement `Ord` due to NaN being
  /// incomparable. You can work around this by using [`Iterator::reduce`](
  /// sus::iter::IteratorBase::reduce):
  ///
  /// ```cpp
  /// sus::check(
  ///     sus::Array<f32, 3>::with(2.4, f32::NAN, 1.3)
  ///         .into_iter()
  ///         .reduce(&f32::max)
  ///         .unwrap() ==
  ///     2.4
  /// );
  /// ```
  Option<Item> max() && noexcept
    requires(::sus::ops::Ord<Item>);

  /// Returns the element that gives the maximum value with respect to the
  /// specified comparison function.
  ///
  /// If several elements are equally maximum, the last element is returned. If
  /// the iterator is empty, None is returned.
  Option<Item> max_by(sus::fn::FnMutRef<std::strong_ordering(
                          const std::remove_reference_t<Item>&,
                          const std::remove_reference_t<Item>&)>
                          compare) && noexcept;

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
  Option<Item> max_by_key(KeyFn fn) && noexcept;

  /// Returns the minimum element of an iterator.
  ///
  /// If several elements are equally minimum, the first element is returned. If
  /// the iterator is empty, None is returned.
  ///
  /// Note that `f32`/`f64` doesn’t implement `Ord` due to NaN being
  /// incomparable. You can work around this by using [`Iterator::reduce`](
  /// sus::iter::IteratorBase::reduce):
  ///
  /// ```cpp
  /// sus::check(
  ///     sus::Array<f32, 3>::with(2.4, f32::NAN, 1.3)
  ///         .into_iter()
  ///         .reduce(&f32::min)
  ///         .unwrap() ==
  ///     2.4
  /// );
  /// ```
  Option<Item> min() && noexcept
    requires(::sus::ops::Ord<Item>);

  /// Returns the element that gives the minimum value with respect to the
  /// specified comparison function.
  ///
  /// If several elements are equally minimum, the first element is returned. If
  /// the iterator is empty, None is returned.
  Option<Item> min_by(sus::fn::FnMutRef<std::strong_ordering(
                          const std::remove_reference_t<Item>&,
                          const std::remove_reference_t<Item>&)>
                          compare) && noexcept;

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
  Option<Item> min_by_key(KeyFn fn) && noexcept;

  /// Determines if the elements of this `Iterator` are not equal to those of
  /// another.
  template <IntoIteratorAny Other, int&...,
            class OtherItem = typename IntoIteratorOutputType<Other>::Item>
    requires(::sus::ops::Eq<ItemT, OtherItem>)
  bool ne(Other&& other) && noexcept;

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
  Option<Item> nth(usize n) noexcept;

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
  Option<Item> nth_back(usize n) noexcept
    requires(DoubleEndedIterator<Iter, Item>);

  /// [Lexicographically](sus::ops::Ord#How-can-I-implement-Ord?) compares
  /// the elements of this `Iterator` with those of another.
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
  std::partial_ordering partial_cmp(Other&& other) && noexcept;

  /// [Lexicographically](sus::ops::Ord#How-can-I-implement-Ord?) compares
  /// the elements of this `Iterator` with those of another with respect to the
  /// specified comparison function.
  template <IntoIteratorAny Other, int&...,
            class OtherItem = typename IntoIteratorOutputType<Other>::Item>
  std::partial_ordering partial_cmp_by(
      Other&& other, ::sus::fn::FnMutBox<std::partial_ordering(
                         const std::remove_reference_t<Item>&,
                         const std::remove_reference_t<OtherItem>&)>
                         cmp) && noexcept;

  /// Consumes an iterator, creating two disjoint collections from it.
  ///
  /// The predicate passed to `partition()` can return `true` or `false`.
  /// `partition()` returns a pair, all of the elements for which the predicate
  /// returned `true`, and all of the elements for which it returned `false`.
  template <class B>
    requires(::sus::construct::Default<B> &&  //
             ::sus::iter::Extend<B, ItemT>)
  sus::Tuple<B, B> partition(
      ::sus::fn::FnMutRef<bool(const std::remove_reference_t<Item>&)>
          pred) && noexcept;

  Iterator<Item> auto peekable() && noexcept
    requires(::sus::mem::relocate_by_memcpy<Iter>);

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
  Option<usize> position(::sus::fn::FnMutRef<bool(Item&&)> pred) noexcept;

  /// Iterates over the entire iterator, multiplying all the elements
  ///
  /// An empty iterator returns the "one" value of the type.
  ///
  /// `product() can be used to multiply any type implementing `Product`,
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
  /// The `subspace/iter/compat_ranges.h` header must be included separately
  /// to use this method, to avoid pulling in large stdlib headers by
  /// default.
  auto range() && noexcept;

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
  /// auto a = sus::Array<i32, 3>::with(2, 3, 4);
  /// auto out = a.iter().copied().reduce(
  ///     [](i32 acc, i32 v) { return acc + v; });
  /// sus::check(out.as_value() == 2 + 3 + 4);
  /// ```
  template <::sus::fn::FnMut<ItemT(ItemT, ItemT)> F, int&...,
            class R = std::invoke_result_t<F&, ItemT&&, ItemT&&>>
    requires(!std::is_reference_v<ItemT> ||  //
             std::is_reference_v<R>)
  Option<Item> reduce(F f) && noexcept;

  /// Reverses an iterator's direction.
  ///
  /// Usually, iterators iterate from front to back. After using `rev()`, an
  /// iterator will instead iterate from back to front.
  ///
  /// This is only possible if the iterator has an end, so `rev()` only works on
  /// `DoubleEndedIterator`s.
  Iterator<Item> auto rev() && noexcept
    requires(::sus::mem::relocate_by_memcpy<Iter> &&
             ::sus::iter::DoubleEndedIterator<Iter, Item>);

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
  Option<Item> rfind(
      ::sus::fn::FnMutRef<bool(const std::remove_reference_t<Item>&)>
          pred) noexcept
    requires(DoubleEndedIterator<Iter, Item>);

  /// An iterator method that reduces the iterator’s elements to a single, final
  /// value, starting from the back.
  ///
  /// This is the reverse version of
  /// [`Iterator::fold()`](sus::iter::IteratorBase::fold): it takes elements
  /// starting from the back of the iterator.
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
  /// [`Iterator::fold()`](sus::iter::IteratorBase::fold).
  template <class B, ::sus::fn::FnMut<::sus::fn::NonVoid(B, ItemT)> F>
    requires(DoubleEndedIterator<Iter, ItemT> &&
             std::convertible_to<std::invoke_result_t<F&, B &&, ItemT &&>, B> &&
             (!std::is_reference_v<B> ||
              std::is_reference_v<std::invoke_result_t<F&, B&&, ItemT&&>>))
  B rfold(B init, F f) && noexcept;

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
  Option<usize> rposition(::sus::fn::FnMutRef<bool(Item&&)> pred) noexcept
    requires(DoubleEndedIterator<Iter, Item> &&  //
             ExactSizeIterator<Iter, Item>);

  /// An iterator adapter which, like `fold()`, holds internal state, but unlike
  /// `fold()`, produces a new iterator.
  ///
  /// To write a function with internal state that receives the current iterator
  /// as input and yields items in arbitrary ways, see `generate()`. `scan()` is
  /// a less general tool where the given function is executed for each item in
  /// the iterator in order.
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
      class InnerR = ::sus::option::__private::IsOptionType<R>::inner_type,
      class B = ::sus::fn::FnMutBox<R(State&, Item&&)>>
    requires(::sus::option::__private::IsOptionType<R>::value &&
             ::sus::construct::Into<F, B>)
  Iterator<InnerR> auto scan(State initial_state, F f) && noexcept
    requires(::sus::mem::relocate_by_memcpy<Iter>);

  /// Creates an iterator that skips the first `n` elements.
  ///
  /// `skip(n)` skips elements until `n` elements are skipped or the end of the
  /// iterator is reached (whichever happens first). After that, all the
  /// remaining elements are yielded. In particular, if the original iterator is
  /// too short, then the returned iterator is empty.
  Iterator<Item> auto skip(usize n) && noexcept
    requires(::sus::mem::relocate_by_memcpy<Iter>);

  /// Creates an iterator that skips elements based on a predicate.
  ///
  /// skip_while() takes a closure as an argument. It will call this closure on
  /// each element of the iterator, and ignore elements until it returns false.
  ///
  /// After false is returned, the closure is not called again, and the
  /// remaining elements are all yielded.
  Iterator<Item> auto skip_while(
      ::sus::fn::FnMutBox<bool(const std::remove_reference_t<Item>&)>
          pred) && noexcept
    requires(::sus::mem::relocate_by_memcpy<Iter>);

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
  Iterator<Item> auto step_by(usize step) && noexcept
    requires(::sus::mem::relocate_by_memcpy<Iter>);

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
  Iterator<Item> auto take(usize n) && noexcept
    requires(::sus::mem::relocate_by_memcpy<Iter>);

  /// Creates an iterator that yields elements based on a predicate.
  ///
  /// `take_while()` takes a closure as an argument. It will call this closure
  /// on each element of the iterator, and yield elements while it returns
  /// `true`.
  ///
  /// After false is returned, the closure is not called again, and the
  /// remaining elements will not be yielded.
  Iterator<Item> auto take_while(
      ::sus::fn::FnMutBox<bool(const std::remove_reference_t<Item>&)>
          pred) && noexcept
    requires(::sus::mem::relocate_by_memcpy<Iter>);

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
  ///
  /// `try_fold()` does not support Try types with a void `Output` type. Since
  /// that implies there's no stored state, use try_for_each() instead.
  template <class B, ::sus::fn::FnMut<::sus::fn::NonVoid(B, ItemT)> F, int&...,
            class R = std::invoke_result_t<F&, B&&, ItemT&&>>
    requires(::sus::ops::Try<R> &&
             std::convertible_to<typename ::sus::ops::TryImpl<R>::Output, B>)
  R try_fold(B init, F f) noexcept;

  /// An iterator method that applies a fallible function to each item in the
  /// iterator, stopping at the first error and returning that error.
  ///
  /// This can also be thought of as the fallible form of `for_each()` or as the
  /// stateless version of `try_fold()`.
  ///
  /// The closure must return a type that satisfies `sus::ops::Try`. For each
  /// success value returned, the iteration will continue. When a failure is
  /// returned from the closure, iteration will stop and the failure will be
  /// returned from `for_each()`.
  ///
  /// If the `Try` type's `Output` type is not void, the first argument to
  /// try_for_each() will be a `success` value that is returned when the end of
  /// the iterator is reached.
  ///
  /// Unlike `for_each()` this function may be used on an iterator without fully
  /// consuming it, since it can stop iterating early.
  template <::sus::fn::FnMut<::sus::fn::NonVoid(ItemT)> F, int&...,
            class R = std::invoke_result_t<F&, ItemT&&>>
    requires(::sus::ops::Try<R> &&  //
             !std::is_void_v<typename ::sus::ops::TryImpl<R>::Output>)
  R try_for_each(typename ::sus::ops::TryImpl<R>::Output success, F f) noexcept;

  template <::sus::fn::FnMut<::sus::fn::NonVoid(ItemT)> F, int&...,
            class R = std::invoke_result_t<F&, ItemT&&>>
    requires(::sus::ops::Try<R> &&  //
             std::is_void_v<typename ::sus::ops::TryImpl<R>::Output>)
  R try_for_each(F f) noexcept;

  /// [Lexicographically](sus::ops::Ord#How-can-I-implement-Ord?) compares
  /// the elements of this `Iterator` with those of another.
  ///
  /// The comparison works like short-circuit evaluation, returning a result
  /// without comparing the remaining elements. As soon as an order can be
  /// determined, the evaluation stops and a result is returned.
  template <IntoIteratorAny Other, int&...,
            class OtherItem = typename IntoIteratorOutputType<Other>::Item>
    requires(::sus::ops::WeakOrd<ItemT, OtherItem>)
  std::weak_ordering weak_cmp(Other&& other) && noexcept;

  /// [Lexicographically](sus::ops::Ord#How-can-I-implement-Ord?) compares
  /// the elements of this `Iterator` with those of another with respect to the
  /// specified comparison function.
  template <IntoIteratorAny Other, int&...,
            class OtherItem = typename IntoIteratorOutputType<Other>::Item>
  std::weak_ordering weak_cmp_by(Other&& other,
                                 ::sus::fn::FnMutBox<std::weak_ordering(
                                     const std::remove_reference_t<Item>&,
                                     const std::remove_reference_t<OtherItem>&)>
                                     cmp) && noexcept;

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
  /// collections. For example, (TODO: a String can be built from chars, and) an
  /// iterator of Result<T, E> items can be collected into Result<Collection<T>,
  /// E>. Or an iterator of Option<T> can be collected into
  /// Option<Collection<T>>.
  ///
  /// Because collect() is so general, and C++ lacks strong type inference,
  /// collect() doesn't know the type of collection that you want to produce, so
  /// you will always need to pass it a type argument, such as:
  /// ```cpp
  /// sus::move(iter).collect<MyContainer<i32>>()
  /// ```
  template <::sus::iter::FromIterator<ItemT> C>
  ::sus::iter::FromIterator<ItemT> auto collect() && noexcept;

  /// Transforms an iterator into a Vec.
  ///
  /// This function is a shorthand for `it.collect<Vec<Item>>()` in order to
  /// avoid the need for specifying a template argument.
  ///
  /// See `collect()` for more details.
  //
  // TODO: If the iterator is over references, collect_vec() could map them to
  // NonNull.
  ::sus::containers::Vec<ItemT> collect_vec() && noexcept;
};

template <class Iter, class Item>
bool IteratorBase<Iter, Item>::all(::sus::fn::FnMutRef<bool(Item)> f) noexcept {
  while (true) {
    Option<Item> item = as_subclass_mut().next();
    if (item.is_none()) return true;
    // SAFETY: `item` was checked to hold Some already.
    if (!f(::sus::move(item).unwrap_unchecked(::sus::marker::unsafe_fn)))
      return false;
  }
}

template <class Iter, class Item>
bool IteratorBase<Iter, Item>::any(::sus::fn::FnMutRef<bool(Item)> f) noexcept {
  while (true) {
    Option<Item> item = as_subclass_mut().next();
    if (item.is_none()) return false;
    // SAFETY: `item` was checked to hold Some already.
    if (f(::sus::move(item).unwrap_unchecked(::sus::marker::unsafe_fn)))
      return true;
  }
}

template <class Iter, class Item>
Iterator<Item> auto IteratorBase<Iter, Item>::by_ref() & noexcept {
  return ByRef<Iter>::with(as_subclass_mut());
}

template <class Iter, class Item>
Iterator<Item> auto IteratorBase<Iter, Item>::box() && noexcept
  requires(!::sus::mem::relocate_by_memcpy<Iter>)
{
  using BoxedIterator =
      BoxedIterator<Item, ::sus::mem::size_of<Iter>(), alignof(Iter),
                    ::sus::iter::DoubleEndedIterator<Iter, Item>>;
  return BoxedIterator::with(static_cast<Iter&&>(*this));
}

template <class Iter, class Item>
template <IntoIterator<Item> Other>
Iterator<Item> auto IteratorBase<Iter, Item>::chain(Other&& other) && noexcept
  requires(::sus::mem::relocate_by_memcpy<Iter> &&
           ::sus::mem::relocate_by_memcpy<IntoIteratorOutputType<Other>>)
{
  using Sized = SizedIteratorType<Iter>::type;
  using OtherSized = SizedIteratorType<IntoIteratorOutputType<Other>>::type;
  using Chain = Chain<Sized, OtherSized>;
  return Chain::with(make_sized_iterator(static_cast<Iter&&>(*this)),
                     make_sized_iterator(::sus::move(other).into_iter()));
}

template <class Iter, class Item>
Iterator<std::remove_cvref_t<Item>> auto
IteratorBase<Iter, Item>::cloned() && noexcept
  requires(::sus::mem::Clone<Item> && ::sus::mem::relocate_by_memcpy<Iter>)
{
  using Sized = SizedIteratorType<Iter>::type;
  using Cloned = Cloned<Sized>;
  return Cloned::with(make_sized_iterator(static_cast<Iter&&>(*this)));
}

template <class Iter, class Item>
template <IntoIteratorAny Other, int&..., class OtherItem>
  requires(::sus::ops::Ord<Item, OtherItem>)
std::strong_ordering IteratorBase<Iter, Item>::cmp(Other&& other) && noexcept {
  return static_cast<Iter&&>(*this).cmp_by(
      ::sus::move(other),
      [](const std::remove_reference_t<Item>& x,
         const std::remove_reference_t<OtherItem>& y) -> std::strong_ordering {
        return x <=> y;
      });
}

template <class Iter, class Item>
template <IntoIteratorAny Other, int&..., class OtherItem>
std::strong_ordering IteratorBase<Iter, Item>::cmp_by(
    Other&& other, ::sus::fn::FnMutBox<std::strong_ordering(
                       const std::remove_reference_t<Item>&,
                       const std::remove_reference_t<OtherItem>&)>
                       cmp) && noexcept {
  return __private::iter_compare<std::strong_ordering, Item, OtherItem>(
      static_cast<Iter&&>(*this), ::sus::move(other).into_iter(),
      ::sus::move(cmp));
}

template <class Iter, class Item>
Iterator<std::remove_cvref_t<Item>> auto
IteratorBase<Iter, Item>::copied() && noexcept
  requires(::sus::mem::Copy<Item> && ::sus::mem::relocate_by_memcpy<Iter>)
{
  using Sized = SizedIteratorType<Iter>::type;
  using Copied = Copied<Sized>;
  return Copied::with(make_sized_iterator(static_cast<Iter&&>(*this)));
}

template <class Iter, class Item>
::sus::num::usize IteratorBase<Iter, Item>::count() && noexcept {
  auto c = 0_usize;
  while (as_subclass_mut().next().is_some()) c += 1_usize;
  return c;
}

template <class Iter, class Item>
Iterator<Item> auto IteratorBase<Iter, Item>::cycle() && noexcept
  requires(::sus::mem::Clone<Iter> && ::sus::mem::relocate_by_memcpy<Iter>)
{
  using Sized = SizedIteratorType<Iter>::type;
  using Cycle = Cycle<Sized>;
  return Cycle::with(make_sized_iterator(static_cast<Iter&&>(*this)));
}

template <class Iter, class Item>
auto IteratorBase<Iter, Item>::enumerate() && noexcept
  requires(::sus::mem::relocate_by_memcpy<Iter>)
{
  using Sized = SizedIteratorType<Iter>::type;
  using Enumerate = Enumerate<Sized>;
  return Enumerate::with(make_sized_iterator(static_cast<Iter&&>(*this)));
}

template <class Iter, class Item>
template <IntoIteratorAny Other, int&..., class OtherItem>
  requires(::sus::ops::Eq<Item, OtherItem>)
bool IteratorBase<Iter, Item>::eq(Other&& other) && noexcept {
  return static_cast<Iter&&>(*this).eq_by(
      ::sus::move(other),
      [](const std::remove_reference_t<Item>& x,
         const std::remove_reference_t<OtherItem>& y) { return x == y; });
}

template <class Iter, class Item>
template <IntoIteratorAny Other, int&..., class OtherItem>
bool IteratorBase<Iter, Item>::eq_by(
    Other&& other,
    ::sus::fn::FnMutBox<bool(const std::remove_reference_t<Item>&,
                             const std::remove_reference_t<OtherItem>&)>
        eq_fn) && noexcept {
  return __private::iter_compare_eq<Item, OtherItem>(
      static_cast<Iter&&>(*this), ::sus::move(other).into_iter(),
      ::sus::move(eq_fn));
}

template <class Iter, class Item>
Iterator<Item> auto IteratorBase<Iter, Item>::filter(
    ::sus::fn::FnMutBox<bool(const std::remove_reference_t<Item>&)>
        pred) && noexcept
  requires(::sus::mem::relocate_by_memcpy<Iter>)
{
  using Sized = SizedIteratorType<Iter>::type;
  using Filter = Filter<Sized>;
  return Filter::with(::sus::move(pred),
                      make_sized_iterator(static_cast<Iter&&>(*this)));
}

template <class Iter, class Item>
template <class F, int&..., class R, class InnerR, class B>
  requires(::sus::option::__private::IsOptionType<R>::value && Into<F, B>)
Iterator<InnerR> auto IteratorBase<Iter, Item>::filter_map(F f) && noexcept
  requires(::sus::mem::relocate_by_memcpy<Iter>)
{
  using Sized = SizedIteratorType<Iter>::type;
  using FilterMap = FilterMap<InnerR, Sized>;
  return FilterMap::with(::sus::move(f),
                         make_sized_iterator(static_cast<Iter&&>(*this)));
}

template <class Iter, class Item>
Option<Item> IteratorBase<Iter, Item>::find(
    ::sus::fn::FnMutRef<bool(const std::remove_reference_t<Item>&)>
        pred) noexcept {
  while (true) {
    Option<Item> o = as_subclass_mut().next();
    if (o.is_none() || pred(o.as_value())) return o;
  }
}

template <class Iter, class Item>
template <::sus::fn::FnMut<::sus::fn::NonVoid(Item&&)> FindFn, int&..., class R,
          class InnerR>
  requires(::sus::option::__private::IsOptionType<R>::value)
Option<InnerR> IteratorBase<Iter, Item>::find_map(FindFn f) noexcept {
  while (true) {
    Option<Option<InnerR>> o = as_subclass_mut().next().map(f);
    if (o.is_none()) return sus::Option<InnerR>();
    if (o.as_value().is_some()) return sus::move(o).flatten();
  }
}

template <class Iter, class Item>
auto IteratorBase<Iter, Item>::flatten() && noexcept
  requires(IntoIteratorAny<Item> &&  //
           ::sus::mem::relocate_by_memcpy<Iter>)
{
  using Sized = SizedIteratorType<Iter>::type;
  using Flatten = Flatten<IntoIteratorOutputType<Item>, Sized>;
  return Flatten::with(make_sized_iterator(static_cast<Iter&&>(*this)));
}

template <class Iter, class Item>
template <class F, int&..., class R, class B>
  requires(Into<F, B>)
auto IteratorBase<Iter, Item>::flat_map(F fn) && noexcept
  requires(::sus::mem::relocate_by_memcpy<Iter>)
{
  using Sized = SizedIteratorType<Iter>::type;
  using Flatten = FlatMap<R, Sized>;
  return Flatten::with(::sus::move_into(fn),
                       make_sized_iterator(static_cast<Iter&&>(*this)));
}

template <class Iter, class Item>
template <class B, ::sus::fn::FnMut<::sus::fn::NonVoid(B, Item)> F>
  requires(std::convertible_to<std::invoke_result_t<F&, B &&, Item &&>, B> &&
           (!std::is_reference_v<B> ||
            std::is_reference_v<std::invoke_result_t<F&, B&&, Item&&>>))
B IteratorBase<Iter, Item>::fold(B init, F f) && noexcept {
  if constexpr (std::is_reference_v<B>) {
    std::remove_reference_t<B>* out = ::sus::mem::addressof(init);
    while (true) {
      if (Option<Item> o = as_subclass_mut().next(); o.is_none())
        return *out;
      else
        out = ::sus::mem::addressof(f(*out, sus::move(o).unwrap()));
    }
  } else {
    while (true) {
      if (Option<Item> o = as_subclass_mut().next(); o.is_none())
        return init;
      else
        init = f(::sus::move(init), sus::move(o).unwrap());
    }
  }
}

template <class Iter, class Item>
template <::sus::fn::FnMut<void(Item&&)> F>
void IteratorBase<Iter, Item>::for_each(F f) && noexcept {
  // TODO: Implement with fold()? Allow fold to take B=void?
  while (true) {
    if (Option<Item> o = as_subclass_mut().next(); o.is_none())
      break;
    else
      f(std::move(o).unwrap());
  }
}

template <class Iter, class Item>
::sus::iter::Iterator<Item> auto IteratorBase<Iter, Item>::fuse() && noexcept {
  return Fuse<Iter>::with(static_cast<Iter&&>(*this));
}

template <class Iter, class Item>
template <::sus::fn::FnOnce<::sus::iter::Generator<Item>(Iter&&)> GenFn>
::sus::iter::Iterator<Item> auto IteratorBase<Iter, Item>::generate(
    GenFn&& generator_fn) && noexcept {
  return ::sus::move(generator_fn)(static_cast<Iter&&>(*this));
}

template <class Iter, class Item>
template <IntoIteratorAny Other, int&..., class OtherItem>
  requires(::sus::ops::PartialOrd<Item, OtherItem>)
bool IteratorBase<Iter, Item>::ge(Other&& other) && noexcept {
  return static_cast<Iter&&>(*this).partial_cmp(::sus::move(other)) >= 0;
}

template <class Iter, class Item>
template <IntoIteratorAny Other, int&..., class OtherItem>
  requires(::sus::ops::PartialOrd<Item, OtherItem>)
bool IteratorBase<Iter, Item>::gt(Other&& other) && noexcept {
  return static_cast<Iter&&>(*this).partial_cmp(::sus::move(other)) > 0;
}

template <class Iter, class Item>
template <class F, int&..., class B>
  requires(Into<F, B>)
Iterator<Item> auto IteratorBase<Iter, Item>::inspect(F fn) && noexcept
  requires(::sus::mem::relocate_by_memcpy<Iter>)
{
  using Sized = SizedIteratorType<Iter>::type;
  using Inspect = Inspect<Sized>;
  return Inspect::with(::sus::move_into(fn),
                       make_sized_iterator(static_cast<Iter&&>(*this)));
}

template <class Iter, class Item>
template <IntoIteratorAny Other, int&..., class OtherItem>
  requires(::sus::ops::PartialOrd<Item, OtherItem>)
bool IteratorBase<Iter, Item>::le(Other&& other) && noexcept {
  return static_cast<Iter&&>(*this).partial_cmp(::sus::move(other)) <= 0;
}

template <class Iter, class Item>
template <IntoIteratorAny Other, int&..., class OtherItem>
  requires(::sus::ops::PartialOrd<Item, OtherItem>)
bool IteratorBase<Iter, Item>::lt(Other&& other) && noexcept {
  return static_cast<Iter&&>(*this).partial_cmp(::sus::move(other)) < 0;
}

template <class Iter, class Item>
Option<Item> IteratorBase<Iter, Item>::last() && noexcept {
  return static_cast<Iter&&>(*this).fold(
      Option<Item>(),
      [](Option<Item>&&, Item&& cur) { return Option<Item>::with(cur); });
}

template <class Iter, class Item>
template <class T, int&..., class R, class B>
  requires(!std::is_void_v<R> && Into<T, B>)
Iterator<R> auto IteratorBase<Iter, Item>::map(T fn) && noexcept
  requires(::sus::mem::relocate_by_memcpy<Iter>)
{
  using Sized = SizedIteratorType<Iter>::type;
  using Map = Map<R, Sized>;
  return Map::with(sus::move_into(fn),
                   make_sized_iterator(static_cast<Iter&&>(*this)));
}

template <class Iter, class Item>
template <class F, int&..., class R, class InnerR, class B>
  requires(::sus::option::__private::IsOptionType<R>::value && Into<F, B>)
Iterator<InnerR> auto IteratorBase<Iter, Item>::map_while(F fn) && noexcept
  requires(::sus::mem::relocate_by_memcpy<Iter>)
{
  using Sized = SizedIteratorType<Iter>::type;
  using MapWhile = MapWhile<InnerR, Sized>;
  return MapWhile::with(sus::move_into(fn),
                        make_sized_iterator(static_cast<Iter&&>(*this)));
}

template <class Iter, class Item>
Option<Item> IteratorBase<Iter, Item>::max() && noexcept
  requires(::sus::ops::Ord<Item>)
{
  return static_cast<Iter&&>(*this).max_by(
      [](const std::remove_reference_t<Item>& a,
         const std::remove_reference_t<Item>& b) { return a <=> b; });
}

template <class Iter, class Item>
Option<Item> IteratorBase<Iter, Item>::max_by(
    sus::fn::FnMutRef<
        std::strong_ordering(const std::remove_reference_t<Item>&,
                             const std::remove_reference_t<Item>&)>
        compare) && noexcept {
  return static_cast<Iter&&>(*this).reduce(
      [&compare](Item acc, Item item) -> Item {
        auto acc_cref = static_cast<const std::remove_reference_t<Item>&>(acc);
        if (compare(item, acc_cref) >= 0) return ::sus::forward<Item>(item);
        return ::sus::forward<Item>(acc);
      });
}

template <class Iter, class Item>
template <
    ::sus::fn::FnMut<::sus::fn::NonVoid(const std::remove_reference_t<Item>&)>
        KeyFn,
    int&..., class Key>
  requires(::sus::ops::Ord<Key> &&  //
           !std::is_reference_v<Key>)
Option<Item> IteratorBase<Iter, Item>::max_by_key(KeyFn fn) && noexcept {
  auto fold = [&fn](sus::Tuple<Key, Item>&& acc, Item&& item) {
    Key key = fn(item);
    if (key >= acc.template at<0>())
      return sus::Tuple<Key, Item>::with(::sus::move(key),
                                         ::sus::forward<Item>(item));
    return ::sus::move(acc);
  };

  // TODO: We could do .map() to make the tuple and use max_by(), and not need
  // the if statement but for that .map() would need to take a reference
  // on/ownership of `fn` and that requires heap allocations for FnMutBox.
  auto first = as_subclass_mut().next();
  if (first.is_none()) return Option<Item>();
  Key first_key = fn(first.as_value());
  return Option<Item>::with(
      // Run fold() over a Tuple<Key, Item> to find the max Key.
      static_cast<Iter&&>(*this)
          .fold(sus::Tuple<Key, Item>::with(first_key,
                                            ::sus::move(first).unwrap_unchecked(
                                                ::sus::marker::unsafe_fn)),
                fold)
          // Pull out the Item for the max Key.
          .template into_inner<1>());
}

template <class Iter, class Item>
Option<Item> IteratorBase<Iter, Item>::min() && noexcept
  requires(::sus::ops::Ord<Item>)
{
  return static_cast<Iter&&>(*this).min_by(
      [](const std::remove_reference_t<Item>& a,
         const std::remove_reference_t<Item>& b) { return a <=> b; });
}

template <class Iter, class Item>
Option<Item> IteratorBase<Iter, Item>::min_by(
    sus::fn::FnMutRef<
        std::strong_ordering(const std::remove_reference_t<Item>&,
                             const std::remove_reference_t<Item>&)>
        compare) && noexcept {
  return static_cast<Iter&&>(*this).reduce(
      [&compare](Item acc, Item item) -> Item {
        auto acc_cref = static_cast<const std::remove_reference_t<Item>&>(acc);
        if (compare(item, acc_cref) < 0) return ::sus::forward<Item>(item);
        return ::sus::forward<Item>(acc);
      });
}

template <class Iter, class Item>
template <
    ::sus::fn::FnMut<::sus::fn::NonVoid(const std::remove_reference_t<Item>&)>
        KeyFn,
    int&..., class Key>
  requires(::sus::ops::Ord<Key> &&  //
           !std::is_reference_v<Key>)
Option<Item> IteratorBase<Iter, Item>::min_by_key(KeyFn fn) && noexcept {
  auto fold = [&fn](sus::Tuple<Key, Item>&& acc, Item&& item) {
    Key key = fn(item);
    if (key < acc.template at<0>())
      return sus::Tuple<Key, Item>::with(::sus::move(key),
                                         ::sus::forward<Item>(item));
    return ::sus::move(acc);
  };

  // TODO: We could do .map() to make the tuple and use min_by(), and not need
  // the if statement but for that .map() would need to take a reference
  // on/ownership of `fn` and that requires heap allocations for FnMutBox.
  auto first = as_subclass_mut().next();
  if (first.is_none()) return Option<Item>();
  Key first_key = fn(first.as_value());
  return Option<Item>::with(
      // Run fold() over a Tuple<Key, Item> to find the min Key.
      static_cast<Iter&&>(*this)
          .fold(sus::Tuple<Key, Item>::with(first_key,
                                            ::sus::move(first).unwrap_unchecked(
                                                ::sus::marker::unsafe_fn)),
                fold)
          // Pull out the Item for the min Key.
          .template into_inner<1>());
}

template <class Iter, class Item>
template <IntoIteratorAny Other, int&..., class OtherItem>
  requires(::sus::ops::Eq<Item, OtherItem>)
bool IteratorBase<Iter, Item>::ne(Other&& other) && noexcept {
  return !static_cast<Iter&&>(*this).eq(::sus::move(other));
}

template <class Iter, class Item>
Option<Item> IteratorBase<Iter, Item>::nth(usize n) noexcept {
  while (true) {
    if (n == 0u) return as_subclass_mut().next();
    if (as_subclass_mut().next().is_none()) return Option<Item>();
    n -= 1u;
  }
}

template <class Iter, class Item>
Option<Item> IteratorBase<Iter, Item>::nth_back(usize n) noexcept
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
std::partial_ordering IteratorBase<Iter, Item>::partial_cmp(
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
std::partial_ordering IteratorBase<Iter, Item>::partial_cmp_by(
    Other&& other, ::sus::fn::FnMutBox<std::partial_ordering(
                       const std::remove_reference_t<Item>&,
                       const std::remove_reference_t<OtherItem>&)>
                       cmp) && noexcept {
  return __private::iter_compare<std::partial_ordering, Item, OtherItem>(
      static_cast<Iter&&>(*this), ::sus::move(other).into_iter(),
      ::sus::move(cmp));
}

template <class Iter, class Item>
template <class B>
  requires(::sus::construct::Default<B> &&  //
           ::sus::iter::Extend<B, Item>)
sus::Tuple<B, B> IteratorBase<Iter, Item>::partition(
    ::sus::fn::FnMutRef<bool(const std::remove_reference_t<Item>&)>
        pred) && noexcept {
  B left;
  B right;

  auto extend = [&pred, &left, &right](Item&& i) mutable {
    if (pred(static_cast<const std::remove_reference_t<Item>&>(i))) {
      // TODO: Consider adding extend_one() to the Extend concept, which can
      // take Item instead of an Option<Item>.
      left.extend(::sus::Option<Item>::with(::sus::forward<Item>(i)));
    } else {
      right.extend(::sus::Option<Item>::with(::sus::forward<Item>(i)));
    }
  };

  static_cast<Iter&&>(*this).for_each(extend);
  return sus::Tuple<B, B>::with(sus::move(left), sus::move(right));
}

template <class Iter, class Item>
Iterator<Item> auto IteratorBase<Iter, Item>::peekable() && noexcept
  requires(::sus::mem::relocate_by_memcpy<Iter>)
{
  using Sized = SizedIteratorType<Iter>::type;
  using Peekable = Peekable<Sized>;
  return Peekable::with(make_sized_iterator(static_cast<Iter&&>(*this)));
}

template <class Iter, class Item>
Option<usize> IteratorBase<Iter, Item>::position(
    ::sus::fn::FnMutRef<bool(Item&&)> pred) noexcept {
  usize pos;
  while (true) {
    Option<Item> o = as_subclass_mut().next();
    if (o.is_none()) return Option<usize>();
    if (pred(::sus::move(o).unwrap_unchecked(::sus::marker::unsafe_fn)))
      return Option<usize>::with(pos);
    pos += 1u;
  }
}

template <class Iter, class Item>
template <class P>
  requires(::sus::iter::Product<P, Item>)
constexpr P IteratorBase<Iter, Item>::product() && noexcept {
  return P::from_product(static_cast<Iter&&>(*this));
}

template <class Iter, class Item>
auto IteratorBase<Iter, Item>::range() && noexcept {
  return ::sus::iter::IteratorRange<Iter>::with(static_cast<Iter&&>(*this));
}

template <class Iter, class Item>
template <::sus::fn::FnMut<Item(Item, Item)> F, int&..., class R>
  requires(!std::is_reference_v<Item> ||  //
           std::is_reference_v<R>)
Option<Item> IteratorBase<Iter, Item>::reduce(F f) && noexcept {
  Option<Item> first = as_subclass_mut().next();
  if (first.is_some()) {
    first = Option<Item>::with(static_cast<Iter&&>(*this).template fold<Item>(
        ::sus::move(first).unwrap_unchecked(::sus::marker::unsafe_fn),
        ::sus::move(f)));
  }
  return first;
}

template <class Iter, class Item>
Iterator<Item> auto IteratorBase<Iter, Item>::rev() && noexcept
  requires(::sus::mem::relocate_by_memcpy<Iter> &&
           ::sus::iter::DoubleEndedIterator<Iter, Item>)
{
  using Sized = SizedIteratorType<Iter>::type;
  using Reverse = Reverse<Sized>;
  return Reverse::with(make_sized_iterator(static_cast<Iter&&>(*this)));
}

template <class Iter, class Item>
Option<Item> IteratorBase<Iter, Item>::rfind(
    ::sus::fn::FnMutRef<bool(const std::remove_reference_t<Item>&)>
        pred) noexcept
  requires(DoubleEndedIterator<Iter, Item>)
{
  while (true) {
    Option<Item> o = as_subclass_mut().next_back();
    if (o.is_none() || pred(o.as_value())) return o;
  }
}

template <class Iter, class Item>
template <class B, ::sus::fn::FnMut<::sus::fn::NonVoid(B, Item)> F>
  requires(DoubleEndedIterator<Iter, Item> &&
           std::convertible_to<std::invoke_result_t<F&, B &&, Item &&>, B> &&
           (!std::is_reference_v<B> ||
            std::is_reference_v<std::invoke_result_t<F&, B&&, Item&&>>))
B IteratorBase<Iter, Item>::rfold(B init, F f) && noexcept {
  while (true) {
    if (Option<Item> o = as_subclass_mut().next_back(); o.is_none())
      return init;
    else
      init = f(::sus::move(init), sus::move(o).unwrap());
  }
}

template <class Iter, class Item>
Option<usize> IteratorBase<Iter, Item>::rposition(
    ::sus::fn::FnMutRef<bool(Item&&)> pred) noexcept
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
    if (pred(::sus::move(o).unwrap_unchecked(::sus::marker::unsafe_fn)))
      return Option<usize>::with(pos);
  }
}

template <class Iter, class Item>
template <class State, ::sus::fn::FnMut<::sus::fn::NonVoid(State&, Item&&)> F,
          int&..., class R, class InnerR, class B>
  requires(::sus::option::__private::IsOptionType<R>::value &&
           ::sus::construct::Into<F, B>)
Iterator<InnerR> auto IteratorBase<Iter, Item>::scan(State initial_state,
                                                     F f) && noexcept
  requires(::sus::mem::relocate_by_memcpy<Iter>)
{
  using Sized = SizedIteratorType<Iter>::type;
  using Scan = Scan<InnerR, State, Sized>;
  return Scan::with(::sus::move(initial_state), ::sus::move_into(f),
                    make_sized_iterator(static_cast<Iter&&>(*this)));
}

template <class Iter, class Item>
Iterator<Item> auto IteratorBase<Iter, Item>::skip(usize n) && noexcept
  requires(::sus::mem::relocate_by_memcpy<Iter>)
{
  using Sized = SizedIteratorType<Iter>::type;
  using Skip = Skip<Sized>;
  return Skip::with(n, make_sized_iterator(static_cast<Iter&&>(*this)));
}

template <class Iter, class Item>
Iterator<Item> auto IteratorBase<Iter, Item>::skip_while(
    ::sus::fn::FnMutBox<bool(const std::remove_reference_t<Item>&)>
        pred) && noexcept
  requires(::sus::mem::relocate_by_memcpy<Iter>)
{
  using Sized = SizedIteratorType<Iter>::type;
  using SkipWhile = SkipWhile<Sized>;
  return SkipWhile::with(::sus::move(pred),
                         make_sized_iterator(static_cast<Iter&&>(*this)));
}

template <class Iter, class Item>
Iterator<Item> auto IteratorBase<Iter, Item>::step_by(usize step) && noexcept
  requires(::sus::mem::relocate_by_memcpy<Iter>)
{
  using Sized = SizedIteratorType<Iter>::type;
  using StepBy = StepBy<Sized>;
  return StepBy::with(step, make_sized_iterator(static_cast<Iter&&>(*this)));
}

template <class Iter, class Item>
template <class P>
  requires(Sum<P, Item>)
constexpr P IteratorBase<Iter, Item>::sum() && noexcept {
  return P::from_sum(static_cast<Iter&&>(*this));
}

template <class Iter, class Item>
Iterator<Item> auto IteratorBase<Iter, Item>::take(usize n) && noexcept
  requires(::sus::mem::relocate_by_memcpy<Iter>)
{
  using Sized = SizedIteratorType<Iter>::type;
  using Take = Take<Sized>;
  return Take::with(n, make_sized_iterator(static_cast<Iter&&>(*this)));
}

template <class Iter, class Item>
Iterator<Item> auto IteratorBase<Iter, Item>::take_while(
    ::sus::fn::FnMutBox<bool(const std::remove_reference_t<Item>&)>
        pred) && noexcept
  requires(::sus::mem::relocate_by_memcpy<Iter>)
{
  using Sized = SizedIteratorType<Iter>::type;
  using TakeWhile = TakeWhile<Sized>;
  return TakeWhile::with(::sus::move(pred),
                         make_sized_iterator(static_cast<Iter&&>(*this)));
}

template <class Iter, class Item>
template <class B, ::sus::fn::FnMut<::sus::fn::NonVoid(B, Item)> F, int&...,
          class R>
  requires(::sus::ops::Try<R> &&
           std::convertible_to<typename ::sus::ops::TryImpl<R>::Output, B>)
R IteratorBase<Iter, Item>::try_fold(B init, F f) noexcept {
  while (true) {
    if (Option<Item> o = as_subclass_mut().next(); o.is_none())
      return ::sus::ops::TryImpl<R>::from_output(::sus::move(init));
    else {
      R out = f(::sus::move(init), sus::move(o).unwrap());
      if (!::sus::ops::TryImpl<R>::is_success(out)) return out;
      init = ::sus::ops::TryImpl<R>::to_output(::sus::move(out));
    }
  }
}

template <class Iter, class Item>
template <::sus::fn::FnMut<::sus::fn::NonVoid(Item)> F, int&..., class R>
  requires(::sus::ops::Try<R> &&  //
           !std::is_void_v<typename ::sus::ops::TryImpl<R>::Output>)
R IteratorBase<Iter, Item>::try_for_each(
    typename ::sus::ops::TryImpl<R>::Output success, F f) noexcept {
  // TODO: Implement with try_fold()? Allow try_fold to take B=void?
  R out = ::sus::ops::TryImpl<R>::from_output(::sus::move(success));
  while (true) {
    if (Option<Item> o = as_subclass_mut().next(); o.is_none()) {
      break;
    } else {
      R test = f(std::move(o).unwrap());
      if (!::sus::ops::TryImpl<R>::is_success(test)) {
        out = ::sus::move(test);  // Store the failre to be returned.
        break;
      }
    }
  }
  return out;
}

template <class Iter, class Item>
template <::sus::fn::FnMut<::sus::fn::NonVoid(Item)> F, int&..., class R>
  requires(::sus::ops::Try<R> &&  //
           std::is_void_v<typename ::sus::ops::TryImpl<R>::Output>)
R IteratorBase<Iter, Item>::try_for_each(F f) noexcept {
  // TODO: Implement with try_fold()? Allow try_fold to take B=void?
  auto out = ::sus::ops::TryImpl<R>::from_output();
  while (true) {
    if (Option<Item> o = as_subclass_mut().next(); o.is_none()) {
      break;
    } else {
      R test = f(std::move(o).unwrap());
      if (!::sus::ops::TryImpl<R>::is_success(test)) {
        out = ::sus::move(test);  // Store the failre to be returned.
        break;
      }
    }
  }
  return out;
}

template <class Iter, class Item>
template <IntoIteratorAny Other, int&..., class OtherItem>
  requires(::sus::ops::WeakOrd<Item, OtherItem>)
std::weak_ordering IteratorBase<Iter, Item>::weak_cmp(
    Other&& other) && noexcept {
  return static_cast<Iter&&>(*this).weak_cmp_by(
      ::sus::move(other),
      [](const std::remove_reference_t<Item>& x,
         const std::remove_reference_t<OtherItem>& y) -> std::weak_ordering {
        return x <=> y;
      });
}

template <class Iter, class Item>
template <IntoIteratorAny Other, int&..., class OtherItem>
std::weak_ordering IteratorBase<Iter, Item>::weak_cmp_by(
    Other&& other, ::sus::fn::FnMutBox<std::weak_ordering(
                       const std::remove_reference_t<Item>&,
                       const std::remove_reference_t<OtherItem>&)>
                       cmp) && noexcept {
  return __private::iter_compare<std::weak_ordering, Item, OtherItem>(
      static_cast<Iter&&>(*this), ::sus::move(other).into_iter(),
      ::sus::move(cmp));
}

template <class Iter, class Item>
template <::sus::iter::FromIterator<Item> C>
::sus::iter::FromIterator<Item> auto
IteratorBase<Iter, Item>::collect() && noexcept {
  return C::from_iter(static_cast<Iter&&>(*this));
}

template <class Iter, class Item>
::sus::containers::Vec<Item>
IteratorBase<Iter, Item>::collect_vec() && noexcept {
  return ::sus::containers::Vec<Item>::from_iter(static_cast<Iter&&>(*this));
}

}  // namespace sus::iter
