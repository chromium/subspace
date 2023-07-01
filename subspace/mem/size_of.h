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

#include "subspace/macros/inline.h"
#include "subspace/macros/pure.h"
#include "subspace/mem/__private/data_size_finder.h"

namespace sus::mem {

/// Returns the size of the type T.
///
/// This is the number of bytes that will be allocated for a type T, and
/// includes any tail padding.
///
/// Disallows calls with a reference type, since C++ does a surprising thing,
/// where `sizeof(T&) == sizeof(T)`.
template <class T>
  requires(!std::is_reference_v<T>)
sus_pure_const consteval sus_always_inline size_t size_of() noexcept {
  return sizeof(T);
}

/// Returns the data size of the type T.
///
/// This is the number of bytes for the type excluding any tail padding, which
/// is the number of bytes that can be memcpy'd into the type without
/// potentially overwriting other objects. This is due to the fact that other
/// objects can be placed inside tail padding of an object in some scenarios.
///
/// Returns `size_t(-1)` for types where the tail padding can not be determined.
/// In particular this is the case for union types unless and until compilers
/// provide additional support to determine the maximum data size of all
/// union members.
///
/// From @ssbr:
///
/// If type `T` has padding at its end, such as:
/// ```
/// class T { i64 a; i32 b; };
/// ```
///
/// Then there are two ways for another type to place a field inside the padding
/// adjacent to `b` and inside area allocated for `sizeof(T)`:
///
/// 1. A subclass of a non-POD type can insert its fields into the padding of
/// the base class.
///
/// So a subclass of `T` may have its first field inside the padding adjacent to
/// `b`:
/// ```
/// class S : T { i32 c; };
/// ```
/// In this example, `sizeof(S) == sizeof(T)` because `c` sits inside the
/// trailing padding of `T`.
///
/// 2. A class with a `[[no_unique_address]]` field may insert other fields
/// below it into the padding of the `[[no_unique_address]]` field.
///
/// So a class that contains `T` as a field can insert another field into `T`:
/// ```
/// class S { [[no_unique_address]] T t; i32 c; };
/// ```
/// In this example, `sizeof(S) == sizeof(T)` because `c` sits inside the
/// trailing padding of `T`.
///
/// So the dsizeof(T) algorithm [to determine how much to memcpy safely] is
/// something like:
///
/// * A: find out how many bytes fit into the padding via inheritance
///   (`struct S : T { bytes }` for all `bytes` until
///   `sizeof(T) != sizeof(S)`).
/// * B: find out how many bytes fit into the padding via no_unique_address
///   (`struct S { [[no_unique_address]] T x; bytes }` for all `bytes` until
///   `sizeof(T) != sizeof(S)`).
///
/// ```
/// return sizeof(T) - max(A, B)
/// ```
///
/// And I think on every known platform, A == B. It might even be guaranteed by
/// the standard, but I wouldn't know how to check.
///
/// From @danakj:
///
/// On MSVC 19, an empty class has size 1, but with the above formula:
/// * A = 1, as the 1 byte gets reused by a subclass.
/// * B = 0, as [[no_unique_address]] does not reuse the one byte of the empty
///   class.
///
/// The result is that the data_size_of<T> should be 0, since the 1 byte _can_
/// be reused.
///
/// In general, [[no_unique_address]] and [[msvc::no_unique_address]] doesn't
/// appear do anything on MSVC 19, and subclasses also do not use padding
/// bytes of the base class, with the exception of the base class being empty.
template <class T>
  requires(!std::is_reference_v<T>)
sus_pure_const consteval sus_always_inline size_t data_size_of() noexcept {
  static_assert(alignof(T) <= alignof(std::max_align_t),
                "data_size_of() does not support types with alignment greater "
                "than std::max_align_t.");
  return __private::data_size_finder<T>;
}

}  // namespace sus::mem

// Promote data_size_of() and size_of() into the `sus` namespace.
namespace sus {
using sus::mem::data_size_of;
using sus::mem::size_of;
}  // namespace sus
