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

#include "sus/error/error.h"
#include "sus/mem/clone.h"

namespace sus::boxed::__private {

struct StringError {
  std::string s;
};

}  // namespace sus::boxed::__private

template <>
struct sus::error::ErrorImpl<sus::boxed::__private::StringError> {
    using StringError = sus::boxed::__private::StringError;
    constexpr static std::string display(const StringError& e) noexcept {
        return sus::clone(e.s);
    }
};
