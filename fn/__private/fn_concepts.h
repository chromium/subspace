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
concept FnCallableRun = requires(F f, Args&&... args) {
                          { f(forward<Args>(args)...) } -> std::same_as<R>;
                        };

template <class F, class R, class ReceiverArgsPack, class StoredArgsPack,
          class CallArgsPack>
struct FnCallableWithOnceStorage;

template <class F, class R, class... ReceiverArgs, class... StoredArgs,
          class... CallArgs>
  requires requires(F f, StoredArgs&&... stored, CallArgs&&... called) {
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

             // Can't use FnCallableRun here since we can't unpack
             // a `Pack` of types in a concept, and a template can only have
             // one set of var args. So we can't pass the ReceiverArgs,
             // StoredArgs and CallArgs separately, but we need that to build
             // the appropriate FlexRefs.
             {
               static_cast<F&&>(f)(
                   FlexRef<ReceiverArgs>(static_cast<StoredArgs&&>(stored))...,
                   forward<CallArgs>(called)...)
               } -> std::same_as<R>;
           }
struct FnCallableWithOnceStorage<F, R, Pack<ReceiverArgs...>,
                                 Pack<StoredArgs...>, Pack<CallArgs...>> {
  static constexpr bool value = true;
};

// TODO: Make a nicer error message for:
// ```
// auto c = CopyOnly(1);
// auto fn2 = FnOnce<int()>::with_storage([](CopyOnly c) { return c.i; }, c);
// ```
template <class F, class R, class StoredArgsPack, class CallArgsPack>
concept FnCompatibleOnce =
    requires {
      ::sus::concepts::callable::Callable<F>;
      FnCallableWithOnceStorage<
          F, R, __private::runtype_args_for<F, StoredArgsPack::size>,
          StoredArgsPack, CallArgsPack>::value;
    };

template <class F, class R, class StoredArgsPack, class CallArgsPack>
struct FnCallableWithMutStorage;

template <class F, class R, class... StoredArgs, class... CallArgs>
  requires(FnCallableRun<F&, R, StoredArgs&..., CallArgs...>)
struct FnCallableWithMutStorage<F, R, Pack<StoredArgs...>, Pack<CallArgs...>> {
  static constexpr bool value = true;
};

template <class F, class R, class StoredArgsPack, class CallArgsPack>
concept FnCompatibleMut =
    requires {
      ::sus::concepts::callable::Callable<F>;
      FnCallableWithMutStorage<F, R, StoredArgsPack, CallArgsPack>::value;
    };

template <class F, class R, class StoredArgsPack, class CallArgsPack>
struct FnCallableWithConstStorage;

template <class F, class R, class... StoredArgs, class... CallArgs>
  requires(FnCallableRun<const F&, R, const StoredArgs&..., CallArgs...>)
struct FnCallableWithConstStorage<F, R, Pack<StoredArgs...>,
                                  Pack<CallArgs...>> {
  static constexpr bool value = true;
};

template <class F, class R, class StoredArgsPack, class CallArgsPack>
concept FnCompatibleConst =
    requires {
      ::sus::concepts::callable::Callable<F>;
      FnCallableWithConstStorage<F, R, StoredArgsPack, CallArgsPack>::value;
    };

}  // namespace sus::fn::__private
