
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

#include <fstream>

#include "fmt/format.h"
#include "sus/macros/lifetimebound.h"
#include "sus/ops/range.h"
#include "sus/prelude.h"

namespace subdoc::gen {

class JsonWriter {
 public:
  class JsonObject {
   public:
    ~JsonObject() noexcept {
      writer_->indent_ = writer_->indent_.saturating_sub(2u);
      writer_->stream_ << '\n';
      for (auto x : sus::ops::range(0_u32, writer_->indent_)) {
        writer_->stream_ << ' ';
      }
      writer_->stream_ << '}';
    }

    void add_string(std::string_view key, std::string_view string) noexcept {
      if (wrote_one_) writer_->stream_ << ",\n";
      wrote_one_ = true;
      for (auto x : sus::ops::range(0_u32, writer_->indent_)) {
        writer_->stream_ << ' ';
      }
      writer_->stream_ << '\"' << key << "\": " << '\"' << string << '\"';
    }

    void add_int(std::string_view key, i32 i) noexcept {
      if (wrote_one_) writer_->stream_ << ",\n";
      wrote_one_ = true;
      for (auto x : sus::ops::range(0_u32, writer_->indent_)) {
        writer_->stream_ << ' ';
      }
      writer_->stream_ << '\"' << key << "\": " << i;
    }

    JsonObject open_object(std::string_view key) noexcept {
      if (wrote_one_) writer_->stream_ << ",\n";
      wrote_one_ = true;
      for (auto x : sus::ops::range(0_u32, writer_->indent_)) {
        writer_->stream_ << ' ';
      }
      writer_->stream_ << '\"' << key << "\": ";
      return writer_->open_sub_object();
    }

   private:
    friend class JsonWriter;

    explicit JsonObject(JsonWriter* writer sus_lifetimebound) noexcept
        : writer_(writer) {
      writer_->indent_ += 2u;
      writer_->stream_ << "{\n";
    }

    bool wrote_one_ = false;
    JsonWriter* writer_;
  };

  class JsonArray {
   public:
    ~JsonArray() noexcept {
      writer_->indent_ = writer_->indent_.saturating_sub(2u);
      writer_->stream_ << '\n';
      for (auto x : sus::ops::range(0_u32, writer_->indent_)) {
        writer_->stream_ << ' ';
      }
      writer_->stream_ << ']';
    }

    JsonObject open_object() noexcept {
      if (len_ > 0) writer_->stream_ << ",\n";
      len_ += 1;
      for (auto x : sus::ops::range(0_u32, writer_->indent_)) {
        writer_->stream_ << ' ';
      }
      return writer_->open_sub_object();
    }

    void add_string(std::string_view string) noexcept {
      if (len_ > 0) writer_->stream_ << ",\n";
      len_ += 1;
      for (auto x : sus::ops::range(0_u32, writer_->indent_)) {
        writer_->stream_ << ' ';
      }
      writer_->stream_ << '\"' << string << '\"';
    }

    void add_int(i32 i) noexcept {
      if (len_ > 0) writer_->stream_ << ",\n";
      len_ += 1;
      for (auto x : sus::ops::range(0_u32, writer_->indent_)) {
        writer_->stream_ << ' ';
      }
      writer_->stream_ << i;
    }

    i32 len() const noexcept { return len_; }

   private:
    friend class JsonWriter;

    explicit JsonArray(JsonWriter* writer sus_lifetimebound) noexcept
        : writer_(writer) {
      writer_->indent_ += 2u;
      writer_->stream_ << "[\n";
    }

    i32 len_;
    JsonWriter* writer_;
  };

  explicit JsonWriter(Option<std::string> varname,
                      std::ofstream stream) noexcept
      : stream_(sus::move(stream)) {
    if (varname.is_some()) {
      stream_ << "const " << varname.as_value() << " = ";
    }
  }
  ~JsonWriter() noexcept = default;

  JsonArray open_array() noexcept {
    sus_check_with_message(!wrote_one_,
                           "JsonWriter can only open a single root object");
    wrote_one_ = true;
    return open_sub_array();
  }
  JsonObject open_object() noexcept {
    sus_check_with_message(!wrote_one_,
                           "JsonWriter can only open a single root object");
    wrote_one_ = true;
    return open_sub_object();
  }

 private:
  JsonArray open_sub_array() { return JsonArray(this); }
  JsonObject open_sub_object() { return JsonObject(this); }

  bool wrote_one_ = false;
  u32 indent_;
  std::ofstream stream_;
};

}  // namespace subdoc::gen
