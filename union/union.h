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

#include <stdint.h>

#include <type_traits>

#include "macros/no_unique_address.h"
#include "mem/clone.h"
#include "mem/copy.h"
#include "mem/move.h"
#include "mem/never_value.h"
#include "mem/replace.h"
#include "num/__private/intrinsics.h"
#include "ops/eq.h"
#include "ops/ord.h"
#include "tuple/tuple.h"
#include "union/__private/all_values_are_unique.h"
#include "union/__private/index_of_value.h"
#include "union/__private/index_type.h"
#include "union/__private/ops_concepts.h"
#include "union/__private/pack_index.h"
#include "union/__private/storage.h"
#include "union/__private/type_list.h"
#include "union/value_types.h"

namespace sus::union_type {

template <class TypeListOfMemberTypes, auto... Tags>
class Union;

template <class... Ts, auto... Tags>
class Union<__private::TypeList<Ts...>, Tags...> {
  static_assert(sizeof...(Ts) > 0,
                "A Union must have at least one value-type pairs.");
  static_assert(
      sizeof...(Ts) == sizeof...(Tags),
      "The number of types and values in the Union don't match. Use "
      "`sus_value_types()` to define the Union's value-type pairings.");

  using Storage = __private::Storage<0, Ts...>;
  using TagsType = __private::PackFirst<decltype(Tags)...>;

  static_assert((... && std::same_as<TagsType, decltype(Tags)>),
                "All tag values must be the same type.");

  // As of GCC 13, it makes class-type template parameters be const
  // (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=108169) which means they will
  // not be Copy, as you can't copy into a const type. So we have to strip the
  // const.
  // clang-format off
  template <class T>
  using GccBug = sus_gcc_bug_108169(std::remove_const_t <) T sus_gcc_bug_108169(>);
  // clang-format on

  // This is required in order for the implementation of `which()` to not be a
  // performance cliff of potentially-expensive `clone()`s.
  static_assert(::sus::mem::Copy<GccBug<TagsType>>,
                "The tag values must be Copy.");
  static_assert(::sus::ops::Eq<TagsType>,
                "The tag values must be Eq in order to find the storage from a "
                "tag value.");
  static_assert(
      __private::AllValuesAreUnique<Tags...>,
      "All tag values must be unique or some of them would be inaccessible.");

  // We add 2 to `sizeof...(Tags)` for the range of the index.
  //
  // The ~0 value (all bits are 1) is reserved for using the index as a
  // never-value field, so the index's type must be large enough to hold all
  // indices without using that bit sequence.
  //
  // The (~0 - 1) index (all bits are 1 except the lowest order bit) is
  // reserved to mark the type as moved-from. Any use of the type afterward will
  // panic due to the index being outside the range of acceptable values.
  using IndexType =
      __private::IndexType<sizeof...(Tags) + 2u, ::sus::size_of<Storage>(),
                           ::sus::data_size_of<Storage>()>;
  static_assert(!std::is_void_v<IndexType>,
                "A union can only hold a number of members representable in "
                "size_t (minus two).");
  static constexpr IndexType kNeverValue =
      ::sus::num::__private::unchecked_not(IndexType{0u});
  static constexpr IndexType kUseAfterMove =
      ::sus::num::__private::unchecked_sub(
          ::sus::num::__private::unchecked_not(IndexType{0u}), IndexType{1u});

  template <TagsType V>
  static constexpr IndexType get_index_for_value() {
    using Index = __private::IndexOfValue<V, Tags...>;
    static_assert(!std::is_void_v<Index>,
                  "The value V is not part of the Union.");
    // SAFETY: We know `I` fits inside `IndexType` because `I` is the index of a
    // union member, and `IndexType` is chosen specifically to be large enough
    // to hold the index of all union members.
    return static_cast<IndexType>(Index::value);
  }

  template <TagsType V>
  static constexpr IndexType index = get_index_for_value<V>();

  template <TagsType V>
  using StorageTypeOfTag = __private::StorageTypeOfTag<size_t{index<V>}, Ts...>;

 public:
  // TODO: Can we construct Tuples of trivially constructible things (or some
  // set of things) without placement new and thus make this constexpr?
  template <TagsType V, class U, int&...,
            __private::ValueIsNotVoid Arg = StorageTypeOfTag<V>>
    requires(std::convertible_to<U &&, Arg>)
  static Union with(U&& values) {
    auto u = Union(index<V>);
    find_storage_mut<index<V>>(u.storage_).construct(::sus::forward<U>(values));
    return u;
  }

  template <TagsType V, int&...,
            __private::ValueIsVoid Arg = StorageTypeOfTag<V>>
  static Union with() {
    return Union(index<V>);
  }

  ~Union()
    requires((std::is_trivially_destructible_v<TagsType> && ... &&
              std::is_trivially_destructible_v<Ts>))
  = default;
  ~Union()
    requires(!(std::is_trivially_destructible_v<TagsType> && ... &&
               std::is_trivially_destructible_v<Ts>))
  {
    if (index_ != kUseAfterMove) storage_.destroy(size_t{index_});
  }

  Union(Union&& o)
    requires((... && ::sus::mem::Move<Ts>) &&
             (std::is_trivially_move_constructible_v<TagsType> && ... &&
              std::is_trivially_move_constructible_v<Ts>))
  = default;
  Union(Union&& o)
    requires(!(... && ::sus::mem::Move<Ts>))
  = delete;

  Union(Union&& o)
    requires((... && ::sus::mem::Move<Ts>) &&
             !(std::is_trivially_move_constructible_v<TagsType> && ... &&
               std::is_trivially_move_constructible_v<Ts>))
      : index_(::sus::mem::replace(::sus::mref(o.index_),
                                   // Attempt to catch use-after-move by setting
                                   // the tag to an unused value.
                                   kUseAfterMove)) {
    check(index_ != kUseAfterMove);
    storage_.move_construct(size_t{index_}, ::sus::move(o.storage_));
  }

  Union& operator=(Union&& o)
    requires((... && ::sus::mem::Move<Ts>) &&
             (std::is_trivially_move_assignable_v<TagsType> && ... &&
              std::is_trivially_move_assignable_v<Ts>))
  = default;
  Union& operator=(Union&& o)
    requires(!(... && ::sus::mem::Move<Ts>))
  = delete;

  Union& operator=(Union&& o)
    requires((... && ::sus::mem::Move<Ts>) &&
             !(std::is_trivially_move_assignable_v<TagsType> && ... &&
               std::is_trivially_move_assignable_v<Ts>))
  {
    check(o.index_ != kUseAfterMove);
    if (index_ == o.index_) {
      storage_.move_assign(size_t{index_}, ::sus::move(o.storage_));
    } else {
      if (index_ != kUseAfterMove) storage_.destroy(size_t{index_});
      index_ = ::sus::move(o.index_);
      storage_.move_construct(size_t{index_}, ::sus::move(o.storage_));
    }
    o.index_ = kUseAfterMove;
    return *this;
  }

  Union(const Union& o)
    requires((... && ::sus::mem::Copy<Ts>) &&
             (std::is_trivially_copy_constructible_v<TagsType> && ... &&
              std::is_trivially_copy_constructible_v<Ts>))
  = default;
  Union(const Union& o)
    requires(!(... && ::sus::mem::Copy<Ts>))
  = delete;

  Union(const Union& o)
    requires((... && ::sus::mem::Copy<Ts>) &&
             !(std::is_trivially_copy_constructible_v<TagsType> && ... &&
               std::is_trivially_copy_constructible_v<Ts>))
      : index_(o.index_) {
    check(o.index_ != kUseAfterMove);
    storage_.copy_construct(size_t{index_}, o.storage_);
  }

  Union& operator=(const Union& o)
    requires((... && ::sus::mem::Copy<Ts>) &&
             (std::is_trivially_copy_assignable_v<TagsType> && ... &&
              std::is_trivially_copy_assignable_v<Ts>))
  = default;
  Union& operator=(const Union& o)
    requires(!(... && ::sus::mem::Copy<Ts>))
  = delete;

  Union& operator=(const Union& o)
    requires((... && ::sus::mem::Copy<Ts>) &&
             !(std::is_trivially_copy_assignable_v<TagsType> && ... &&
               std::is_trivially_copy_assignable_v<Ts>))
  {
    check(o.index_ != kUseAfterMove);
    if (index_ == o.index_) {
      storage_.copy_assign(size_t{index_}, o.storage_);
    } else {
      if (index_ != kUseAfterMove) storage_.destroy(size_t{index_});
      index_ = o.index_;
      storage_.copy_construct(size_t{index_}, o.storage_);
    }
    return *this;
  }

  // sus::mem::Clone<Union<Ts...>> trait.
  constexpr Union clone() const& noexcept
    requires((... && ::sus::mem::Clone<Ts>) && !(... && ::sus::mem::Copy<Ts>))
  {
    check(index_ != kUseAfterMove);
    auto u = Union(::sus::clone(index_));
    u.storage_.clone_construct(size_t{index_}, storage_);
    return u;
  }

  /// Support for using Union in a `switch()` statment.
  inline operator TagsType() const& { return which(); }

  /// Returns which is the active member of the Union.
  ///
  /// Typically to access the data in the Union, a switch statement would be
  /// used, so as to call the getter or setter methods with the right value
  /// specified as a template parameter. When used in a switch statement, the
  /// which() method can be omitted.
  ///
  /// # Example
  /// ```
  /// switch (my_union) {
  ///   case Value1: return my_union.get_ref<Value1>().stuff;
  ///   case Value2: return my_union.get_ref<Value2>().andmore;
  ///   case Value3: return my_union.get_ref<Value3>().stufftoo;
  /// }
  /// ```
  ///
  /// # Inspiration
  ///
  ///                       ████████
  ///                   ████▓▓░░▓▓██
  ///                 ██▓▓▓▓▓▓▓▓██
  ///               ██▓▓▓▓░░▓▓██
  ///             ██░░▓▓▓▓▓▓██
  ///           ██▓▓▓▓▓▓▓▓▓▓██
  ///           ██▓▓▓▓░░▓▓▓▓██
  ///   ████████▓▓▓▓▓▓▓▓▓▓▓▓▓▓████████
  /// ██▓▓░░▓▓▓▓▓▓░░▓▓▓▓▓▓▓▓▓▓░░▓▓▓▓▓▓██
  /// ██████████████████▓▓██████████████
  ///         ██      ██      ██
  ///         ██  ██  ██      ██
  ///         ██  ██  ████      ██
  ///         ██        ██      ██
  ///         ██▒▒      ██      ██
  ///         ██▒▒        ██      ██
  ///           ████████████████████
  ///                   ██  ██
  ///                 ██  ██▓▓██
  ///                 ▓▓  ██▓▓▓▓██
  ///               ██  ████░░▓▓▓▓██            ▓▓▓▓▓▓▓▓▓▓▓▓
  ///             ██  ██  ██▓▓▓▓░░▓▓██        ▓▓░░░░░░░░░░░░
  ///             ██  ██    ████▓▓▓▓▓▓██      ▓▓▓▓▓▓▓▓▓▓▓▓▓▓
  /// ▓▓▓▓      ██  ██    ██▓▓▓▓▓▓░░▓▓██    ▓▓▓▓░░░░░░░░░░░░
  /// ▓▓▓▓▓▓▓▓▓▓██████▓▓▓▓██▓▓░░▓▓▓▓██▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
  ///                     ██████████        ▓▓▓▓░░░░░░░░░░░░
  ///                       ██  ██            ▓▓▓▓▓▓▓▓▓▓▓▓▓▓
  ///                         ██  ██          ▓▓░░░░░░░░░░░░
  ///                           ██  ██          ▓▓▓▓▓▓▓▓▓▓▒▒
  ///                             ████
  TagsType which() const& {
    check(index_ != kUseAfterMove);
    constexpr TagsType tags[] = {Tags...};
    return tags[size_t{index_}];
  }

  // clang-format off
  template <TagsType V>
    requires(__private::ValueIsNotVoid<StorageTypeOfTag<V>>)
  decltype(auto) get_ref() const& {
    ::sus::check(index_ == index<V>);
    return __private::find_storage<index<V>>(storage_).get_ref();
  }

  template <TagsType V>
    requires(__private::ValueIsNotVoid<StorageTypeOfTag<V>>)
  decltype(auto) get_mut() & {
    ::sus::check(index_ == index<V>);
    return __private::find_storage_mut<index<V>>(storage_).get_mut();
  }

  template <TagsType V>
    requires(__private::ValueIsNotVoid<StorageTypeOfTag<V>>)
  decltype(auto) into_inner() && {
    ::sus::check(index_ == index<V>);
    auto& s = __private::find_storage_mut<index<V>>(storage_);
    return ::sus::move(s).into_inner();
  }
  // clang-format on

  template <TagsType V, class U, int&..., class Arg = StorageTypeOfTag<V>>
    requires(std::convertible_to<U &&, Arg> &&
             __private::ValueIsNotVoid<StorageTypeOfTag<V>>)
  void set(U&& values) {
    if (index_ == index<V>) {
      __private::find_storage_mut<index<V>>(storage_).set(::sus::move(values));
    } else {
      if (index_ != kUseAfterMove) storage_.destroy(size_t{index_});
      index_ = index<V>;
      __private::find_storage_mut<index<V>>(storage_).construct(
          ::sus::move(values));
    }
  }

  template <TagsType V, int&...,
            __private::ValueIsVoid Arg = StorageTypeOfTag<V>>
  void set() {
    index_ = index<V>;
  }

  /// sus::ops::Eq<Union<Ts...>, Union<Us...>> trait.
  template <class... Us, auto V, auto... Vs>
    requires(__private::UnionIsEq<TagsType, __private::TypeList<Ts...>,
                                  decltype(V), __private::TypeList<Us...>>)
  friend inline constexpr bool operator==(
      const Union& l,
      const Union<__private::TypeList<Us...>, V, Vs...>& r) noexcept {
    check(l.index_ != kUseAfterMove && r.index_ != kUseAfterMove);
    return l.index_ == r.index_ && l.storage_.eq(size_t{l.index_}, r.storage_);
  }

  template <class... Us, auto V, auto... Vs>
    requires(!__private::UnionIsEq<TagsType, __private::TypeList<Ts...>,
                                   decltype(V), __private::TypeList<Us...>>)
  friend inline constexpr bool operator==(
      const Union& l,
      const Union<__private::TypeList<Us...>, V, Vs...>& r) noexcept = delete;

  /// sus::ops::Ord<Union<Ts...>, Union<Us...>> trait.
  template <class... Us, auto V, auto... Vs>
    requires(__private::UnionIsOrd<TagsType, __private::TypeList<Ts...>,
                                   decltype(V), __private::TypeList<Us...>>)
  friend inline constexpr auto operator<=>(
      const Union& l,
      const Union<__private::TypeList<Us...>, V, Vs...>& r) noexcept {
    check(l.index_ != kUseAfterMove && r.index_ != kUseAfterMove);
    const auto value_order = std::strong_order(l.which(), r.which());
    if (value_order != std::strong_ordering::equivalent) {
      return value_order;
    } else {
      return l.storage_.ord(size_t{l.index_}, r.storage_);
    }
  }

  /// sus::ops::WeakOrd<Union<Ts...>, Union<Us...>> trait.
  template <class... Us, auto V, auto... Vs>
    requires(__private::UnionIsWeakOrd<TagsType, __private::TypeList<Ts...>,
                                       decltype(V), __private::TypeList<Us...>>)
  friend inline constexpr auto operator<=>(
      const Union& l,
      const Union<__private::TypeList<Us...>, V, Vs...>& r) noexcept {
    check(l.index_ != kUseAfterMove && r.index_ != kUseAfterMove);
    const auto value_order = std::weak_order(l.which(), r.which());
    if (value_order != std::weak_ordering::equivalent) {
      return value_order;
    } else {
      return l.storage_.weak_ord(size_t{l.index_}, r.storage_);
    }
  }

  /// sus::ops::PartialOrd<Union<Ts...>, Union<Us...>> trait.
  template <class... Us, auto V, auto... Vs>
    requires(
        __private::UnionIsPartialOrd<TagsType, __private::TypeList<Ts...>,
                                     decltype(V), __private::TypeList<Us...>>)
  friend inline constexpr auto operator<=>(
      const Union& l,
      const Union<__private::TypeList<Us...>, V, Vs...>& r) noexcept {
    check(l.index_ != kUseAfterMove && r.index_ != kUseAfterMove);
    const auto value_order = std::partial_order(l.which(), r.which());
    if (value_order != std::partial_ordering::equivalent) {
      return value_order;
    } else {
      return l.storage_.partial_ord(size_t{l.index_}, r.storage_);
    }
  }

  template <class... RhsTs, auto RhsTag, auto... RhsTags>
    requires(!__private::UnionIsAnyOrd<TagsType, __private::TypeList<Ts...>,
                                       decltype(RhsTag),
                                       __private::TypeList<RhsTs...>>)
  friend inline constexpr auto operator<=>(
      const Union<__private::TypeList<Ts...>, Tags...>& l,
      const Union<__private::TypeList<RhsTs...>, RhsTag, RhsTags...>& r) =
      delete;

 private:
  Union(IndexType i) : index_(i) {}

  [[sus_no_unique_address]] Storage storage_;
  IndexType index_;

  sus_class_never_value_field(unsafe_fn, Union, index_, kNeverValue);
};

}  // namespace sus::union_type

// Promote Union into the `sus` namespace.
namespace sus {
using ::sus::union_type::Union;
}
