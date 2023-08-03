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

#include "googletest/include/gtest/gtest.h"
#include "nanobench.h"
#include "sus/iter/iterator.h"
#include "sus/iter/zip.h"
#include "sus/prelude.h"

using sus::iter::zip;

// Benchmarks based on
// https://matklad.github.io/2023/04/09/can-you-trust-a-compiler-to-optimize-your-code.html

const char PREFIX1[] =
    "fhfkasj;dlsjf;laksdfj;lksdjfasd;fusdopfjasio;fsjdmfa;sofuo9psfp; "
    "fhfkasj;dlsjf;laksdfj;lksdjfasd;fusdopfjasio;fsjdmfa;sofuo9psfp; "
    "fhfkasj;dlsjf;laksdfj;lksdjfasd;fusdopfjasio;fsjdmfa;sofuo9psfp; "
    "fhfkasj;dlsjf;laksdfj;lksdjfasd;fusdopfjasio;fsjdmfa;sofuo9psfp; "
    "fhfkasj;dlsjf;laksdfj;lksdjfasd;fusdopfjasio;fsjdmfa;sofuo9psfp; "
    "fhfkasj;dlsjf;laksdfj;lksdjfasd;fusdopfjasio;fsjdmfa;sofuo9psfp; "
    "fhfkasj;dlsjf;laksdfj;lksdjfasd;fusdopfjasio;fsjdmfa;sofuo9psfp; "
    "fhfkasj;dlsjf;laksdfj;lksdjfasd;fusdopfjasio;fsjdmfa;sofuo9psfp; "
    "lkffpoasjf;sadp;fsapfksa;kdfposa'pf";
const char PREFIX2[] =
    "fhfkasj;dlsjf;laksdfj;lksdjfasd;fusdopfjasio;fsjdmfa;sofuo9psfp; "
    "fhfkasj;dlsjf;laksdfj;lksdjfasd;fusdopfjasio;fsjdmfa;sofuo9psfp; "
    "fhfkasj;dlsjf;laksdfj;lksdjfasd;fusdopfjasio;fsjdmfa;sofuo9psfp; "
    "fhfkasj;dlsjf;laksdfj;lksdjfasd;fusdopfjasio;fsjdmfa;sofuo9psfp; "
    "fhfkasj;dlsjf;laksdfj;lksdjfasd;fusdopfjasio;fsjdmfa;sofuo9psfp; "
    "fhfkasj;dlsjf;laksdfj;lksdjfasd;fusdopfjasio;fsjdmfa;sofuo9psfp; "
    "fhfkasj;dlsjf;laksdfj;lksdjfasd;fusdopfjasio;fsjdmfa;sofuo9psfp; "
    "fhfkasj;dlsjf;laksdfj;lksdjfasd;fusdopfjasio;fsjdmfa;sofuo9psfp; "
    "lkffpoasjf;sadp;fsapfksa;kdfposa'pfmfa;sofuo9psfp; "
    "lkffpoasjf;sadp;fsapfksa;kdfposa'pf";

auto common_prefix_naive(const sus::Slice<u8>& xs, const sus::Slice<u8>& ys)
    -> usize {
  auto result = 0_usize;
  while (result < xs.len() && result < ys.len()) {
    if (xs[result] != ys[result]) break;
    result += 1u;
  }
  return result;
}

// This should be about the same as common_prefix_naive, it's just nicer
// iterating.
auto common_prefix_zip(const sus::Slice<u8>& xs, const sus::Slice<u8>& ys)
    -> usize {
  auto result = 0_usize;
  for (auto [x, y] : zip(xs.iter(), ys.iter())) {
    if (x != y) break;
    result += 1u;
  }
  return result;
}

// This should be slightly faster than `common_prefix_zip`. It is in Rust.
auto common_prefix_chunks_exact(const sus::Slice<u8>& xs,
                                const sus::Slice<u8>& ys) -> usize {
  const auto chunk_size = 16_usize;
  auto result = 0_usize;

  bool escape = false;
  for (auto [xs_chunk, ys_chunk] :
       zip(xs.chunks_exact(chunk_size), ys.chunks_exact(chunk_size))) {
    for (auto [x, y] : zip(sus::move(xs_chunk), sus::move(ys_chunk))) {
      if (x != y) {
        escape = true;
        break;
      }
      result += 1u;
    }
    if (escape) break;
  }
  for (auto [x, y] : zip(xs[sus::ops::range_from(result)],
                         ys[sus::ops::range_from(result)])) {
    if (x != y) break;
    result += 1u;
  }
  return result;
}

// This should be significantly faster if SIMD auto vectorization kicks in.
auto common_prefix_no_shortcircuit(const sus::Slice<u8>& xs,
                                   const sus::Slice<u8>& ys) -> usize {
  const auto chunk_size = 16_usize;
  auto result = 0_usize;
  for (auto [xs_chunk, ys_chunk] :
       zip(xs.chunks_exact(chunk_size), ys.chunks_exact(chunk_size))) {
    bool chunk_equal = true;
    for (auto [x, y] : zip(sus::move(xs_chunk), sus::move(ys_chunk))) {
      // NB: &, unlike &&, doesn't short-circuit.
      chunk_equal = chunk_equal & (x == y);
    }
    if (!chunk_equal) {
      break;
    }
    result += chunk_size;
  }
  for (auto [x, y] : zip(xs[sus::ops::range_from(result)],
                         ys[sus::ops::range_from(result)])) {
    if (x != y) break;
    result += 1u;
  }
  return result;
}

auto common_prefix_take_while(const sus::Slice<u8>& xs,
                              const sus::Slice<u8>& ys) -> usize {
  const auto chunk_size = 16_usize;
  auto off =
      zip(xs.chunks_exact(chunk_size), ys.chunks_exact(chunk_size))
          .take_while(
              [](const sus::Tuple<sus::Slice<u8>, sus::Slice<u8>>& chunks) {
                const auto& [xs_chunk, ys_chunk] = chunks;
                return xs_chunk == ys_chunk;
              })
          .count() *
      chunk_size;
  return off +
         zip(xs[sus::ops::range_from(off)], ys[sus::ops::range_from(off)])
             .take_while([](const sus::Tuple<const u8&, const u8&>& pair) {
               const auto& [x, y] = pair;
               return x == y;
             })
             .count();
}

TEST(SimdChunks, common_prefix) {
  auto b = ankerl::nanobench::Bench();

  auto v1 = sus::Vec<u8>::from(PREFIX1);
  auto v2 = sus::Vec<u8>::from(PREFIX2);

  b.run("common_prefix_naive", [&]() {
    auto r = common_prefix_naive(v1, v2);
    ankerl::nanobench::doNotOptimizeAway(r);
  });
  b.run("common_prefix_zip", [&]() {
    auto r = common_prefix_zip(v1, v2);
    ankerl::nanobench::doNotOptimizeAway(r);
  });
  b.run("common_prefix_chunks_exact", [&]() {
    auto r = common_prefix_chunks_exact(v1, v2);
    ankerl::nanobench::doNotOptimizeAway(r);
  });
  b.run("common_prefix_no_shortcircuit", [&]() {
    auto r = common_prefix_no_shortcircuit(v1, v2);
    ankerl::nanobench::doNotOptimizeAway(r);
  });
  b.run("common_prefix_take_while", [&]() {
    auto r = common_prefix_take_while(v1, v2);
    ankerl::nanobench::doNotOptimizeAway(r);
  });
}
