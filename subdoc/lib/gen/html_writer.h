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

#include "subspace/prelude.h"

namespace subdoc::gen {

class HtmlWriter;

class HtmlWriter {
 public:
  template <class T>
  class [[nodiscard]] Html {
   public:
    void add_class(std::string_view c) noexcept {
      // TODO: Add Vec::back() and use it.
      classes_.push(std::string(c));
    }

    void write_text(std::string_view text) noexcept {
      write_open();
      writer_.write_text(text);
    }

    auto open_div() noexcept {
      write_open();
      return writer_.open_div();
    }
    auto open_span() noexcept {
      write_open();
      return writer_.open_span();
    }

   protected:
    Html(HtmlWriter& writer) noexcept : writer_(writer) {}

    virtual void write_open() = 0;

    HtmlWriter& writer_;
    sus::Vec<std::string> classes_;
    bool wrote_open_ = false;
  };

  class [[nodiscard]] OpenDiv : public Html<OpenDiv> {
   public:
    ~OpenDiv() noexcept {
      write_open();
      writer_.write_close("div");
    }

   private:
    friend HtmlWriter;
    OpenDiv(HtmlWriter& writer) noexcept : Html(writer) {}

    void write_open() noexcept override {
      if (!wrote_open_) {
        writer_.write_open("div", classes_.iter());
        wrote_open_ = true;
      }
    }
  };

  class [[nodiscard]] OpenSpan : public Html<OpenDiv> {
   public:
    ~OpenSpan() noexcept {
      write_open();
      writer_.write_close("span");
    }

   private:
    friend HtmlWriter;
    OpenSpan(HtmlWriter& writer) noexcept : Html(writer) {}

    void write_open() noexcept override {
      if (!wrote_open_) {
        writer_.write_open("span", classes_.iter());
        wrote_open_ = true;
      }
    }
  };

  class [[nodiscard]] OpenBody : public Html<OpenBody> {
   public:
    ~OpenBody() noexcept {
      write_open();
      writer_.write_close("body");
    }

   private:
    friend HtmlWriter;
    OpenBody(HtmlWriter& writer) noexcept : Html(writer) {}

    void write_open() noexcept override {
      if (!wrote_open_) {
        writer_.write_open("body", classes_.iter());
        wrote_open_ = true;
      }
    }
  };

  explicit HtmlWriter(std::ofstream stream) noexcept
      : stream_(sus::move(stream)) {}
  ~HtmlWriter() noexcept { stream_.close(); }

  OpenBody open_body() noexcept { return OpenBody(*this); }

 private:
  friend class OpenDiv;
  friend class OpenSpan;

  OpenDiv open_div() noexcept { return OpenDiv(*this); }
  OpenSpan open_span() noexcept { return OpenSpan(*this); }

  void write_text(std::string_view text) noexcept {
    write_indent();
    stream_ << text << "\n";
  }

  // TODO: Add an Iterator<T> concept and use that to know what T is here.
  template <class Iter>
  void write_open(std::string_view type, Iter classes_iter) noexcept {
    write_indent();
    stream_ << "<" << type;
    if (sus::Option<const std::string&> first_class = classes_iter.next();
        first_class.is_some()) {
      stream_ << " class=\"" << sus::move(first_class).unwrap();
      for (const std::string& c : classes_iter) {
        stream_ << " " << c;
      }
      stream_ << "\"";
    }
    stream_ << ">\n";
    indent_ += 2u;
  }
  void write_close(std::string_view type) noexcept {
    indent_ -= 2u;
    write_indent();
    stream_ << "</" << type << ">\n";
  }
  void write_indent() noexcept {
    for (auto i = 0_u32; i < indent_; i += 1u) stream_ << " ";
  }

  u32 indent_ = 0_u32;
  std::ofstream stream_;
};

}  // namespace subdoc::gen
