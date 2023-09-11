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

#include "subdoc/lib/gen/generate_requires.h"

#include "subdoc/lib/database.h"
#include "subdoc/lib/gen/html_writer.h"

namespace subdoc::gen {

void generate_requires_constraints(HtmlWriter::OpenDiv& div,
                                   const RequiresConstraints& constraints) {
  if (!constraints.list.is_empty()) {
    auto requires_div = div.open_div();
    requires_div.add_class("requires");
    {
      auto keyword_div = requires_div.open_span();
      keyword_div.add_class("requires-keyword");
      keyword_div.add_class("keyword");
      keyword_div.write_text("requires");
    }
    for (const RequiresConstraint& constraint : constraints.list) {
      auto clause_div = requires_div.open_div(HtmlWriter::SingleLine);
      clause_div.add_class("requires-constraint");
      switch (constraint) {
        using enum RequiresConstraint::Tag;
        case Concept: {
          auto clause_line_pre = clause_div.open_pre();
          clause_line_pre.add_class("requires-constraint-line");
          clause_line_pre.write_text(constraint.as<Concept>().concept_name);
          clause_line_pre.write_text("<");
          for (const auto& [i, s] :
               constraint.as<Concept>().args.iter().enumerate()) {
            if (i > 0u) clause_line_pre.write_text(", ");
            clause_line_pre.write_text(s);
          }
          clause_line_pre.write_text(">");
          break;
        }
        case Text: {
          auto clause = std::string_view(constraint.as<Text>());
          while (true) {
            if (usize pos = clause.find('\n');
                pos != std::string::npos && pos != clause.size() - 1u) {
              auto clause_line_pre = clause_div.open_pre();
              clause_line_pre.add_class("requires-constraint-line");
              clause_line_pre.write_text(clause.substr(0u, pos));
              clause = clause.substr(pos + 1u);
            } else {
              auto clause_line_pre = clause_div.open_pre();
              clause_line_pre.add_class("requires-constraint-line");
              clause_line_pre.write_text(clause);
              break;
            }
          }
        }
      }
    }
  }
}

}  // namespace subdoc::gen
