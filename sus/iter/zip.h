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

// IWYU pragma: private, include "sus/iter/iterator.h"
// IWYU pragma: friend "sus/.*"
#pragma once

#include "sus/iter/into_iterator.h"
#include "sus/iter/iterator_defn.h"
#include "sus/mem/move.h"

namespace sus::iter {

/// Converts the arguments to iterators and zips them.
///
/// See the documentation of [`Iterator::zip]`(sus::iter::IteratorBase::zip) for
/// more.
///
/// # Example
/// ```
/// auto a = sus::Array<i32, 2>(2, 3);
/// auto b = sus::Array<f32, 5>(3.f, 4.f, 5.f, 6.f, 7.f);
/// auto it = sus::iter::zip(::sus::move(a), ::sus::move(b));
/// sus::check(it.next() == sus::some(sus::tuple(2, 3.f)));
/// sus::check(it.next() == sus::some(sus::tuple(3, 4.f)));
/// sus::check(it.next() == sus::none());
/// ```
inline constexpr auto zip(IntoIteratorAny auto&& iia,
                          IntoIteratorAny auto&& iib) noexcept
    -> Iterator<
        ::sus::Tuple<typename IntoIteratorOutputType<decltype(iia)>::Item,
                     typename IntoIteratorOutputType<decltype(iib)>::Item>> auto
  requires(::sus::mem::IsMoveRef<decltype(iia)> &&
           ::sus::mem::IsMoveRef<decltype(iib)>)
{
  return ::sus::move(iia).into_iter().zip(::sus::move(iib));
}

}  // namespace sus::iter
