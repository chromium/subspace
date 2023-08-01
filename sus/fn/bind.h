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

#include <type_traits>

#include "sus/fn/callable.h"
#include "sus/fn/fn_box_defn.h"
#include "sus/macros/__private/compiler_bugs.h"
#include "sus/macros/eval_macro.h"
#include "sus/macros/for_each.h"
#include "sus/macros/remove_parens.h"
#include "sus/marker/unsafe.h"
#include "sus/mem/forward.h"
#include "sus/mem/move.h"

/// Bind a const lambda to storage for its bound arguments. The output can be
/// used to construct a FnOnceBox, FnMutBox, or FnBox.
///
/// The first argument is a list of variables that will be bound into storage
/// for access from the lambda, wrapped in sus_store(). If there are no
/// variables to mention, sus_store() can be empty, or use the sus_bind0() macro
/// which omits this list.
///
/// The second argument is a lambda, which can include captures. Any captures of
/// variables outside the lambda must be referenced in the sus_store() list.
///
/// Use `sus_take(x)` in the sus_store() list to move `x` into storage instead
/// of copying it.
///
/// Use `sus_unsafe_pointer(x)` to store a pointer named `x`. This is dangerous
/// and discouraged, and using smart pointers is strongly preferred.
///
/// # Example
///
/// This binds a lambda with 3 captures, the first two being variables from the
/// outside scope. The second variable is used as a reference to the storage,
/// rather that copying or moving it into the lambda.
/// ```
/// sus_bind(sus_store(a, b), [a, &b, c = 1]() {}))
/// ```
///
/// # Implementation note
/// The lambda may arrive in multiple arguments, if there is a comma in the
/// definition of it. Thus we use variadic arguments to capture all of the
/// lambda.
#define sus_bind(names, lambda, ...)                                        \
  [&]() {                                                                   \
    [&]() consteval {                                                       \
      sus_for_each(_sus__check_storage, sus_for_each_sep_none,              \
                   _sus__unpack names)                                      \
    }();                                                                    \
    using ::sus::fn::__private::SusBind;                                    \
    return SusBind(                                                         \
        [sus_for_each(_sus__declare_storage, sus_for_each_sep_comma,        \
                      _sus__unpack names)]<class... Args>(Args&&... args) { \
          const auto x = lambda __VA_OPT__(, ) __VA_ARGS__;                 \
          const bool is_const =                                             \
              ::sus::fn::callable::CallableObjectConst<decltype(x)>;        \
          if constexpr (!is_const) {                                        \
            return ::sus::fn::__private::CheckCallableObjectConst<          \
                is_const>::template error<void>();                          \
          } else {                                                          \
            return x(::sus::forward<Args>(args)...);                        \
          }                                                                 \
        });                                                                 \
  }()

/// A variant of `sus_bind()` which only takes a lambda, omitting the
/// `sus_store()` list.  The output can be used to construct a FnOnceBox,
/// FnMutBox, or FnBox.
///
/// Because there is no `sus_store()` list, the lambda can not capture variables
/// from the outside scope, however it can still declare captures contained
/// entirely inside the lambda.
///
/// # Example
///
/// This defines a lambda with a capture `a` of type `int`, and binds it so it
/// can be used to construct a FnOnceBox, FnMutBox, or FnBox.
/// ```
/// sus_bind0([a = int(1)](char, int){})
/// ```
#define sus_bind0(lambda, ...) \
  sus_bind(sus_store(), lambda __VA_OPT__(, ) __VA_ARGS__)

/// Bind a mutable lambda to storage for its bound arguments. The output can be
/// used to construct a FnOnceBox or FnMutBox.
///
/// Because the storage is mutable, the lambda may capture references to the
/// storage and mutate it, and the lambda itself may be marked mutable.
///
/// The first argument is a list of variables that will be bound into storage
/// for access from the lambda, wrapped in sus_store(). If there are no
/// variables to mention, sus_store() can be empty, or use the sus_bind0() macro
/// which omits this list.
///
/// The second argument is a lambda, which can include captures. Any captures of
/// variables outside the lambda must be referenced in the sus_store() list.
///
/// Use `sus_take(x)` in the sus_store() list to move `x` into storage instead
/// of copying it.
///
/// Use `sus_unsafe_pointer(x)` to store a pointer named `x`. This is dangerous
/// and discouraged, and using smart pointers is strongly preferred.
///
/// # Example
///
/// This binds a lambda with 3 captures, the first two being variables from the
/// outside scope. The second variable is used as a reference to the storage,
/// rather that copying or moving it into the lambda.
/// ```
/// sus_bind_mut(sus_store(a, b), [a, &b, c = 1]() {}))
/// ```
///
/// # Implementation note The lambda may arrive in multiple arguments, if there
/// is a comma in the definition of it. Thus we use variadic arguments to
/// capture all of the lambda.
#define sus_bind_mut(names, lambda, ...)                                 \
  [&]() {                                                                \
    [&]() consteval {                                                    \
      sus_for_each(_sus__check_storage, sus_for_each_sep_none,           \
                   _sus__unpack names)                                   \
    }();                                                                 \
    return ::sus::fn::__private::SusBind(                                \
        [sus_for_each(_sus__declare_storage_mut, sus_for_each_sep_comma, \
                      _sus__unpack names)]<class... Args>(               \
            Args&&... args) mutable {                                    \
          auto x = lambda __VA_OPT__(, ) __VA_ARGS__;                    \
          return x(::sus::mem::forward<Args>(args)...);                  \
        });                                                              \
  }()

/// A variant of `sus_bind_mut()` which only takes a lambda, omitting the
/// `sus_store()` list.  The output can be used to construct a FnOnceBox or
/// FnMutBox.
///
/// Because there is no `sus_store()` list, the lambda can not capture variables
/// from the outside scope, however it can still declare captures contained
/// entirely inside the lambda.
///
/// Can be used with a mutable lambda that can mutate its captures.
///
/// # Example
///
/// This defines a lambda with a capture `a` of type `int`, and binds it so it
/// can be used to construct a FnOnceBox, FnMutBox, or FnBox.
/// ```
/// sus_bind0_mut([a = int(1)](char, int){})
/// ```
#define sus_bind0_mut(lambda, ...) \
  sus_bind_mut(sus_store(), lambda __VA_OPT__(, ) __VA_ARGS__)

/// Declares the set of captures from the outside scope in `sus_bind()` or
/// `sus_bind_mut()`.
#define sus_store(...) (__VA_ARGS__)
/// Marks a capture in the `sus_store()` list to be moved from the outside scope
/// instead of copied.
#define sus_take(x) (x, _sus__bind_move)
/// Marks a capture in the `sus_store()` list as a pointer which is being
/// intentionally and unsafely captured. Otherwise, pointers are not allowed to
/// be captured.
#define sus_unsafe_pointer(x) (x, _sus__bind_pointer)

namespace sus::fn::__private {

/// Helper type returned by sus_bind() and used to construct a closure.
template <class F>
struct SusBind final {
  constexpr inline SusBind(F&& lambda) : lambda(::sus::move(lambda)) {}

  /// The lambda generated by sus_bind() which holds the user-provided
  /// lambda and any storage required for it.
  F lambda;
};

// The type generated by sus_unsafe_pointer() for storage in sus_bind().
template <class T>
struct UnsafePointer;

template <class T>
struct UnsafePointer<T*> final {
  constexpr inline UnsafePointer(::sus::marker::UnsafeFnMarker, T* pointer)
      : pointer(pointer) {}
  T* pointer;
};

template <class T>
UnsafePointer(::sus::marker::UnsafeFnMarker, T*) -> UnsafePointer<T*>;

template <class T>
auto make_storage(T&& t) {
  return std::decay_t<T>(forward<T>(t));
}
template <class T>
auto make_storage(T*) {
  static_assert(!std::is_pointer_v<T*>,
                "Can not store a pointer in sus_bind() except through "
                "sus_unsafe_pointer().");
}
template <class T>
auto make_storage(UnsafePointer<T*> p) {
  return static_cast<const T*>(p.pointer);
}

template <class T>
auto make_storage_mut(T&& t) {
  return std::decay_t<T>(forward<T>(t));
}
template <class T>
auto make_storage_mut(T* t) {
  make_storage(t);
}
template <class T>
auto make_storage_mut(UnsafePointer<T*> p) {
  return p.pointer;
}

// Verifies the input is an lvalue (a name), so we can bind it to that same
// lvalue name in the resulting lambda.
template <class T>
std::true_type is_lvalue(T&);
std::false_type is_lvalue(...);

/// Helper used when verifying if a lambda is const. The template parameter
/// represents the constness of the lambda. When false, the error() function
/// generates a compiler error.
template <bool = true>
struct CheckCallableObjectConst final {
  template <class U>
  static constexpr inline auto error() {}
};

template <>
struct CheckCallableObjectConst<false> final {
  template <class U>
  static consteval inline auto error() {
    throw "Use sus_bind_mut() to bind a mutable lambda";
  }
};

}  // namespace sus::fn::__private

// Private helper.
#define _sus__declare_storage(x)                                   \
  sus_eval_macro(_sus__declare_storage_impl, sus_remove_parens(x), \
                 _sus__bind_noop)
#define _sus__declare_storage_impl(x, modify, ...) \
  x = ::sus::fn::__private::make_storage(modify(x))
#define _sus__declare_storage_mut(x)                                   \
  sus_eval_macro(_sus__declare_storage_impl_mut, sus_remove_parens(x), \
                 _sus__bind_noop)
#define _sus__declare_storage_impl_mut(x, modify, ...) \
  x = ::sus::fn::__private::make_storage_mut(modify(x))
#define _sus__check_storage(x, ...)                              \
  sus_eval_macro(_sus__check_storage_impl, sus_remove_parens(x), \
                 _sus__bind_noop)
#define _sus__check_storage_impl(x, modify, ...)                     \
  static_assert(decltype(::sus::fn::__private::is_lvalue(x))::value, \
                "sus_bind() can only bind to variable names (lvalues).");

#define _sus__bind_noop(x) x
#define _sus__bind_move(x) ::sus::move(x)
#define _sus__bind_pointer(x) \
  ::sus::fn::__private::UnsafePointer(::sus::marker::unsafe_fn, x)

// Private helper.
#define _sus__unpack sus_bind_stored_argumnts_should_be_wrapped_in_sus_store
#define sus_bind_stored_argumnts_should_be_wrapped_in_sus_store(...) __VA_ARGS__
