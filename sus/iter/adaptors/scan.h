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

#include "sus/fn/fn_box_defn.h"
#include "sus/iter/iterator_defn.h"
#include "sus/mem/move.h"
#include "sus/mem/relocate.h"

namespace sus::iter {

using ::sus::mem::relocate_by_memcpy;

/// An iterator to maintain state while iterating another iterator.
///
/// This type is returned from `Iterator::scan()`.
template <class OutType, class State, class InnerSizedIter, class Fn>
class [[nodiscard]] Scan final
    : public IteratorBase<Scan<OutType, State, InnerSizedIter, Fn>, OutType> {
  static_assert(!std::is_reference_v<State>);
  static_assert(::sus::fn::FnMut<
                Fn, Option<OutType>(State&, typename InnerSizedIter::Item&&)>);

 public:
  using Item = OutType;

  // The type is Move and (can be) Clone.
  Scan(Scan&&) = default;
  Scan& operator=(Scan&&) = default;

  // sus::mem::Clone trait.
  constexpr Scan clone() const noexcept
    requires(::sus::mem::Clone<State> &&  //
             ::sus::mem::Clone<Fn> &&     //
             ::sus::mem::Clone<InnerSizedIter>)
  {
    return Scan(::sus::clone(state_), ::sus::clone(fn_),
                ::sus::clone(next_iter_));
  }

  // sus::iter::Iterator trait.
  constexpr Option<Item> next() noexcept {
    Option<Item> out;
    if (Option<typename InnerSizedIter::Item> o = next_iter_.next();
        o.is_some()) {
      out = ::sus::fn::call_mut(
          fn_, state_,
          ::sus::move(o).unwrap_unchecked(::sus::marker::unsafe_fn));
    }
    return out;
  }
  /// sus::iter::Iterator trait.
  constexpr SizeHint size_hint() const noexcept {
    // Can't know a lower bound, due to the function returning None at any time.
    return SizeHint(0u, next_iter_.size_hint().upper);
  }

 private:
  template <class U, class V>
  friend class IteratorBase;

  explicit constexpr Scan(State&& state, Fn&& fn,
                          InnerSizedIter&& next_iter) noexcept
      : state_(::sus::move(state)),
        fn_(::sus::move(fn)),
        next_iter_(::sus::move(next_iter)) {}

  State state_;
  Fn fn_;
  InnerSizedIter next_iter_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(state_), decltype(fn_),
                                           decltype(next_iter_));
};

}  // namespace sus::iter
