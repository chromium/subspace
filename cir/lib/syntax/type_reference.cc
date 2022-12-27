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

#include "cir/lib/syntax/type_reference.h"

#include "cir/lib/output.h"
#include "cir/lib/syntax/function.h"

namespace cir {

std::string to_string(const syntax::TypeReference& typeref,
                      const Output& output) noexcept {
  using enum syntax::TypeRefKind::Tag;
  switch (typeref.kind) {
    case Builtin: {
      const auto& [builtin, obj_anno] = typeref.kind.get_ref<Builtin>();
      std::ostringstream s;
      s << cir::to_string(obj_anno, output);
      s << builtin_type_to_string(builtin);
      return s.str();
    }
    case Declared: {
      const auto& [declared, obj_anno] = typeref.kind.get_ref<Declared>();
      std::ostringstream s;
      s << cir::to_string(obj_anno, output);
      s << "(TODO: declared type name)";
      return s.str();
    }
    case Pointer: {
      const auto& [pointer, obj_anno] = typeref.kind.get_ref<Pointer>();
      std::ostringstream s;
      s << "pointer(TODO: pointee types) ";
      s << cir::to_string(obj_anno, output);
      s << cir::to_string(pointer, output);
      return s.str();
    }
    case FnPointer: {
      const auto& [function_id, obj_anno] = typeref.kind.get_ref<FnPointer>();
      std::ostringstream s;
      s << cir::to_string(obj_anno, output);
      s << "fn pointer(";
      s << cir::to_string(output.functions.at(function_id), output);
      s << ")";
      return s.str();
    }
  }
  sus::unreachable();
}

}  // namespace cir
