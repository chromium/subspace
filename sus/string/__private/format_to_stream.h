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
    /// Ensure that we don't accidentally recursively check `U`.
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
/// The `U` template type parameter allows `_sus_format_to_stream` to be a
/// hidden friend when `Type` is not a class template. Directly using `Type`
/// means that the function is immediately instantiated, and we'll get a
/// compile-time error unless `fmt::formatter` is specialised before `Type`'s
/// definition, since `fmt::is_formattable` doesn't know it's a formattable type
/// at this point in the code. This isn't a problem for primary class templates,
/// but it is problematic for regular classes and full specialisations. Using a
/// dependent type `U` defers instantiation until `operator<<` is first used.
/// We need to constrain `U` to be the same type as `Type`, otherwise it will
/// be ambiguous as to which overload we want.
// clang-format off
#define _sus_format_to_stream(Type)                                                   \
  template<                                                                           \
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
