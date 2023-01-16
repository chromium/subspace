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

#include "subdoc/lib/gen/generate_record.h"
#include "subdoc/lib/gen/files.h"
#include "subdoc/lib/gen/html_writer.h"
#include "subspace/ops/eq.h"
#include "subspace/ops/ord.h"
#include "subspace/prelude.h"

namespace subdoc::gen {

void generate(const Database& db, const Options& options) {
  std::filesystem::remove_all(options.output_root);

  for (const auto& [u, element] : db.global.records) {
    generate_record(element, options);
  }
}

}  // namespace subdoc::gen
