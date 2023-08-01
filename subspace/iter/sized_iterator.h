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

#include <new>  // std::lanuder

#include "subspace/iter/iterator_concept.h"
#include "subspace/iter/size_hint.h"
#include "subspace/mem/clone.h"
#include "subspace/mem/never_value.h"
#include "subspace/mem/relocate.h"
#include "subspace/mem/size_of.h"
#include "subspace/option/option.h"
#include "subspace/ptr/copy.h"
// Doesn't include iterator_defn.h because it's included from there.

namespace sus::iter {

template <class ItemT, size_t SubclassSize, size_t SubclassAlign,
          bool DoubleEndedB, bool ExactSizeB, bool CloneB>
struct [[sus_trivial_abi]] SizedIterator final {
  using Item = ItemT;
  static constexpr bool DoubleEnded = DoubleEndedB;
  static constexpr bool ExactSize = ExactSizeB;
  static constexpr bool Clone = CloneB;

  constexpr SizedIterator(void (*destroy)(char& sized),
                          Option<Item> (*next)(char& sized),
                          Option<Item> (*next_back)(char& sized),
                          SizeHint (*size_hint)(const char& sized),
                          usize (*exact_size_hint)(const char& sized),
                          SizedIterator (*clone)(const char& sized))
      : destroy_(destroy),
        next_(next),
        next_back_(next_back),
        size_hint_(size_hint),
        exact_size_hint_(exact_size_hint),
        clone_(clone) {}

  SizedIterator(SizedIterator&& o) noexcept
      : destroy_(::sus::mem::replace(o.destroy_, nullptr)),
        // next_ is left non-null to allow NeverValueField.
        next_(o.next_),
        next_back_(o.next_back_),
        size_hint_(o.size_hint_),
        exact_size_hint_(o.exact_size_hint_),
        clone_(o.clone_) {
    ::sus::ptr::copy_nonoverlapping(::sus::marker::unsafe_fn, o.sized_, sized_,
                                    SubclassSize);
  }
  SizedIterator& operator=(SizedIterator&& o) noexcept {
    if (destroy_) destroy_(*sized_);
    destroy_ = ::sus::mem::replace(o.destroy_, nullptr);
    // next_ is left non-null to allow NeverValueField.
    next_ = o.next_;
    next_back_ = o.next_back_;
    size_hint_ = o.size_hint_;
    exact_size_hint_ = o.exact_size_hint_;
    clone_ = o.clone_;
    ::sus::ptr::copy_nonoverlapping(::sus::marker::unsafe_fn, o.sized_, sized_,
                                    SubclassSize);
    return *this;
  }

  ~SizedIterator() noexcept {
    if (destroy_) destroy_(*sized_);
  }

  Option<Item> next() noexcept { return next_(*sized_); }
  Option<Item> next_back() noexcept
    requires(DoubleEnded)
  {
    return next_back_(*sized_);
  }
  SizeHint size_hint() const noexcept { return size_hint_(*sized_); }
  usize exact_size_hint() const noexcept
    requires(ExactSize)
  {
    return exact_size_hint_(*sized_);
  }
  SizedIterator clone() const noexcept
    requires(Clone)
  {
    return clone_(*sized_);
  }

  char* as_mut_ptr() noexcept { return sized_; }

 private:
  alignas(SubclassAlign) char sized_[SubclassSize];
  void (*destroy_)(char& sized);
  Option<Item> (*next_)(char& sized);
  // TODO: We could remove this field with a nested struct + template
  // specialization when DoubleEnded is false.
  Option<Item> (*next_back_)(char& sized);
  SizeHint (*size_hint_)(const char& sized);
  // TODO: We could remove this field with a nested struct + template
  // specialization when ExactSize is false.
  usize (*exact_size_hint_)(const char& sized);
  SizedIterator (*clone_)(const char& sized);

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(sized_),
                                  decltype(destroy_), decltype(next_),
                                  decltype(next_back_), decltype(size_hint_),
                                  decltype(exact_size_hint_), decltype(clone_));

  // Declare that the `destroy_` field is never set to `nullptr` for library
  // optimizations.
  sus_class_never_value_field(::sus::marker::unsafe_fn, SizedIterator, next_,
                              nullptr, nullptr);
  // For the NeverValueField.
  explicit constexpr SizedIterator(::sus::mem::NeverValueConstructor) noexcept
      : destroy_(nullptr), next_(nullptr) {}
};

template <class Iter>
struct SizedIteratorType {
  using type =
      SizedIterator<typename Iter::Item, ::sus::mem::size_of<Iter>(),
                    alignof(Iter),
                    ::sus::iter::DoubleEndedIterator<Iter, typename Iter::Item>,
                    ::sus::iter::ExactSizeIterator<Iter, typename Iter::Item>,
                    ::sus::mem::Clone<Iter>>;
};

/// Make a SizedIterator which wraps a trivially relocatable iterator and erases
/// its type.
///
/// This type may only be used when the IteratorSubclass can be trivially
/// relocated. It stores the SubclassIterator directly into the SizedIterator,
/// erasing its type but remaining trivially relocatable.
template <::sus::mem::Move Iter, int&..., class Item = typename Iter::Item>
inline SizedIteratorType<Iter>::type make_sized_iterator(Iter&& iter)
  requires(::sus::iter::Iterator<Iter, Item> &&
           ::sus::mem::relocate_by_memcpy<Iter>)
{
  // IteratorBase also checks this. It's needed for correctness of the casts
  // here.
  static_assert(std::is_final_v<Iter>);

  void (*destroy)(char& sized) = [](char& sized) {
    std::launder(reinterpret_cast<Iter*>(&sized))->~Iter();
  };
  Option<Item> (*next)(char& sized) = [](char& sized) {
    return std::launder(reinterpret_cast<Iter*>(&sized))->next();
  };
  Option<Item> (*next_back)(char& sized);
  if constexpr (SizedIteratorType<Iter>::type::DoubleEnded) {
    next_back = [](char& sized) {
      return std::launder(reinterpret_cast<Iter*>(&sized))->next_back();
    };
  } else {
    next_back = nullptr;
  }
  SizeHint (*size_hint)(const char& sized) = [](const char& sized) {
    return std::launder(reinterpret_cast<const Iter*>(&sized))->size_hint();
  };
  usize (*exact_size_hint)(const char& sized);
  if constexpr (SizedIteratorType<Iter>::type::ExactSize) {
    exact_size_hint = [](const char& sized) {
      return std::launder(reinterpret_cast<const Iter*>(&sized))
          ->exact_size_hint();
    };
  } else {
    exact_size_hint = nullptr;
  }
  typename SizedIteratorType<Iter>::type (*clone)(const char& sized);
  if constexpr (SizedIteratorType<Iter>::type::Clone) {
    clone = [](const char& sized) {
      return make_sized_iterator(
          sus::clone(*std::launder(reinterpret_cast<const Iter*>(&sized))));
    };
  } else {
    clone = nullptr;
  }

  auto it = typename SizedIteratorType<Iter>::type(
      destroy, next, next_back, size_hint, exact_size_hint, clone);
  std::construct_at(reinterpret_cast<Iter*>(it.as_mut_ptr()), ::sus::move(iter));
  return it;
}

}  // namespace sus::iter
