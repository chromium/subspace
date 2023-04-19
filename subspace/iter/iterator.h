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

// Provides the concept for using iterators in generic code.
#include "subspace/iter/iterator_concept.h"

// Once is included here, because there is a cycle between:
// * Option->Once->IteratorBase->Option
// * Result->Iterator->usize->Result
// So Option and Result can't include Once or Iterator directly. But as long
// as the user includes "iterator.h" they should be able to use the iterators on
// Option.
#include "subspace/iter/once.h"

// Headers that define iterators that Iterator can construct and return. They
// are forward declared in iterator_defn.h so that transitive includes don't get
// them all every time.
#include "subspace/iter/enumerate.h"
#include "subspace/iter/filter.h"
#include "subspace/iter/map.h"
#include "subspace/iter/reverse.h"
