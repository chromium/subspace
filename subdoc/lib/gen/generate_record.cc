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

#include "subdoc/lib/gen/generate_record.h"

#include <sstream>

#include "subdoc/lib/database.h"
#include "subdoc/lib/gen/files.h"
#include "subdoc/lib/gen/generate_function.h"
#include "subdoc/lib/gen/generate_head.h"
#include "subdoc/lib/gen/html_writer.h"
#include "subdoc/lib/gen/options.h"
#include "subspace/prelude.h"

namespace subdoc::gen {

namespace {

void generate_record_overview(HtmlWriter::OpenDiv& record_div,
                              const RecordElement& element) {
  auto section_div = record_div.open_div();
  section_div.add_class("section");
  section_div.add_class("overview");

  {
    auto record_header_div = section_div.open_div();
    record_header_div.add_class("section-header");
    {
      auto record_type_span = record_header_div.open_span();
      record_type_span.write_text(
          friendly_record_type_name(element.record_type, true));
    }
    {
      auto name_anchor = record_header_div.open_a(HtmlWriter::SingleLine);
      name_anchor.add_href("#");
      name_anchor.add_class("type-name");
      name_anchor.write_text(element.name);
    }
  }
  {
    auto type_sig_div = section_div.open_div();
    type_sig_div.add_class("type-signature");
    {
      auto record_type_span = type_sig_div.open_span();
      std::string record_type_name =
          friendly_record_type_name(element.record_type, false);
      record_type_span.add_class(record_type_name);
      record_type_span.write_text(record_type_name);
    }
    {
      auto name_span = type_sig_div.open_span();
      name_span.add_class("type-name");
      name_span.write_text(element.name);
    }
    {
      auto record_body_div = type_sig_div.open_div();
      record_body_div.add_class("record-body");
      record_body_div.write_text("{ ... };");
    }
  }
  if (element.has_comment()) {
    auto desc_div = section_div.open_div();
    desc_div.add_class("description");
    desc_div.write_html(element.comment.raw_text);
  }
}

void generate_record_fields(
    HtmlWriter::OpenDiv& record_div, const RecordElement& element,
    bool static_fields,
    sus::Slice<const sus::Tuple<std::string_view, UniqueSymbol>> fields) {
  if (fields.is_empty()) return;

  auto section_div = record_div.open_div();
  section_div.add_class("section");
  section_div.add_class("fields");
  section_div.add_class(static_fields ? "static" : "nonstatic");

  {
    auto fields_header_div = section_div.open_div();
    fields_header_div.add_class("section-header");
    fields_header_div.write_text(static_fields ? "Static Data Members"
                                               : "Data Members");
  }
  {
    for (auto&& [name, field_unique_symbol] : fields) {
      const FieldElement& fe = element.fields.at(field_unique_symbol);

      auto field_div = section_div.open_div();
      field_div.add_class("section-item");

      if (static_fields) {
        auto static_span = field_div.open_span();
        static_span.add_class("static");
        static_span.write_text("static");
      }
      if (fe.is_const) {
        auto field_type_span = field_div.open_span();
        field_type_span.add_class("const");
        field_type_span.write_text("const");
      }
      if (fe.is_volatile) {
        auto field_type_span = field_div.open_span();
        field_type_span.add_class("volatile");
        field_type_span.write_text("volatile");
      }
      {
        auto field_type_link = field_div.open_a(HtmlWriter::MultiLine);
        field_type_link.add_class("type-name");
        field_type_link.add_title(fe.type_name);
        if (fe.type_element.is_some()) {
          field_type_link.add_href(
              construct_html_file_path(std::filesystem::path(),
                                       fe.type_element->namespace_path.as_ref(),
                                       fe.type_element->record_path.as_ref(),
                                       fe.type_element->name)
                  .string());
        }
        field_type_link.write_text(fe.short_type_name);
      }
      {
        auto field_name_anchor = field_div.open_a(HtmlWriter::SingleLine);
        std::ostringstream anchor;
        anchor << "field.";
        anchor << (static_fields ? "static." : "");
        anchor << fe.name;
        field_name_anchor.add_name(anchor.str());
        field_name_anchor.add_href(std::string("#") + anchor.str());
        field_name_anchor.add_class("field-name");
        field_name_anchor.write_text(fe.name);
      }
      if (fe.has_comment()) {
        auto desc_div = field_div.open_div();
        desc_div.add_class("description");
        desc_div.write_html(fe.comment.raw_text);
      }
    }
  }
}

void generate_record_methods(
    HtmlWriter::OpenDiv& record_div, const RecordElement& element,
    bool static_methods,
    sus::Slice<const sus::Tuple<std::string_view, const FunctionId&>> methods) {
  if (methods.is_empty()) return;

  auto section_div = record_div.open_div();
  section_div.add_class("section");
  section_div.add_class("methods");
  section_div.add_class(static_methods ? "static" : "nonstatic");

  {
    auto methods_header_div = section_div.open_div();
    methods_header_div.add_class("section-header");
    methods_header_div.write_text(static_methods ? "Static Methods"
                                                 : "Methods");
  }
  {
    for (auto&& [name, function_id] : methods) {
      generate_function(section_div, element.methods.at(function_id),
                        static_methods);
    }
  }
}

}  // namespace

void generate_record(const RecordElement& element,
                     const Options& options) noexcept {
  const std::filesystem::path path = construct_html_file_path(
      options.output_root, element.namespace_path.as_ref(),
      element.record_path.as_ref(), element.name);
  std::filesystem::create_directories(path.parent_path());
  auto html = HtmlWriter(open_file_for_writing(path).unwrap());

  generate_head(html, element.name, options);

  auto body = html.open_body();

  auto record_div = body.open_div();
  record_div.add_class("type");
  record_div.add_class("record");
  record_div.add_class(friendly_record_type_name(element.record_type, false));
  generate_record_overview(mref(record_div), element);

  sus::Vec<sus::Tuple<std::string_view, UniqueSymbol>> sorted_static_fields;
  sus::Vec<sus::Tuple<std::string_view, UniqueSymbol>> sorted_fields;
  for (const auto& [symbol, field_element] : element.fields) {
    switch (field_element.is_static) {
      case FieldElement::Static:
        sorted_static_fields.push(sus::tuple(field_element.name, symbol));
        break;
      case FieldElement::NonStatic:
        sorted_fields.push(sus::tuple(field_element.name, symbol));
        break;
    }
  }
  sorted_static_fields.sort_unstable_by(
      [](const sus::Tuple<std::string_view, UniqueSymbol>& a,
         const sus::Tuple<std::string_view, UniqueSymbol>& b) {
        return a.get_ref<0>() <=> b.get_ref<0>();
      });
  sorted_fields.sort_unstable_by(
      [](const sus::Tuple<std::string_view, UniqueSymbol>& a,
         const sus::Tuple<std::string_view, UniqueSymbol>& b) {
        return a.get_ref<0>() <=> b.get_ref<0>();
      });

  generate_record_fields(mref(record_div), element, true,
                         sorted_static_fields.as_ref());
  generate_record_fields(mref(record_div), element, false,
                         sorted_fields.as_ref());

  sus::Vec<sus::Tuple<std::string_view, const FunctionId&>>
      sorted_static_methods;
  sus::Vec<sus::Tuple<std::string_view, const FunctionId&>> sorted_methods;
  for (const auto& [method_id, method_element] : element.methods) {
    if (method_id.is_static) {
      sorted_static_methods.push(sus::tuple(method_element.name, method_id));
    } else {
      sorted_methods.push(sus::tuple(method_element.name, method_id));
    }
  }
  sorted_static_methods.sort_unstable_by(
      [](const sus::Tuple<std::string_view, const FunctionId&>& a,
         const sus::Tuple<std::string_view, const FunctionId&>& b) {
        return a.get_ref<0>() <=> b.get_ref<0>();
      });
  sorted_methods.sort_unstable_by(
      [](const sus::Tuple<std::string_view, const FunctionId&>& a,
         const sus::Tuple<std::string_view, const FunctionId&>& b) {
        return a.get_ref<0>() <=> b.get_ref<0>();
      });

  generate_record_methods(mref(record_div), element, true,
                          sorted_static_methods.as_ref());
  generate_record_methods(mref(record_div), element, false,
                          sorted_methods.as_ref());

  for (const auto& [key, subrecord]: element.records) {
    generate_record(subrecord, options);
  }
}

void generate_record_reference(HtmlWriter::OpenDiv& section_div,
                               const RecordElement& element) noexcept {
  auto item_div = section_div.open_div();
  item_div.add_class("section-item");

  {
    auto type_sig_div = item_div.open_div();
    type_sig_div.add_class("type-signature");

    {
      auto record_type_span = type_sig_div.open_span();
      std::string record_type_name =
          friendly_record_type_name(element.record_type, false);
      record_type_span.add_class(record_type_name);
      record_type_span.write_text(record_type_name);
    }
    {
      auto name_link = type_sig_div.open_a(HtmlWriter::SingleLine);
      name_link.add_class("type-name");
      name_link.add_href(
          construct_html_file_path(std::filesystem::path(),
                                   element.namespace_path.as_ref(),
                                   element.record_path.as_ref(), element.name)
              .string());
      name_link.write_text(element.name);
    }
  }
  if (element.has_comment()) {
    auto desc_div = item_div.open_div();
    desc_div.add_class("description");
    desc_div.write_html(element.comment.summary());
  }
}

}  // namespace subdoc::gen
