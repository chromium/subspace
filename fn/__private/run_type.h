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

#include <stddef.h>
// TODO: Replace std::index_sequence to remove this heavy include.
#include <utility>

#include "assertions/arch.h"
#include "assertions/windows.h"
#include "concepts/callable.h"
#include "mem/forward.h"

namespace sus::fn::__private {

using sus::concepts::callable::CallableObject;
using sus::concepts::callable::FunctionPointer;

template <class... T>
struct Pack {
  static constexpr size_t size = sizeof...(T);
};

template <class F>
struct RunType;
template <bool IsMethod, class F>
struct RunTypeImpl;

template <size_t QueryIndex, size_t CurrentSearchIndex, class Pack>
struct AtPackIndex;

template <size_t Q, class T, class... MoreT>
struct AtPackIndex<Q, Q, Pack<T, MoreT...>> {
  using Type = T;
};

struct UnknownType;

template <size_t Q, size_t I>
struct AtPackIndex<Q, I, Pack<>> {
  using Type = UnknownType;
};

template <size_t Q, size_t I, class T, class... MoreT>
  requires(Q > I)
struct AtPackIndex<Q, I, Pack<T, MoreT...>> {
  using Type = typename AtPackIndex<Q, I + 1, Pack<MoreT...>>::Type;
};

template <class Pack, class Indicies>
struct PackPrefix;

template <class... T, size_t... Indicies>
struct PackPrefix<Pack<T...>, std::index_sequence<Indicies...>> {
  using Types = Pack<typename AtPackIndex<Indicies, 0, Pack<T...>>::Type...>;
};

template <bool IsMethod, class R, class... FnArgs>
struct RunTypeImpl<IsMethod, R(FnArgs...)> {
  using Args = Pack<FnArgs...>;
  static constexpr size_t num_args = sizeof...(FnArgs);
  static constexpr bool is_method = IsMethod;
};

// Provides the first `N` receiver parameter types of a callable `F`. If the
// callable is a method on an object, the first argument type is a pointer to
// the receiver itself.
template <class F, size_t N = sizeof(RunType<F>::NumArgs)>
  requires requires { typename RunType<F>::Args; }
using runtype_args_for =
    typename PackPrefix<typename RunType<F>::Args,
                        std::make_index_sequence<N>>::Types;

// RunType<F> provides the receiver argument types of a callable `F`.

#define SUS_FUNCTION_POINTER(CallType, Qualifiers)                 \
  template <class R, class... FnArgs>                              \
  struct RunType<R(CallType*)(FnArgs...) Qualifiers>               \
      : RunTypeImpl<false, R(FnArgs...)> {                         \
    template <class... Args>                                       \
    static constexpr auto call(R(CallType* f)(Args...) Qualifiers, \
                               Args&&... args) {                   \
      return f(forward<Args>(args)...);                            \
    }                                                              \
  };

SUS_FUNCTION_POINTER(, )
SUS_FUNCTION_POINTER(, noexcept)

#if sus_assertions_is_windows() && !sus_assertions_is_64bit()
SUS_FUNCTION_POINTER(__stdcall, )
SUS_FUNCTION_POINTER(__stdcall, noexcept)
SUS_FUNCTION_POINTER(__fastcall, )
SUS_FUNCTION_POINTER(__fastcall, noexcept)
#endif

#define SUS_METHOD_POINTER(CallType, PointerQualifiers, Qualifiers)          \
  template <class R, class C, class... FnArgs>                               \
  struct RunType<R (CallType C::*PointerQualifiers)(FnArgs...) Qualifiers>   \
      : RunTypeImpl<true, R(C* PointerQualifiers, FnArgs...)> {              \
    template <class... Args>                                                 \
    static constexpr auto call(R (CallType C::*PointerQualifiers f)(Args...) \
                                   Qualifiers,                               \
                               C* PointerQualifiers c, Args&&... args) {     \
      return (c->*f)(forward<Args>(args)...);                                \
    }                                                                        \
  };

SUS_METHOD_POINTER(, , )
SUS_METHOD_POINTER(, , noexcept)
SUS_METHOD_POINTER(, , const)
SUS_METHOD_POINTER(, , const noexcept)
SUS_METHOD_POINTER(, , const&)
SUS_METHOD_POINTER(, , const& noexcept)
SUS_METHOD_POINTER(, const, const)
SUS_METHOD_POINTER(, const, const noexcept)
SUS_METHOD_POINTER(, const, const&)
SUS_METHOD_POINTER(, const, const& noexcept)
SUS_METHOD_POINTER(, , &)
SUS_METHOD_POINTER(, , & noexcept)
SUS_METHOD_POINTER(, , &&)
SUS_METHOD_POINTER(, , && noexcept)

#if sus_assertions_is_windows() && !sus_assertions_is_64bit()
SUS_METHOD_POINTER(__stdcall, , )
SUS_METHOD_POINTER(__stdcall, , noexcept)
SUS_METHOD_POINTER(__stdcall, const, const)
SUS_METHOD_POINTER(__stdcall, const, const noexcept)
SUS_METHOD_POINTER(__stdcall, const, const&)
SUS_METHOD_POINTER(__stdcall, const, const& noexcept)
SUS_METHOD_POINTER(__stdcall, , &)
SUS_METHOD_POINTER(__stdcall, , & noexcept)
SUS_METHOD_POINTER(__stdcall, , &&)
SUS_METHOD_POINTER(__stdcall, , && noexcept)
#endif

#undef SUS_METHOD_POINTER

template <class F>
  requires requires {
             // A `CallableObject` excludes `FunctionPointer`.
             requires CallableObject<F>;
             { &F::operator() };
           }
struct RunType<F> : RunType<decltype(&F::operator())> {
  template <class Func, class... Args>
  static constexpr auto call(Func&& f, Args&&... args) {
    return forward<Func>(f)(forward<Args>(args)...);
  }
};

template <class F>
  requires requires {
             requires FunctionPointer<F>;
             { &F::operator() };
           }
struct RunType<F> : RunType<decltype(+std::declval<F>())> {
  template <class Func, class... Args>
  static constexpr auto call(Func&& f, Args&&... args) {
    return forward<Func>(f)(forward<Args>(args)...);
  }
};

}  // namespace sus::fn::__private
