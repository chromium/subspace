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
#include <utility>  // TODO: replace std::make_index_sequence.

#include "assertions/check.h"
#include "concepts/make_default.h"
#include "marker/unsafe.h"

namespace sus::containers {

namespace __private {

template <class T, size_t N>
struct Storage {
  T data_[N];
};

template <class T>
struct Storage<T, 0> {};

}  // namespace __private

template <class T, size_t N>
  requires(N <= PTRDIFF_MAX)
class Array;

/// A container of objects of type T, with a fixed size N.
///
/// An Array can not be larger than PTRDIFF_MAX, as subtracting a pointer at a
/// greater distance results in Undefined Behaviour.
template <class T, size_t N>
  requires(N <= PTRDIFF_MAX)
class Array {
 public:
  constexpr static Array with_default()
    requires(::sus::concepts::MakeDefault<T>::has_concept)
  {
    auto a = Array();
    if constexpr (N > 0) {
      for (size_t i = 0; i < N; ++i) a.storage_.data_[i] = T();
    }
    return a;
  }

  constexpr static Array with_uninitialized(::sus::marker::UnsafeFnMarker)
    requires(::std::is_trivially_default_constructible_v<T>)
  {
    return Array();
  }

  template <class InitializerFn>
  constexpr static Array with_initializer(InitializerFn f) {
    return Array(kWithInitializer, static_cast<decltype(f)&&>(f),
                 std::make_index_sequence<N>());
  }

  constexpr static Array with_value(const T& t)
    requires(std::is_copy_constructible_v<T>)
  {
    return Array(kWithValue, t, std::make_index_sequence<N>());
  }

  constexpr const T& get(size_t i) const&
    requires(N > 0)
  {
    ::sus::check(i < N);
    return storage_.data_[i];
  }
  constexpr const T& at(size_t i) && = delete;

  T& get_mut(size_t i) &
        requires(N > 0)
  {
    ::sus::check(i < N);
    return storage_.data_[i];
  }

 private:
  Array() {}

  enum WithInitializer { kWithInitializer };
  template <class F, size_t... Is>
  constexpr Array(WithInitializer, F&& f, std::index_sequence<Is...>) noexcept
      : storage_{((void)Is, f())...} {}

  enum WithValue { kWithValue };
  template <size_t... Is>
  constexpr Array(WithValue, const T& t, std::index_sequence<Is...>) noexcept
      : storage_{((void)Is, t)...} {}

  // Using a union ensures that the default constructor doesn't initialize
  // anything.
  union {
    ::sus::containers::__private::Storage<T, N> storage_;
  };
};

}  // namespace sus::containers

namespace sus {
using containers::Array;
}
