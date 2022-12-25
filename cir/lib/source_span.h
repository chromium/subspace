// Copyright 2022 Google LLC
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

#include "cir/llvm.h"

namespace cir {

struct SourceSpan {
  static SourceSpan from_decl(const clang::Decl& decl) {
    return SourceSpan(decl.getASTContext().getFullLoc(decl.getBeginLoc()),
                      decl.getASTContext().getFullLoc(decl.getEndLoc()));
  }

  clang::FullSourceLoc begin;
  clang::FullSourceLoc end;

  void dump() {
    llvm::errs() << begin.getManager().getFilename(begin) << ": ";
    llvm::errs() << begin.getLineNumber() << ":" << begin.getColumnNumber();
    llvm::errs() << "...";
    llvm::errs() << end.getLineNumber() << ":" << end.getColumnNumber();
    llvm::errs() << "\n";
    llvm::StringRef buf = begin.getBufferData();
    llvm::StringRef range_buf =
        buf.substr(begin.getFileOffset(),
                   end.getFileOffset() - begin.getFileOffset() + 1u);
    auto line_number = u32::from(begin.getLineNumber());
    llvm::errs() << line_number.primitive_value << ":";
    for (auto i = 1_u32; i < begin.getColumnNumber(); i += 1u)
      llvm::errs() << " ";
    for (char c : range_buf) {
      llvm::errs() << c;
      if (c == '\n') {
        line_number += 1u;
        llvm::errs() << line_number.primitive_value << ":";
      }
    }
    llvm::errs() << "\n";
  }
};

}  // namespace cir
