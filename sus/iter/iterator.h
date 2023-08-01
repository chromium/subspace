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

#include "sus/iter/iterator_defn.h"

// The usize formatting is in unsigned_integer_impl.h which has an include cycle
// with usize->Array->Array iterators->SizeHint->usize. So the SizeHint
// formatting needs to be in a separate header instead of in size_hint.h in
// order to use the usize formatting. As long as the user includes "iterator.h"
// they can format SizeHint.
#include "sus/iter/size_hint_impl.h"

// Provides the concept for using iterators in generic code.
#include "sus/iter/iterator_concept.h"

// Once is included here, because there is a cycle between:
// * Option->Once->IteratorBase->Option
// So Option can't include Once or Iterator directly. But as long as the user
// includes "iterator.h" they should be able to use the iterators on Option.
#include "sus/iter/once.h"

// Headers that define iterators that Iterator can construct and return. They
// are forward declared in iterator_defn.h so that transitive includes don't get
// them all every time.
#include "sus/iter/by_ref.h"
#include "sus/iter/chain.h"
#include "sus/iter/cloned.h"
#include "sus/iter/copied.h"
#include "sus/iter/cycle.h"
#include "sus/iter/enumerate.h"
#include "sus/iter/filter.h"
#include "sus/iter/filter_map.h"
#include "sus/iter/flat_map.h"
#include "sus/iter/flatten.h"
#include "sus/iter/fuse.h"
#include "sus/iter/inspect.h"
#include "sus/iter/map.h"
#include "sus/iter/map_while.h"
#include "sus/iter/peekable.h"
#include "sus/iter/reverse.h"
#include "sus/iter/scan.h"
#include "sus/iter/skip.h"
#include "sus/iter/skip_while.h"
#include "sus/iter/step_by.h"
#include "sus/iter/take.h"
#include "sus/iter/take_while.h"
#include "sus/iter/zip.h"
