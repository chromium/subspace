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

#include "sus/env/var.h"

#include <stdlib.h>

#include <sstream>

#include "sus/assertions/debug_check.h"
#include "sus/collections/array.h"
#include "sus/collections/vec.h"
#include "sus/iter/compat_ranges.h"
#include "sus/iter/once.h"
#include "sus/marker/unsafe.h"
#include "sus/num/types.h"
#include "sus/ptr/copy.h"

#if defined(WIN32)
#define NOMINMAX
#define UNICODE
#include <windows.h>
#endif

using sus::collections::Array;
using sus::collections::Slice;
using sus::collections::Vec;
using sus::marker::unsafe_fn;
using sus::num::usize;
using sus::option::Option;
using sus::result::Result;

namespace {

#if defined(WIN32)
/// Makes a NUL-terminated string buffer of wchar_t from a multi-byte encoded
/// input string (usually utf8).
Option<Vec<wchar_t>> to_u16s(const std::string& s) noexcept {
  auto output = sus::Vec<wchar_t>();

  while (true) {
    constexpr usize CAPACITY = 1024u;
    auto buf = Array<wchar_t, CAPACITY>();
    size_t total_chars_written = 0;
    size_t chars_written;
    errno_t e = mbstowcs_s(&chars_written, buf.as_mut_ptr(), CAPACITY,
                           s.c_str() + total_chars_written,
                           s.size() - total_chars_written);
    if (e == 0) {
      // Don't subtract 1 to keep the NUL.
      output.extend(sus::move(buf).into_iter().take(chars_written));
      return sus::some(sus::move(output));
    } else if (e == ERANGE) {
      output.extend(sus::move(buf).into_iter().take(chars_written));
      total_chars_written += chars_written;
    } else {
      return sus::none();
    }
  }
}

/// Makes a NUL-terminated multi-byte encoded string (usually utf8) from a
/// (non-NUL-terminated) wchar_t string buffer.
Option<std::string> to_u8s(Slice<wchar_t> s) noexcept {
  sus_debug_check(s[s.len() - 1u] != L'\0');
  std::ostringstream output;

  while (true) {
    constexpr usize CAPACITY = 1024u;
    auto buf = Array<char, CAPACITY>();
    size_t total_chars_written = 0;
    size_t chars_written;
    errno_t e = wcstombs_s(&chars_written, buf.as_mut_ptr(), CAPACITY,
                           s.as_ptr() + total_chars_written,
                           s.len() - total_chars_written);
    if (e == 0) {
      // Subtract 1 to drop the NUL. The std::string will add it.
      output << std::string_view(buf.as_ptr(), chars_written - 1u);
      return sus::some(sus::move(output).str());
    } else if (e == ERANGE) {
      output << std::string_view(buf.as_ptr(), chars_written);
      total_chars_written += chars_written;
    } else {
      return sus::none();
    }
  }
}
#endif

}  // namespace

namespace sus::env {

Result<std::string, VarError> var(const std::string& key) noexcept {
#if defined(WIN32)
  auto key16 = to_u16s(key).ok_or(VarError::InvalidKeyEncoding);
  if (key16.is_err()) return sus::err(sus::move(key16).unwrap_err());

  auto buf = Vec<wchar_t>::with_capacity(1024u);
  // Note that GetEnvironmentVariableW does write a terminating NUL, but
  // `chars` does not include it. So if it returns `buf.capacity() - 1u` then
  // it wrote `buf.capacity()` bytes, and it fit in the buffer. Success.
  //
  // A var can be up to 32,767 characters, which will fit in a `u32` so the
  // unwrap() is ok.
  u32 chars =
      GetEnvironmentVariableW(key16.as_value().as_ptr(), buf.as_mut_ptr(),
                              u32::try_from(buf.capacity()).unwrap());
  if (chars >= buf.capacity()) {
    buf.grow_to_exact(chars + 1u);  // Space for the NUL too.
    chars = GetEnvironmentVariableW(key16.as_value().as_ptr(), buf.as_mut_ptr(),
                                    u32::try_from(buf.capacity()).unwrap());
  }
  if (chars == 0u || chars >= buf.capacity()) {
    return sus::err(VarError::NotFound);
  }
  // SAFETY: `chars` is the number of bytes written excluding the NUL.
  buf.set_len(unsafe_fn, chars);
  return sus::ok(to_u8s(buf).expect("env var has invalid encoding?"));
#else
  const char* value = getenv(key.c_str());
  if (value != nullptr)
    return sus::ok(std::string(value));
  else
    return sus::err(VarError::NotFound);
#endif
}

void set_var(const std::string& key, const std::string& value) noexcept {
  // TODO: Abstract this out to an OS (`sys/os`) layer of the library.
#if defined(WIN32)
  auto key16 = to_u16s(key).expect("key was invalid utf8");
  auto value16 = to_u16s(value).expect("value was invalid utf8");
  sus_check_with_message(
      SetEnvironmentVariableW(key16.as_ptr(), value16.as_ptr()),
      "SetEnvironmentVariable failed");
#else
  auto v = sus::iter::from_range(key)
               .copied()
               .chain(sus::iter::once('='))
               .chain(sus::iter::from_range(value).copied())
               .chain(sus::iter::once('\0'))
               .collect<Vec<char>>();
  // Take the pointer out of the Vec, as putenv retains ownership of it. The Vec
  // must be using the default allocator as well as a result.
  auto [ptr, len, cap] = sus::move(v).into_raw_parts();
  sus_check_with_message(putenv(ptr) == 0, "set_env failed");
#endif
}

}  // namespace sus::env
