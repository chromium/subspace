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

#include "sus/iter/empty.h"
#include "sus/prelude.h"

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
    void add_id(std::string_view id) noexcept {
      attributes_.push(HtmlAttribute{
          .name = std::string("id"),
          .value = std::string(id),
      });
    }
    void add_search_weight(f32 weight) noexcept {
      attributes_.push(HtmlAttribute{
          .name = std::string("data-pagefind-weight"),
          .value = fmt::to_string(weight),
      });
    }

    void write_text(std::string_view text) noexcept {
      write_open();
      writer_.write_text(text, /*has_newlines=*/has_newlines_);
    }
    void write_html(std::string_view text) noexcept {
      write_open();
      writer_.write_html(text, /*has_newlines=*/has_newlines_);
    }

    auto open_main(NewlineStrategy newlines = MultiLine) noexcept {
      write_open();
      return writer_.open_main(has_newlines_, newlines);
    }
    auto open_section(NewlineStrategy newlines = MultiLine) noexcept {
      write_open();
      return writer_.open_section(has_newlines_, newlines);
    }
    auto open_div(NewlineStrategy newlines = MultiLine) noexcept {
      write_open();
      return writer_.open_div(has_newlines_, newlines);
    }
    auto open_form(NewlineStrategy newlines = MultiLine) noexcept {
      write_open();
      return writer_.open_form(has_newlines_, newlines);
    }
    auto open_h(u32 level, NewlineStrategy newlines = MultiLine) noexcept {
      write_open();
      return writer_.open_h(level, has_newlines_, newlines);
    }
    auto open_search(NewlineStrategy newlines = MultiLine) noexcept {
      write_open();
      return writer_.open_search(has_newlines_, newlines);
    }
    auto open_nav(NewlineStrategy newlines = MultiLine) noexcept {
      write_open();
      return writer_.open_nav(has_newlines_, newlines);
    }
    auto open_span(NewlineStrategy newlines = MultiLine) noexcept {
      write_open();
      return writer_.open_span(has_newlines_, newlines);
    }
    auto open_button() noexcept {
      write_open();
      return writer_.open_button(has_newlines_);
    }
    auto open_ul() noexcept {
      write_open();
      return writer_.open_ul(has_newlines_);
    }
    auto open_pre() noexcept {
      write_open();
      return writer_.open_pre(has_newlines_);
    }
    auto open_a() noexcept {
      write_open();
      return writer_.open_a(has_newlines_);
    }
    auto open_img() noexcept {
      write_open();
      return writer_.open_img(has_newlines_);
    }

   protected:
    Html(HtmlWriter& writer) noexcept : writer_(writer) {}

    virtual void write_open() = 0;

    HtmlWriter& writer_;
    Vec<std::string> classes_;
    Vec<HtmlAttribute> attributes_;
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

  class [[nodiscard]] OpenImg : public Html {
   public:
    ~OpenImg() noexcept {
      write_open();
      writer_.write_close("img", inside_has_newlines_, has_newlines_);
    }

    void add_src(std::string_view href) {
      attributes_.push(HtmlAttribute{
          .name = std::string("src"),
          .value = std::string(href),
      });
    }

   private:
    friend HtmlWriter;
    OpenImg(HtmlWriter& writer, bool inside_has_newlines) noexcept
        : Html(writer) {
      has_newlines_ = false;
      inside_has_newlines_ = inside_has_newlines;
    }

    void write_open() noexcept override {
      if (!wrote_open_) {
        writer_.write_open("img", classes_.iter(), attributes_.iter(),
                           inside_has_newlines_, has_newlines_);
        wrote_open_ = true;
      }
    }
  };

  class [[nodiscard]] OpenMeta : public Html {
   public:
    ~OpenMeta() noexcept {
      write_open();
      writer_.write_close("meta", inside_has_newlines_, has_newlines_);
    }

    void add_property(std::string_view value) {
      attributes_.push(HtmlAttribute{
          .name = std::string("property"),
          .value = std::string(value),
      });
    }

    void add_content(std::string_view value) {
      attributes_.push(HtmlAttribute{
          .name = std::string("content"),
          .value = std::string(value),
      });
    }

    void add_name(std::string_view value) {
      attributes_.push(HtmlAttribute{
          .name = std::string("name"),
          .value = std::string(value),
      });
    }

   private:
    friend HtmlWriter;
    OpenMeta(HtmlWriter& writer, bool inside_has_newlines) noexcept
        : Html(writer) {
      has_newlines_ = false;
      inside_has_newlines_ = inside_has_newlines;
    }

    void write_open() noexcept override {
      if (!wrote_open_) {
        writer_.write_open("meta", classes_.iter(), attributes_.iter(),
                           inside_has_newlines_, has_newlines_);
        wrote_open_ = true;
      }
    }
  };

  class [[nodiscard]] OpenMain : public Html {
   public:
    ~OpenMain() noexcept {
      write_open();
      writer_.write_close("main", inside_has_newlines_, has_newlines_);
    }

    void add_title(std::string_view title) {
      attributes_.push(HtmlAttribute{
          .name = std::string("title"),
          .value = std::string(title),
      });
    }

   private:
    friend HtmlWriter;
    OpenMain(HtmlWriter& writer, bool inside_has_newlines,
             NewlineStrategy newlines) noexcept
        : Html(writer) {
      has_newlines_ = newlines == MultiLine;
      inside_has_newlines_ = inside_has_newlines;
    }

    void write_open() noexcept override {
      if (!wrote_open_) {
        writer_.write_open("main", classes_.iter(), attributes_.iter(),
                           inside_has_newlines_, has_newlines_);
        wrote_open_ = true;
      }
    }
  };

  class [[nodiscard]] OpenSection : public Html {
   public:
    ~OpenSection() noexcept {
      write_open();
      writer_.write_close("section", inside_has_newlines_, has_newlines_);
    }

   private:
    friend HtmlWriter;
    OpenSection(HtmlWriter& writer, bool inside_has_newlines,
            NewlineStrategy newlines) noexcept
        : Html(writer) {
      has_newlines_ = newlines == MultiLine;
      inside_has_newlines_ = inside_has_newlines;
    }

    void write_open() noexcept override {
      if (!wrote_open_) {
        writer_.write_open("section", classes_.iter(), attributes_.iter(),
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

   private:
    friend HtmlWriter;
    OpenDiv(HtmlWriter& writer, bool inside_has_newlines,
            NewlineStrategy newlines) noexcept
        : Html(writer) {
      has_newlines_ = newlines == MultiLine;
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

  class [[nodiscard]] OpenForm : public Html {
   public:
    ~OpenForm() noexcept {
      write_open();
      writer_.write_close("form", inside_has_newlines_, has_newlines_);
    }

    void add_action(std::string_view action) {
      attributes_.push(HtmlAttribute{
          .name = std::string("action"),
          .value = std::string(action),
      });
    }

    auto open_input(NewlineStrategy newlines = MultiLine) noexcept {
      write_open();
      return writer_.open_input(has_newlines_, newlines);
    }

   private:
    friend HtmlWriter;
    OpenForm(HtmlWriter& writer, bool inside_has_newlines,
            NewlineStrategy newlines) noexcept
        : Html(writer) {
      has_newlines_ = newlines == MultiLine;
      inside_has_newlines_ = inside_has_newlines;
    }

    void write_open() noexcept override {
      if (!wrote_open_) {
        writer_.write_open("form", classes_.iter(), attributes_.iter(),
                           inside_has_newlines_, has_newlines_);
        wrote_open_ = true;
      }
    }
  };

  class [[nodiscard]] OpenH : public Html {
   public:
    ~OpenH() noexcept {
      write_open();
      writer_.write_close(fmt::format("h{}", level_), inside_has_newlines_,
                          has_newlines_);
    }

    void add_title(std::string_view title) {
      attributes_.push(HtmlAttribute{
          .name = std::string("title"),
          .value = std::string(title),
      });
    }

   private:
    friend HtmlWriter;
    OpenH(HtmlWriter& writer, u32 level, bool inside_has_newlines,
          NewlineStrategy newlines) noexcept
        : Html(writer), level_(level) {
      sus_check(level_ <= 7u);
      has_newlines_ = newlines == MultiLine;
      inside_has_newlines_ = inside_has_newlines;
    }

    void write_open() noexcept override {
      if (!wrote_open_) {
        writer_.write_open(fmt::format("h{}", level_), classes_.iter(),
                           attributes_.iter(), inside_has_newlines_,
                           has_newlines_);
        wrote_open_ = true;
      }
    }

    u32 level_;
  };

  class [[nodiscard]] OpenSearch : public Html {
   public:
    ~OpenSearch() noexcept {
      write_open();
      writer_.write_close("search", inside_has_newlines_, has_newlines_);
    }

   private:
    friend HtmlWriter;
    OpenSearch(HtmlWriter& writer, bool inside_has_newlines,
               NewlineStrategy newlines) noexcept
        : Html(writer) {
      has_newlines_ = newlines == MultiLine;
      inside_has_newlines_ = inside_has_newlines;
    }

    void write_open() noexcept override {
      if (!wrote_open_) {
        writer_.write_open("search", classes_.iter(), attributes_.iter(),
                           inside_has_newlines_, has_newlines_);
        wrote_open_ = true;
      }
    }
  };

  class [[nodiscard]] OpenNav : public Html {
   public:
    ~OpenNav() noexcept {
      write_open();
      writer_.write_close("nav", inside_has_newlines_, has_newlines_);
    }

    void add_title(std::string_view title) {
      attributes_.push(HtmlAttribute{
          .name = std::string("title"),
          .value = std::string(title),
      });
    }

   private:
    friend HtmlWriter;
    OpenNav(HtmlWriter& writer, bool inside_has_newlines,
            NewlineStrategy newlines) noexcept
        : Html(writer) {
      has_newlines_ = newlines == MultiLine;
      inside_has_newlines_ = inside_has_newlines;
    }

    void write_open() noexcept override {
      if (!wrote_open_) {
        writer_.write_open("nav", classes_.iter(), attributes_.iter(),
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

  class [[nodiscard]] OpenInput : public Html {
   public:
    ~OpenInput() noexcept {
      write_open();
      writer_.write_close("input", inside_has_newlines_, has_newlines_);
    }

    void add_type(std::string_view type) {
      attributes_.push(HtmlAttribute{
          .name = std::string("type"),
          .value = std::string(type),
      });
    }
    void add_name(std::string_view name) {
      attributes_.push(HtmlAttribute{
          .name = std::string("name"),
          .value = std::string(name),
      });
    }
    void add_autocomplete(std::string_view autocomplete) {
      attributes_.push(HtmlAttribute{
          .name = std::string("autocomplete"),
          .value = std::string(autocomplete),
      });
    }
    void add_spellcheck(std::string_view spellcheck) {
      attributes_.push(HtmlAttribute{
          .name = std::string("spellcheck"),
          .value = std::string(spellcheck),
      });
    }
    void add_placeholder(std::string_view placeholder) {
      attributes_.push(HtmlAttribute{
          .name = std::string("placeholder"),
          .value = std::string(placeholder),
      });
    }
    void add_onfocus(std::string_view onfocus) {
      attributes_.push(HtmlAttribute{
          .name = std::string("onfocus"),
          .value = std::string(onfocus),
      });
    }
    void add_onblur(std::string_view onblur) {
      attributes_.push(HtmlAttribute{
          .name = std::string("onblur"),
          .value = std::string(onblur),
      });
    }

   private:
    friend HtmlWriter;
    OpenInput(HtmlWriter& writer, bool inside_has_newlines,
             NewlineStrategy newlines) noexcept
        : Html(writer) {
      has_newlines_ = newlines == MultiLine;
      inside_has_newlines_ = inside_has_newlines;
    }

    void write_open() noexcept override {
      if (!wrote_open_) {
        writer_.write_open("input", classes_.iter(), attributes_.iter(),
                           inside_has_newlines_, has_newlines_);
        wrote_open_ = true;
      }
    }
  };

  class [[nodiscard]] OpenButton : public Html {
   public:
    ~OpenButton() noexcept {
      write_open();
      writer_.write_close("button", inside_has_newlines_, has_newlines_);
    }

    void add_onclick(std::string_view script) {
      attributes_.push(HtmlAttribute{
          .name = std::string("onclick"),
          .value = std::string(script),
      });
    }

   private:
    friend HtmlWriter;
    OpenButton(HtmlWriter& writer, bool inside_has_newlines) noexcept
        : Html(writer) {
      inside_has_newlines_ = inside_has_newlines;
    }

    void write_open() noexcept override {
      if (!wrote_open_) {
        writer_.write_open("button", classes_.iter(), attributes_.iter(),
                           inside_has_newlines_, has_newlines_);
        wrote_open_ = true;
      }
    }
  };

  class [[nodiscard]] OpenUl : public Html {
   public:
    ~OpenUl() noexcept {
      write_open();
      writer_.write_close("ul", inside_has_newlines_, has_newlines_);
    }

    auto open_li() noexcept {
      write_open();
      return writer_.open_li(has_newlines_);
    }

   private:
    friend HtmlWriter;
    OpenUl(HtmlWriter& writer, bool inside_has_newlines) noexcept
        : Html(writer) {
      inside_has_newlines_ = inside_has_newlines;
    }

    void write_open() noexcept override {
      if (!wrote_open_) {
        writer_.write_open("ul", classes_.iter(), attributes_.iter(),
                           inside_has_newlines_, has_newlines_);
        wrote_open_ = true;
      }
    }
  };

  class [[nodiscard]] OpenLi : public Html {
   public:
    ~OpenLi() noexcept {
      write_open();
      writer_.write_close("li", inside_has_newlines_, has_newlines_);
    }

   private:
    friend HtmlWriter;
    OpenLi(HtmlWriter& writer, bool inside_has_newlines) noexcept
        : Html(writer) {
      inside_has_newlines_ = inside_has_newlines;
    }

    void write_open() noexcept override {
      if (!wrote_open_) {
        writer_.write_open("li", classes_.iter(), attributes_.iter(),
                           inside_has_newlines_, has_newlines_);
        wrote_open_ = true;
      }
    }
  };

  class [[nodiscard]] OpenPre : public Html {
   public:
    ~OpenPre() noexcept {
      write_open();
      writer_.write_close("pre", inside_has_newlines_, has_newlines_);
    }

   private:
    friend HtmlWriter;
    OpenPre(HtmlWriter& writer, bool inside_has_newlines) noexcept
        : Html(writer) {
      // Don't add newlines from multiple `write_text()` calls inside a <pre>,
      // they change formatting.
      has_newlines_ = false;
      inside_has_newlines_ = inside_has_newlines;
    }

    void write_open() noexcept override {
      if (!wrote_open_) {
        writer_.write_open("pre", classes_.iter(), attributes_.iter(),
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
      has_newlines_ = false;
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

    void add_type(std::string_view type) {
      attributes_.push(HtmlAttribute{
          .name = std::string("type"),
          .value = std::string(type),
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

    Vec<HtmlAttribute> attributes_;
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
    auto open_meta() noexcept {
      write_open();
      return writer_.open_meta(has_newlines_);
    }
    auto open_script(NewlineStrategy newlines = MultiLine) noexcept {
      write_open();
      return writer_.open_script(has_newlines_, newlines);
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

  class [[nodiscard]] OpenScript : public Html {
   public:
    ~OpenScript() noexcept {
      write_open();
      writer_.write_close("script", inside_has_newlines_, has_newlines_);
    }

    void add_src(std::string_view src) {
      attributes_.push(HtmlAttribute{
          .name = std::string("src"),
          .value = std::string(src),
      });
    }

    void add_type(std::string_view type) {
      attributes_.push(HtmlAttribute{
          .name = std::string("type"),
          .value = std::string(type),
      });
    }

   private:
    friend HtmlWriter;
    OpenScript(HtmlWriter& writer, bool inside_has_newlines,
               NewlineStrategy newlines) noexcept
        : Html(writer) {
      has_newlines_ = newlines == MultiLine;
      inside_has_newlines_ = inside_has_newlines;
    }

    void write_open() noexcept override {
      if (!wrote_open_) {
        writer_.write_open("script", classes_.iter(), attributes_.iter(),
                           inside_has_newlines_, has_newlines_);
        wrote_open_ = true;
      }
    }
  };

  explicit HtmlWriter(std::ofstream stream) noexcept
      : stream_(sus::move(stream)) {
    stream_ << "<!DOCTYPE html>\n";
    write_open("html", sus::iter::empty<const std::string&>(),
               sus::iter::empty<const HtmlAttribute&>(), true, true);
  }
  ~HtmlWriter() noexcept {
    write_close("html", true, true);
    stream_.close();
  }

  OpenBody open_body() noexcept { return OpenBody(*this); }
  OpenHead open_head() noexcept { return OpenHead(*this); }

  void write_empty_line() noexcept { stream_ << "\n"; }

 private:
  friend class OpenDiv;
  friend class OpenSpan;
  friend class OpenUl;
  friend class OpenLi;

  OpenMain open_main(bool inside_has_newlines,
                     NewlineStrategy newlines) noexcept {
    return OpenMain(*this, inside_has_newlines, newlines);
  }
  OpenSection open_section(bool inside_has_newlines,
                   NewlineStrategy newlines) noexcept {
    return OpenSection(*this, inside_has_newlines, newlines);
  }
  OpenDiv open_div(bool inside_has_newlines,
                   NewlineStrategy newlines) noexcept {
    return OpenDiv(*this, inside_has_newlines, newlines);
  }
  OpenForm open_form(bool inside_has_newlines,
                   NewlineStrategy newlines) noexcept {
    return OpenForm(*this, inside_has_newlines, newlines);
  }
  OpenH open_h(u32 level, bool inside_has_newlines,
               NewlineStrategy newlines) noexcept {
    return OpenH(*this, level, inside_has_newlines, newlines);
  }
  OpenSearch open_search(bool inside_has_newlines,
                         NewlineStrategy newlines) noexcept {
    return OpenSearch(*this, inside_has_newlines, newlines);
  }
  OpenNav open_nav(bool inside_has_newlines,
                   NewlineStrategy newlines) noexcept {
    return OpenNav(*this, inside_has_newlines, newlines);
  }
  OpenSpan open_span(bool inside_has_newlines,
                     NewlineStrategy newlines) noexcept {
    return OpenSpan(*this, inside_has_newlines, newlines);
  }
  OpenInput open_input(bool inside_has_newlines,
                     NewlineStrategy newlines) noexcept {
    return OpenInput(*this, inside_has_newlines, newlines);
  }
  OpenButton open_button(bool inside_has_newlines) noexcept {
    return OpenButton(*this, inside_has_newlines);
  }
  OpenUl open_ul(bool inside_has_newlines) noexcept {
    return OpenUl(*this, inside_has_newlines);
  }
  OpenLi open_li(bool inside_has_newlines) noexcept {
    return OpenLi(*this, inside_has_newlines);
  }
  OpenPre open_pre(bool inside_has_newlines) noexcept {
    return OpenPre(*this, inside_has_newlines);
  }
  OpenA open_a(bool inside_has_newlines) noexcept {
    return OpenA(*this, inside_has_newlines);
  }
  OpenImg open_img(bool inside_has_newlines) noexcept {
    return OpenImg(*this, inside_has_newlines);
  }
  OpenTitle open_title(bool inside_has_newlines) noexcept {
    return OpenTitle(*this, inside_has_newlines);
  }
  OpenLink open_link(bool inside_has_newlines) noexcept {
    return OpenLink(*this, inside_has_newlines);
  }
  OpenMeta open_meta(bool inside_has_newlines) noexcept {
    return OpenMeta(*this, inside_has_newlines);
  }
  OpenScript open_script(bool inside_has_newlines,
                         NewlineStrategy newlines) noexcept {
    return OpenScript(*this, inside_has_newlines, newlines);
  }

  // Quote any <>.
  static Option<std::string> quote_angle_brackets(std::string_view text) {
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

      Option<std::string> quoted = quote_angle_brackets(text);
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

  void write_open(std::string_view type,
                  sus::iter::Iterator<const std::string&> auto classes_iter,
                  sus::iter::Iterator<const HtmlAttribute&> auto attr_iter,
                  bool inside_has_newlines, bool has_newlines) noexcept {
    if (inside_has_newlines) write_indent();
    stream_ << "<" << type;
    if (Option<const std::string&> first_class = classes_iter.next();
        first_class.is_some()) {
      stream_ << " class=\"" << sus::move(first_class).unwrap();
      for (const std::string& c : classes_iter) {
        stream_ << " " << c;
      }
      stream_ << "\"";
    }
    for (const HtmlAttribute& attr : attr_iter) {
      stream_ << " " << attr.name << "=\"";
      Option<std::string> quoted = quote_angle_brackets(attr.value);
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
