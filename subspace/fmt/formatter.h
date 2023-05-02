#include <iostream>
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

#include "subspace/containers/array.h"
#include "subspace/containers/slice.h"
#include "subspace/fmt/debug.h"
#include "subspace/fmt/result.h"
#include "subspace/iter/iterator_defn.h"
#include "subspace/iter/reverse.h"
#include "subspace/macros/lifetimebound.h"
#include "subspace/mem/addressof.h"
#include "subspace/num/float_concepts.h"
#include "subspace/num/integer_concepts.h"
#include "subspace/num/unsigned_integer.h"

namespace sus::fmt {

struct Argument;
struct Arguments;
class Formatter;
class Write;
::sus::fmt::Result write(Write&, const Arguments&);

using FormatFunc = ::sus::fmt::Result (*)(const void* value,
                                          ::sus::fmt::Formatter&);

struct Argument {
  const void* value;
  FormatFunc format_func;
};

struct Arguments {
  ::sus::Slice<Argument> args;
};

class Write {
 public:
  virtual ::sus::fmt::Result write_str(::sus::Slice<unsigned char> data) = 0;

  // TODO: utf8 string view type?
  // TODO: byte string view type?

  virtual ::sus::fmt::Result write_char(unsigned char data) {
    return write_str(::sus::Slice<unsigned char>::from({data}));
  }

  // TODO: 32-bit unicode codepoint char type also?
  // https://doc.rust-lang.org/stable/std/primitive.char.html

  // TODO: Write Arguments.
  virtual ::sus::fmt::Result write_fmt(const Arguments& args) {
    return write(*this, args);
  }
};

/// Configuration for formatting.
///
/// A Formatter represents various options related to formatting. Users do not
/// construct Formatters directly; a mutable reference to one is passed to the
/// fmt method of all formatting traits, like `Debug` and `Display`.
///
/// To interact with a `Formatter`, you'll call various methods to change the
/// various options related to formatting. For examples, please see the
/// documentation of the methods defined on Formatter below.
class Formatter final {
 public:
  explicit Formatter(Write& write sus_lifetimebound) noexcept
      : write_(::sus::mem::addressof(write)) {}

  ::sus::fmt::Result write_str(::sus::Slice<unsigned char> data) {
    return write_->write_str(data);
  }

  ::sus::fmt::Result write_char(unsigned char data) {
    return write_->write_char(data);
  }

  ::sus::fmt::Result write_fmt(const Arguments& args) {
    return write_->write_fmt(args);
  }

 private:
  Write* write_;
};

::sus::fmt::Result write(Write& write, const Arguments& args) {
  auto formatter = Formatter(write);
  for (const Argument& arg : args.args) {
    if (Result r = arg.format_func(arg.value, formatter); r.is_err()) return r;
  }
  return Result::with(Void{});
}

inline Argument new_debug(const ::sus::fmt::Debug auto& value) {
  return Argument{
      ::sus::mem::addressof(value),
      [](const void* p, ::sus::fmt::Formatter& f) {
        return value->fmt_debug(f);
      },
  };
}

inline Argument new_display(const ::sus::fmt::Display auto& value) {
  return Argument{
      ::sus::mem::addressof(value),
      [](const void* p, ::sus::fmt::Formatter& f) { return value->fmt(f); },
  };
}

template <::sus::num::UnsignedPrimitiveInteger T>
inline Argument new_unsigned(const T& value) {
  return Argument{
      &value,
      [](const void* p, ::sus::fmt::Formatter& f) {
        u64 u = *reinterpret_cast<const T*>(p);
        ::sus::Array<u8, 18u> values;
        usize array_index;
        while (u > 0u) {
          values[array_index] = u8::from(u % 10u);
          u /= 10u;
          array_index += 1u;
        }
        bool leading_zeros = true;
        for (u8 value : values.iter().rev()) {
          if (value > 0 || !leading_zeros) {
            leading_zeros = false;
            if (Result r = f.write_char(uint8_t{value} + '0'); r.is_err())
              return r;
          }
        }
        if (leading_zeros) {
          if (Result r = f.write_char('0'); r.is_err()) return r;
        }
        return Result::with(Void{});
      },
  };
}

template <::sus::num::SignedPrimitiveInteger T>
inline Argument new_signed(const T& value) {
  return Argument{
      &value,
      [](const void* p, ::sus::fmt::Formatter& f) {
        i64 i = *reinterpret_cast<const T*>(p);
        if (i < 0) {
          if (Result r = f.write_char('-'); r.is_err()) return r;
        }
        u64 u = i.unsigned_abs();
        ::sus::Array<u8, 18u> values;
        usize array_index;
        while (u > 0u) {
          values[array_index] = u8::from(u % 10u);
          u /= 10u;
          array_index += 1u;
        }
        bool leading_zeros = true;
        for (u8 value : values.iter().rev()) {
          if (value > 0 || !leading_zeros) {
            leading_zeros = false;
            if (Result r = f.write_char(uint8_t{value} + '0'); r.is_err())
              return r;
          }
        }
        if (leading_zeros) {
          if (Result r = f.write_char('0'); r.is_err()) return r;
        }
        return Result::with(Void{});
      },
  };
}

inline Argument new_pointer(const void* value) {
  return Argument{
      value,
      [](const void* p, ::sus::fmt::Formatter& f) {
        if (Result r =
                f.write_str(::sus::Slice<unsigned char>::from({'0', 'x'}));
            r.is_err()) {
          return r;
        }
        usize u = reinterpret_cast<uintptr_t>(p);
        bool leading_zeros = true;
        for (u8 b : u.to_be_bytes()) {
          if (b > 0 || !leading_zeros) {
            leading_zeros = false;
            const auto high = (b & 0xf0) >> 4;
            const auto low = b & 0xf;
            if (Result r = f.write_char(uint8_t{'0' + high}); r.is_err())
              return r;
            if (Result r = f.write_char(uint8_t{'0' + low}); r.is_err())
              return r;
          }
        }
        return Result::with(Void{});
      },
  };
}

}  // namespace sus::fmt
