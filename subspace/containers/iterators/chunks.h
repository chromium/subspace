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
template <class T>
class SliceMut;
}  // namespace sus::containers

namespace sus::containers {

/// An iterator over a slice in (non-overlapping) chunks (`chunk_size` elements
/// at a time), starting at the beginning of the slice.
///
/// When the slice len is not evenly divided by the chunk size, the last slice
/// of the iteration will be the remainder.
///
/// This struct is created by the `chunks()` method on slices.
template <class ItemT>
struct [[nodiscard]] [[sus_trivial_abi]] Chunks final
    : public ::sus::iter::IteratorBase<Chunks<ItemT>,
                                       ::sus::containers::Slice<ItemT>> {
 public:
  // `Item` is a `Slice<T>`.
  using Item = ::sus::containers::Slice<ItemT>;

 private:
  // `SliceItem` is a `T`.
  using SliceItem = ItemT;

 public:
  Chunks(Chunks&&) = default;
  Chunks& operator=(Chunks&&) = default;

  /// sus::mem::Clone trait.
  Chunks clone() const noexcept {
    return Chunks(::sus::clone(v_), chunk_size_);
  }

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept {
    if (v_.is_empty()) [[unlikely]] {
      return Option<Item>();
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
      return ::sus::Option<Item>();
    } else {
      const auto len = v_.len();
      const auto remainder = len % chunk_size_;
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
          v_.split_at_unchecked(::sus::marker::unsafe_fn, len - chunksz);
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

  static constexpr auto with(const Slice<ItemT>& values,
                             ::sus::num::usize chunk_size) noexcept {
    return Chunks(values, chunk_size);
  }

  constexpr Chunks(const Slice<ItemT>& values,
                   ::sus::num::usize chunk_size) noexcept
      : v_(values), chunk_size_(chunk_size) {}

  Slice<ItemT> v_;
  ::sus::num::usize chunk_size_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(v_),
                                  decltype(chunk_size_));
};

/// An iterator over mutable a slice in (non-overlapping) chunks (`chunk_size`
/// elements at a time), starting at the beginning of the slice.
///
/// When the slice len is not evenly divided by the chunk size, the last slice
/// of the iteration will be the remainder.
///
/// This struct is created by the `chunks_mut()` method on slices.
template <class ItemT>
struct [[nodiscard]] [[sus_trivial_abi]] ChunksMut final
    : public ::sus::iter::IteratorBase<ChunksMut<ItemT>,
                                       ::sus::containers::SliceMut<ItemT>> {
 public:
  // `Item` is a `SliceMut<T>`.
  using Item = ::sus::containers::SliceMut<ItemT>;

 private:
  static_assert(!std::is_reference_v<ItemT>);
  static_assert(!std::is_const_v<ItemT>);
  // `SliceItem` is a `T`.
  using SliceItem = ItemT;

 public:
  ChunksMut(ChunksMut&&) = default;
  ChunksMut& operator=(ChunksMut&&) = default;

  /// sus::mem::Clone trait.
  ChunksMut clone() const noexcept {
    return ChunksMut(::sus::clone(v_), chunk_size_);
  }

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept {
    if (v_.is_empty()) [[unlikely]] {
      return Option<Item>();
    } else {
      const auto chunksz = ::sus::ops::min(v_.len(), chunk_size_);
      auto [fst, snd] =
          v_.split_at_mut_unchecked(::sus::marker::unsafe_fn, chunksz);
      v_ = snd;
      return Option<SliceMut<ItemT>>::some(fst);
    }
  }

  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept {
    if (v_.is_empty()) [[unlikely]] {
      return ::sus::Option<Item>();
    } else {
      const auto len = v_.len();
      const auto remainder = len % chunk_size_;
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
          v_.split_at_mut_unchecked(::sus::marker::unsafe_fn, len - chunksz);
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
  // Constructed by SliceMut.
  friend class SliceMut<ItemT>;

  static constexpr auto with(const SliceMut<ItemT>& values,
                             ::sus::num::usize chunk_size) noexcept {
    return ChunksMut(values, chunk_size);
  }

  constexpr ChunksMut(const SliceMut<ItemT>& values,
                      ::sus::num::usize chunk_size) noexcept
      : v_(values), chunk_size_(chunk_size) {}

  SliceMut<ItemT> v_;
  ::sus::num::usize chunk_size_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(v_),
                                  decltype(chunk_size_));
};

/// An iterator over a slice in (non-overlapping) chunks (`chunk_size` elements
/// at a time), starting at the beginning of the slice.
///
/// When the slice len is not evenly divided by the chunk size, the last up to
/// `chunk_size-1` elements will be omitted but can be retrieved from the
/// remainder function from the iterator.
///
/// This struct is created by the `chunks_exact()` method on slices.
template <class ItemT>
struct [[nodiscard]] [[sus_trivial_abi]] ChunksExact final
    : public ::sus::iter::IteratorBase<ChunksExact<ItemT>,
                                       ::sus::containers::Slice<ItemT>> {
 public:
  // `Item` is a `Slice<T>`.
  using Item = ::sus::containers::Slice<ItemT>;

 private:
  // `SliceItem` is a `T`.
  using SliceItem = ItemT;

 public:
  ChunksExact(ChunksExact&&) = default;
  ChunksExact& operator=(ChunksExact&&) = default;

  /// sus::mem::Clone trait.
  ChunksExact clone() const noexcept {
    return ChunksExact(::sus::clone(v_), chunk_size_);
  }

  /// Returns the remainder of the original slice that is not going to be
  /// returned by the iterator. The returned slice has at most `chunk_size-1`
  /// elements.
  [[nodiscard]] Item remainder() const& { return rem_; }

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept {
    if (v_.len() < chunk_size_) [[unlikely]] {
      return Option<Item>();
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
      return ::sus::Option<Item>();
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

  static constexpr auto with(const Slice<ItemT>& values,
                             ::sus::num::usize chunk_size) noexcept {
    auto rem = values.len() % chunk_size;
    auto fst_len = values.len() - rem;
    // SAFETY: 0 <= fst_len <= values.len() by construction above.
    auto [fst, snd] =
        values.split_at_unchecked(::sus::marker::unsafe_fn, fst_len);
    return ChunksExact(fst, snd, chunk_size);
  }

  constexpr ChunksExact(const Slice<ItemT>& values,
                        const Slice<ItemT>& remainder,
                        ::sus::num::usize chunk_size) noexcept
      : v_(values), rem_(remainder), chunk_size_(chunk_size) {}

  Slice<ItemT> v_;
  Slice<ItemT> rem_;
  ::sus::num::usize chunk_size_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(v_),
                                  decltype(rem_), decltype(chunk_size_));
};

/// An iterator over a mutable slice in (non-overlapping) chunks (`chunk_size`
/// elements at a time), starting at the beginning of the slice.
///
/// When the slice len is not evenly divided by the chunk size, the last up to
/// `chunk_size-1` elements will be omitted but can be retrieved from the
/// remainder function from the iterator.
///
/// This struct is created by the `chunks_exact_mut()` method on slices.
template <class ItemT>
struct [[nodiscard]] [[sus_trivial_abi]] ChunksExactMut final
    : public ::sus::iter::IteratorBase<ChunksExactMut<ItemT>,
                                       ::sus::containers::SliceMut<ItemT>> {
 public:
  // `Item` is a `SliceMut<T>`.
  using Item = ::sus::containers::SliceMut<ItemT>;

 private:
  // `SliceItem` is a `T`.
  using SliceItem = ItemT;

 public:
  ChunksExactMut(ChunksExactMut&&) = default;
  ChunksExactMut& operator=(ChunksExactMut&&) = default;

  /// sus::mem::Clone trait.
  ChunksExactMut clone() const noexcept {
    return ChunksExactMut(::sus::clone(v_), chunk_size_);
  }
  /// Returns the remainder of the original slice that is not going to be
  /// returned by the iterator. The returned slice has at most `chunk_size-1`
  /// elements.
  [[nodiscard]] Item remainder() const& { return rem_; }

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept {
    if (v_.len() < chunk_size_) [[unlikely]] {
      return Option<Item>();
    } else {
      // SAFETY: `split_at_mut_unchecked` requires the argument be less than or
      // equal to the length. This is guaranteed by the checking exactly that
      // in the condition above, and we are in the else branch.
      auto [fst, snd] =
          v_.split_at_mut_unchecked(::sus::marker::unsafe_fn, chunk_size_);
      v_ = snd;
      return Option<Item>::some(fst);
    }
  }

  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept {
    if (v_.len() < chunk_size_) [[unlikely]] {
      return ::sus::Option<Item>();
    } else {
      // SAFETY: `split_at_mut_unchecked` requires the argument be less than or
      // equal to the length. This is guaranteed by subtracting an unsigned (and
      // thus non-negative) value from the length.
      auto [fst, snd] = v_.split_at_mut_unchecked(::sus::marker::unsafe_fn,
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
  friend class SliceMut<ItemT>;

  static constexpr auto with(const SliceMut<ItemT>& values,
                             ::sus::num::usize chunk_size) noexcept {
    auto rem = values.len() % chunk_size;
    auto fst_len = values.len() - rem;
    // SAFETY: 0 <= fst_len <= values.len() by construction above.
    auto [fst, snd] =
        values.split_at_mut_unchecked(::sus::marker::unsafe_fn, fst_len);
    return ChunksExactMut(fst, snd, chunk_size);
  }

  constexpr ChunksExactMut(const SliceMut<ItemT>& values,
                           const SliceMut<ItemT>& remainder,
                           ::sus::num::usize chunk_size) noexcept
      : v_(values), rem_(remainder), chunk_size_(chunk_size) {}

  SliceMut<ItemT> v_;
  SliceMut<ItemT> rem_;
  ::sus::num::usize chunk_size_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(v_),
                                  decltype(rem_), decltype(chunk_size_));
};

/// An iterator over a slice in (non-overlapping) chunks (`chunk_size` elements
/// at a time), starting at the end of the slice.
///
/// When the slice len is not evenly divided by the chunk size, the last slice
/// of the iteration will be the remainder.
///
/// This struct is created by the `rchunks()` method on slices.
template <class ItemT>
struct [[nodiscard]] [[sus_trivial_abi]] RChunks final
    : public ::sus::iter::IteratorBase<RChunks<ItemT>,
                                       ::sus::containers::Slice<ItemT>> {
 public:
  // `Item` is a `Slice<T>`.
  using Item = ::sus::containers::Slice<ItemT>;

 private:
  // `SliceItem` is a `T`.
  using SliceItem = ItemT;

 public:
  RChunks(RChunks&&) = default;
  RChunks& operator=(RChunks&&) = default;

  /// sus::mem::Clone trait.
  RChunks clone() const noexcept {
    return RChunks(::sus::clone(v_), chunk_size_);
  }

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept {
    if (v_.is_empty()) [[unlikely]] {
      return Option<Item>();
    } else {
      const auto len = v_.len();
      auto chunksz = ::sus::ops::min(len, chunk_size_);
      auto [fst, snd] =
          v_.split_at_unchecked(::sus::marker::unsafe_fn, len - chunksz);
      v_ = fst;
      return Option<Item>::some(snd);
    }
  }

  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept {
    if (v_.is_empty()) [[unlikely]] {
      return ::sus::Option<Item>();
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
          v_.split_at_unchecked(::sus::marker::unsafe_fn, chunksz);
      v_ = snd;
      return ::sus::Option<Item>::some(fst);
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

  static constexpr auto with(const Slice<ItemT>& values,
                             ::sus::num::usize chunk_size) noexcept {
    return RChunks(values, chunk_size);
  }

  constexpr RChunks(const Slice<ItemT>& values,
                    ::sus::num::usize chunk_size) noexcept
      : v_(values), chunk_size_(chunk_size) {}

  Slice<ItemT> v_;
  ::sus::num::usize chunk_size_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(v_),
                                  decltype(chunk_size_));
};

/// An iterator over a mutable slice in (non-overlapping) chunks (`chunk_size`
/// elements at a time), starting at the end of the slice.
///
/// When the slice len is not evenly divided by the chunk size, the last slice
/// of the iteration will be the remainder.
///
/// This struct is created by the `rchunks_mut()` method on slices.
template <class ItemT>
struct [[nodiscard]] [[sus_trivial_abi]] RChunksMut final
    : public ::sus::iter::IteratorBase<RChunksMut<ItemT>,
                                       ::sus::containers::SliceMut<ItemT>> {
 public:
  // `Item` is a `SliceMut<T>`.
  using Item = ::sus::containers::SliceMut<ItemT>;

 private:
  static_assert(!std::is_reference_v<ItemT>);
  static_assert(!std::is_const_v<ItemT>);
  // `SliceItem` is a `T`.
  using SliceItem = ItemT;

 public:
  RChunksMut(RChunksMut&&) = default;
  RChunksMut& operator=(RChunksMut&&) = default;

  /// sus::mem::Clone trait.
  RChunksMut clone() const noexcept {
    return ChunksMut(::sus::clone(v_), chunk_size_);
  }

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept {
    if (v_.is_empty()) [[unlikely]] {
      return Option<Item>();
    } else {
      const auto len = v_.len();
      auto chunksz = ::sus::ops::min(len, chunk_size_);
      auto [fst, snd] =
          v_.split_at_mut_unchecked(::sus::marker::unsafe_fn, len - chunksz);
      v_ = fst;
      return Option<SliceMut<ItemT>>::some(snd);
    }
  }

  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept {
    if (v_.is_empty()) [[unlikely]] {
      return ::sus::Option<Item>();
    } else {
      const auto remainder = v_.len() % chunk_size_;
      const auto chunksz = remainder != 0 ? remainder : chunk_size_;
      // SAFETY: Similar to `RChunks::next_back`
      auto [fst, snd] =
          v_.split_at_mut_unchecked(::sus::marker::unsafe_fn, chunksz);
      v_ = snd;
      return ::sus::Option<Item>::some(fst);
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
  // Constructed by SliceMut.
  friend class SliceMut<ItemT>;

  static constexpr auto with(const SliceMut<ItemT>& values,
                             ::sus::num::usize chunk_size) noexcept {
    return RChunksMut(values, chunk_size);
  }

  constexpr RChunksMut(const SliceMut<ItemT>& values,
                       ::sus::num::usize chunk_size) noexcept
      : v_(values), chunk_size_(chunk_size) {}

  SliceMut<ItemT> v_;
  ::sus::num::usize chunk_size_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(v_),
                                  decltype(chunk_size_));
};

/// An iterator over a slice in (non-overlapping) chunks (`chunk_size` elements
/// at a time), starting at the end of the slice.
///
/// When the slice len is not evenly divided by the chunk size, the last up to
/// `chunk_size-1` elements will be omitted but can be retrieved from the
/// remainder function from the iterator.
///
/// This struct is created by the `rchunks_exact()` method on slices.
template <class ItemT>
struct [[nodiscard]] [[sus_trivial_abi]] RChunksExact final
    : public ::sus::iter::IteratorBase<RChunksExact<ItemT>,
                                       ::sus::containers::Slice<ItemT>> {
 public:
  // `Item` is a `Slice<T>`.
  using Item = ::sus::containers::Slice<ItemT>;

 private:
  // `SliceItem` is a `T`.
  using SliceItem = ItemT;

 public:
  RChunksExact(RChunksExact&&) = default;
  RChunksExact& operator=(RChunksExact&&) = default;

  /// sus::mem::Clone trait.
  RChunksExact clone() const noexcept {
    return RChunksExact(::sus::clone(v_), chunk_size_);
  }

  /// Returns the remainder of the original slice that is not going to be
  /// returned by the iterator. The returned slice has at most `chunk_size-1`
  /// elements.
  [[nodiscard]] Item remainder() const& { return rem_; }

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept {
    if (v_.len() < chunk_size_) [[unlikely]] {
      return Option<Item>();
    } else {
      // SAFETY: `split_at_unchecked` requires the argument be less than or
      // equal to the length. This is guaranteed by subtracting a non-negative
      // value from the len.
      auto [fst, snd] = v_.split_at_unchecked(::sus::marker::unsafe_fn,
                                              v_.len() - chunk_size_);
      v_ = fst;
      return Option<Item>::some(snd);
    }
  }

  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept {
    if (v_.len() < chunk_size_) [[unlikely]] {
      return ::sus::Option<Item>();
    } else {
      // SAFETY: `split_at_unchecked` requires the argument be less than or
      // equal to the length. This is guaranteed by checking the condition
      // above, and that we are in the else branch.
      auto [fst, snd] =
          v_.split_at_unchecked(::sus::marker::unsafe_fn, chunk_size_);
      v_ = snd;
      return ::sus::Option<Item>::some(fst);
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

  static constexpr auto with(const Slice<ItemT>& values,
                             ::sus::num::usize chunk_size) noexcept {
    auto rem = values.len() % chunk_size;
    // SAFETY: 0 <= rem <= values.len() by construction above.
    auto [fst, snd] = values.split_at_unchecked(::sus::marker::unsafe_fn, rem);
    return RChunksExact(snd, fst, chunk_size);
  }

  constexpr RChunksExact(const Slice<ItemT>& values,
                         const Slice<ItemT>& remainder,
                         ::sus::num::usize chunk_size) noexcept
      : v_(values), rem_(remainder), chunk_size_(chunk_size) {}

  Slice<ItemT> v_;
  Slice<ItemT> rem_;
  ::sus::num::usize chunk_size_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(v_),
                                  decltype(rem_), decltype(chunk_size_));
};

/// An iterator over a mutable slice in (non-overlapping) chunks (`chunk_size`
/// elements at a time), starting at the end of the slice.
///
/// When the slice len is not evenly divided by the chunk size, the last up to
/// `chunk_size-1` elements will be omitted but can be retrieved from the
/// remainder function from the iterator.
///
/// This struct is created by the `rchunks_exact_mut()` method on slices.
template <class ItemT>
struct [[nodiscard]] [[sus_trivial_abi]] RChunksExactMut final
    : public ::sus::iter::IteratorBase<RChunksExactMut<ItemT>,
                                       ::sus::containers::SliceMut<ItemT>> {
 public:
  // `Item` is a `SliceMut<T>`.
  using Item = ::sus::containers::SliceMut<ItemT>;

 private:
  // `SliceItem` is a `T`.
  using SliceItem = ItemT;

 public:
  RChunksExactMut(RChunksExactMut&&) = default;
  RChunksExactMut& operator=(RChunksExactMut&&) = default;

  /// sus::mem::Clone trait.
  RChunksExactMut clone() const noexcept {
    return RChunksExactMut(::sus::clone(v_), chunk_size_);
  }
  /// Returns the remainder of the original slice that is not going to be
  /// returned by the iterator. The returned slice has at most `chunk_size-1`
  /// elements.
  [[nodiscard]] Item remainder() const& { return rem_; }

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept {
    if (v_.len() < chunk_size_) [[unlikely]] {
      return Option<Item>();
    } else {
      // SAFETY: `split_at_mut_unchecked` requires the argument be less than or
      // equal to the length. This is guaranteed by subtracting a non-negative
      // value from the len.
      auto [fst, snd] = v_.split_at_mut_unchecked(::sus::marker::unsafe_fn,
                                                  v_.len() - chunk_size_);
      v_ = fst;
      return Option<Item>::some(snd);
    }
  }

  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept {
    if (v_.len() < chunk_size_) [[unlikely]] {
      return ::sus::Option<Item>();
    } else {
      // SAFETY: `split_at_unchecked` requires the argument be less than or
      // equal to the length. This is guaranteed by checking the condition
      // above, and that we are in the else branch.
      auto [fst, snd] =
          v_.split_at_mut_unchecked(::sus::marker::unsafe_fn, chunk_size_);
      v_ = snd;
      return ::sus::Option<Item>::some(fst);
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
  friend class SliceMut<ItemT>;

  static constexpr auto with(const SliceMut<ItemT>& values,
                             ::sus::num::usize chunk_size) noexcept {
    auto rem = values.len() % chunk_size;
    // SAFETY: 0 <= rem <= values.len() by construction above.
    auto [fst, snd] =
        values.split_at_mut_unchecked(::sus::marker::unsafe_fn, rem);
    return RChunksExactMut(snd, fst, chunk_size);
  }

  constexpr RChunksExactMut(const SliceMut<ItemT>& values,
                            const SliceMut<ItemT>& remainder,
                            ::sus::num::usize chunk_size) noexcept
      : v_(values), rem_(remainder), chunk_size_(chunk_size) {}

  SliceMut<ItemT> v_;
  SliceMut<ItemT> rem_;
  ::sus::num::usize chunk_size_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(v_),
                                  decltype(rem_), decltype(chunk_size_));
};

}  // namespace sus::containers
