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

#include <string>

#include "subdoc/lib/gen/generate_cpp_path.h"
#include "subdoc/lib/gen/html_writer.h"
#include "subdoc/lib/gen/options.h"
#include "sus/iter/iterator.h"
#include "sus/prelude.h"

namespace subdoc::gen {

void generate_search_header(HtmlWriter::OpenMain& main) noexcept;

// Synchronously show search results, replacing the main content of the
// page, if a search is being done. Must be after the main-content section
// is created, since it refers to it. So that is the expected parameter here.
void generate_search_result_loading(
    HtmlWriter::OpenSection& main_content) noexcept;

}  // namespace subdoc::gen
