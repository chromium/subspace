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

// IWYU pragma: begin_exports
#include "sus/fn/fn_concepts.h"
#include "sus/fn/fn_dyn.h"
// IWYU pragma: end_exports

namespace sus {

/// The [`Fn`]($sus::fn::Fn), [`FnMut`]($sus::fn::FnMut) and
/// [`FnOnce`]($sus::fn::FnOnce) concepts for working with functors
/// and callable types.
///
/// There are three main concepts that model anything callable:
/// * A [`Fn`]($sus::fn::Fn) represents a callable type which is const and will
///   return the same outputs given the same inputs.
/// * A [`FnMut`]($sus::fn::FnMut) represents a callable type which is allowed
///   to generate unique outputs on each call. This is the most commonly used
///   of the three.
/// * A [`FnOnce`]($sus::fn::FnOnce) represents a callable type which will only
///   be called once.
///
/// As these are concepts, not concrete types, they can not enforce any
/// behaviour but rather represent a protocol of expectations. Types designed to
/// satisfy these concepts should adhere to them, and safely handle misuse, such
/// as panicking (via [`panic`]($sus::assertions::panic)) if called twice
/// when it is not supported.
///
/// To make a type satisfy [`Fn`]($sus::fn::Fn) it should have a
/// const call `operator()`, to safisfy [`FnMut`]($sus::fn::FnMut) it
/// should have a mutable call `operator()` and to satisfy
/// [`FnOnce`]($sus::fn::FnOnce), it should have an
/// rvalue-qualfied call `operator()`.
///
/// A [`Fn`]($sus::fn::Fn) type will also satisfy the other two, since a const
/// function that chooses not to mutate, or that is called only once, does not
/// violate the protocol.
///
/// Similarly, a [`FnMut`]($sus::fn::FnMut) type will also satisfy
/// [`FnOnce`]($sus::fn::FnOnce) as it is valid to only call it a single time.
///
/// The `fn` namespace provides matchers for use in the function concepts to
/// match against and constrain the return type of a function.
/// * [`NonVoid`]($sus::fn::NonVoid) will match function types that return
///   a type other than void.
/// * [`Anything`]($sus::fn::Anything) will match function types that return
///   any type.
///
/// An example of using [`NonVoid`]($sus::fn::NonVoid) to match the return
/// type of a [`FnMut`]($sus::fn::FnMut):
/// ```
/// // Accepts a function that can be called repeatedly with `i32` and which
/// // returns something other than void. A void type would break compilation
/// // as it can not be assigned to a variable, so it rejects functions with a
/// // void return type.
/// auto func = [](FnMut<NonVoid(i32)> auto f) {
///   auto x = f(0);
///   x += 3;
/// };
/// func([](i32) { return 3; });
/// ```
///
/// The same with [`FnMut`]($sus::fn::FnMut) being
/// [type-erased]($sus::boxed::DynConcept) as [`DynFnMut`](
/// $sus::fn::DynFnMut) to avoid templates. The full type must be specified
/// when not working with templates, so [`NonVoid`]($sus::fn::NonVoid) can
/// not be used.
/// ```
/// auto func = [](DynFnMut<i32(i32)>& f) {
///   auto x = f(0);
///   x += 3;
/// };
///
/// func(sus::dyn<DynFnMut<i32(i32)>>([](i32) { return 3; }));
/// ```
namespace fn {}

}  // namespace sus
