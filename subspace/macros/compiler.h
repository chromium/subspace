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

#if _MSC_VER
#define sus_if_msvc(x) x
#else
#define sus_if_msvc(x)
#endif

#if _MSC_VER
#define sus_if_msvc_else(x, y) x
#else
#define sus_if_msvc_else(x, y) y
#endif

#if defined(__clang__)
#define sus_if_clang(x) x
#else
#define sus_if_clang(x)
#endif

#if defined(__clang__)
#define sus_if_clang_else(x, y) x
#else
#define sus_if_clane_else(x, y) y
#endif
