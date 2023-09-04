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

#include "subdoc/lib/gen/generate_type.h"

#include <sstream>

#include "subdoc/lib/gen/files.h"
#include "sus/mem/replace.h"

namespace subdoc::gen {

namespace {

std::string make_title_string(TypeToStringQuery q) {
  std::ostringstream str;
  for (const std::string& s : q.namespace_path) str << s << "::";
  for (const std::string& s : q.record_path) str << s << "::";
  str << q.name;
  return sus::move(str).str();
}

}  // namespace

void generate_type(
    HtmlWriter::OpenDiv& div, const LinkedType& linked_type,
    sus::fn::FnMutRef<void(HtmlWriter::OpenDiv&)> const_qualifier_fn,
    sus::fn::FnMutRef<void(HtmlWriter::OpenDiv&)> volatile_qualifier_fn,
    sus::fn::FnOnceRef<void(HtmlWriter::OpenDiv&)> var_name_fn) noexcept {
  auto text_fn = [&](std::string_view text) { div.write_text(text); };
  auto type_fn = [&, i_ = 0_usize](TypeToStringQuery q) mutable {
    const sus::Option<TypeRef>& maybe_ref =
        linked_type.type_element_refs[sus::mem::replace(i_, i_ + 1u)];
    if (maybe_ref.is_none()) {
      div.write_text(q.name);
      return;
    }

    auto anchor = div.open_a();
    anchor.add_class("type-name");

    switch (*maybe_ref) {
      case TypeRef::Tag::Concept: {
        const ConceptElement& e = maybe_ref->as<TypeRef::Tag::Concept>();
        sus::check_with_message(
            !e.hidden(), fmt::format("reference to hidden Concept {}", e.name));
        anchor.add_href(construct_html_url_for_concept(e));
        break;
      }
      case TypeRef::Tag::Record: {
        const RecordElement& e = maybe_ref->as<TypeRef::Tag::Record>();
        sus::check_with_message(
            !e.hidden(), fmt::format("reference to hidden Record {}", e.name));
        anchor.add_href(construct_html_url_for_type(e));
        break;
      }
    }

    anchor.add_title(make_title_string(q));
    anchor.write_text(q.name);
  };
  auto const_fn = [&]() { const_qualifier_fn(div); };
  auto volatile_fn = [&]() { volatile_qualifier_fn(div); };
  auto var_fn = [&]() { sus::move(var_name_fn)(div); };

  type_to_string(linked_type.type, text_fn, type_fn, const_fn, volatile_fn,
                 sus::some(sus::move(var_fn)));
}

}  // namespace subdoc::gen
