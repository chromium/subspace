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

#include "subspace/string/__private/format_to_stream.h"

#include <ostream>
#include <string>

namespace sus::string::__private {

std::ostream& format_to_stream(std::ostream& os, const std::string& s) {
  os << s;
  return os;
}

}  // namespace sus::string::__private
