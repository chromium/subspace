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
  enum NewlineStrategy {
    SingleLine,
    MultiLine,
  };

  class [[nodiscard]] Html {
   public:
    void add_class(std::string_view c) noexcept {
      classes_.push(std::string(c));
    }

    void write_text(std::string_view text) noexcept {
      write_open();
      writer_.write_text(text, /*has_newlines=*/has_newlines_);
    }
    void write_html(std::string_view text) noexcept {
      write_open();
      writer_.write_html(text, /*has_newlines=*/has_newlines_);
    }

    auto open_div() noexcept {
      write_open();
      return writer_.open_div(has_newlines_);
    }
    auto open_span(NewlineStrategy newlines = MultiLine) noexcept {
      write_open();
      return writer_.open_span(has_newlines_, newlines);
    }
    auto open_a() noexcept {
      write_open();
      return writer_.open_a(has_newlines_);
    }

   protected:
    Html(HtmlWriter& writer) noexcept : writer_(writer) {}

    virtual void write_open() = 0;

    HtmlWriter& writer_;
    sus::Vec<std::string> classes_;
    sus::Vec<HtmlAttribute> attributes_;
    bool wrote_open_ = false;
    bool has_newlines_ = true;
    bool inside_has_newlines_ = true;
  };

  class [[nodiscard]] OpenA : public Html {
   public:
    ~OpenA() noexcept {
      write_open();
      writer_.write_close("a", inside_has_newlines_, has_newlines_);
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
    void add_title(std::string_view title) {
      attributes_.push(HtmlAttribute{
          .name = std::string("title"),
          .value = std::string(title),
      });
    }

   private:
    friend HtmlWriter;
    OpenA(HtmlWriter& writer, bool inside_has_newlines) noexcept
        : Html(writer) {
      // Avoid newlines in <a> tags as they extend the link decoration
      // into whitespace.
      has_newlines_ = false;
      inside_has_newlines_ = inside_has_newlines;
    }

    void write_open() noexcept override {
      if (!wrote_open_) {
        writer_.write_open("a", classes_.iter(), attributes_.iter(),
                           inside_has_newlines_, has_newlines_);
        wrote_open_ = true;
      }
    }
  };

  class [[nodiscard]] OpenDiv : public Html {
   public:
    ~OpenDiv() noexcept {
      write_open();
      writer_.write_close("div", inside_has_newlines_, has_newlines_);
    }

    void add_title(std::string_view title) {
      attributes_.push(HtmlAttribute{
          .name = std::string("title"),
          .value = std::string(title),
      });
    }

   private:
    friend HtmlWriter;
    OpenDiv(HtmlWriter& writer, bool inside_has_newlines) noexcept
        : Html(writer) {
      inside_has_newlines_ = inside_has_newlines;
    }

    void write_open() noexcept override {
      if (!wrote_open_) {
        writer_.write_open("div", classes_.iter(), attributes_.iter(),
                           inside_has_newlines_, has_newlines_);
        wrote_open_ = true;
      }
    }
  };

  class [[nodiscard]] OpenSpan : public Html {
   public:
    ~OpenSpan() noexcept {
      write_open();
      writer_.write_close("span", inside_has_newlines_, has_newlines_);
    }

    void add_title(std::string_view title) {
      attributes_.push(HtmlAttribute{
          .name = std::string("title"),
          .value = std::string(title),
      });
    }

   private:
    friend HtmlWriter;
    OpenSpan(HtmlWriter& writer, bool inside_has_newlines,
             NewlineStrategy newlines) noexcept
        : Html(writer) {
      has_newlines_ = newlines == MultiLine;
      inside_has_newlines_ = inside_has_newlines;
    }

    void write_open() noexcept override {
      if (!wrote_open_) {
        writer_.write_open("span", classes_.iter(), attributes_.iter(),
                           inside_has_newlines_, has_newlines_);
        wrote_open_ = true;
      }
    }
  };

  class [[nodiscard]] OpenBody : public Html {
   public:
    ~OpenBody() noexcept {
      write_open();
      writer_.write_close("body", inside_has_newlines_, has_newlines_);
    }

   private:
    friend HtmlWriter;
    OpenBody(HtmlWriter& writer) noexcept : Html(writer) {}

    void write_open() noexcept override {
      if (!wrote_open_) {
        writer_.write_open("body", classes_.iter(), attributes_.iter(),
                           inside_has_newlines_, has_newlines_);
        wrote_open_ = true;
      }
    }
  };

  class [[nodiscard]] OpenTitle : public Html {
   public:
    ~OpenTitle() noexcept {
      write_open();
      writer_.write_close("title", inside_has_newlines_, has_newlines_);
    }

   private:
    friend HtmlWriter;
    OpenTitle(HtmlWriter& writer, bool inside_has_newlines) noexcept
        : Html(writer) {
      inside_has_newlines_ = inside_has_newlines;
    }

    void write_open() noexcept override {
      if (!wrote_open_) {
        writer_.write_open("title", classes_.iter(), attributes_.iter(),
                           inside_has_newlines_, has_newlines_);
        wrote_open_ = true;
      }
    }
  };

  class [[nodiscard]] OpenLink : public Html {
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
    OpenLink(HtmlWriter& writer, bool inside_has_newlines) noexcept
        : Html(writer) {
      inside_has_newlines_ = inside_has_newlines;
    }

    void write_open() noexcept override {
      if (!wrote_open_) {
        writer_.write_open("link", classes_.iter(), attributes_.iter(),
                           inside_has_newlines_, has_newlines_);
        wrote_open_ = true;
      }
    }

    sus::Vec<HtmlAttribute> attributes_;
  };

  class [[nodiscard]] OpenHead : public Html {
   public:
    ~OpenHead() noexcept {
      write_open();
      writer_.write_close("head", inside_has_newlines_, has_newlines_);
    }

    auto open_title() noexcept {
      write_open();
      return writer_.open_title(has_newlines_);
    }
    auto open_link() noexcept {
      write_open();
      return writer_.open_link(has_newlines_);
    }

   private:
    friend HtmlWriter;
    OpenHead(HtmlWriter& writer) noexcept : Html(writer) {}

    void write_open() noexcept override {
      if (!wrote_open_) {
        writer_.write_open("head", classes_.iter(), attributes_.iter(),
                           inside_has_newlines_, has_newlines_);
        wrote_open_ = true;
      }
    }
  };

  explicit HtmlWriter(std::ofstream stream) noexcept
      : stream_(sus::move(stream)) {}
  ~HtmlWriter() noexcept { stream_.close(); }

  OpenBody open_body() noexcept { return OpenBody(*this); }
  OpenHead open_head() noexcept { return OpenHead(*this); }

  void write_empty_line() noexcept { stream_ << "\n"; }

 private:
  friend class OpenDiv;
  friend class OpenSpan;

  OpenDiv open_div(bool inside_has_newlines) noexcept {
    return OpenDiv(*this, inside_has_newlines);
  }
  OpenSpan open_span(bool inside_has_newlines,
                     NewlineStrategy newlines) noexcept {
    return OpenSpan(*this, inside_has_newlines, newlines);
  }
  OpenA open_a(bool inside_has_newlines) noexcept {
    return OpenA(*this, inside_has_newlines);
  }
  OpenTitle open_title(bool inside_has_newlines) noexcept {
    return OpenTitle(*this, inside_has_newlines);
  }
  OpenLink open_link(bool inside_has_newlines) noexcept {
    return OpenLink(*this, inside_has_newlines);
  }

  // Quote any <>.
  static sus::Option<std::string> quote_angle_brackets(std::string_view text) {
    size_t pos = text.find_first_of("<>");
    if (pos == std::string::npos) {
      return sus::none();
    }
    auto copy = std::string(text);
    while (pos != std::string::npos) {
      if (copy[pos] == '<')
        copy.replace(pos, 1u, "&lt;");
      else
        copy.replace(pos, 1u, "&gt;");

      pos = copy.find_first_of("<>");
    }
    return sus::some(sus::move(copy));
  }

  void write_text(std::string_view text, bool has_newlines = true) noexcept {
    if (!text.empty()) {
      if (has_newlines) write_indent();

      sus::Option<std::string> quoted = quote_angle_brackets(text);
      switch (quoted) {
        case sus::Some: stream_ << *quoted; break;
        case sus::None: stream_ << text; break;
      }
      if (has_newlines) stream_ << "\n";
    }
  }
  void write_html(std::string_view html, bool has_newlines = true) noexcept {
    if (!html.empty()) {
      if (has_newlines) write_indent();
      stream_ << html;
      if (has_newlines) stream_ << "\n";
    }
  }

  void write_open(
      std::string_view type,
      sus::iter::Iterator<const std::string&> auto classes_iter,
      sus::iter::Iterator<const HtmlAttribute&> auto attr_iter,
      bool inside_has_newlines, bool has_newlines) noexcept {
    if (inside_has_newlines) write_indent();
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
      stream_ << " " << attr.name << "=\"";
      sus::Option<std::string> quoted = quote_angle_brackets(attr.value);
      switch (quoted) {
        case sus::Some: stream_ << *quoted; break;
        case sus::None: stream_ << attr.value; break;
      }
      stream_ << "\"";
    }
    stream_ << ">";
    if (has_newlines) stream_ << "\n";
    indent_ += 2u;
  }
  void skip_close() noexcept { indent_ -= 2u; }
  void write_close(std::string_view type, bool inside_has_newlines,
                   bool has_newlines) noexcept {
    indent_ -= 2u;
    if (has_newlines) write_indent();
    stream_ << "</" << type << ">";
    if (inside_has_newlines) stream_ << "\n";
  }
  void write_indent() noexcept {
    for (auto i = 0_u32; i < indent_; i += 1u) stream_ << " ";
  }

  u32 indent_ = 0_u32;
  std::ofstream stream_;
};

}  // namespace subdoc::gen
