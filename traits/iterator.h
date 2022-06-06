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

namespace sus::option {
template <class T>
class Option;
}

namespace sus::traits {

using ::sus::option::Option;

template <class Item>
class Iterator;

struct IteratorEnd {};
extern const IteratorEnd iterator_end;

template <class Item>
struct IteratorStep {
  Option<Item> item_;
  Iterator<Item>& iter_;

  inline bool operator==(const IteratorEnd&) const { return item_.is_nome(); }
  inline bool operator!=(const IteratorEnd&) const { return item_.is_some(); }
  inline void operator++() & { item_ = iter_.next(); }
  inline Item operator*() & { return item_.take().unwrap(); }
};

template <class Item>
class Iterator {
 public:
  virtual Option<Item> next() = 0;

  Iterator(const Iterator&) = delete;
  Iterator& operator=(const Iterator&) = delete;
  Iterator(Iterator&&) = delete;
  Iterator& operator=(Iterator&&) = delete;

  IteratorStep<Item> begin() & {
    return IteratorStep{first_item_.take().flatten(), *this};
  }
  IteratorEnd end() & { return iterator_end; }

 protected:
  Iterator() : first_item_(Option<Option<Item>>::some(next())) {}
  Iterator(Option<Item> first)
      : first_item_(
            Option<Option<Item>>::some(static_cast<decltype(first)&&>(first))) {
  }

 private:
  Option<Option<Item>> first_item_;
};

}  // namespace sus::traits
