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

#include <map>
#include <string>
#include <string_view>

#include "sus/prelude.h"

namespace subdoc {

/// Find, store, and return the "resource dir" for finding system headers from
/// Clang.
///
/// Clang tools need to know where the "resource dir" is in order to find
/// system headers there, if Clang was the compiler that's being used for
/// building the target.
///
/// For other compilers, the headers come from the system header location, but
/// Clang has a resource dir that is known to the compiler, and which Subdoc
/// can't know apriori. So it has to query the Clang compiler to get it.
class ClangResourceDir {
 public:
  Option<std::string> find_resource_dir(std::string_view tool);

  std::map<std::string /* tool */, std::string /* resource_dir */, std::less<>>
      cache_;
};

}  // namespace subdoc
