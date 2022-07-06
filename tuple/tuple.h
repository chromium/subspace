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

#include "assertions/check.h"
#include "mem/forward.h"
#include "mem/replace.h"
#include "tuple/__private/storage.h"

namespace sus::tuple {

/// A Tuple holds 1 or more values, where each value can be a different type.
template <class T, class... Ts>
class Tuple {
 public:
  /// Construct a Tuple with the given values.
  constexpr inline static Tuple with(T first, Ts... more) noexcept {
    return Tuple(static_cast<T&&>(first), static_cast<Ts>(more)...);
  }

  /// Gets a const reference to the `I`th element in the tuple.
  template <size_t I>
    requires(I <= sizeof...(Ts))
  constexpr inline const auto& get() const& noexcept {
    ::sus::check(!moved_from());
    return Access<I + 1>::get(storage_);
  }

  /// Disallows getting a reference to temporary Tuple.
  template <size_t I>
  constexpr inline auto get() && =
      delete;  // Don't return reference to temporary.

  /// Gets a mutable reference to the `I`th element in the tuple.
  template <size_t I>
    requires(I <= sizeof...(Ts))
  inline auto& get_mut() & noexcept {
    ::sus::check(!moved_from());
    return Access<I + 1>::get_mut(storage_);
  }

  /// Returns the `I`th element in the tuple.
  template <size_t I>
    requires(I <= sizeof...(Ts))
  constexpr inline auto unwrap() && noexcept {
    ::sus::check(!set_moved_from());
    return Access<I + 1>::unwrap(static_cast<Storage&&>(storage_));
  }

 private:
  /// Storage for the tuple elements. The first element is the moved-from flag.
  using Storage = __private::TupleStorage<2 + sizeof...(Ts), bool, T, Ts...>;
  /// A helper type used for accessing the `Storage`.
  template <size_t I>
  using Access = __private::TupleAccess<Storage, I>;

  constexpr inline Tuple(T&& first, Ts&&... more) noexcept
      : storage_(false, forward<T>(first), forward<Ts>(more)...) {}

  // TODO: Provide a way to opt out of all moved-from checks?
  constexpr inline bool moved_from() const& noexcept {
    return Access<0>::get(storage_);
  }
  constexpr inline bool set_moved_from() & noexcept {
    return ::sus::mem::replace(mref(Access<0>::get_mut(storage_)), true);
  }

  Storage storage_;
};

}  // namespace sus::tuple
