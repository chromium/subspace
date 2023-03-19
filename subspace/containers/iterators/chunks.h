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

namespace sus::containers {

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
  // sus::iter::Iterator trait.
  Option<Item> next() noexcept {
    if (v_.is_empty()) [[unlikely]] {
      return Option<Item>::none();
    } else {
      auto chunksz = ::sus::ops::min(v_.len(), chunk_size_);
      auto [fst, snd] =
          v_.split_at_unchecked(::sus::marker::unsafe_fn, chunksz);
      v_ = snd;
      return Option<Item>::some(fst);
    }
  }

  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept {
    if (v_.is_empty()) [[unlikely]] {
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
          v_.split_at_unchecked(::sus::marker::unsafe_fn, v_.len() - chunksz);
      v_ = fst;
      return ::sus::Option<Item>::some(snd);
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

  // TODO: Impl count(), nth(), last(), nth_back().

 private:
  // Constructed by Slice.
  friend class Slice<ItemT>;

  static constexpr auto with(Slice<ItemT>&& values,
                             ::sus::num::usize chunk_size) noexcept {
    return Chunks(::sus::move(values), chunk_size);
  }

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
      v_ = snd;
      return Option<Slice<ItemT>>::some(fst);
    }
  }

  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept {
    if (v_.is_empty()) [[unlikely]] {
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
      auto [fst, snd] = v_.split_at_mut_unchecked(::sus::marker::unsafe_fn,
                                                  v_.len() - chunksz);
      v_ = fst;
      return ::sus::Option<Item>::some(snd);
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

  // TODO: Impl count(), nth(), last(), nth_back().

 private:
  // Constructed by Slice.
  friend class Slice<ItemT>;

  static constexpr auto with(Slice<ItemT>&& values,
                             ::sus::num::usize chunk_size) noexcept {
    return ChunksMut(::sus::move(values), chunk_size);
  }

  constexpr ChunksMut(Slice<ItemT>&& values,
                      ::sus::num::usize chunk_size) noexcept
      : v_(::sus::move(values)), chunk_size_(chunk_size) {}

  Slice<ItemT> v_;
  ::sus::num::usize chunk_size_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(v_),
                                  decltype(chunk_size_));
};

template <class ItemT>
struct [[nodiscard]] [[sus_trivial_abi]] ChunksExact final
    : public ::sus::iter::IteratorBase<ChunksExact<ItemT>,
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
  /// Returns the remainder of the original slice that is not going to be
  /// returned by the iterator. The returned slice has at most `chunk_size-1`
  /// elements.
  [[nodiscard]] Item remainder() const& { return rem_; }

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept {
    if (v_.len() < chunk_size_) [[unlikely]] {
      return Option<Item>::none();
    } else {
      // SAFETY: `split_at_unchecked` requires the argument be less than or
      // equal to the length. This is guaranteed by the checking exactly that
      // in the condition above, and we are in the else branch.
      auto [fst, snd] =
          v_.split_at_unchecked(::sus::marker::unsafe_fn, chunk_size_);
      v_ = snd;
      return Option<Item>::some(fst);
    }
  }

  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept {
    if (v_.len() < chunk_size_) [[unlikely]] {
      return ::sus::Option<Item>::none();
    } else {
      // SAFETY: `split_at_unchecked` requires the argument be less than or
      // equal to the length. This is guaranteed by subtracting an unsigned (and
      // thus non-negative) value from the length.
      auto [fst, snd] = v_.split_at_unchecked(::sus::marker::unsafe_fn,
                                              v_.len() - chunk_size_);
      v_ = fst;
      return ::sus::Option<Item>::some(snd);
    }
  }

  // Replace the default impl in sus::iter::IteratorBase.
  ::sus::iter::SizeHint size_hint() const noexcept final {
    const auto remaining = exact_size_hint();
    return {remaining, ::sus::Option<::sus::num::usize>::some(remaining)};
  }

  /// sus::iter::ExactSizeIterator trait.
  ::sus::num::usize exact_size_hint() const noexcept {
    return v_.len() / chunk_size_;
  }

  // TODO: Impl count(), nth(), last(), nth_back().

 private:
  // Constructed by Slice.
  friend class Slice<ItemT>;

  static constexpr auto with(Slice<ItemT>&& values,
                             ::sus::num::usize chunk_size) noexcept {
    auto rem = values.len() % chunk_size;
    auto fst_len = values.len() - rem;
    // SAFETY: 0 <= fst_len <= values.len() by construction above.
    auto [fst, snd] =
        values.split_at_unchecked(::sus::marker::unsafe_fn, fst_len);
    return ChunksExact(::sus::move(fst), ::sus::move(snd), chunk_size);
  }

  constexpr ChunksExact(Slice<ItemT>&& values, Slice<ItemT>&& remainder,
                        ::sus::num::usize chunk_size) noexcept
      : v_(::sus::move(values)),
        rem_(::sus::move(remainder)),
        chunk_size_(chunk_size) {}

  Slice<ItemT> v_;
  Slice<ItemT> rem_;
  ::sus::num::usize chunk_size_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(v_),
                                  decltype(rem_), decltype(chunk_size_));
};

}  // namespace sus::containers
