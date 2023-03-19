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

#include "subspace/iter/iterator_defn.h"
#include "subspace/num/unsigned_integer.h"

namespace sus::containers {
template <class T>
class Slice;
}

namespace sus::containers::__private {

template <class ItemT>
struct [[nodiscard]] [[sus_trivial_abi]] Chunks final
    : public ::sus::iter::IteratorBase<Chunks<ItemT>,
                                       ::sus::containers::Slice<ItemT>> {
 public:
  using Item = ::sus::containers::Slice<ItemT>;

 private:
  // `Item` is a `Slice<const T>`.
  static_assert(!std::is_reference_v<ItemT>);
  static_assert(std::is_const_v<ItemT>);
  // `SliceItem` is a `const T`.
  using SliceItem = std::remove_const_t<ItemT>;

 public:
  static constexpr auto with(Slice<ItemT>&& values,
                             ::sus::num::usize chunk_size) noexcept {
    return Chunks(::sus::move(values), chunk_size);
  }

  Chunks(Chunks&&) = default;
  Chunks& operator=(Chunks&&) = default;

  ~Chunks() = default;

  /// sus::mem::Clone trait.
  Chunks clone() const& noexcept {
    return Chunks(::sus::clone(v_), chunk_size_);
  }

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept {
    if (v_.is_empty()) [[unlikely]] {
      return Option<Item>::none();
    } else {
      auto chunksz = ::sus::ops::min(v_.len(), chunk_size_);
      auto [fst, snd] =
          v_.split_at_unchecked(::sus::marker::unsafe_fn, chunksz);
      v_ = ::sus::move(snd);
      return Option<Item>::some(::sus::move(fst));
    }
  }

  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept {
    if (v_.is_empty()) {
      return ::sus::Option<Item>::none();
    } else {
      ::sus::num::usize remainder = v_.len() % chunk_size_;
      ::sus::num::usize chunksz = remainder != 0u ? remainder : chunk_size_;
      // SAFETY: `split_at_unchecked()` requires the argument be less than or
      // equal to the length. This is guaranteed, but subtle: `chunksz`
      // will always either be `v_.len() % chunk_size_`, which will always
      // evaluate to strictly less than `v_.len()` (or panic, in the case that
      // `chunk_size_` is zero), or it can be `chunk_size_`, in the case that
      // the length is exactly divisible by the chunk size.
      //
      // While it seems like using `chunk_size_` in this case could lead to a
      // value greater than `v_.len()`, it cannot: if `chunk_size_` were greater
      // than `v_.len()`, then `v_.len() % chunk_size_` would return nonzero
      // (note that in this branch of the `if`, we already know that `v_` is
      // non-empty).
      auto [fst, snd] =
          v_.split_at_unchecked(::sus::marker::unsafe_fn, v_.len() - chunksz);
      v_ = ::sus::move(fst);
      return ::sus::Option<Item>::some(::sus::move(snd));
    }
  }

  // Replace the default impl in sus::iter::IteratorBase.
  ::sus::iter::SizeHint size_hint() const noexcept final {
    const auto remaining = exact_size_hint();
    return {remaining, ::sus::Option<::sus::num::usize>::some(remaining)};
  }

  /// sus::iter::ExactSizeIterator trait.
  ::sus::num::usize exact_size_hint() const noexcept {
    if (v_.is_empty()) {
      return 0u;
    } else {
      ::sus::num::usize n = v_.len() / chunk_size_;
      ::sus::num::usize rem = v_.len() % chunk_size_;
      return rem > 0u ? n + 1u : n;
    }
  }

  ::sus::num::usize count() noexcept override { return v_.len() / chunk_size_; }

  // TODO: Impl nth(), last(), nth_back().

 private:
  constexpr Chunks(Slice<ItemT>&& values, ::sus::num::usize chunk_size) noexcept
      : v_(::sus::move(values)), chunk_size_(chunk_size) {}

  Slice<ItemT> v_;
  ::sus::num::usize chunk_size_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(v_),
                                  decltype(chunk_size_));
};

template <class ItemT>
struct [[nodiscard]] [[sus_trivial_abi]] ChunksMut final
    : public ::sus::iter::IteratorBase<ChunksMut<ItemT>,
                                       ::sus::containers::Slice<ItemT>> {
 public:
  using Item = ::sus::containers::Slice<ItemT>;

 private:
  // `Item` is a `Slice<T>`.
  static_assert(!std::is_reference_v<ItemT>);
  static_assert(!std::is_const_v<ItemT>);
  // `SliceItem` is a `T`.
  using SliceItem = std::remove_const_t<ItemT>;

 public:
  static constexpr auto with(Slice<ItemT>&& values,
                             ::sus::num::usize chunk_size) noexcept {
    return ChunksMut(::sus::move(values), chunk_size);
  }

  ChunksMut(ChunksMut&&) = default;
  ChunksMut& operator=(ChunksMut&&) = default;

  ~ChunksMut() = default;

  /// sus::mem::Clone trait.
  ChunksMut clone() const& noexcept {
    return ChunksMut(::sus::clone(v_), chunk_size_);
  }

  // sus::iter::Iterator trait.
  Option<Slice<ItemT>> next() noexcept {
    if (v_.is_empty()) [[unlikely]] {
      return Option<Item>::none();
    } else {
      const auto chunksz = ::sus::ops::min(v_.len(), chunk_size_);
      auto [fst, snd] =
          v_.split_at_mut_unchecked(::sus::marker::unsafe_fn, chunksz);
      static_assert(std::same_as<Slice<::sus::num::i32>, decltype(v_)>);
      static_assert(std::same_as<Slice<::sus::num::i32>, decltype(fst)>);
      static_assert(std::same_as<Slice<::sus::num::i32>, decltype(snd)>);
      v_ = ::sus::move(snd);
      return Option<Slice<ItemT>>::some(::sus::move(fst));
    }
  }

  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept {
    if (v_.is_empty()) {
      return ::sus::Option<Item>::none();
    } else {
      const auto remainder = v_.len() % chunk_size_;
      const auto chunksz = remainder != 0u ? remainder : chunk_size_;
      // SAFETY: `split_at_unchecked()` requires the argument be less than or
      // equal to the length. This is guaranteed, but subtle: `chunksz`
      // will always either be `v_.len() % chunk_size_`, which will always
      // evaluate to strictly less than `v_.len()` (or panic, in the case that
      // `chunk_size_` is zero), or it can be `chunk_size_`, in the case that
      // the length is exactly divisible by the chunk size.
      //
      // While it seems like using `chunk_size_` in this case could lead to a
      // value greater than `v_.len()`, it cannot: if `chunk_size_` were greater
      // than `v_.len()`, then `v_.len() % chunk_size_` would return nonzero
      // (note that in this branch of the `if`, we already know that `v_` is
      // non-empty).
      auto [fst, snd] =
          v_.split_at_mut_unchecked(::sus::marker::unsafe_fn, v_.len() - chunksz);
      v_ = ::sus::move(fst);
      return ::sus::Option<Item>::some(::sus::move(snd));
    }
  }

  // Replace the default impl in sus::iter::IteratorBase.
  ::sus::iter::SizeHint size_hint() const noexcept final {
    const auto remaining = exact_size_hint();
    return {remaining, ::sus::Option<::sus::num::usize>::some(remaining)};
  }

  /// sus::iter::ExactSizeIterator trait.
  ::sus::num::usize exact_size_hint() const noexcept {
    if (v_.is_empty()) {
      return 0u;
    } else {
      const auto n = v_.len() / chunk_size_;
      const auto rem = v_.len() % chunk_size_;
      return rem > 0u ? n + 1u : n;
    }
  }

  ::sus::num::usize count() noexcept override { return v_.len() / chunk_size_; }

  // TODO: Impl nth(), last(), nth_back().

 private:
  constexpr ChunksMut(Slice<ItemT>&& values, ::sus::num::usize chunk_size) noexcept
      : v_(::sus::move(values)), chunk_size_(chunk_size) {}

  Slice<ItemT> v_;
  ::sus::num::usize chunk_size_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(v_),
                                  decltype(chunk_size_));
};

}  // namespace sus::containers::__private
