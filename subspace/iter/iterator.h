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

// The usize formatting is in unsigned_integer_impl.h which has an include cycle
// with usize->Array->Array iterators->SizeHint->usize. So the SizeHint
// formatting needs to be in a separate header instead of in size_hint.h in
// order to use the usize formatting. As long as the user includes "iterator.h"
// they can format SizeHint.
#include "subspace/iter/size_hint_impl.h"

// Provides the concept for using iterators in generic code.
#include "subspace/iter/iterator_concept.h"

// Once is included here, because there is a cycle between:
// * Option->Once->IteratorBase->Option
// So Option can't include Once or Iterator directly. But as long as the user
// includes "iterator.h" they should be able to use the iterators on Option.
#include "subspace/iter/once.h"

// Headers that define iterators that Iterator can construct and return. They
// are forward declared in iterator_defn.h so that transitive includes don't get
// them all every time.
#include "subspace/iter/by_ref.h"
#include "subspace/iter/chain.h"
#include "subspace/iter/cloned.h"
#include "subspace/iter/copied.h"
#include "subspace/iter/cycle.h"
#include "subspace/iter/enumerate.h"
#include "subspace/iter/filter.h"
#include "subspace/iter/filter_map.h"
#include "subspace/iter/flat_map.h"
#include "subspace/iter/flatten.h"
#include "subspace/iter/fuse.h"
#include "subspace/iter/inspect.h"
#include "subspace/iter/map.h"
#include "subspace/iter/map_while.h"
#include "subspace/iter/peekable.h"
#include "subspace/iter/reverse.h"
#include "subspace/iter/scan.h"
#include "subspace/iter/skip.h"
#include "subspace/iter/skip_while.h"
#include "subspace/iter/step_by.h"
