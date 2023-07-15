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
// Define `_ptr_expr`, `_len_expr` `_iter_refs_expr` and `_iter_refs_view_expr`
// when including this file to the expressions of getting the slice's data
// pointer and length.
//
// Define `_self` as the type declaring the methods including template
// parameters, such as `S<A, B>`.
//
// Define `_self_template` as the template parameter declarations for the type,
// such as `class A, class B`.
///////////////////////////////////////////////////////////////////////////

template <_self_template>
Vec<T> _self::to_vec() const& noexcept
  requires(::sus::mem::Clone<T>)
{
  auto v = Vec<T>::with_capacity(_len_expr);
  for (::sus::usize i; i < _len_expr; i += 1u) {
    v.push(::sus::clone(*(_ptr_expr + i)));
  }
  return v;
}

#undef _self
#undef _self_template
#undef _ptr_expr
#undef _len_expr
#undef _delete_rvalue
#undef _iter_refs_expr
#undef _iter_refs_view_expr
