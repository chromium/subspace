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

#include "subdoc/lib/gen/generate.h"

#include <map>
#include <set>

#include "subdoc/lib/gen/html_writer.h"
#include "subspace/ops/eq.h"
#include "subspace/ops/ord.h"
#include "subspace/prelude.h"

namespace subdoc::gen {

struct NamespaceName {
  // The full namespace name, e.g. `a::b::c`.
  std::string full_name;

  bool operator==(const NamespaceName& rhs) const = default;
  auto operator<=>(const NamespaceName& rhs) const = default;
};
static_assert(sus::ops::Eq<NamespaceName>);
static_assert(sus::ops::Ord<NamespaceName>);

struct InsideNamespace {};

void generate(const subdoc::Database& db, const subdoc::gen::Options& options) {
  std::filesystem::remove_all(options.output_root);

  for (const auto& [u, c] : db.records) {
    std::filesystem::path path = options.output_root;
    path.append([&]() {
      std::string fname;
      // TODO: Add Iterator::reverse.
      for (const auto& n : c.namespace_path.iter() /*.reverse()*/) {
        switch (n) {
          case Namespace::Tag::Global: break;
          case Namespace::Tag::Anonymous:
            fname += "anonymous";
            fname += "-";
            break;
          case Namespace::Tag::Named:
            fname += n.get_ref<Namespace::Tag::Named>();
            fname += "-";
            break;
        }
      }
      // TODO: Add Iterator::reverse.
      for (const auto& n : c.class_path.iter() /*.reverse()*/) {
        fname += n + "-";
      }
      fname += c.name;
      fname += ".html";
      return fname;
    }());

    std::filesystem::create_directories(path.parent_path());
    std::ofstream out = open_file_for_writing(path).unwrap();
    auto html = HtmlWriter(sus::move(out));
    auto body = html.open_body();
    {
      auto type_div = body.open_div();
      type_div.add_class("type");
      switch (c.record_type) {
        case RecordElement::Class: {
          auto class_span = type_div.open_span();
          class_span.add_class("type");
          class_span.add_class("class");
          class_span.write_text("class");
          break;
        }
        case RecordElement::Struct: {
          auto struct_span = type_div.open_span();
          struct_span.add_class("type");
          struct_span.add_class("struct");
          struct_span.write_text("struct");
          break;
        }
        case RecordElement::Union: {
          auto struct_span = type_div.open_span();
          struct_span.add_class("type");
          struct_span.add_class("union");
          struct_span.write_text("union");
          break;
        }
      }
      {
        auto name_span = type_div.open_span();
        name_span.add_class("typename");
        name_span.write_text(c.name);
      }
      {
        auto desc_div = type_div.open_div();
        desc_div.add_class("description");
        desc_div.write_text(c.comment.raw_text);
      }

      {
        auto section_div = type_div.open_div();
        section_div.add_class("section");
        {
          auto fields_header_div = section_div.open_div();
          fields_header_div.add_class("header");
          fields_header_div.add_class("fields");
          fields_header_div.write_text("Data Members");
        }
        {
          auto fields_div = section_div.open_div();
          fields_div.add_class("list");
          fields_div.add_class("fields");
          // TODO: List fields, methods, concepts, etc.
          /*
          {
            auto field_type_span = fields_div.open_span();
            field_type_span.add_class("type");
            field_type_span.write_text("int");
          }
          {
            auto field_name_span = fields_div.open_span();
            field_name_span.add_class("fieldname");
            field_name_span.write_text("f");
          }
          {
            auto desc_div = fields_div.open_div();
            desc_div.add_class("description");
            desc_div.write_text("/// Comment headline f");
          }
          */
        }
      }
    }
  }
}

}  // namespace subdoc::gen
