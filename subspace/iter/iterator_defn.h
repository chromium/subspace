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

#include "fn/fn_defn.h"
#include "iter/from_iterator.h"
#include "macros/__private/compiler_bugs.h"
#include "num/unsigned_integer.h"
#include "option/option.h"

namespace sus::iter {

using ::sus::option::Option;

// TODO: Move forward decls somewhere?
template <class Item, size_t InnerIterSize, size_t InnerIterAlign,
          bool InnerIterInlineStorage>
class Filter;

namespace __private {
template <class IteratorBase>
class IteratorLoop;

template <class Iterator>
class IteratorImplicitLoop;
struct IteratorEnd;

template <class T>
constexpr auto begin(const T& t) noexcept;
template <class T>
constexpr auto end(const T& t) noexcept;
}  // namespace __private

// TODO: Do we want to be able to pass IteratorBase& as a "generic" iterator?
// Then it needs access to the adapator methods of Iterator<T>, so make them
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

  // Provided methods.

  /// Tests whether all elements of the iterator match a predicate.
  ///
  /// If the predicate returns `true` for all elements in the iterator, this
  /// functions returns `true`, otherwise `false`. The function is
  /// short-circuiting; it stops iterating on the first `false` returned from
  /// the predicate.
  ///
  /// Returns `true` if the iterator is empty.
  virtual bool all(::sus::fn::FnMut<bool(Item)> f) noexcept;

  /// Tests whether any elements of the iterator match a predicate.
  ///
  /// If the predicate returns `true` for any elements in the iterator, this
  /// functions returns `true`, otherwise `false`. The function is
  /// short-circuiting; it stops iterating on the first `true` returned from
  /// the predicate.
  ///
  /// Returns `false` if the iterator is empty.
  virtual bool any(::sus::fn::FnMut<bool(Item)> f) noexcept;

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
  virtual ::sus::usize count() noexcept;

  /// Adaptor for use in ranged for loops.
  __private::IteratorLoop<IteratorBase<Item>&> begin() & noexcept;
  /// Adaptor for use in ranged for loops.
  __private::IteratorEnd end() & noexcept;

 protected:
  IteratorBase() = default;
};

template <class I>
class Iterator final : public I {
 private:
  using sus_clang_bug_58837(Item =) typename I::Item;
  template <class T>
  friend class __private::IteratorLoop;  // Can see Item.
  friend I;                              // I::foo() can construct Iterator<I>.

  template <class J>
  friend class Iterator;  // Iterator<J>::foo() can construct Iterator<I>.

  // Option can't include Iterator, due to a circular dependency between
  // Option->Iterator->Option. So it forward declares Iterator, and needs
  // to use the constructor directly.
  template <class T>
  friend class sus_clang_bug_58836(
      ::sus::option::) Option;  // Option<T>::foo() can construct Iterator<I>.

  template <class... Args>
  Iterator(Args&&... args) : I(static_cast<Args&&>(args)...) {
    // We want to be able to use Iterator<I> and I interchangably, so that if an
    // `I` gets stored in SizedIterator, it doesn't misbehave.
    static_assert(sizeof(I) == sizeof(Iterator<I>), "");
  }

 public:
  // Adaptor methods.

  // TODO: map()

  /// Creates an iterator which uses a closure to determine if an element should
  /// be yielded.
  ///
  /// Given an element the closure must return true or false. The returned
  /// iterator will yield only the elements for which the closure returns true.
  Iterator<Filter<Item, sizeof(I), alignof(I),
                  ::sus::mem::relocate_one_by_memcpy<I>>>
  filter(::sus::fn::FnMut<bool(const std::remove_reference_t<Item>&)>
             pred) && noexcept;

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
  /// you will always need to pass it a type argument, such as: ```cpp
  /// sus::move(iter).collect<MyContainer<i32>>() ```
  template <::sus::iter::FromIterator<typename I::Item> C>
  C collect() && noexcept;

  // TODO: cloned().
};

}  // namespace sus::iter
