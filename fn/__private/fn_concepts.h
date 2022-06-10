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

#include <concepts>

#include "concepts/callable.h"
#include "fn/__private/flex_ref.h"
#include "fn/__private/run_type.h"

namespace sus::fn::__private {

template <class F, class R, class... Args>
concept FnCompatible = requires(F f, Args&&... args) {
                         ::sus::concepts::callable::Callable<F>;
                         { f(forward<Args>(args)...) } -> std::same_as<R>;
                       };

// TODO: Make a nicer error message for:
// ```
// auto c = CopyOnly(1);
// auto fn2 = FnOnce<int()>::with_storage([](MoveOnly& c) { return c.i; }, c);
// ```
//
// We do this with a static function instead of a concept in order to unpack
// Packs of var args. A concept can't unpack in that way AFAIK.
template <class F, class R, class... ReceiverArgs, class... StoredArgs,
          class... CallArgs>
  requires requires(F&& f, StoredArgs... stored, CallArgs... called) {
             ::sus::concepts::callable::Callable<F>;
#if defined(_MSC_VER)
             // Workaround an MSVC bug where copy-constructible types end up
             // being moved when we call_once() even if they are not
             // move-constructible. The `std::constructible_from` concept
             // reports the correct thing, but the function call itself does
             // not.
             //
             // Bug:
             // https://developercommunity2.visualstudio.com/t/Doesnt-fail-to-move-with-move-construct/986314?space=62&entry=problem&dialog=comment
             requires(
                 std::constructible_from<ReceiverArgs, FlexRef<ReceiverArgs>> &&
                 ...);
#endif

             {
               static_cast<F&&>(f)(
                   FlexRef<ReceiverArgs>(static_cast<StoredArgs&&>(stored))...,
                   forward<CallArgs>(called)...)
               } -> std::same_as<R>;
           }
static constexpr bool CheckFnOnceCompatible(__private::Pack<ReceiverArgs...>,
                                            __private::Pack<StoredArgs...>,
                                            __private::Pack<CallArgs...>) {
  return true;
}

}  // namespace sus::fn::__private
