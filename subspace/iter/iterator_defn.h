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

using ::sus::option::Option;

// TODO: Move forward decls somewhere?
template <class Item, size_t InnerIterSize, size_t InnerIterAlign>
class Filter;
template <class FromItem, class Item, size_t InnerIterSize,
          size_t InnerIterAlign>
class Map;
template <class Item, class InnerIter>
class Reverse;

struct SizeHint {
  ::sus::num::usize lower;
  ::sus::Option<::sus::num::usize> upper;
};

// TODO: Do we want to be able to pass IteratorBase& as a "generic" iterator?
// Then it needs access to the adaptor methods of Iterator<T>, so make them
// virtual methods on IteratorBase?
//
// TODO: We need virtual methods because we erase the type in SizedIterator and
// call the virtual methods there. But when the iterator is being used directly,
// do we need each call to next() to go through virtual? Could CRTP, so we can
// call `Subclass::next()`, with the next() method being marked `final` in the
// subclass, bypass the vtable pointer?
template <class ItemT>
class IteratorBase {
 public:
  using Item = ItemT;

  // Required methods.

  /// Gets the next element from the iterator, if there is one. Otherwise, it
  /// returns an Option holding #None.
  virtual Option<Item> next() noexcept = 0;

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
  virtual SizeHint size_hint() noexcept {
    return SizeHint(0_usize, ::sus::Option<::sus::num::usize>::none());
  }

  // Adaptors for ranged for loops.
  //
  // They are in the base class for use in FromIterator implementations which
  // see the base class type only.

  /// Adaptor for use in ranged for loops.
  auto begin() & noexcept {
    return __private::IteratorLoop<IteratorBase<Item>&>(*this);
  }
  /// Adaptor for use in ranged for loops.
  auto end() & noexcept { return __private::IteratorEnd(); }

 protected:
  IteratorBase() = default;
};

template <class Iter, class Item>
class [[nodiscard]] IteratorImpl : public IteratorBase<Item> {
 protected:
  IteratorImpl() noexcept : IteratorBase<Item>() {
    static_assert(std::is_final_v<Iter>,
                  "Iterator implementations must be `final`, as the provided "
                  "methods must know the complete type.");
  }

 public:
  // Adaptors for ranged for loops.
  //
  // Shadows the ones in the base class so that the full type is known along
  // with the `final` next() definition.

  /// Adaptor for use in ranged for loops.
  auto begin() & noexcept {
    return __private::IteratorLoop<Iter&>(static_cast<Iter&>(*this));
  }
  /// Adaptor for use in ranged for loops.
  auto end() & noexcept { return __private::IteratorEnd(); }

  /// An Iterator also satisfies IntoIterator, which simply returns itself.
  ///
  /// sus::iter::IntoIterator trait implementation.
  Iter&& into_iter() && noexcept { return ::sus::move(*this); }

  // Provided methods.

  /// Tests whether all elements of the iterator match a predicate.
  ///
  /// If the predicate returns `true` for all elements in the iterator, this
  /// functions returns `true`, otherwise `false`. The function is
  /// short-circuiting; it stops iterating on the first `false` returned
  /// from the predicate.
  ///
  /// Returns `true` if the iterator is empty.
  bool all(::sus::fn::FnMut<bool(Item)> f) noexcept;

  /// Tests whether any elements of the iterator match a predicate.
  ///
  /// If the predicate returns `true` for any elements in the iterator, this
  /// functions returns `true`, otherwise `false`. The function is
  /// short-circuiting; it stops iterating on the first `true` returned from
  /// the predicate.
  ///
  /// Returns `false` if the iterator is empty.
  bool any(::sus::fn::FnMut<bool(Item)> f) noexcept;

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
  ::sus::num::usize count() noexcept;

  /// Creates an iterator which uses a closure to map each element to another
  /// type.
  ///
  /// The returned iterator's type is whatever is returned by the closure.
  template <class MapFn, int&..., class R = std::invoke_result_t<MapFn, Item&&>,
            class MapFnMut = ::sus::fn::FnMut<R(Item&&)>>
    requires(::sus::construct::Into<MapFn, MapFnMut> && !std::is_void_v<R>)
  auto map(MapFn fn) && noexcept
    requires(::sus::mem::relocate_by_memcpy<Iter>);

  /// Creates an iterator which uses a closure to determine if an element should
  /// be yielded.
  ///
  /// Given an element the closure must return true or false. The returned
  /// iterator will yield only the elements for which the closure returns true.
  auto filter(::sus::fn::FnMut<bool(const std::remove_reference_t<Item>&)>
                  pred) && noexcept
    requires(::sus::mem::relocate_by_memcpy<Iter>);

  auto reverse() && noexcept
    requires(::sus::mem::relocate_by_memcpy<Iter>);

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
  template <::sus::iter::FromIterator<Item> C>
  ::sus::iter::FromIterator<Item> auto collect() && noexcept;

  /// Transforms an iterator into a Vec.
  ///
  /// This function is a shorthand for `it.collect<Vec<Item>>()` in order to
  /// avoid the need for specifying a template argument.
  ///
  /// See `collect()` for more details.
  //
  // TODO: If the iterator is over references, collect_vec() could map them to
  // NonNull.
  template <int&..., class Vec = ::sus::containers::Vec<Item>>
  Vec collect_vec() && noexcept;

  // TODO: cloned().
};

template <class Iter, class Item>
bool IteratorImpl<Iter, Item>::all(::sus::fn::FnMut<bool(Item)> f) noexcept {
  // TODO: If constexpr(I::all() exists) then call that instead.
  while (true) {
    Option<Item> item = static_cast<Iter&>(*this).next();
    if (item.is_none()) return true;
    // Safety: `item` was checked to hold Some already.
    if (!f(item.take().unwrap_unchecked(::sus::marker::unsafe_fn)))
      return false;
  }
}

template <class Iter, class Item>
bool IteratorImpl<Iter, Item>::any(::sus::fn::FnMut<bool(Item)> f) noexcept {
  // TODO: If constexpr(I::any() exists) then call that instead.
  while (true) {
    Option<Item> item = static_cast<Iter&>(*this).next();
    if (item.is_none()) return false;
    // Safety: `item` was checked to hold Some already.
    if (f(item.take().unwrap_unchecked(::sus::marker::unsafe_fn))) return true;
  }
}

template <class Iter, class Item>
::sus::num::usize IteratorImpl<Iter, Item>::count() noexcept {
  // TODO: If constexpr(I::count() exists) then call that instead.
  auto c = 0_usize;
  while (static_cast<Iter&>(*this).next().is_some()) c += 1_usize;
  return c;
}

template <class Iter, class Item>
auto IteratorImpl<Iter, Item>::box() && noexcept
  requires(!::sus::mem::relocate_by_memcpy<Iter>)
{
  using BoxedIterator =
      BoxedIterator<Item, ::sus::mem::size_of<Iter>(), alignof(Iter)>;
  return BoxedIterator::with(static_cast<Iter&&>(*this));
}

template <class Iter, class Item>
template <class MapFn, int&..., class R, class MapFnMut>
  requires(::sus::construct::Into<MapFn, MapFnMut> && !std::is_void_v<R>)
auto IteratorImpl<Iter, Item>::map(MapFn fn) && noexcept
  requires(::sus::mem::relocate_by_memcpy<Iter>)
{
  using Map = Map<Item, R, ::sus::mem::size_of<Iter>(), alignof(Iter)>;
  return Map::with(sus::into(::sus::move(fn)),
                   make_sized_iterator(static_cast<Iter&&>(*this)));
}

template <class Iter, class Item>
auto IteratorImpl<Iter, Item>::filter(
    ::sus::fn::FnMut<bool(const std::remove_reference_t<Item>&)>
        pred) && noexcept
  requires(::sus::mem::relocate_by_memcpy<Iter>)
{
  using Filter = Filter<Item, ::sus::mem::size_of<Iter>(), alignof(Iter)>;
  return Filter::with(::sus::move(pred),
                      make_sized_iterator(static_cast<Iter&&>(*this)));
}

template <class Iter, class Item>
auto IteratorImpl<Iter, Item>::reverse() && noexcept
  requires(::sus::mem::relocate_by_memcpy<Iter>)
{
  using Reverse = Reverse<Item, Iter>;
  return Reverse::with(make_sized_iterator(static_cast<Iter&&>(*this)));
}

template <class Iter, class Item>
template <::sus::iter::FromIterator<Item> C>
::sus::iter::FromIterator<Item> auto
IteratorImpl<Iter, Item>::collect() && noexcept {
  return C::from_iter(static_cast<Iter&&>(*this));
}

template <class Iter, class Item>
template <int&..., class Vec>
Vec IteratorImpl<Iter, Item>::collect_vec() && noexcept {
  return Vec::from_iter(static_cast<Iter&&>(*this));
}

}  // namespace sus::iter
