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

#include <vector>

#include "googletest/include/gtest/gtest.h"
#include "nanobench.h"
#include "sus/collections/compat_vector.h"
#include "sus/iter/compat_ranges.h"
#include "sus/iter/iterator.h"
#include "sus/prelude.h"

namespace {
struct Key {
  std::uint32_t id;
};
constexpr size_t to_index(Key k) { return k.id; }

static std::vector<int> generate_data(usize sz) {
  std::vector<int> data;
  data.reserve(sz);
  for (i32 i; i < i32::try_from(sz).unwrap(); i += 1) {
    data.push_back(i);
  }
  return data;
}

static std::vector<Key> generate_key_data(usize sz) {
  std::vector<Key> data;
  data.reserve(sz);
  for (u32 i; i < u32::try_from(sz).unwrap(); i += 1u) {
    data.push_back(Key{i});
  }
  return data;
}
}  // namespace

static void copy_and_multiply_ints(ankerl::nanobench::Bench& b,
                                   const std::vector<int>& data,
                                   usize num_elements) {
  b.run(fmt::format("std::vector::push_back, n = {}", num_elements), [&]() {
    std::vector<int> out;
    out.reserve(data.size());
    for (int d : data) {
      out.push_back(2 * d);
    }
    return out;
  });

  b.run(fmt::format("std::vector collect, n = {}", num_elements), [&]() {
    return sus::iter::from_range(data)
        .map([](int d) { return 2 * d; })
        .collect<std::vector<int>>();
  });

  b.run(fmt::format("sus::Vec::push, n = {}", num_elements), [&]() {
    sus::Vec<int> out;
    out.reserve(data.size());
    for (int d : data) {
      out.push(2 * d);
    }
    return out;
  });

  b.run(fmt::format("sus::Vec collect, n = {}", num_elements), [&]() {
    return sus::iter::from_range(data)
        .map([](int d) { return 2 * d; })
        .collect<sus::Vec<int>>();
  });
}

TEST(BenchVecMap, CopyAndMultiplyInts_1000) {
  auto data = generate_data(1'000u);
  auto b = ankerl::nanobench::Bench();
  copy_and_multiply_ints(b, data, 1'000u);
}
TEST(BenchVecMap, CopyAndMultiplyInts_100_000) {
  auto data = generate_data(100'000u);
  auto b = ankerl::nanobench::Bench();
  copy_and_multiply_ints(b, data, 100'000u);
}
TEST(BenchVecMap, CopyAndMultiplyInts_10_000_000) {
  auto data = generate_data(10'000'000u);
  auto b = ankerl::nanobench::Bench();
  copy_and_multiply_ints(b, data, 10'000'000u);
}

static void transform_to_indicies(ankerl::nanobench::Bench& b,
                                  const std::vector<Key>& data,
                                  usize num_elements) {
  b.run(fmt::format("std::vector::push_back, n = {}", num_elements), [&]() {
    std::vector<size_t> out;
    out.reserve(data.size());
    for (Key k : data) {
      out.push_back(to_index(k));
    }
    return out;
  });

  b.run(fmt::format("std::vector collect, n = {}", num_elements), [&]() {
    return sus::iter::from_range(data)
        .map(to_index)
        .collect<std::vector<size_t>>();
  });

  b.run(fmt::format("sus::Vec::push, n = {}", num_elements), [&]() {
    sus::Vec<size_t> out;
    out.reserve(data.size());
    for (Key k : data) {
      out.push(to_index(k));
    }
    return out;
  });

  b.run(fmt::format("sus::Vec collect, n = {}", num_elements), [&]() {
    return sus::iter::from_range(data)
        .map(to_index)
        .collect<sus::Vec<size_t>>();
  });
}

TEST(BenchVecMap, TransformToIndices_1000) {
  auto data = generate_key_data(1'000u);
  auto b = ankerl::nanobench::Bench();
  transform_to_indicies(b, data, 1'000u);
}
TEST(BenchVecMap, TransformToIndices_100_000) {
  auto data = generate_key_data(100'000u);
  auto b = ankerl::nanobench::Bench();
  transform_to_indicies(b, data, 100'000u);
}
TEST(BenchVecMap, TransformToIndices_10_000_000) {
  auto data = generate_key_data(10'000'000u);
  auto b = ankerl::nanobench::Bench();
  transform_to_indicies(b, data, 10'000'000u);
}

static void more_expensive_int_transformation(ankerl::nanobench::Bench& b,
                                              const std::vector<int>& data,
                                              usize num_elements) {
  b.run(fmt::format("std::vector::push_back, n = {}", num_elements), [&]() {
		std::vector<int> out;
		out.reserve(data.size());
		for (int d : data) {
			out.push_back(static_cast<int>(std::sin(d)));
		}
		return out;
  });

  b.run(fmt::format("std::vector collect, n = {}", num_elements), [&]() {
    return sus::iter::from_range(data)
        .map([](int d) { return static_cast<int>(std::sin(d)); })
        .collect<std::vector<int>>();
  });

  b.run(fmt::format("sus::Vec::push, n = {}", num_elements), [&]() {
    sus::Vec<size_t> out;
    out.reserve(data.size());
    for (int d : data) {
			out.push(static_cast<int>(std::sin(d)));
    }
    return out;
  });

  b.run(fmt::format("sus::Vec collect, n = {}", num_elements), [&]() {
    return sus::iter::from_range(data)
        .map([](int d) { return static_cast<int>(std::sin(d)); })
        .collect<sus::Vec<int>>();
  });
}

TEST(BenchVecMap, MoreExpensiveIntTransformation_1000) {
  auto data = generate_data(1'000u);
  auto b = ankerl::nanobench::Bench();
  more_expensive_int_transformation(b, data, 1'000u);
}
TEST(BenchVecMap, MoreExpensiveIntTransformation_100_000) {
  auto data = generate_data(100'000u);
  auto b = ankerl::nanobench::Bench();
  more_expensive_int_transformation(b, data, 100'000u);
}
TEST(BenchVecMap, MoreExpensiveIntTransformation_10_000_000) {
  auto data = generate_data(10'000'000u);
  auto b = ankerl::nanobench::Bench();
  more_expensive_int_transformation(b, data, 10'000'000u);
}

