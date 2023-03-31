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

///////////////////////////////////////////////////////////////////////////
//
// Out-of-line definitions for methods defined in `slice_methods.inc` that
// provide const access or no access to the contained objects. These methods are
// suitable to be exposed from a Slice for const access.
//
// TO USE THIS INC FILE:
//
// Include it after the body of your class.
//
// Define `_ptr_expr` and `_len_expr` when including this file to the
// expressions of getting the slice's data pointer and length.
//
// Define `Self` as the type declaring the methods.
///////////////////////////////////////////////////////////////////////////

template <class T>
Vec<T> Self<T>::to_vec() const& noexcept
  requires(::sus::mem::Clone<T>)
{
  auto v = Vec<T>::with_capacity(_len_expr);
  for (::sus::usize i; i < _len_expr; i += 1u) {
    v.push(::sus::clone(*(_ptr_expr + i)));
  }
  return v;
}
