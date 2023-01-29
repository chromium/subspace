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

#include "subspace/fn/fn.h"
#include "subspace/iter/__private/iterator_end.h"
#include "subspace/iter/__private/iterator_loop.h"
#include "subspace/iter/boxed_iterator.h"
#include "subspace/iter/from_iterator.h"
#include "subspace/iter/iterator_defn.h"
#include "subspace/iter/sized_iterator.h"
#include "subspace/macros/__private/compiler_bugs.h"
#include "subspace/mem/move.h"
#include "subspace/construct/into.h"
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

// TODO: Do we want to be able to pass IteratorBase& as a "generic" iterator?
// Then it needs access to the adaptor methods of Iterator<T>, so make them
// virtual methods on IteratorBase?
//
// TODO: Do we want to be able to pass Iterator by value as a "generic"
// iterator? Then we need an opaque Iterator type, which can be returned from an
// adaptor method (and we can have an explicit operator to convert to it)?
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

template <class I>
class [[nodiscard]] Iterator final : public I {
 public:
  using sus_clang_bug_58837(Item =) typename I::Item;

 private:
  template <class T>
  friend class __private::IteratorLoop;  // Can see Item.
  friend I;                              // I::foo() can construct Iterator<I>.

  template <class J>
  friend class Iterator;  // Iterator<J>::foo() can construct Iterator<I>.

  // Option can't include Iterator, due to a circular dependency between
  // Option->Once->IteratorBase->Option. So it forward declares Iterator, and
  // needs to use the constructor directly.
  template <class T>
  friend class ::sus::option::Option;

  // Result can't include Iterator, due to a circular dependency between
  // Result->Iterator->usize->Result. So it forward declares Iterator, and needs
  // to use the constructor directly.
  template <class T, class E>
  friend class ::sus::result::Result;

  template <class... Args>
  Iterator(Args&&... args) : I(static_cast<Args&&>(args)...) {
    // We want to be able to use Iterator<I> and I interchangeably, so that if
    // an `I` gets stored in SizedIterator, it doesn't misbehave.
    static_assert(::sus::mem::size_of<I>() ==
                  ::sus::mem::size_of<Iterator<I>>());
  }

 public:
  // Provided methods.

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
  Iterator<
      BoxedIterator<typename I::Item, ::sus::mem::size_of<I>(), alignof(I)>>
  box() && noexcept
    requires(!::sus::mem::relocate_by_memcpy<I>);

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
  template <class MapFn, int&..., class R = std::invoke_result_t<MapFn, Item&&>>
    requires(::sus::construct::Into<MapFn, ::sus::fn::FnMut<R(Item&&)>> &&
             !std::is_void_v<R>)
  Iterator<Map<Item, R, ::sus::mem::size_of<I>(), alignof(I)>> map(
      MapFn fn) && noexcept
    requires(::sus::mem::relocate_by_memcpy<I>)
  {
    // TODO: Move out of line.
    return {sus::into(::sus::move(fn)), make_sized_iterator(::sus::move(*this))};
  }

  /// Creates an iterator which uses a closure to determine if an element should
  /// be yielded.
  ///
  /// Given an element the closure must return true or false. The returned
  /// iterator will yield only the elements for which the closure returns true.
  Iterator<Filter<Item, ::sus::mem::size_of<I>(), alignof(I)>> filter(
      ::sus::fn::FnMut<bool(const std::remove_reference_t<Item>&)>
          pred) && noexcept
    requires(::sus::mem::relocate_by_memcpy<I>);

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
  template <::sus::iter::FromIterator<typename I::Item> C>
  C collect() && noexcept;

  /// Transforms an iterator into a Vec.
  ///
  /// This function is a shorthand for `it.collect<Vec<Item>>()` in order to
  /// avoid the need for specifying a template argument.
  ///
  /// See `collect()` for more details.
  template <int&..., class Vec = ::sus::containers::Vec<typename I::Item>>
    requires(  // Vec requires Move for its items.
        ::sus::mem::Move<typename I::Item>)
  // TODO: If the iterator is over references, collect_vec() should map them to
  // NonNull.
  Vec collect_vec() && noexcept;

  // TODO: cloned().
};

template <class I>
bool Iterator<I>::all(::sus::fn::FnMut<bool(typename I::Item)> f) noexcept {
  // TODO: If constexpr(I::all() exists) then call that instead.
  while (true) {
    Option<typename I::Item> item = this->next();
    if (item.is_none()) return true;
    // Safety: `item` was checked to hold Some already.
    if (!f(item.take().unwrap_unchecked(::sus::marker::unsafe_fn)))
      return false;
  }
}

template <class I>
bool Iterator<I>::any(::sus::fn::FnMut<bool(typename I::Item)> f) noexcept {
  // TODO: If constexpr(I::any() exists) then call that instead.
  while (true) {
    Option<typename I::Item> item = this->next();
    if (item.is_none()) return false;
    // Safety: `item` was checked to hold Some already.
    if (f(item.take().unwrap_unchecked(::sus::marker::unsafe_fn))) return true;
  }
}

template <class I>
::sus::num::usize Iterator<I>::count() noexcept {
  // TODO: If constexpr(I::count() exists) then call that instead.
  auto c = 0_usize;
  while (this->next().is_some()) c += 1_usize;
  return c;
}

template <class I>
Iterator<BoxedIterator<typename I::Item, ::sus::mem::size_of<I>(), alignof(I)>>
Iterator<I>::box() && noexcept
  requires(!::sus::mem::relocate_by_memcpy<I>)
{
  return make_boxed_iterator(::sus::move(*this));
}

template <class I>
Iterator<Filter<typename I::Item, ::sus::mem::size_of<I>(), alignof(I)>>
Iterator<I>::filter(
    ::sus::fn::FnMut<bool(const std::remove_reference_t<typename I::Item>&)>
        pred) && noexcept
  requires(::sus::mem::relocate_by_memcpy<I>)
{
  return {::sus::move(pred), make_sized_iterator(::sus::move(*this))};
}

template <class I>
template <::sus::iter::FromIterator<typename I::Item> C>
C Iterator<I>::collect() && noexcept {
  return C::from_iter(::sus::move(*this));
}

template <class I>
template <int&..., class Vec>
  requires(::sus::mem::Move<typename I::Item>)
Vec Iterator<I>::collect_vec() && noexcept {
  return Vec::from_iter(::sus::move(*this));
}

}  // namespace sus::iter
