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

#pragma once

#include <coroutine>

#include "subspace/assertions/unreachable.h"
#include "subspace/iter/__private/iterator_end.h"
#include "subspace/iter/iterator_defn.h"
#include "subspace/macros/lifetimebound.h"
#include "subspace/mem/copy.h"
#include "subspace/mem/move.h"
#include "subspace/mem/relocate.h"
#include "subspace/mem/replace.h"

namespace sus::iter {

namespace __private {

template <class Generator, class T>
class IterPromise {
 public:
  auto get_return_object() noexcept { return Generator(*this); }

  constexpr auto yield_value(T v) noexcept
    requires(std::is_reference_v<T>)
  {
    yielded_.insert(::sus::forward<T>(v));
    return std::suspend_always();
  }
  constexpr auto yield_value(const T& v) noexcept
    requires(::sus::mem::Copy<T> && !std::is_reference_v<T>)
  {
    yielded_.insert(v);
    return std::suspend_always();
  }
  constexpr auto yield_value(T&& v) noexcept
    requires(::sus::mem::Move<T> && !std::is_reference_v<T>)
  {
    yielded_.insert(::sus::move(v));
    return std::suspend_always();
  }

  void return_void() noexcept {
    yielded_ = Option<T>();  // Yield None at the end of the generator.
  }

  // Awaits in an iterator generator would yield None, which is unintuitive, so
  // disallow co_await.
  //
  // TODO: Provide an UnfusedGenerator that expects `co_yield Option<T>` instead
  // of `co_yield T` so that it can return interspersed Some and None?
  void await_transform() = delete;

  constexpr auto initial_suspend() noexcept { return std::suspend_always(); }
  constexpr auto final_suspend() noexcept { return std::suspend_always(); }
  void unhandled_exception() noexcept { ::sus::unreachable(); }

  Option<T> take() & noexcept { return yielded_.take(); }

 private:
  Option<T> yielded_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(yielded_));
};

template <class Generator>
class [[nodiscard]] [[sus_trivial_abi]] GeneratorLoop {
 public:
  GeneratorLoop(Generator& generator sus_lifetimebound) noexcept
      : generator_(generator) {}

  constexpr bool operator==(
      const ::sus::iter::__private::IteratorEnd&) noexcept {
    return generator_.co_handle_.done();
  }
  constexpr GeneratorLoop& operator++() & noexcept {
    // UB occurs if this is called after GeneratorLoop == IteratorEnd. This
    // can't happen in a ranged-for loop, but GeneratorLoop should not be
    // held onto and used in other contexts.
    //
    // TODO: Write a clang-tidy for this.
    generator_.co_handle_.resume();
    return *this;
  }
  constexpr decltype(auto) operator*() & noexcept {
    // UB occurs if this is called after GeneratorLoop == IteratorEnd. This
    // can't happen in a ranged-for loop, but GeneratorLoop should not be
    // held onto and used in other contexts.
    //
    // TODO: Write a clang-tidy for this.
    return generator_.co_handle_.promise().take().unwrap_unchecked(
        ::sus::marker::unsafe_fn);
  }

 private:
  Generator& generator_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn,
                                  decltype(generator_));
};

}  // namespace __private

/// A generator type that is a `sus::iter::Iterator` over type `T`.
///
/// To implement a generator iterator, write a function that returns
/// `sus::iter::Generator<T>`. Then `co_yield` values of type `T`, and each one
/// will be returned from the resulting `Iterator` in the same order.
template <class T>
class [[nodiscard]] [[sus_trivial_abi]] Generator final
    : public ::sus::iter::IteratorBase<Generator<T>, T> {
 public:
  // Coroutine implementation.
  using promise_type = __private::IterPromise<Generator<T>, T>;

  using Item = T;

 public:
  constexpr ~Generator() noexcept {
    if (co_handle_ != nullptr) co_handle_.destroy();
  }

  /// sus::mem::Move trait.
  constexpr Generator(Generator&& o) noexcept
      : co_handle_(::sus::mem::replace(o.co_handle_, nullptr)) {
    ::sus::check(co_handle_ != nullptr);
  }
  /// sus::mem::Move trait.
  constexpr Generator& operator=(Generator&& o) noexcept {
    co_handle_ = ::sus::mem::replace(o.co_handle_, nullptr);
    ::sus::check(co_handle_ != nullptr);
  }

  /// sus::iter::Iterator trait.
  constexpr Option<T> next() noexcept {
    if (!co_handle_.done()) co_handle_.resume();
    return co_handle_.promise().take();
  }

  /// sus::iter::Iterator trait.
  ::sus::iter::SizeHint size_hint() const noexcept {
    return SizeHint(0u, sus::Option<::sus::num::usize>());
  }

  /// Adaptor method for using Generator in ranged for loops.
  ///
  /// This replaces the implementation in IteratorBase with something more
  /// efficient, as the yielded `T` from the generator must be held in the
  /// promise, and this avoids moving it and holding it in the type returned by
  /// begin() as well.
  constexpr auto begin() & noexcept {
    // Ensure the first item is yielded and ready to be returned from the
    // iterator's operator*().
    if (!co_handle_.done()) co_handle_.resume();
    return __private::GeneratorLoop(*this);
  }

 private:
  // The promise constructs this type.
  friend promise_type;
  // The GeneratorLoop makes use of the `co_handle_` to resume the generator at
  // each step, and to pull each Item out of the promise without next(), which
  // would require storing the Item again in the GeneratorLoop.
  friend class __private::GeneratorLoop<Generator>;

  constexpr Generator(promise_type& p) noexcept
      : co_handle_(std::coroutine_handle<promise_type>::from_promise(p)) {}

  std::coroutine_handle<promise_type> co_handle_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn,
                                  decltype(co_handle_));
};

}  // namespace sus::iter
