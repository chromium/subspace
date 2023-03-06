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

#include "tidy/smoke_check.h"

#include "subspace/prelude.h"
#include "tidy/llvm.h"

namespace clang::tidy::subspace {

SmokeCheck::SmokeCheck(llvm::StringRef name, ClangTidyContext* context)
    : ClangTidyCheck(sus::move(name), context) {}

void SmokeCheck::registerMatchers(ast_matchers::MatchFinder* finder) {
  using namespace ast_matchers;

  finder->addMatcher(functionDecl().bind("x"), this);
}

void SmokeCheck::check(const ast_matchers::MatchFinder::MatchResult& match) {
  const auto* MatchedDecl = match.Nodes.getNodeAs<FunctionDecl>("x");
  if (!MatchedDecl->getIdentifier() ||
      MatchedDecl->getName().startswith("awesome_"))
    return;
  diag(MatchedDecl->getLocation(), "function %0 is insufficiently awesome")
      << MatchedDecl
      << FixItHint::CreateInsertion(MatchedDecl->getLocation(), "awesome_");
}

}  // namespace clang::tidy::subspace
