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

#include "sus/marker/unsafe.h"
#include "sus/mem/relocate.h"
#include "sus/mem/replace.h"
#include "sus/num/unsigned_integer.h"

#if !defined(SUS_ITERATOR_INVALIDATION)
#define SUS_ITERATOR_INVALIDATION 1
#endif
static_assert(SUS_ITERATOR_INVALIDATION == 0 || SUS_ITERATOR_INVALIDATION == 1);

namespace sus::iter {

struct IterRefCounter;

#if SUS_ITERATOR_INVALIDATION

/// An iterator's refcount on the owning container, preventig mutation while the
/// iterator is alive.
struct [[sus_trivial_abi]] IterRef final {
  constexpr IterRef(usize* ptr) noexcept : count_ptr_(ptr) { inc(); }
  constexpr ~IterRef() noexcept { dec(); }
  constexpr IterRef(const IterRef& rhs) noexcept : count_ptr_(rhs.count_ptr_) {
    inc();
  }
  constexpr IterRef& operator=(const IterRef& rhs) noexcept {
    dec();
    count_ptr_ = rhs.count_ptr_;
    inc();
    return *this;
  }
  constexpr IterRef(IterRef&& rhs) noexcept
      : count_ptr_(::sus::mem::replace(rhs.count_ptr_, nullptr)) {}
  constexpr IterRef& operator=(IterRef&& rhs) noexcept {
    dec();
    count_ptr_ = ::sus::mem::replace(rhs.count_ptr_, nullptr);
    return *this;
  }

  // Used to rebuild a view type from an iterator.
  constexpr IterRefCounter to_view() const noexcept;

 private:
  usize* count_ptr_;

  constexpr void inc() {
    // TODO: Remove this condition? Some slices have no container so the
    // iterator doesn't either.
    if (count_ptr_) *count_ptr_ += 1u;
  }
  constexpr void dec() {
    if (count_ptr_) *count_ptr_ -= 1u;
  }

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn,
                                  decltype(count_ptr_));
};

/// Reference counting outstanding iterators (and view types since they need to
/// be able to produce iterators) in order to catch iterator invalidation and
/// prevent them from being used afterward. Mutating the container should check
/// that the `count` is empty. This is much like a `RefCell` in Rust, using
/// runtime verfication that modification does not occur while there are
/// outstanding references.
struct [[sus_trivial_abi]] IterRefCounter final {
  static constexpr IterRefCounter for_owner() noexcept {
    return IterRefCounter(FOR_OWNER, 0_usize);
  }
  static constexpr IterRefCounter empty_for_view() noexcept {
    return IterRefCounter(FOR_VIEW, nullptr);
  }

  /// Only valid to be called on owning containers such as Vec.
  constexpr IterRef to_iter_from_owner() const noexcept {
    return IterRef(&count);
  }
  /// Only valid to be called on non-owning views such as Slice.
  constexpr IterRef to_iter_from_view() const noexcept {
    return IterRef(count_ptr);
  }

  /// Only valid to be called on owning containers such as Vec.
  constexpr IterRefCounter to_view_from_owner() const noexcept {
    return IterRefCounter(FOR_VIEW, &count);
  }
  /// Only valid to be called on non-owning views such as Slice.
  constexpr IterRefCounter to_view_from_view() const noexcept {
    // Clone of a view points to the same owner.
    return IterRefCounter(FOR_VIEW, count_ptr);
  }

  /// Only valid to be called on owning containers such as Vec.
  constexpr usize count_from_owner() const noexcept { return count; }

  /// Resets self to no ref counts, returning a new IterRefCounter containing
  /// the old ref counts.
  ///
  /// Only valid to be called on owning containers such as Vec.
  constexpr IterRefCounter take_for_owner() & noexcept {
    return IterRefCounter(FOR_OWNER, ::sus::mem::replace(count, 0u));
  }
  /// Resets self to no pointer to a ref count, returning a new IterRefCounter
  /// containing the old pointetr.
  ///
  constexpr IterRefCounter take_for_view() & noexcept {
    return IterRefCounter(FOR_VIEW, ::sus::mem::replace(count_ptr, nullptr));
  }

 private:
  friend struct IterRef;  // IterRef can re-construct IterRefCounter.

  enum ForOwner { FOR_OWNER };
  constexpr IterRefCounter(ForOwner, usize count) noexcept : count(count) {}
  enum ForView { FOR_VIEW };
  constexpr IterRefCounter(ForView, usize* ptr) noexcept : count_ptr(ptr) {}

  union {
    /// The `count` member is active in owning containers like `Vec`.
    mutable usize count;  // TODO: should be uptr.
    /// The `count_ptr` member is active in view containers like `Slice`. It
    /// points to he owning container. The presence of a `count_ptr` must also
    /// indicate an increment in the `count` it points to, lest the `count_ptr`
    /// become invalid without terminating the program.
    usize* count_ptr;
  };

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(count),
                                  decltype(count_ptr));
};

constexpr IterRefCounter IterRef::to_view() const noexcept {
  return IterRefCounter(IterRefCounter::FOR_VIEW, count_ptr_);
}

#else

struct [[sus_trivial_abi]] IterRef final {
  // Used to rebuild a view type from an iterator.
  constexpr IterRefCounter to_view() const noexcept;

 private:
  sus_class_trivially_relocatable_unchecked(::sus::marker::unsafe_fn);
};

/// Reference counting outstanding iterators (and view types since they need to
/// be able to produce iterators) in order to catch iterator invalidation and
/// prevent them from being used afterward. Mutating the container should check
/// that the `count` is empty. This is much like a `RefCell` in Rust, using
/// runtime verfication that modification does not occur while there are
/// outstanding references.
struct [[sus_trivial_abi]] IterRefCounter final {
  static constexpr IterRefCounter for_owner() noexcept {
    return IterRefCounter();
  }
  static constexpr IterRefCounter empty_for_view() noexcept {
    return IterRefCounter();
  }
  constexpr IterRef to_iter_from_owner() const noexcept { return IterRef(); }
  constexpr IterRef to_iter_from_view() const noexcept { return IterRef(); }
  constexpr IterRefCounter to_view_from_owner() const noexcept {
    return IterRefCounter();
  }
  constexpr IterRefCounter to_view_from_view() const noexcept {
    return IterRefCounter();
  }
  constexpr usize count_from_owner() const noexcept { return 0u; }

  constexpr IterRefCounter take_for_owner() & noexcept {
    return IterRefCounter();
  }
  constexpr IterRefCounter take_for_view() & noexcept {
    return IterRefCounter();
  }

 private:
  sus_class_trivially_relocatable_unchecked(::sus::marker::unsafe_fn);
};

constexpr IterRefCounter IterRef::to_view() const noexcept {
  return IterRefCounter();
}

#endif

static_assert(sus::mem::Copy<IterRefCounter>);
static_assert(sus::mem::Move<IterRefCounter>);
static_assert(sus::mem::relocate_by_memcpy<IterRefCounter>);

}  // namespace sus::iter
