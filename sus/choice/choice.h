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

#include <concepts>
#include <type_traits>

#include "fmt/core.h"
#include "sus/assertions/debug_check.h"
#include "sus/choice/__private/all_values_are_unique.h"
#include "sus/choice/__private/index_of_value.h"
#include "sus/choice/__private/index_type.h"
#include "sus/choice/__private/ops_concepts.h"
#include "sus/choice/__private/pack_index.h"
#include "sus/choice/__private/storage.h"
#include "sus/choice/__private/type_list.h"
#include "sus/choice/choice_types.h"
#include "sus/cmp/eq.h"
#include "sus/cmp/ord.h"
#include "sus/lib/__private/forward_decl.h"
#include "sus/macros/lifetimebound.h"
#include "sus/macros/no_unique_address.h"
#include "sus/marker/unsafe.h"
#include "sus/mem/clone.h"
#include "sus/mem/copy.h"
#include "sus/mem/move.h"
#include "sus/mem/never_value.h"
#include "sus/mem/replace.h"
#include "sus/num/__private/intrinsics.h"
#include "sus/option/option.h"
#include "sus/string/__private/any_formatter.h"
#include "sus/string/__private/format_to_stream.h"
#include "sus/tuple/tuple.h"

namespace sus {
/// The [`Choice`]($sus::choice_type::Choice) type.
namespace choice_type {}
}  // namespace sus

namespace sus::choice_type {

/// A helper concept that reports if the value in a `Choice` for the given `Tag`
/// is null. When true, the accessor and setter methods are not available.
template <class Choice, auto Tag>
concept ChoiceValueIsVoid = !requires(const Choice& c) {
  { c.template get<Tag>() };
};

template <size_t I, class Storage, class... Us>
struct CanConvertToStorage;

template <size_t I, class Storage, class U>
struct CanConvertToStorage<I, Storage, U> {
  static_assert(I == 0u);

  static constexpr bool value = std::convertible_to<U, Storage>;
};

template <size_t I, class... Ts, class U, class... Us>
  requires(sizeof...(Us) > 0u)
struct CanConvertToStorage<I, ::sus::tuple_type::Tuple<Ts...>, U, Us...> {
  static_assert(I < sizeof...(Ts) - 1u);

  using TargetType = std::tuple_element<I, ::sus::tuple_type::Tuple<Ts...>>;
  static constexpr bool value =
      std::convertible_to<U, TargetType> &&
      CanConvertToStorage<I + 1u, ::sus::tuple_type::Tuple<Ts...>,
                          Us...>::value;
};

template <size_t I, class... Ts, class U>
struct CanConvertToStorage<I, ::sus::tuple_type::Tuple<Ts...>, U> {
  static_assert(I == sizeof...(Ts) - 1u);

  using TargetType = std::tuple_element<I, ::sus::tuple_type::Tuple<Ts...>>;
  static constexpr bool value = std::convertible_to<U, TargetType>;
};

/// A tagged union, or sum type.
///
/// A `Choice` is always set to one of its `Tags` values, and each tag has zero
/// or more types attached to it as data values.
///
/// `Choice` can be thought of as a combination of an [`enum`](
/// https://en.cppreference.com/w/cpp/language/enum) and a
/// [`std::variant`](https://en.cppreference.com/w/cpp/utility/variant), and
/// typically the `Tags` are specified to be values in an `enum`.
///
/// A `Choice` always has an active member, as the tag must be specified at
/// construction, and the asociated values for the tag are always set as they
/// must be set when the tag is specified. This means a `Choice` is always in a
/// fully specified state, or it is moved-from. Once it is moved from it may not
/// be used except to be re-initialized.
///
/// Use the `sus_choice_types()` macro to specify the types in a `Choice` type.
///
/// To access the values in `Choice`, the current tag must be specified as a
/// template parameter, and it will be checked for correctness. When it does not
/// match, the `Choice` method will panic.
/// * [`as<Tag>()`]($sus::choice_type::Choice::as) gives const access to all the
///   values attached to the tag.
/// * [`as_mut<Tag>()`]($sus::choice_type::Choice::as_mut) gives mutable access
///   to all the values attached to the tag. It is only callable on mutable
///   `Choice`.
/// * [`into_inner<Tag>()`]($sus::choice_type::Choice::into_inner) moves all
///   values attached to the tag out of the `Choice` and marks the `Choice` as
///   moved-from. It is only callable on an rvalue `Choice.`
/// * [`get<Tag>()`]($sus::choice_type::Choice::get) returns a const reference
///   to the values attached to the tag if its currently active, and returns
///   `None` if the tag is not active.
/// * [`get_mut<Tag>()`]($sus::choice_type::Choice::get) returns a mutable
///   reference to the values attached to the tag if its currently active, and
///   returns `None` if the tag is not active.
///
/// # Examples
/// This `Choice` holds either a [`u64`]($sus::num::u64) with
/// the `First` tag or a [`u32`]($sus::num::u32) with the `Second` tag.
/// ```
/// enum class Order { First, Second };
/// using EitherOr = Choice<sus_choice_types(
///     (Order::First, u64),
///     (Order::Second, u32)
/// )>;
/// ```
/// A `Choice` tag may be associated with no values by making its type `void` or
/// may be associated with more than one type in which case all access will be
/// done with a [`Tuple`]($sus::tuple_type::Tuple).
/// ```
/// enum class Order { First, Second };
/// using EitherOr = Choice<sus_choice_types(
///     (Order::First, void),
///     (Order::Second, std::string, i32)
/// )>;
/// auto e1 = EitherOr::with<Order::First>();
/// auto e2 = EitherOr::with<Order::Second>("text", 123);
/// ```
/// The `Choice` type can be used in a `switch`, with each case being one of its
/// possible tag values. Within each tag case block, the values can be pulled
/// out of the `Choice` with [`as`]($sus::choice_type::Choice::as) in a
/// type-safe and memory-safe way.
/// ```
/// enum class Order { First, Second };
/// using EitherOr = Choice<sus_choice_types(
///     (Order::First, u64),
///     (Order::Second, std::string, i32)
/// )>;
/// auto e = EitherOr::with<Order::Second>("hello worl", 0xd);
/// switch (e) {
///   case Order::First: {
///     const auto& i = e.as<Order::First>();
///     // We don't get here.
///     fmt::println("First has u64 {}", i);
///     break;
///   }
///   case Order::Second: {
///     const auto& [s, i] = e.as<Order::Second>();
///     // Prints "Second has hello world".
///     fmt::println("Second has {}{:x}", s, i);
///     break;
///   }
/// }
/// ```
/// `Choice` will re-export the tag value type as a nested `Tag` subtype. This
/// allows access to the Choice's values though `MyChoiceType::Tag::Name`.
/// ```
/// enum class Order { First, Second };
/// using EitherOr = Choice<sus_choice_types(
///     (Order::First, u64),
///     (Order::Second, u32)
/// )>;
/// auto x = EitherOr::with<EitherOr::Tag::First>(987u);
/// ```
template <class... Ts, auto... Tags>
class Choice<__private::TypeList<Ts...>, Tags...> final {
  static_assert(sizeof...(Ts) > 0,
                "A Choice must have at least one value-type pairs.");
  static_assert(
      sizeof...(Ts) == sizeof...(Tags),
      "The number of types and values in the Choice don't match. Use "
      "`sus_choice_types()` to define the Choice's value-type pairings.");

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
  static_assert(::sus::cmp::Eq<TagsType>,
                "The tag values must be Eq in order to find the storage from a "
                "tag value.");
  static_assert(
      __private::AllValuesAreUnique<Tags...>,
      "All tag values must be unique or some of them would be inaccessible.");

  // The data size of a union is not known from sus::data_size_of(), but is the
  // max of all its members. We know all the member types so we can compute it
  // ourselves.
  static constexpr size_t union_data_size = []() -> size_t {
    auto find_max = [max = size_t{1u}](size_t a) mutable {
      if (a == 0 || max == 0)
        max = 0;
      else if (a > max)
        max = a;
      return max;
    };
    auto max = (find_max(::sus::data_size_of<Ts>()), ..., find_max(1));
    // If the result was 0 then a member of the union does not have a defined
    // data size.
    return max > 0u ? max : ::sus::size_of<Storage>();
  }();
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
                           union_data_size>;
  static_assert(!std::is_void_v<IndexType>,
                "A union can only hold a number of members representable in "
                "size_t (minus two).");
  static constexpr IndexType kNeverValue =
      ::sus::num::__private::unchecked_not(IndexType{0u});
  static constexpr IndexType kUseAfterMove =
      ::sus::num::__private::unchecked_sub(
          ::sus::num::__private::unchecked_not(IndexType{0u}), IndexType{1u});

  template <TagsType V>
  static constexpr auto index =
      // SAFETY: We know the index fits inside `IndexType` because it is the
      // index of a union member, and `IndexType` is chosen specifically to be
      // large enough to hold the index of all union members.
      static_cast<IndexType>(__private::get_index_for_value<V, Tags...>());

  template <TagsType V>
  using StorageTypeOfTag = __private::StorageTypeOfTag<index<V>, Ts...>;

  template <TagsType V>
  using AccessTypeOfTagConst =
      decltype(__private::find_choice_storage<index<V>>(
                   std::declval<const Storage&>())
                   .as());
  template <TagsType V>
  using AccessTypeOfTagMut =
      decltype(__private::find_choice_storage_mut<index<V>>(
                   std::declval<Storage&>())
                   .as_mut());

 public:
  using Tag = TagsType;

  /// The type associated with a tag. If multiple types are associated, the
  /// resulting type here is a Tuple of those types.
  ///
  /// The `Tag` must be valid or it will fail to compile.
  template <TagsType Tag>
  using TypeForTag = __private::PublicTypeForStorageType<StorageTypeOfTag<Tag>>;

  /// Constructs a `Choice` with the `V` tag indicating its active member, and
  /// with the parameters used to set the associated values.
  ///
  /// If the associated type of the tag is `void` then `with()` takes no
  /// parameters.
  template <TagsType V>
    requires(__private::StorageCount<StorageTypeOfTag<V>> == 0u)
  constexpr static Choice with() noexcept {
    return Choice(index<V>);
  }
  template <TagsType V, class U>
    requires(__private::StorageCount<StorageTypeOfTag<V>> == 1u &&  //
             ::sus::construct::SafelyConstructibleFromReference<
                 StorageTypeOfTag<V>, U &&> &&  //
             std::convertible_to<U &&, StorageTypeOfTag<V>>)
  constexpr static Choice with(U&& value) noexcept {
    using StorageType = StorageTypeOfTag<V>;
    // TODO: Convert the requires check to a static_assert when we can test that
    // with a nocompile test.
    static_assert(
        ::sus::construct::SafelyConstructibleFromReference<StorageType, U&&>,
        "Unable to safely convert to a different reference type, as conversion "
        "would produce a reference to a temporary. The Choice's parameter "
        "type must match the Choice's stored reference. For example a Choice "
        "holding `const i32&, u32` can not be constructed from "
        "`const i16&, u32` parameters but it can be constructed from "
        " `i32, u16`.");
    auto u = Choice(index<V>);
    u.storage_.activate_for_construct(index<V>);
    find_choice_storage_mut<index<V>>(u.storage_)
        .construct(StorageType(::sus::forward<U>(value)));
    return u;
  }
  template <TagsType V, class... Us>
    requires(__private::StorageCount<StorageTypeOfTag<V>> > 1u &&  //
             sizeof...(Us) ==
                 __private::StorageCount<StorageTypeOfTag<V>> &&  //
             __private::StorageIsSafelyConstructibleFromReference<
                 StorageTypeOfTag<V>, Us && ...> &&  //
             std::constructible_from<StorageTypeOfTag<V>, Us && ...>)
  constexpr static Choice with(Us&&... values) noexcept {
    using StorageType = StorageTypeOfTag<V>;
    // TODO: Convert the requires check to a static_assert when we can test that
    // with a nocompile test.
    static_assert(
        __private::StorageIsSafelyConstructibleFromReference<StorageType,
                                                             Us&&...>,
        "Unable to safely convert to a different reference type, as conversion "
        "would produce a reference to a temporary. The Choice's parameter "
        "type must match the Choice's stored reference. For example a Choice "
        "holding `const i32&, u32` can not be constructed from "
        "`const i16&, u32` parameters but it can be constructed from "
        " `i32, u16`.");
    auto u = Choice(index<V>);
    u.storage_.activate_for_construct(index<V>);
    find_choice_storage_mut<index<V>>(u.storage_)
        .construct(StorageType(::sus::forward<Us>(values)...));
    return u;
  }

  constexpr ~Choice()
    requires((std::is_trivially_destructible_v<TagsType> && ... &&
              std::is_trivially_destructible_v<Ts>))
  = default;
  constexpr ~Choice() noexcept
    requires(!(std::is_trivially_destructible_v<TagsType> && ... &&
               std::is_trivially_destructible_v<Ts>))
  {
    if (index_ != kUseAfterMove && index_ != kNeverValue)
      storage_.destroy(index_);
  }

  /// Move constructor.
  ///
  /// `Choice` satisfies [`Move`]($sus::mem::Move) when the types it holds all
  /// satisfy [`Move`]($sus::mem::Move).
  /// #[doc.overloads=move]
  constexpr Choice(Choice&& o) noexcept
    requires((... && ::sus::mem::Move<Ts>) &&
             (std::is_trivially_move_constructible_v<TagsType> && ... &&
              std::is_trivially_move_constructible_v<Ts>))
  = default;
  /// #[doc.overloads=move]
  constexpr Choice(Choice&& o) noexcept
    requires((... && ::sus::mem::Move<Ts>) &&
             !(std::is_trivially_move_constructible_v<TagsType> && ... &&
               std::is_trivially_move_constructible_v<Ts>))
      : index_(::sus::mem::replace(o.index_,
                                   // Attempt to catch use-after-move by setting
                                   // the tag to an unused value.
                                   kUseAfterMove)) {
    sus_check(index_ != kUseAfterMove);
    storage_.move_construct(index_, ::sus::move(o.storage_));
  }
  /// #[doc.overloads=move]
  Choice(Choice&& o)
    requires(!(... && ::sus::mem::Move<Ts>))
  = delete;

  /// Move assignment.
  ///
  /// `Choice` satisfies [`Move`]($sus::mem::Move) when the types it holds all
  /// satisfy [`Move`]($sus::mem::Move).
  /// #[doc.overloads=move]
  Choice& operator=(Choice&& o) noexcept
    requires((... && ::sus::mem::Move<Ts>) &&
             (std::is_trivially_move_assignable_v<TagsType> && ... &&
              std::is_trivially_move_assignable_v<Ts>))
  = default;
  /// #[doc.overloads=move]
  constexpr Choice& operator=(Choice&& o) noexcept
    requires((... && ::sus::mem::Move<Ts>) &&
             !(std::is_trivially_move_assignable_v<TagsType> && ... &&
               std::is_trivially_move_assignable_v<Ts>))
  {
    sus_check(o.index_ != kUseAfterMove);
    if (index_ == o.index_) {
      storage_.move_assign(index_, ::sus::move(o.storage_));
    } else {
      if (index_ != kUseAfterMove) storage_.destroy(index_);
      index_ = ::sus::move(o.index_);
      storage_.move_construct(index_, ::sus::move(o.storage_));
    }
    o.index_ = kUseAfterMove;
    return *this;
  }
  /// #[doc.overloads=move]
  Choice& operator=(Choice&& o)
    requires(!(... && ::sus::mem::Move<Ts>))
  = delete;

  /// Copy constructor.
  ///
  /// `Choice` satisfies [`Copy`]($sus::mem::Copy) when the types it holds all
  /// satisfy [`Copy`]($sus::mem::Copy).
  /// #[doc.overloads=copy]
  constexpr Choice(const Choice& o)
    requires((... && ::sus::mem::Copy<Ts>) &&
             (std::is_trivially_copy_constructible_v<TagsType> && ... &&
              std::is_trivially_copy_constructible_v<Ts>))
  = default;
  /// #[doc.overloads=copy]
  constexpr Choice(const Choice& o) noexcept
    requires((... && ::sus::mem::Copy<Ts>) &&
             !(std::is_trivially_copy_constructible_v<TagsType> && ... &&
               std::is_trivially_copy_constructible_v<Ts>))
      : index_(o.index_) {
    sus_check(o.index_ != kUseAfterMove);
    storage_.copy_construct(index_, o.storage_);
  }
  /// #[doc.overloads=copy]
  Choice(const Choice& o)
    requires(!(... && ::sus::mem::Copy<Ts>))
  = delete;

  /// Copy assignment.
  ///
  /// `Choice` satisfies [`Copy`]($sus::mem::Copy) when the types it holds all
  /// satisfy [`Copy`]($sus::mem::Copy).
  /// #[doc.overloads=copy]
  constexpr Choice& operator=(const Choice& o)
    requires((... && ::sus::mem::Copy<Ts>) &&
             (std::is_trivially_copy_assignable_v<TagsType> && ... &&
              std::is_trivially_copy_assignable_v<Ts>))
  = default;
  /// #[doc.overloads=copy]
  constexpr Choice& operator=(const Choice& o) noexcept
    requires((... && ::sus::mem::Copy<Ts>) &&
             !(std::is_trivially_copy_assignable_v<TagsType> && ... &&
               std::is_trivially_copy_assignable_v<Ts>))
  {
    sus_check(o.index_ != kUseAfterMove);
    if (index_ == o.index_) {
      storage_.copy_assign(index_, o.storage_);
    } else {
      if (index_ != kUseAfterMove) storage_.destroy(index_);
      index_ = o.index_;
      storage_.copy_construct(index_, o.storage_);
    }
    return *this;
  }
  /// #[doc.overloads=copy]
  Choice& operator=(const Choice& o)
    requires(!(... && ::sus::mem::Copy<Ts>))
  = delete;

  /// [`Clone`]($sus::mem::Clone) support.
  /// `Choice` satisfies [`Clone`]($sus::mem::Clone) when the types it holds all
  /// satisfy [`Clone`]($sus::mem::Clone).
  constexpr Choice clone() const& noexcept
    requires((... && ::sus::mem::Clone<Ts>) && !(... && ::sus::mem::Copy<Ts>))
  {
    sus_check(index_ != kUseAfterMove);
    auto u = Choice(::sus::clone(index_));
    u.storage_.clone_construct(index_, storage_);
    return u;
  }

  /// Support for using Choice in a `switch()` statment.
  ///
  /// See the type's top level documentation for an example.
  constexpr inline operator TagsType() const& noexcept { return which(); }

  /// Returns which is the active member of the `Choice`.
  ///
  /// Typically to access the data in the `Choice`, a `switch` statement would
  /// be used, so as to call the getter or setter methods with the right value
  /// specified as a template parameter. When used in a `switch` statement, the
  /// `which` method can be omitted.
  ///
  /// # Example
  /// ```
  /// switch (my_choice) {
  ///   case Value1: return my_choice.as<Value1>().stuff;
  ///   case Value2: return my_choice.as<Value2>().andmore;
  ///   case Value3: return my_choice.as<Value3>().stufftoo;
  /// }
  /// ```
  ///
  /// # Inspiration
  /// ```txt
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
  /// ```
  constexpr inline TagsType which() const& noexcept {
    sus_check(index_ != kUseAfterMove);
    constexpr TagsType tags[] = {Tags...};
    return tags[size_t{index_}];
  }

  /// Returns a const reference to the value(s) inside the `Choice`.
  ///
  /// The function has a template parameter specifying the tag of the active
  /// member in the `Choice`.
  ///
  /// If the active member's associated type is `void` then this method is
  /// deleted and can't be called.
  /// If the active member has a single value, a reference to it is returned
  /// directly, otherwise a Tuple of references is returned to all values in the
  /// active member.
  ///
  /// # Panics
  /// The function will panic if the active member does not match the tag value
  /// passed as the template parameter.
  template <TagsType V>
    requires(__private::StorageCount<StorageTypeOfTag<V>> > 0u)
  constexpr inline decltype(auto) as() const& noexcept {
    sus_check(index_ == index<V>);
    return __private::find_choice_storage<index<V>>(storage_).as();
  }
  // If the storage is a value type, it can't be accessed by reference in an
  // rvalue (temporary) Choice object.
  template <TagsType V>
    requires(!std::is_reference_v<StorageTypeOfTag<V>>)
  constexpr inline const StorageTypeOfTag<V>& as() && noexcept = delete;

  /// Returns a mutable reference to the value(s) inside the `Choice`.
  ///
  /// The function has a template parameter specifying the tag of the active
  /// member in the `Choice`.
  ///
  /// If the active member's associated type is `void` then this method is
  /// deleted and can't be called.
  /// If the active member has a single value, a reference to it is returned
  /// directly, otherwise a Tuple of references is returned to all values in the
  /// active member.
  ///
  /// # Panics
  /// The function will panic if the active member does not match the tag value
  /// passed as the template parameter.
  template <TagsType V>
    requires(__private::StorageCount<StorageTypeOfTag<V>> > 0u)
  constexpr inline decltype(auto) as_mut() & noexcept {
    sus_check(index_ == index<V>);
    return __private::find_choice_storage_mut<index<V>>(storage_).as_mut();
  }

  /// Unwraps the `Choice` to move out the current value(s) inside the `Choice`.
  ///
  /// The function has a template parameter specifying the tag of the active
  /// member in the `Choice`.
  ///
  /// If the active member's associated type is `void` then this method is
  /// deleted and can't be called.
  /// If the active member has a single value, an rvalue reference to it is
  /// returned directly, otherwise a [`Tuple`]($sus::tuple_type::Tuple) of
  /// rvalue references is returned to all values in the active member.
  ///
  /// # Panics
  /// The function will panic if the active member does not match the tag value
  /// passed as the template parameter.
  template <TagsType V>
    requires(__private::StorageCount<StorageTypeOfTag<V>> > 0u)
  constexpr inline decltype(auto) into_inner() && noexcept {
    sus_check(index_ == index<V>);
    auto& s = __private::find_choice_storage_mut<index<V>>(storage_);
    return ::sus::move(s).into_inner();
  }

  /// Returns a const reference to the value(s) inside the `Choice`.
  ///
  /// If the template parameter does not match the active member in the
  /// `Choice`, the function returns `None`.
  ///
  /// If the active member's associated type is `void` then this method is
  /// deleted and can't be called.
  /// If the active member has a single value, a reference to it is returned
  /// directly, otherwise a [`Tuple`]($sus::tuple_type::Tuple) of references is
  /// returned to all values in the active member.
  template <TagsType V>
    requires(__private::StorageCount<StorageTypeOfTag<V>> > 0u)
  constexpr inline Option<AccessTypeOfTagConst<V>> get() const& noexcept {
    if (index_ != index<V>) return ::sus::none();
    return ::sus::some(__private::find_choice_storage<index<V>>(storage_).as());
  }
  // If the storage is a value type, it can't be accessed by reference in an
  // rvalue (temporary) Choice object.
  template <TagsType V>
    requires(!std::is_reference_v<StorageTypeOfTag<V>>)
  constexpr inline Option<AccessTypeOfTagConst<V>> get() && noexcept = delete;

  /// Returns a mutable reference to the value(s) inside the `Choice`.
  ///
  /// If the template parameter does not match the active member in the
  /// `Choice`, the function returns `None`.
  ///
  /// If the active member's associated type is `void` then this method is
  /// deleted and can't be called.
  /// If the active member has a single value, a reference to it is returned
  /// directly, otherwise a [`Tuple`]($sus::tuple_type::Tuple) of references is
  /// returned to all values in the active member.
  template <TagsType V>
    requires(__private::StorageCount<StorageTypeOfTag<V>> > 0u)
  constexpr inline Option<AccessTypeOfTagMut<V>> get_mut() & noexcept {
    if (index_ != index<V>) return ::sus::none();
    return ::sus::some(
        __private::find_choice_storage_mut<index<V>>(storage_).as_mut());
  }

  /// Changes the `Choice` to make the tag `V` active, and sets the associated
  /// values of `V` from the paramters.
  ///
  /// If the associated type of the tag is `void` then `set()` takes no
  /// parameters.
  template <TagsType V>
    requires(__private::StorageCount<StorageTypeOfTag<V>> == 0u)
  constexpr void set() & noexcept {
    if (index_ != index<V>) {
      if (index_ != kUseAfterMove) storage_.destroy(index_);
      index_ = index<V>;
    }
  }
  template <TagsType V, class U>
    requires(__private::StorageCount<StorageTypeOfTag<V>> == 1u &&  //
             ::sus::construct::SafelyConstructibleFromReference<
                 StorageTypeOfTag<V>, U &&> &&  //
             std::convertible_to<U &&, StorageTypeOfTag<V>>)
  constexpr void set(U&& value) & noexcept {
    using StorageType = StorageTypeOfTag<V>;
    // TODO: Convert the requires check to a static_assert when we can test that
    // with a nocompile test.
    static_assert(
        ::sus::construct::SafelyConstructibleFromReference<StorageType, U&&>,
        "Unable to safely convert to a different reference type, as conversion "
        "would produce a reference to a temporary. The Choice's parameter "
        "type must match the Choice's stored reference. For example a Choice "
        "holding `const i32&, u32` can not be constructed from "
        "`const i16&, u32` parameters but it can be constructed from "
        " `i32, u16`.");
    if (index_ == index<V>) {
      __private::find_choice_storage_mut<index<V>>(storage_).assign(
          ::sus::forward<U>(value));
    } else {
      if (index_ != kUseAfterMove) storage_.destroy(index_);
      index_ = index<V>;
      storage_.activate_for_construct(index<V>);
      __private::find_choice_storage_mut<index<V>>(storage_).construct(
          ::sus::forward<U>(value));
    }
  }
  template <TagsType V, class... Us>
    requires(__private::StorageCount<StorageTypeOfTag<V>> > 1u &&  //
             sizeof...(Us) ==
                 __private::StorageCount<StorageTypeOfTag<V>> &&  //
             __private::StorageIsSafelyConstructibleFromReference<
                 StorageTypeOfTag<V>, Us && ...> &&  //
             std::constructible_from<StorageTypeOfTag<V>, Us && ...>)
  constexpr void set(Us&&... values) & noexcept {
    using StorageType = StorageTypeOfTag<V>;
    // TODO: Convert the requires check to a static_assert when we can test that
    // with a nocompile test.
    static_assert(
        __private::StorageIsSafelyConstructibleFromReference<StorageType,
                                                             Us&&...>,
        "Unable to safely convert to a different reference type, as conversion "
        "would produce a reference to a temporary. The Choice's parameter "
        "type must match the Choice's stored reference. For example a Choice "
        "holding `const i32&, u32` can not be constructed from "
        "`const i16&, u32` parameters but it can be constructed from "
        " `i32, u16`.");
    if (index_ == index<V>) {
      __private::find_choice_storage_mut<index<V>>(storage_).assign(
          StorageType(::sus::forward<Us>(values)...));
    } else {
      if (index_ != kUseAfterMove) storage_.destroy(index_);
      index_ = index<V>;
      storage_.activate_for_construct(index<V>);
      __private::find_choice_storage_mut<index<V>>(storage_).construct(
          StorageType(::sus::forward<Us>(values)...));
    }
  }

  /// Returns a const reference to the value(s) inside the `Choice`.
  ///
  /// The function has a template parameter specifying the tag of the active
  /// member in the `Choice`.
  ///
  /// If the active member's associated type is `void` then this method is
  /// deleted and can't be called.
  /// If the active member has a single value, a reference to it is returned
  /// directly, otherwise a [`Tuple`]($sus::tuple_type::Tuple) of references is
  /// returned to all values in the active member.
  ///
  /// # Safety
  /// If the active member does not match the tag value passed as the template
  /// parameter, Undefined Behaviour results.
  template <TagsType V>
    requires(__private::StorageCount<StorageTypeOfTag<V>> > 0u)
  constexpr inline decltype(auto) get_unchecked(
      ::sus::marker::UnsafeFnMarker) const& noexcept {
    return __private::find_choice_storage<index<V>>(storage_).as();
  }
  // If the storage is a value type, it can't be accessed by reference in an
  // rvalue (temporary) Choice object.
  template <TagsType V>
    requires(!std::is_reference_v<StorageTypeOfTag<V>>)
  constexpr inline const StorageTypeOfTag<V>& get_unchecked(
      ::sus::marker::UnsafeFnMarker) && noexcept = delete;

  /// Returns a mutable reference to the value(s) inside the `Choice`.
  ///
  /// The function has a template parameter specifying the tag of the active
  /// member in the `Choice`.
  ///
  /// If the active member's associated type is `void` then this method is
  /// deleted and can't be called.
  /// If the active member has a single value, a reference to it is returned
  /// directly, otherwise a [`Tuple`]($sus::tuple_type::Tuple) of references is
  /// returned to all values in the active member.
  ///
  /// # Safety
  /// If the active member does not match the tag value passed as the template
  /// parameter, Undefined Behaviour results.
  template <TagsType V>
    requires(__private::StorageCount<StorageTypeOfTag<V>> > 0u)
  constexpr inline decltype(auto) get_unchecked_mut(
      ::sus::marker::UnsafeFnMarker) & noexcept {
    return __private::find_choice_storage_mut<index<V>>(storage_).as_mut();
  }

  /// Compares two `Choice`s for equality if the
  /// types inside satisfy [`Eq`]($sus::cmp::Eq).
  ///
  /// Satisfies the [`Eq`]($sus::cmp::Eq) concept for `Choice`.
  friend constexpr bool operator==(const Choice& l, const Choice& r) noexcept
    requires(__private::ChoiceIsEq<TagsType, __private::TypeList<Ts...>,
                                   TagsType, __private::TypeList<Ts...>>)
  {
    sus_check(l.index_ != kUseAfterMove && r.index_ != kUseAfterMove);
    return l.index_ == r.index_ && l.storage_.eq(l.index_, r.storage_);
  }

  template <class... Us, auto V, auto... Vs>
    requires(__private::ChoiceIsEq<TagsType, __private::TypeList<Ts...>,
                                   decltype(V), __private::TypeList<Us...>>)
  friend constexpr bool operator==(
      const Choice& l,
      const Choice<__private::TypeList<Us...>, V, Vs...>& r) noexcept {
    sus_check(l.index_ != kUseAfterMove && r.index_ != kUseAfterMove);
    return l.index_ == r.index_ && l.storage_.eq(l.index_, r.storage_);
  }

  template <class... Us, auto V, auto... Vs>
    requires(!__private::ChoiceIsEq<TagsType, __private::TypeList<Ts...>,
                                    decltype(V), __private::TypeList<Us...>>)
  friend constexpr bool operator==(
      const Choice& l,
      const Choice<__private::TypeList<Us...>, V, Vs...>& r) noexcept = delete;

  /// Compares two `Choice`s for ordering.
  ///
  /// `Choice` satisfies the strongest of [`StrongOrd`](
  /// $sus::cmp::StrongOrd), [`Ord`]($sus::cmp::Ord), and [`PartialOrd`](
  /// $sus::cmp::PartialOrd) that all the values inside the `Choice` types
  /// satisfy.
  friend constexpr std::strong_ordering operator<=>(const Choice& l,
                                                    const Choice& r) noexcept
    requires(__private::ChoiceIsStrongOrd<TagsType, __private::TypeList<Ts...>,
                                          TagsType, __private::TypeList<Ts...>>)
  {
    sus_check(l.index_ != kUseAfterMove && r.index_ != kUseAfterMove);
    const auto value_order = std::strong_order(l.which(), r.which());
    if (value_order != std::strong_ordering::equivalent) {
      return value_order;
    } else {
      return l.storage_.strong_ord(l.index_, r.storage_);
    }
  }

  template <class... Us, auto V, auto... Vs>
    requires(
        __private::ChoiceIsStrongOrd<TagsType, __private::TypeList<Ts...>,
                                     decltype(V), __private::TypeList<Us...>>)
  friend constexpr std::strong_ordering operator<=>(
      const Choice& l,
      const Choice<__private::TypeList<Us...>, V, Vs...>& r) noexcept {
    sus_check(l.index_ != kUseAfterMove && r.index_ != kUseAfterMove);
    const auto value_order = std::strong_order(l.which(), r.which());
    if (value_order != std::strong_ordering::equivalent) {
      return value_order;
    } else {
      return l.storage_.strong_ord(l.index_, r.storage_);
    }
  }

  friend constexpr std::weak_ordering operator<=>(const Choice& l,
                                                  const Choice& r) noexcept
    requires(__private::ChoiceIsOrd<TagsType, __private::TypeList<Ts...>,
                                    TagsType, __private::TypeList<Ts...>>)
  {
    sus_check(l.index_ != kUseAfterMove && r.index_ != kUseAfterMove);
    const auto value_order = std::weak_order(l.which(), r.which());
    if (value_order != std::weak_ordering::equivalent) {
      return value_order;
    } else {
      return l.storage_.weak_ord(l.index_, r.storage_);
    }
  }

  template <class... Us, auto V, auto... Vs>
    requires(__private::ChoiceIsOrd<TagsType, __private::TypeList<Ts...>,
                                    decltype(V), __private::TypeList<Us...>>)
  friend constexpr std::weak_ordering operator<=>(
      const Choice& l,
      const Choice<__private::TypeList<Us...>, V, Vs...>& r) noexcept {
    sus_check(l.index_ != kUseAfterMove && r.index_ != kUseAfterMove);
    const auto value_order = std::weak_order(l.which(), r.which());
    if (value_order != std::weak_ordering::equivalent) {
      return value_order;
    } else {
      return l.storage_.weak_ord(l.index_, r.storage_);
    }
  }

  friend constexpr std::partial_ordering operator<=>(const Choice& l,
                                                     const Choice& r) noexcept
    requires(
        __private::ChoiceIsPartialOrd<TagsType, __private::TypeList<Ts...>,
                                      TagsType, __private::TypeList<Ts...>>)
  {
    sus_check(l.index_ != kUseAfterMove && r.index_ != kUseAfterMove);
    const auto value_order = std::partial_order(l.which(), r.which());
    if (value_order != std::partial_ordering::equivalent) {
      return value_order;
    } else {
      return l.storage_.partial_ord(l.index_, r.storage_);
    }
  }

  template <class... Us, auto V, auto... Vs>
    requires(
        __private::ChoiceIsPartialOrd<TagsType, __private::TypeList<Ts...>,
                                      decltype(V), __private::TypeList<Us...>>)
  friend constexpr std::partial_ordering operator<=>(
      const Choice& l,
      const Choice<__private::TypeList<Us...>, V, Vs...>& r) noexcept {
    sus_check(l.index_ != kUseAfterMove && r.index_ != kUseAfterMove);
    const auto value_order = std::partial_order(l.which(), r.which());
    if (value_order != std::partial_ordering::equivalent) {
      return value_order;
    } else {
      return l.storage_.partial_ord(l.index_, r.storage_);
    }
  }

  template <class... RhsTs, auto RhsTag, auto... RhsTags>
    requires(!__private::ChoiceIsAnyOrd<TagsType, __private::TypeList<Ts...>,
                                        decltype(RhsTag),
                                        __private::TypeList<RhsTs...>>)
  friend constexpr auto operator<=>(
      const Choice<__private::TypeList<Ts...>, Tags...>& l,
      const Choice<__private::TypeList<RhsTs...>, RhsTag, RhsTags...>& r) =
      delete;

 private:
  constexpr explicit Choice(IndexType i) noexcept : index_(i) {}

  // TODO: We don't use `[[_sus_no_unique_address]]` here as the compiler
  // overwrites the `index_` when we move-construct into the Storage union.
  // Clang: https://github.com/llvm/llvm-project/issues/60711
  // GCC: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=108775
  Storage storage_;
  IndexType index_;

  // Declare that this type can always be trivially relocated for library
  // optimizations.
  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn, IndexType,
                                           Ts...);

  sus_class_never_value_field(::sus::marker::unsafe_fn, Choice, index_,
                              kNeverValue, kNeverValue);
  // For the NeverValueField.
  constexpr Choice(sus::mem::NeverValueConstructor) noexcept
      : index_(kNeverValue) {}
};

}  // namespace sus::choice_type

// fmt support.
template <class... Ts, auto... Tags, class Char>
struct fmt::formatter<
    ::sus::choice_type::Choice<sus::choice_type::__private::TypeList<Ts...>,
                               Tags...>,
    Char> {
  using Choice =
      ::sus::choice_type::Choice<sus::choice_type::__private::TypeList<Ts...>,
                                 Tags...>;

  template <class ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    return ctx.begin();
  }

  template <class FormatContext>
  constexpr auto format(const Choice& choice, FormatContext& ctx) const {
    return find_choice<FormatContext, Tags...>(choice, ctx);
  }

 private:
  template <class FormatContext, auto Tag, auto... MoreTags>
    requires(sizeof...(MoreTags) > 0)
  static auto find_choice(const Choice& choice, FormatContext& ctx) {
    if (choice.which() == Tag) {
      return format_choice<Tag>(choice, ctx);
    } else {
      return find_choice<FormatContext, MoreTags...>(choice, ctx);
    }
  }

  template <class FormatContext, auto Tag>
  static auto find_choice(const Choice& choice, FormatContext& ctx) {
    sus_debug_check(choice.which() == Tag);
    return format_choice<Tag>(choice, ctx);
  }

  template <auto Tag, class FormatContext>
  static auto format_choice(const Choice& choice, FormatContext& ctx) {
    if constexpr (sus::choice_type::ChoiceValueIsVoid<Choice, Tag>) {
      auto out = fmt::format_to(ctx.out(), "Choice(");
      ctx.advance_to(out);
      using TagFormatter =
          ::sus::string::__private::AnyFormatter<decltype(Tag), Char>;
      out = TagFormatter().format(Tag, ctx);
      return fmt::format_to(ctx.out(), ")");
    } else {
      auto out = fmt::format_to(ctx.out(), "Choice(");
      ctx.advance_to(out);
      using TagFormatter =
          ::sus::string::__private::AnyFormatter<decltype(Tag), Char>;
      out = TagFormatter().format(Tag, ctx);
      out = fmt::format_to(ctx.out(), ", ");
      ctx.advance_to(out);
      using ValueFormatter = ::sus::string::__private::AnyFormatter<
          decltype(choice.template get_unchecked<Tag>(
              ::sus::marker::unsafe_fn)),
          Char>;
      // SAFETY: The Tag here is the active tag as `find_choice()` calls this
      // method only if `which() == Tag`.
      out = ValueFormatter().format(
          choice.template get_unchecked<Tag>(::sus::marker::unsafe_fn), ctx);
      return fmt::format_to(ctx.out(), ")");
    }
  }
};

// Stream support (written out manually due to use of template specialization).
namespace sus::choice_type {
template <class... Ts, auto... Tags,
          ::sus::string::__private::StreamCanReceiveString<char> StreamType>
inline StreamType& operator<<(
    StreamType& stream,
    const Choice<__private::TypeList<Ts...>, Tags...>& value) {
  return ::sus::string::__private::format_to_stream(stream,
                                                    fmt::to_string(value));
}
}  // namespace sus::choice_type

// Promote Choice into the `sus` namespace.
namespace sus {
using ::sus::choice_type::Choice;
}  // namespace sus
