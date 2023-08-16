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

#include "subdoc/llvm.h"

namespace subdoc {

// TODO: Instead of making a string, we should build a data structure that can
// be stringified but also contains info on all types so that we can look them
// up in the database.
std::string stmt_to_string(const clang::Stmt& stmt,
                           const clang::ASTContext& context,
                           clang::Preprocessor& preprocessor);

}  // namespace subdoc
