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

#include "traits/iter/iterator_defn.h"
#include "traits/iter/iterator_fns.h"

// Once is included here, because there is a cycle between
// Option->Once->IteratorBase, so Option can't include Once itself. But as long as
// the user includes "iterator.h" they should be able to use the iterators on
// Option.
#include "traits/iter/once.h"
