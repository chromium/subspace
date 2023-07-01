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

#include "subspace/construct/into.h"
#include "subspace/fn/fn.h"
#include "subspace/iter/__private/iter_compare.h"
#include "subspace/iter/__private/iterator_end.h"
#include "subspace/iter/__private/iterator_loop.h"
#include "subspace/iter/boxed_iterator.h"
#include "subspace/iter/from_iterator.h"
#include "subspace/iter/into_iterator.h"
#include "subspace/iter/sized_iterator.h"
#include "subspace/macros/__private/compiler_bugs.h"
#include "subspace/mem/move.h"
#include "subspace/mem/size_of.h"
#include "subspace/num/unsigned_integer.h"
#include "subspace/ops/eq.h"
#include "subspace/option/option.h"

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
template <class Item>
class Generator;
template <class ToItem, class InnerSizedIter>
class Map;
template <class InnerSizedIter>
class Reverse;
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

  inline const Iter& as_subclass() const {
    return static_cast<const Iter&>(*this);
  }
  inline Iter& as_subclass_mut() { return static_cast<Iter&>(*this); }

 public:
  using Item = ItemT;

  /// Adaptor for use in ranged for loops.
  auto begin() & noexcept {
    return __private::IteratorLoop<Iter&>(as_subclass_mut());
  }
  /// Adaptor for use in ranged for loops.
  auto end() & noexcept { return __private::IteratorEnd(); }

  /// An Iterator also satisfies IntoIterator, which simply returns itself.
  ///
  /// sus::iter::IntoIterator trait implementation.
  Iter&& into_iter() && noexcept { return static_cast<Iter&&>(*this); }

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
      ::sus::fn::FnMutBox<bool(const std::remove_reference_t<Item>&)>
          pred) && noexcept;

  template <
      ::sus::fn::FnMut<::sus::fn::NonVoid(ItemT&&)> FindFn, int&...,
      class R = std::invoke_result_t<FindFn&, ItemT&&>,
      class InnerR = ::sus::option::__private::IsOptionType<R>::inner_type>
    requires(::sus::option::__private::IsOptionType<R>::value)
  Option<InnerR> find_map(FindFn f) && noexcept;

  /// Creates an iterator from a generator function that consumes the current
  /// iterator.
  template <::sus::fn::FnOnce<::sus::iter::Generator<ItemT>(Iter&&)> GenFn>
  ::sus::iter::Iterator<ItemT> auto generate(GenFn&& generator_fn) && noexcept;

  /// Creates an iterator which uses a closure to map each element to another
  /// type.
  ///
  /// The returned iterator's type is whatever is returned by the closure.
  template <class T, int&..., class R = std::invoke_result_t<T, Item&&>,
            class B = ::sus::fn::FnMutBox<R(Item&&)>>
    requires(!std::is_void_v<R> && Into<T, B>)
  Iterator<R> auto map(T fn) && noexcept
    requires(::sus::mem::relocate_by_memcpy<Iter>);

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

  /// Converts the iterator into a `std::ranges::range` for use with the std
  /// ranges library.
  ///
  /// This provides stdlib compatibility for iterators in libraries that expect
  /// stdlib types.
  ///
  /// The `subspace/iter/compat_ranges.h` header must be included separately to
  /// use this method, to avoid pulling in large stdlib headers by default.
  auto range() && noexcept;

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
    if (!f(item.take().unwrap_unchecked(::sus::marker::unsafe_fn)))
      return false;
  }
}

template <class Iter, class Item>
bool IteratorBase<Iter, Item>::any(::sus::fn::FnMutRef<bool(Item)> f) noexcept {
  while (true) {
    Option<Item> item = as_subclass_mut().next();
    if (item.is_none()) return false;
    // SAFETY: `item` was checked to hold Some already.
    if (f(item.take().unwrap_unchecked(::sus::marker::unsafe_fn))) return true;
  }
}

template <class Iter, class Item>
Iterator<Item> auto IteratorBase<Iter, Item>::by_ref() & noexcept {
  return ByRef<Iter>::with(static_cast<Iter&>(*this));
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
    ::sus::fn::FnMutBox<bool(const std::remove_reference_t<Item>&)>
        pred) && noexcept {
  while (true) {
    Option<Item> o = static_cast<Iter&>(*this).next();
    if (o.is_none() || pred(o.as_value())) return o;
  }
}

template <class Iter, class Item>
template <::sus::fn::FnMut<::sus::fn::NonVoid(Item&&)> FindFn, int&..., class R,
          class InnerR>
  requires(::sus::option::__private::IsOptionType<R>::value)
Option<InnerR> IteratorBase<Iter, Item>::find_map(FindFn f) && noexcept {
  while (true) {
    Option<Option<InnerR>> o = static_cast<Iter&>(*this).next().map(f);
    if (o.is_none()) return sus::Option<InnerR>();
    if (o.as_value().is_some()) return sus::move(o).flatten();
  }
}

template <class Iter, class Item>
template <::sus::fn::FnOnce<::sus::iter::Generator<Item>(Iter&&)> GenFn>
::sus::iter::Iterator<Item> auto IteratorBase<Iter, Item>::generate(
    GenFn&& generator_fn) && noexcept {
  return ::sus::move(generator_fn)(static_cast<Iter&&>(*this));
}

template <class Iter, class Item>
template <class T, int&..., class R, class B>
  requires(!std::is_void_v<R> && Into<T, B>)
Iterator<R> auto IteratorBase<Iter, Item>::map(T fn) && noexcept
  requires(::sus::mem::relocate_by_memcpy<Iter>)
{
  using Sized = SizedIteratorType<Iter>::type;
  using Map = Map<R, Sized>;
  return Map::with(sus::into(::sus::move(fn)),
                   make_sized_iterator(static_cast<Iter&&>(*this)));
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
auto IteratorBase<Iter, Item>::range() && noexcept {
  return ::sus::iter::IteratorRange<Iter>::with(static_cast<Iter&&>(*this));
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
