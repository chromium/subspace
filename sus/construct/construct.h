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

namespace sus {

/// Concepts and functions for constructing and converting between types.
///
/// The [`cast`]($sus::construct::cast) function is built on the
/// [`Cast`]($sus::construct::Cast) concept to convert between numeric and
/// primitive types in a safer way than `static_cast`.
///
/// The [`From`]($sus::construct::From) and [`Into`]($sus::construct::Into)
/// concepts allow converting in a lossless and infallible way between types,
/// and accepting generics that can convert to a desired type. The
/// [`into`]($sus::construct::into) function allows explicit conversion while
/// deducing the target type, such as for converting function arguments or
/// return values.
///
/// The [`Default`]($sus::construct::Default) concept matches types which can be
/// default constructed, allowing their use in generic code.
///
/// The [`SafelyConstructibleFromReference`](
/// $sus::construct::SafelyConstructibleFromReference) concept pairs with the
/// `[[clang::lifetimebound]]` attribute, which gives better protection with
/// Clang. The concept can be used in generic code that accepts reference types
/// to prevent receiving references to implicit temporary objects in a
/// standard way.
namespace construct {}
}
