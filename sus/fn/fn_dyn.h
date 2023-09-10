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

// IWYU pragma: private, include "sus/fn/fn.h"
// IWYU pragma: friend "sus/.*"
#pragma once

#include "sus/boxed/dyn.h"
#include "sus/fn/fn_concepts.h"

namespace sus::fn {

template <class T>
struct DynFn;
template <class T>
struct DynFnMut;
template <class T>
struct DynFnOnce;

template <class T, class Store, class R, class... Args>
  requires(Fn<T, R(Args...)>)
struct DynFnTyped;
template <class T, class Store, class R, class... Args>
  requires(FnMut<T, R(Args...)>)
struct DynFnMutTyped;
template <class T, class Store, class R, class... Args>
  requires(FnOnce<T, R(Args...)>)
struct DynFnOnceTyped;

/// A type-erased object which satisifies the concept [`Fn<R(Args...)>`](
/// $sus::fn::Fn).
template <class R, class... Args>
struct DynFn<R(Args...)> {
  template <class T>
  static constexpr bool SatisfiesConcept = Fn<T, R(Args...)>;
  template <class T, class Store>
  using DynTyped = DynFnTyped<T, Store, R, Args...>;

  DynFn() = default;
  virtual ~DynFn() = default;
  DynFn(DynFn&&) = delete;
  DynFn& operator=(DynFn&&) = delete;

  // Virtual concept API.
  virtual R operator()(Args&&...) const = 0;
};

/// The implementation of type-erasure for the Fn concept.
/// #[doc.hidden]
template <class T, class Store, class R, class... Args>
  requires(Fn<T, R(Args...)>)
struct DynFnTyped final : public DynFn<R(Args...)> {
  constexpr DynFnTyped(Store&& c) : c_(sus::forward<Store>(c)) {}

  R operator()(Args&&... args) const override {
    return c_(::sus::forward<Args>(args)...);
  }

 private:
  Store c_;
};

/// A type-erased object which satisifies the concept [`FnMut<R(Args...)>`](
/// $sus::fn::FnMut).
template <class R, class... Args>
struct DynFnMut<R(Args...)> {
  template <class T>
  static constexpr bool SatisfiesConcept = FnMut<T, R(Args...)>;
  template <class T, class Store>
  using DynTyped = DynFnMutTyped<T, Store, R, Args...>;

  DynFnMut() = default;
  virtual ~DynFnMut() = default;
  DynFnMut(DynFnMut&&) = delete;
  DynFnMut& operator=(DynFnMut&&) = delete;

  // Virtual concept API.
  virtual R operator()(Args&&...) = 0;
};

/// The implementation of type-erasure for the DynFnMut concept.
/// #[doc.hidden]
template <class T, class Store, class R, class... Args>
  requires(FnMut<T, R(Args...)>)
struct DynFnMutTyped final : public DynFnMut<R(Args...)> {
  constexpr DynFnMutTyped(Store&& c) : c_(sus::forward<Store>(c)) {}

  R operator()(Args&&... args) override {
    return c_(::sus::forward<Args>(args)...);
  }

 private:
  Store c_;
};

/// A type-erased object which satisifies the concept [`FnOnce<R(Args...)>`](
/// $sus::fn::FnOnce).
template <class R, class... Args>
struct DynFnOnce<R(Args...)> {
  template <class T>
  static constexpr bool SatisfiesConcept = FnOnce<T, R(Args...)>;
  template <class T, class Store>
  using DynTyped = DynFnOnceTyped<T, Store, R, Args...>;

  DynFnOnce() = default;
  virtual ~DynFnOnce() = default;
  DynFnOnce(DynFnOnce&&) = delete;
  DynFnOnce& operator=(DynFnOnce&&) = delete;

  // Virtual concept API.
  virtual R operator()(Args&&...) && = 0;
};

/// The implementation of type-erasure for the DynFnOnce concept.
/// #[doc.hidden]
template <class T, class Store, class R, class... Args>
  requires(FnOnce<T, R(Args...)>)
struct DynFnOnceTyped final : public DynFnOnce<R(Args...)> {
  constexpr DynFnOnceTyped(Store&& c) : c_(sus::forward<Store>(c)) {}

  R operator()(Args&&... args) && override {
    return c_(::sus::forward<Args>(args)...);
  }

 private:
  Store c_;
};

// `DynFn` satisfies `Fn`.
static_assert(Fn<DynFn<int(double)>, int(double)>);
// `DynFn` satisfies `DynConcept`, meaning it type-erases correctly and can
// interact with `Dyn` and `Box`.
static_assert(::sus::boxed::DynConcept<DynFn<int(float)>,
                                       decltype([](float) { return 0; })>);

// `DynFnMut` satisfies `FnMut`.
static_assert(FnMut<DynFnMut<int(double)>, int(double)>);
// `DynFnMut` satisfies `DynConcept`, meaning it type-erases correctly and can
// interact with `Dyn` and `Box`.
static_assert(::sus::boxed::DynConcept<DynFnMut<int(float)>,
                                       decltype([](float) { return 0; })>);

// `DynFnOnce` satisfies `FnOnce`.
static_assert(FnOnce<DynFnOnce<int(double)>, int(double)>);
// `DynFnOnce` satisfies `DynConcept`, meaning it type-erases correctly and can
// interact with `Dyn` and `Box`.
static_assert(::sus::boxed::DynConcept<DynFnOnce<int(float)>,
                                       decltype([](float) { return 0; })>);

}  // namespace sus::fn
