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

// IWYU pragma: private
// IWYU pragma: friend "sus/.*"
#pragma once

// SUS_CHECK_INTEGER_OVERFLOW can be defined to false to disable overflow checks
// in integer arithmetic and shifting operations.
#if !defined(SUS_CHECK_INTEGER_OVERFLOW)
#define SUS_CHECK_INTEGER_OVERFLOW true
#endif
static_assert(SUS_CHECK_INTEGER_OVERFLOW == false ||
              SUS_CHECK_INTEGER_OVERFLOW == true);
