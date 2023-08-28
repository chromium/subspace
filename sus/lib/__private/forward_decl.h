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

// IWYU pragma: private
// IWYU pragma: friend "sus/.*"
#pragma once

#include <stddef.h>
#include <stdint.h>

#include <type_traits>

// Forward declarations of all types that ever need a forward declaration.
// Include this header instead of writing out a forward declaration. And add
// them as needed.

namespace sus::choice_type {
template <class TypeListOfMemberTypes, auto... Tags>
class Choice;
}

namespace sus::collections {
template <class T, size_t N>
class Array;
}

namespace sus::collections {
template <class T, size_t N>
struct ArrayIntoIter;
}

namespace sus::collections {
template <class T>
class Slice;
}

namespace sus::collections {
template <class T>
class SliceMut;
}

namespace sus::collections {
template <class T>
struct SliceIter;
}

namespace sus::collections {
template <class T>
struct SliceIterMut;
}

namespace sus::collections {
template <class T>
class Vec;
}

namespace sus::collections {
template <class T>
struct VecIntoIter;
}

namespace sus::fn {
template <class R, class... Args>
class FnOnceRef;
}

namespace sus::fn {
template <class R, class... Args>
class FnMutRef;
}

namespace sus::fn {
template <class R, class... Args>
class FnRef;
}

namespace sus::fn {
template <class R, class... Args>
class FnOnceBox;
}

namespace sus::fn {
template <class R, class... Args>
class FnMutBox;
}

namespace sus::fn {
template <class R, class... Args>
class FnBox;
}

namespace sus::iter {
template <class Iter, class Item>
class IteratorBase;
}

// Include iter/iterator.h to get the implementation of these.
namespace sus::iter {
template <class RefIterator>
class ByRef;
template <class InnerSizedIter, class OtherSizedIter>
class Chain;
template <class InnerSizedIter>
class Cloned;
template <class InnerSizedIter>
class Copied;
template <class InnerSizedIter>
class Cycle;
template <class InnerSizedIter>
class Enumerate;
template <class InnerSizedIter, class Pred>
class Filter;
template <class ToItem, class InnerSizedIter, class FilterMapFn>
class FilterMap;
template <class IntoIterable, class InnerSizedIter, class MapFn>
class FlatMap;
template <class EachIter, class InnerSizedIter>
class Flatten;
template <class InnerIter>
class Fuse;
template <class Item>
class Generator;
template <class InnerSizedIter, class InspectFn>
class Inspect;
template <class ToItem, class InnerSizedIter, class MapFn>
class Map;
template <class ToItem, class InnerSizedIter, class MapFn>
class MapWhile;
template <class InnerSizedIter>
class Peekable;
template <class InnerSizedIter>
class Reverse;
template <class OutType, class State, class InnerSizedIter, class Fn>
class Scan;
template <class InnerIter>
class Skip;
template <class InnerIter, class Pred>
class SkipWhile;
template <class InnerIter>
class StepBy;
template <class InnerIter>
class Take;
template <class InnerIter, class Pred>
class TakeWhile;
template <class... InnerIters>
class Zip;
template <class Iter>
class IteratorRange;
}  // namespace sus::iter

namespace sus::iter {
struct SizeHint;
}

namespace sus::ptr {
template <class T>
  requires(!std::is_reference_v<T>)
class NonNull;
}

namespace sus::num {
struct i8;
struct i16;
struct i32;
struct i64;
struct isize;
struct u8;
struct u16;
struct u32;
struct u64;
struct usize;
struct uptr;
struct f32;
struct f64;
}  // namespace sus::num

namespace sus::option {
template <class T>
class Option;
}

namespace sus::result {
struct OkVoid;
}

namespace sus::result {
template <class T, class E>
class Result;
}

namespace sus::tuple_type {
template <class T, class... Ts>
class Tuple;
}
