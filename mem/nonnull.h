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
#include "marker/unsafe.h"
#include "mem/addressof.h"

namespace sus::mem {

/// A pointer wrapper which holds a never-null pointer.
template <class T>
struct NonNull {
  static constexpr inline NonNull with(T& t) { return NonNull(addressof(t)); }
  static constexpr inline NonNull with_ptr(T* t) {
    check(t);
    return NonNull(t);
  }
  static constexpr inline NonNull with_ptr_unchecked(
      ::sus::marker::UnsafeFnMarker, T* t) {
    check(t);
    return NonNull(*t);
  }

  constexpr inline const T& as_ref() const { return *ptr_; }
  constexpr inline T& as_ref_mut() { return *ptr_; }

  constexpr inline const T* as_ptr() const { return ptr_; }
  constexpr inline T* as_ptr_mut() { return ptr_; }

 private:
  explicit constexpr inline NonNull(T* t) : ptr_(t) {}

  T* ptr_;

  sus_class_nonzero_field(unsafe_fn, NonNull, ptr_);
};

}  // namespace sus::mem
