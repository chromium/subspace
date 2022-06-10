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

/// This helper allows T&& to downgrade to T&, so that when we move out of
/// storage in call_once(), if the receiver just wants an lvalue reference, that
/// it will pass them one instead.
template <class T>
struct FlexRef;

template <class T>
struct FlexRef<T&> {
  operator T&() { return t; }
  T&& t;
};

template <class T>
struct FlexRef {
  operator T&&() { return static_cast<T&&>(t); }
  T&& t;
};
