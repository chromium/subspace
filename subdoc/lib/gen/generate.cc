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

  for (const auto& [u, c] : db.global.records) {
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

      // TODO: List fields, methods, concepts, etc.

      // TODO: Add sorting to Vec.
      sus::Vec<sus::Tuple<std::string_view, UniqueSymbol>> sorted_static_fields;
      sus::Vec<sus::Tuple<std::string_view, UniqueSymbol>> sorted_fields;
      for (const auto& [field_symbol, field_element] : c.fields) {
        switch (field_element.is_static) {
          case FieldElement::Static:
            sorted_static_fields.push(
                sus::tuple(field_element.name, field_symbol));
            break;
          case FieldElement::NonStatic:
            sorted_fields.push(sus::tuple(field_element.name, field_symbol));
            break;
        }
      }
      if (sorted_static_fields.len() > 0u) {
        std::sort(sorted_static_fields.as_mut_ptr(),
                  sorted_static_fields.as_mut_ptr() +
                      sorted_static_fields.len().primitive_value,
                  [](const sus::Tuple<std::string_view, UniqueSymbol>& a,
                     const sus::Tuple<std::string_view, UniqueSymbol>& b) {
                    return a.get_ref<0>() < b.get_ref<0>();
                  });
      }
      if (sorted_fields.len() > 0u) {
        std::sort(
            sorted_fields.as_mut_ptr(),
            sorted_fields.as_mut_ptr() + sorted_fields.len().primitive_value,
            [](const sus::Tuple<std::string_view, UniqueSymbol>& a,
               const sus::Tuple<std::string_view, UniqueSymbol>& b) {
              return a.get_ref<0>() < b.get_ref<0>();
            });
      }

      if (sorted_static_fields.len() > 0u) {
        auto section_div = type_div.open_div();
        section_div.add_class("section");
        {
          auto fields_header_div = section_div.open_div();
          fields_header_div.add_class("header");
          fields_header_div.add_class("fields");
          fields_header_div.write_text("Static Data Members");
        }
        {
          auto fields_div = section_div.open_div();
          fields_div.add_class("list");
          fields_div.add_class("fields");
          fields_div.add_class("static");
          for (auto&& [name, field_symbol] : sorted_static_fields) {
            const FieldElement& fe = c.fields.at(field_symbol);
            {
              auto field_type_span = fields_div.open_span();
              field_type_span.add_class("static");
              field_type_span.write_text("static");
            }
            if (/* const? */ false) {
              auto field_type_span = fields_div.open_span();
              field_type_span.add_class("const");
              field_type_span.write_text("const");
            }
            {
              auto field_type_span = fields_div.open_span();
              field_type_span.add_class("type");
              field_type_span.write_text("???");
            }
            {
              auto field_type_span = fields_div.open_span();
              field_type_span.add_class("field");
              field_type_span.add_class("name");
              field_type_span.write_text(fe.name);
            }
            {
              auto desc_div = fields_div.open_div();
              desc_div.add_class("description");
              if (fe.has_comment()) {
                desc_div.write_text(fe.comment.raw_text);
              }
            }
          }
        }
      }

      if (sorted_fields.len() > 0u) {
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
          fields_div.add_class("nonstatic");
          for (auto&& [name, field_symbol] : sorted_fields) {
            const FieldElement& fe = c.fields.at(field_symbol);
            if (/* const? */ false) {
              auto field_type_span = fields_div.open_span();
              field_type_span.add_class("const");
              field_type_span.write_text("const");
            }
            {
              auto field_type_span = fields_div.open_span();
              field_type_span.add_class("type");
              field_type_span.write_text("???");
            }
            {
              auto field_type_span = fields_div.open_span();
              field_type_span.add_class("field");
              field_type_span.add_class("name");
              field_type_span.write_text(fe.name);
            }
            {
              auto desc_div = fields_div.open_div();
              desc_div.add_class("description");
              if (fe.has_comment()) {
                desc_div.write_text(fe.comment.raw_text);
              }
            }
          }
        }
      }
    }
  }
}

}  // namespace subdoc::gen
