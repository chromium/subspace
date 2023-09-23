// Copyright 2023 Google LLC
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

#include "sus/cmp/eq.h"
#include "sus/cmp/ord.h"
#include "sus/macros/lifetimebound.h"
#include "sus/mem/addressof.h"
#include "sus/ptr/nonnull.h"

namespace sus::cmp {

/// A helper struct for reverse ordering.
///
/// This struct is a helper to be used with functions like [`Vec::sort_by_key`](
/// $sus::collections::Vec::sort_by_key) and can be used to reverse order a
/// part of a key.
///
/// # Examples
/// ```
/// auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6);
/// v.sort_by_key([](i32 num) {
///   return sus::Tuple<bool, Reverse<i32>>(num > 3, Reverse(num));
/// });
/// sus::check(v == sus::Vec<i32>(3, 2, 1, 6, 5, 4));
/// ```
template <class T>
struct Reverse {
  static_assert(!std::is_reference_v<T>);

  T value;

  Reverse clone() const noexcept
    requires(::sus::mem::Clone<T> && !::sus::mem::Copy<T>)
  {
    return Reverse(sus::mem::clone(value));
  }
  Reverse clone_from(const Reverse& source) const noexcept
    requires(::sus::mem::CloneFrom<T> && !::sus::mem::Copy<T>)
  {
    sus::mem::clone_into(value, source.value);
  }

  /// Compares the `value`s in two `Reserve` objects.
  friend constexpr bool operator==(const Reverse& lhs, const Reverse& rhs)
    requires(::sus::cmp::Eq<T>)
  {
    return rhs.value == lhs.value;
  }

  /// Returns the reverse ordering of the `value`s in two `Reserve` objects.
  friend constexpr auto operator<=>(const Reverse& lhs, const Reverse& rhs)
    requires(::sus::cmp::PartialOrd<T>)
  {
    return rhs.value <=> lhs.value;
  }
};

template <class T>
Reverse(T) -> Reverse<T>;

/// A helper function for reverse ordering.
///
/// This function is a helper to be used with functions like `Vec::sort_by`.
///
/// # Examples
/// ```
/// auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6);
/// v.sort_by(sus::cmp::reverse<i32>);
/// sus::check(v == sus::Vec<i32>(6, 5, 4, 3, 2, 1));
/// ```
template <::sus::cmp::ExclusiveStrongOrd T>
inline constexpr std::strong_ordering reverse(const T& lhs,
                                              const T& rhs) noexcept {
  return rhs <=> lhs;
}

template <::sus::cmp::ExclusiveOrd T>
inline constexpr std::weak_ordering reverse(const T& lhs,
                                            const T& rhs) noexcept {
  return rhs <=> lhs;
}

template <::sus::cmp::ExclusivePartialOrd T>
inline constexpr std::partial_ordering reverse(const T& lhs,
                                               const T& rhs) noexcept {
  return rhs <=> lhs;
}

/// A helper function for reverse ordering produced by another function.
///
/// This function is a helper to be used with functions like `Vec::sort_by` when
/// there is already a function providing an ordering, but it should be
/// reversed.
template <class T,
          ::sus::fn::FnMut<std::strong_ordering(const T&, const T&)> Fn>
  requires(::sus::cmp::ExclusiveStrongOrd<T>)
inline constexpr auto reverse_by(Fn f) noexcept {
  return
      [f = ::sus::move(f)](const T& lhs, const T& rhs) -> std::strong_ordering {
        return f(rhs, lhs);
      };
}

template <class T, ::sus::fn::FnMut<std::weak_ordering(const T&, const T&)> Fn>
  requires(::sus::cmp::ExclusiveOrd<T>)
inline constexpr auto reverse_by(Fn f) noexcept {
  return
      [f = ::sus::move(f)](const T& lhs, const T& rhs) -> std::weak_ordering {
        return f(rhs, lhs);
      };
}

template <class T,
          ::sus::fn::FnMut<std::partial_ordering(const T&, const T&)> Fn>
  requires(::sus::cmp::ExclusivePartialOrd<T>)
inline constexpr auto reverse_by(Fn f) noexcept {
  return [f = ::sus::move(f)](const T& lhs,
                              const T& rhs) -> std::partial_ordering {
    return f(rhs, lhs);
  };
}

}  // namespace sus::cmp
