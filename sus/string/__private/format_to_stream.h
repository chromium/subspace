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

#include <concepts>
#include <iosfwd>
#include <string>

#include "fmt/format.h"
#include "sus/macros/compiler.h"
#include "sus/macros/for_each.h"

namespace sus::string::__private {

template <class To, class From>
concept ConvertibleFrom = std::convertible_to<From, To>;

template <class T, class Char, class U>
concept StreamCanReceiveString =
    !std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<U>> &&
    requires(T& t, const std::basic_string<Char> s) {
      // Check ConvertibleFrom as std streams return std::basic_ostream&, which the
      // input type `T&` is convertible to. The concept ordering means we want
      // `ConvertibleFrom<..the output type.., T&>` to be true then.
      { t << s } -> ConvertibleFrom<T&>;
    };

/// Consumes the string `s` and streams it to the output stream `os`.
template <class Char, StreamCanReceiveString<Char, void> S>
S& format_to_stream(S& os, const std::basic_string<Char>& s) {
  os << s;
  return os;
}

}  // namespace sus::string::__private

#define _sus_format_to_stream_add_class(x) class x
#define _sus_format_to_stream_parameter_concept(...) \
  std::same_as<__VA_ARGS__> Sus_ValueType,
#define _sus_format_to_stream_parameter_concept_with_template(...)
#define _sus_format_to_stream_parameter(...) Sus_ValueType
#define _sus_format_to_stream_parameter_with_template(...) __VA_ARGS__

/// Defines operator<< for type `Type` in namespace `Namespace`. The operator
/// uses fmt::formatter<T, char> to generate a string and stream it.
///
/// # Implementation Notes
///
/// The `Type` argument is encoded as a template argument for the GCC compiler
/// because it does not reject the overload when the `Type` does not match
/// otherwise, and then ends up recursively trying to solve
/// `StreamCanReceiveString<S, char>`. On the first attempt to solve
/// `StreamCanReceiveString<S, char>`, it tries to call this overload with
/// `std::string` which is *not* `Type` and yet it considers it a valid
/// overload, so it tries to again solve `StreamCanReceiveString<S, char>` which
/// is now recursive. See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=99599.
///
/// Actually it gets worse. If the `Type` has template parameters, we can't use
/// `std::same_as<Type<Params..>> Sus_ValueType` as the compiler can't
/// infer the `Params...` there, even though `Sus_ValueType` appears in the
/// parameter list. So we _have_ to put the real type in the parameter list.
/// But also in this case GCC does the same thing as the others and does not
/// include the function in the overload set if the non-concept type doesn't
/// match. So we can revert back to the usual incantation then.
//
// clang-format off
#define _sus_format_to_stream(Type)                                                   \
  template<                                                                           \
      /* Inserts `std::same_as<Type> Sus_ValueType` if required for GCC. */           \
      sus_if_gcc(                                                                     \
          _sus_format_to_stream_parameter_concept(Type)                               \
      )                                                                               \
      sus::string::__private::StreamCanReceiveString<char, Type> Sus_StreamType,      \
      std::same_as<Type> U = Type                                                     \
  >                                                                                   \
  /** Adaptor from fmt to streams.                                                    \
   * #[doc.hidden] */                                                                 \
  friend Sus_StreamType& operator<<(                                                  \
    Sus_StreamType& stream, const U& value) {                                         \
    static_assert(fmt::is_formattable<U>::value);                                     \
    return ::sus::string::__private::format_to_stream(stream, fmt::to_string(value)); \
  }                                                                                   \
  static_assert(true)
// clang-format on
