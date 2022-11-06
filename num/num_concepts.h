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

#include <stdint.h>

#include <concepts>

namespace sus::num {

template <class Rhs, class Output = Rhs>
concept Neg = requires(Rhs rhs) {
                { -static_cast<Rhs&&>(rhs) } -> std::same_as<Output>;
              };

template <class Rhs, class Output = Rhs>
concept BitNot = requires(Rhs rhs) {
                   { ~static_cast<Rhs&&>(rhs) } -> std::same_as<Output>;
                 };

template <class Lhs, class Rhs, class Output = Lhs>
concept Add = requires(const Lhs& lhs, const Rhs& rhs) {
                { lhs + rhs } -> std::same_as<Output>;
              };

template <class Lhs, class Rhs>
concept AddAssign = requires(Lhs& lhs, const Rhs& rhs) {
                      { lhs += rhs } -> std::same_as<void>;
                    };

template <class Lhs, class Rhs, class Output = Lhs>
concept Sub = requires(const Lhs& lhs, const Rhs& rhs) {
                { lhs - rhs } -> std::same_as<Output>;
              };

template <class Lhs, class Rhs>
concept SubAssign = requires(Lhs& lhs, const Rhs& rhs) {
                      { lhs -= rhs } -> std::same_as<void>;
                    };

template <class Lhs, class Rhs, class Output = Lhs>
concept Mul = requires(const Lhs& lhs, const Rhs& rhs) {
                { lhs* rhs } -> std::same_as<Output>;
              };

template <class Lhs, class Rhs>
concept MulAssign = requires(Lhs& lhs, const Rhs& rhs) {
                      { lhs *= rhs } -> std::same_as<void>;
                    };

template <class Lhs, class Rhs, class Output = Lhs>
concept Div = requires(const Lhs& lhs, const Rhs& rhs) {
                { lhs / rhs } -> std::same_as<Output>;
              };

template <class Lhs, class Rhs>
concept DivAssign = requires(Lhs& lhs, const Rhs& rhs) {
                      { lhs /= rhs } -> std::same_as<void>;
                    };

template <class Lhs, class Rhs, class Output = Lhs>
concept Rem = requires(const Lhs& lhs, const Rhs& rhs) {
                { lhs % rhs } -> std::same_as<Output>;
              };

template <class Lhs, class Rhs>
concept RemAssign = requires(Lhs& lhs, const Rhs& rhs) {
                      { lhs %= rhs } -> std::same_as<void>;
                    };

template <class Lhs, class Rhs, class Output = Lhs>
concept BitAnd = requires(const Lhs& lhs, const Rhs& rhs) {
                   { lhs& rhs } -> std::same_as<Output>;
                 };

template <class Lhs, class Rhs>
concept BitAndAssign = requires(Lhs& lhs, const Rhs& rhs) {
                         { lhs &= rhs } -> std::same_as<void>;
                       };

template <class Lhs, class Rhs, class Output = Lhs>
concept BitOr = requires(const Lhs& lhs, const Rhs& rhs) {
                  { lhs | rhs } -> std::same_as<Output>;
                };

template <class Lhs, class Rhs>
concept BitOrAssign = requires(Lhs& lhs, const Rhs& rhs) {
                        { lhs |= rhs } -> std::same_as<void>;
                      };

template <class Lhs, class Rhs, class Output = Lhs>
concept BitXor = requires(const Lhs& lhs, const Rhs& rhs) {
                   { lhs ^ rhs } -> std::same_as<Output>;
                 };

template <class Lhs, class Rhs>
concept BitXorAssign = requires(Lhs& lhs, const Rhs& rhs) {
                         { lhs ^= rhs } -> std::same_as<void>;
                       };

template <class Lhs, class Output = Lhs>
concept Shl = requires(const Lhs& lhs /* TODO: , u32 rhs */) {
                { lhs << 2u } -> std::same_as<Output>;
              };

template <class Lhs>
concept ShlAssign = requires(Lhs& lhs /* TODO: , u32 rhs */) {
                      { lhs <<= 2u } -> std::same_as<void>;
                    };

template <class Lhs, class Output = Lhs>
concept Shr = requires(const Lhs& lhs /* TODO: , u32 rhs */) {
                { lhs >> 2u } -> std::same_as<Output>;
              };

template <class Lhs>
concept ShrAssign = requires(Lhs& lhs /* TODO: , u32 rhs */) {
                      { lhs >>= 2u } -> std::same_as<void>;
                    };

}  // namespace sus::num
