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

#include <sstream>

#include "subdoc/llvm.h"
#include "subspace/prelude.h"

namespace subdoc {

struct UniqueSymbol {
  std::array<uint8_t, 20> bytes;

  std::string to_string() const noexcept {
    std::ostringstream s;
    for (uint8_t b : bytes) {
      s << uint32_t{b};
    }
    return s.str();
  }

  bool operator==(const UniqueSymbol& other) const = default;
};

inline UniqueSymbol unique_from_decl(const clang::Decl* decl) noexcept {
  std::ostringstream s;

  // The USR, or Unique Symbol Resolution, is a unique value for a Decl across
  // all translation units. It does not differentiate on overlods well
  // (specifically on requires clauses). However the UniqueSymbol is not used
  // for functions/mehhods anyhow as we collapse overloads.
  llvm::SmallString<128> usr;
  if (!clang::index::generateUSRForDecl(decl, usr)) {
    s << std::string(sus::move(usr));
  } else {
    s << decl->getCanonicalDecl()->getBeginLoc().getRawEncoding();
  }

  return UniqueSymbol{
      .bytes = llvm::SHA1::hash(llvm::arrayRefFromStringRef(s.str())),
  };
}

}  // namespace subdoc

namespace std {

template <>
struct hash<subdoc::UniqueSymbol> {
  auto operator()(const subdoc::UniqueSymbol& u) const {
    size_t h = 0;
    for (auto b : u.bytes) h += b;
    return h;
  }
};

}  // namespace std
