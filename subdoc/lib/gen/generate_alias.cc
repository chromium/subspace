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

#include "subdoc/lib/gen/generate_alias.h"

#include "subdoc/lib/database.h"
#include "subdoc/lib/gen/files.h"
#include "subdoc/lib/gen/generate_requires.h"
#include "subdoc/lib/gen/generate_type.h"
#include "subdoc/lib/gen/html_writer.h"
#include "subdoc/lib/gen/markdown_to_html.h"
#include "subdoc/lib/gen/options.h"
#include "subdoc/lib/parse_comment.h"
#include "sus/prelude.h"

namespace subdoc::gen {

namespace {

using SortedAliasByName =
    sus::Tuple<std::string_view, /* sort_key */ u32, AliasId>;

/// Compares two `SortedAliasByName` for ordering. It compares by ignoring
/// the `AliasId` (which is not `Ord`).
constexpr inline std::weak_ordering cmp_alias_by_name(
    const SortedAliasByName& a, const SortedAliasByName& b) noexcept {
  // Name comes first.
  auto ord0 = a.at<0>() <=> b.at<0>();
  if (ord0 != 0) return ord0;
  // Then the item sort key.
  return a.at<1>() <=> b.at<1>();
}

}  // namespace

sus::Result<void, MarkdownToHtmlError> generate_alias_reference(
    HtmlWriter::OpenUl& items_list, const AliasElement& element,
    ParseMarkdownPageState& page_state) noexcept {
  auto item_li = items_list.open_li();
  item_li.add_class("section-item");

  {
    auto item_div = item_li.open_div();
    item_div.add_class("item-name");

    auto type_sig_div = item_div.open_div(HtmlWriter::SingleLine);
    type_sig_div.add_class("type-signature");

    {
      auto anchor = type_sig_div.open_a();
      anchor.add_name(construct_html_url_anchor_for_alias(element));
    }
    if (!element.hidden()) {
      if (sus::Option<std::string> url = construct_html_url_for_alias(element);
          url.is_some()) {
        auto anchor = type_sig_div.open_a();
        anchor.add_class("type-name");
        anchor.add_href(sus::move(url).unwrap());
        anchor.write_text(element.name);
      } else {
        auto span = type_sig_div.open_span(HtmlWriter::SingleLine);
        span.add_class("type-name");
        span.write_text(element.name);
      }
    } else {
      llvm::errs() << "WARNING: Reference to hidden AliasElement "
                   << element.name << " in namespace "
                   << element.namespace_path;
      auto span = type_sig_div.open_span(HtmlWriter::SingleLine);
      span.add_class("type-name");
      span.write_text(element.name);
    }
  }
  {
    auto desc_div = item_li.open_div();
    desc_div.add_class("description");
    desc_div.add_class("short");
    if (auto comment = element.get_comment(); comment.is_some()) {
      if (auto md_html = markdown_to_html(comment.as_value(), page_state);
          md_html.is_err()) {
        return sus::err(sus::move(md_html).unwrap_err());
      } else {
        desc_div.write_html(sus::move(md_html).unwrap().summary_html);
      }
    }
  }

  return sus::ok();
}

}  // namespace subdoc::gen
