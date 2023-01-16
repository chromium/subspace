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

#include "subspace/iter/empty.h"
#include "subspace/prelude.h"

namespace subdoc::gen {

struct HtmlAttribute {
  std::string name;
  std::string value;
};

class HtmlWriter {
 public:
  template <class T>
  class [[nodiscard]] Html {
   public:
    void add_class(std::string_view c) noexcept {
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
    auto open_a() noexcept {
      write_open();
      return writer_.open_a();
    }

   protected:
    Html(HtmlWriter& writer) noexcept : writer_(writer) {}

    virtual void write_open() = 0;

    HtmlWriter& writer_;
    sus::Vec<std::string> classes_;
    bool wrote_open_ = false;
  };

  class [[nodiscard]] OpenA : public Html<OpenA> {
   public:
    ~OpenA() noexcept {
      write_open();
      writer_.write_close("a");
    }

    void add_href(std::string_view href) {
      attributes_.push(HtmlAttribute{
          .name = std::string("href"),
          .value = std::string(href),
      });
    }
    void add_name(std::string_view name) {
      attributes_.push(HtmlAttribute{
          .name = std::string("name"),
          .value = std::string(name),
      });
    }

   private:
    friend HtmlWriter;
    OpenA(HtmlWriter& writer) noexcept : Html(writer) {}

    void write_open() noexcept override {
      if (!wrote_open_) {
        writer_.write_open("a", classes_.iter(), attributes_.iter());
        wrote_open_ = true;
      }
    }

    sus::Vec<HtmlAttribute> attributes_;
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
        writer_.write_open("div", classes_.iter(),
                           sus::iter::empty<const HtmlAttribute&>());
        wrote_open_ = true;
      }
    }
  };

  class [[nodiscard]] OpenSpan : public Html<OpenSpan> {
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
        writer_.write_open("span", classes_.iter(),
                           sus::iter::empty<const HtmlAttribute&>());
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
        writer_.write_open("body", classes_.iter(),
                           sus::iter::empty<const HtmlAttribute&>());
        wrote_open_ = true;
      }
    }
  };

  class [[nodiscard]] OpenTitle : public Html<OpenTitle> {
   public:
    ~OpenTitle() noexcept {
      write_open();
      writer_.write_close("title");
    }

   private:
    friend HtmlWriter;
    OpenTitle(HtmlWriter& writer) noexcept : Html(writer) {}

    void write_open() noexcept override {
      if (!wrote_open_) {
        writer_.write_open("title", classes_.iter(),
                           sus::iter::empty<const HtmlAttribute&>());
        wrote_open_ = true;
      }
    }
  };

  class [[nodiscard]] OpenLink : public Html<OpenLink> {
   public:
    ~OpenLink() noexcept {
      write_open();
      // There's no closing tag for a <link> since it is empty.
      writer_.skip_close();
    }

    void add_rel(std::string_view rel) {
      attributes_.push(HtmlAttribute{
          .name = std::string("rel"),
          .value = std::string(rel),
      });
    }

    void add_href(std::string_view href) {
      attributes_.push(HtmlAttribute{
          .name = std::string("href"),
          .value = std::string(href),
      });
    }

   private:
    friend HtmlWriter;
    OpenLink(HtmlWriter& writer) noexcept : Html(writer) {}

    void write_open() noexcept override {
      if (!wrote_open_) {
        writer_.write_open("link", classes_.iter(), attributes_.iter());
        wrote_open_ = true;
      }
    }

    sus::Vec<HtmlAttribute> attributes_;
  };

  class [[nodiscard]] OpenHead : public Html<OpenHead> {
   public:
    ~OpenHead() noexcept {
      write_open();
      writer_.write_close("head");
    }

    auto open_title() noexcept {
      write_open();
      return writer_.open_title();
    }
    auto open_link() noexcept {
      write_open();
      return writer_.open_link();
    }

   private:
    friend HtmlWriter;
    OpenHead(HtmlWriter& writer) noexcept : Html(writer) {}

    void write_open() noexcept override {
      if (!wrote_open_) {
        writer_.write_open("head", classes_.iter(),
                           sus::iter::empty<const HtmlAttribute&>());
        wrote_open_ = true;
      }
    }
  };

  explicit HtmlWriter(std::ofstream stream) noexcept
      : stream_(sus::move(stream)) {}
  ~HtmlWriter() noexcept { stream_.close(); }

  OpenBody open_body() noexcept { return OpenBody(*this); }
  OpenHead open_head() noexcept { return OpenHead(*this); }

 private:
  friend class OpenDiv;
  friend class OpenSpan;

  OpenDiv open_div() noexcept { return OpenDiv(*this); }
  OpenSpan open_span() noexcept { return OpenSpan(*this); }
  OpenA open_a() noexcept { return OpenA(*this); }
  OpenTitle open_title() noexcept { return OpenTitle(*this); }
  OpenLink open_link() noexcept { return OpenLink(*this); }

  void write_text(std::string_view text) noexcept {
    write_indent();
    stream_ << text << "\n";
  }

  // TODO: Add an Iterator<T> concept and use that to know what Item is here.
  template <template <class Item> class ClassIter,
            template <class Item> class AttrIter>
  void write_open(
      std::string_view type,
      sus::iter::Iterator<ClassIter<const std::string&>> classes_iter,
      sus::iter::Iterator<AttrIter<const HtmlAttribute&>> attr_iter) noexcept {
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
    for (const HtmlAttribute& attr : attr_iter) {
      stream_ << " " << attr.name << "=\"" << attr.value << "\"";
    }
    stream_ << ">\n";
    indent_ += 2u;
  }
  void skip_close() noexcept { indent_ -= 2u; }
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
