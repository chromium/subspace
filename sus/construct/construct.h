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
/// # Type conversions
///
/// This namespace provides tools for three general methods of converting
/// between types:
/// * Infallible conversions which preserve values:
///   [`From`]($sus::construct::From) / [`Into`]($sus::construct::Into)
/// * Fallible conversions which preserve values or fail explicitly:
///   [`TryFrom`]($sus::construct::TryFrom) /
///   [`TryInto`]($sus::construct::TryInto)
/// * Infallib;e conversions which can change values or lose data:
///   [`Cast`]($sus::construct::Cast)
///
/// Usually prefer to convert between types with the value-preserving methods
/// of [`From`]($sus::construct::From) and
/// [`Into`]($sus::construct::Into) and [`TryInto`]($sus::construct::TryInto)
/// when possible. [`Cast`]($sus::construct::Cast) is required for converting
/// from floating point to integer values, and from larger integer types to
/// floating point, as these are lossy conversions.
///
/// The [`into`]($sus::construct::into) function allows explicit conversion
/// while deducing the target type, such as for converting function arguments
/// or return values.
///
/// | Concept | Usage | Infallible | Preserves values |
/// | ------- | ----- | ---------- | ---------------- |
/// | [`From`]($sus::construct::From) / [`Into`]($sus::construct::Into) | `T::from(x)` / [`sus::into(x)`]($sus::construct::into) | ✅ | ✅ |
/// | [`TryFrom`]($sus::construct::TryFrom) / [`TryInto`]($sus::construct::TryInto) | `T::try_from(x)` / [`sus::try_into<T>(x)`]($sus::construct::try_into) | ❌ | ✅ |
/// | [`Cast`]($sus::construct::Cast) | [`sus::cast<T>(x)`]($sus::construct::cast) | ✅ | ❌ |
///
/// See [`Cast`]($sus::construct::Cast) for how numeric and
/// primitive values are converted with [`cast`]($sus::construct::cast).
///
/// # Default construction
///
/// The [`Default`]($sus::construct::Default) concept matches types which can be
/// default constructed, allowing their use in generic code.
///
/// # Constructing from and holding references
///
/// The [`SafelyConstructibleFromReference`](
/// $sus::construct::SafelyConstructibleFromReference) concept pairs with the
/// `[[clang::lifetimebound]]` attribute, which gives better protection with
/// Clang. The concept can be used in generic code that accepts reference types
/// to prevent receiving references to implicit temporary objects in a
/// standard way.
namespace construct {}
}
