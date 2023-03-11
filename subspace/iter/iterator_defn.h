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

#include "subspace/construct/into.h"
#include "subspace/fn/fn.h"
#include "subspace/iter/__private/iterator_end.h"
#include "subspace/iter/__private/iterator_loop.h"
#include "subspace/iter/boxed_iterator.h"
#include "subspace/iter/from_iterator.h"
#include "subspace/iter/sized_iterator.h"
#include "subspace/macros/__private/compiler_bugs.h"
#include "subspace/mem/move.h"
#include "subspace/mem/size_of.h"
#include "subspace/num/unsigned_integer.h"
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
template <class InnerSizedIter>
class Filter;
template <class ToItem, class InnerSizedIter>
class Map;
template <class InnerSizedIter>
class Reverse;

struct SizeHint {
  ::sus::num::usize lower;
  ::sus::Option<::sus::num::usize> upper;
};

template <class Iter, class ItemT>
class IteratorBase {
 protected:
  constexpr IteratorBase() noexcept {
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

  /// Returns the bounds on the remaining length of the iterator.
  ///
  /// Specifically, `size_hint()` returns a `SizeHint` with a lower and upper
  /// bound.
  ///
  /// The upper bound is an Option<usize>. A None here means that either there
  /// is no known upper bound, or the upper bound is larger than usize.
  ///
  /// # Implementation notes
  ///
  /// It is not enforced that an iterator implementation yields the declared
  /// number of elements. A buggy iterator may yield less than the lower bound
  /// or more than the upper bound of elements.
  ///
  /// `size_hint()` is primarily intended to be used for optimizations such as
  /// reserving space for the elements of the iterator, but must not be trusted
  /// to e.g., omit bounds checks in unsafe code. An incorrect implementation of
  /// `size_hint()` should not lead to memory safety violations.
  ///
  /// That said, the implementation should provide a correct estimation, because
  /// otherwise it would be a violation of the `Iterator` concept's protocol.
  ///
  /// The default implementation returns `lower = 0` and `upper = None` which is
  /// correct for any iterator.
  virtual SizeHint size_hint() const noexcept;

  /// Tests whether all elements of the iterator match a predicate.
  ///
  /// If the predicate returns `true` for all elements in the iterator, this
  /// functions returns `true`, otherwise `false`. The function is
  /// short-circuiting; it stops iterating on the first `false` returned
  /// from the predicate.
  ///
  /// Returns `true` if the iterator is empty.
  virtual bool all(::sus::fn::FnMutRef<bool(Item)> f) noexcept;

  /// Tests whether any elements of the iterator match a predicate.
  ///
  /// If the predicate returns `true` for any elements in the iterator, this
  /// functions returns `true`, otherwise `false`. The function is
  /// short-circuiting; it stops iterating on the first `true` returned from
  /// the predicate.
  ///
  /// Returns `false` if the iterator is empty.
  virtual bool any(::sus::fn::FnMutRef<bool(Item)> f) noexcept;

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
  virtual ::sus::num::usize count() noexcept;

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
  auto box() && noexcept
    requires(!::sus::mem::relocate_by_memcpy<Iter>);

  /// Creates an iterator which uses a closure to map each element to another
  /// type.
  ///
  /// The returned iterator's type is whatever is returned by the closure.
  template <class T, int&..., class R = std::invoke_result_t<T, Item&&>,
  class B = ::sus::fn::FnMutBox<R(Item&&)>>
    requires(!std::is_void_v<R> && Into<T, B>)
  auto map(T fn) && noexcept
    requires(::sus::mem::relocate_by_memcpy<Iter>);

  /// Creates an iterator which uses a closure to determine if an element should
  /// be yielded.
  ///
  /// Given an element the closure must return true or false. The returned
  /// iterator will yield only the elements for which the closure returns true.
  auto filter(::sus::fn::FnMutBox<bool(const std::remove_reference_t<Item>&)>
                  pred) && noexcept
    requires(::sus::mem::relocate_by_memcpy<Iter>);

  /// Reverses an iterator's direction.
  ///
  /// Usually, iterators iterate from front to back. After using `rev()`, an
  /// iterator will instead iterate from back to front.
  ///
  /// This is only possible if the iterator has an end, so `rev()` only works on
  /// `DoubleEndedIterator`s.
  auto rev() && noexcept
    requires(::sus::mem::relocate_by_memcpy<Iter> &&
             ::sus::iter::DoubleEndedIterator<Iter, Item>);

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

  // TODO: cloned().
};

template <class Iter, class Item>
SizeHint IteratorBase<Iter, Item>::size_hint() const noexcept {
  return SizeHint(0_usize, ::sus::Option<::sus::num::usize>::none());
}

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
auto IteratorBase<Iter, Item>::box() && noexcept
  requires(!::sus::mem::relocate_by_memcpy<Iter>)
{
  using BoxedIterator =
      BoxedIterator<Item, ::sus::mem::size_of<Iter>(), alignof(Iter),
                    ::sus::iter::DoubleEndedIterator<Iter, Item>>;
  return BoxedIterator::with(static_cast<Iter&&>(*this));
}

template <class Iter, class Item>
::sus::num::usize IteratorBase<Iter, Item>::count() noexcept {
  auto c = 0_usize;
  while (as_subclass_mut().next().is_some()) c += 1_usize;
  return c;
}

template <class Iter, class Item>
template <class T, int&..., class R, class B>
  requires(!std::is_void_v<R> && Into<T, B>)
auto IteratorBase<Iter, Item>::map(T fn) && noexcept
  requires(::sus::mem::relocate_by_memcpy<Iter>)
{
  using Sized = SizedIteratorType<Iter>::type;
  using Map = Map<R, Sized>;
  return Map::with(sus::into(::sus::move(fn)),
                   make_sized_iterator(static_cast<Iter&&>(*this)));
}

template <class Iter, class Item>
auto IteratorBase<Iter, Item>::filter(
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
auto IteratorBase<Iter, Item>::rev() && noexcept
  requires(::sus::mem::relocate_by_memcpy<Iter> &&
           ::sus::iter::DoubleEndedIterator<Iter, Item>)
{
  using Sized = SizedIteratorType<Iter>::type;
  using Reverse = Reverse<Sized>;
  return Reverse::with(make_sized_iterator(static_cast<Iter&&>(*this)));
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
