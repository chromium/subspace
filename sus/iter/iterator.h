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

// IWYU pragma: begin_exports
#include "sus/iter/iterator_defn.h"

// The usize formatting is in unsigned_integer_impl.h which has an include cycle
// with usize->Array->Array iterators->SizeHint->usize. So the SizeHint
// formatting needs to be in a separate header instead of in size_hint.h in
// order to use the usize formatting. As long as the user includes "iterator.h"
// they can format SizeHint.
#include "sus/iter/size_hint_impl.h"

// Provides the concept for using iterators in generic code.
#include "sus/iter/iterator_concept.h"

// Headers that define iterators that Iterator can construct and return. They
// are forward declared in iterator_defn.h so that transitive includes don't get
// them all every time.
#include "sus/iter/adaptors/boxed_iterator.h"
#include "sus/iter/adaptors/by_ref.h"
#include "sus/iter/adaptors/chain.h"
#include "sus/iter/adaptors/cloned.h"
#include "sus/iter/adaptors/copied.h"
#include "sus/iter/adaptors/cycle.h"
#include "sus/iter/adaptors/enumerate.h"
#include "sus/iter/adaptors/filter.h"
#include "sus/iter/adaptors/filter_map.h"
#include "sus/iter/adaptors/flat_map.h"
#include "sus/iter/adaptors/flatten.h"
#include "sus/iter/adaptors/fuse.h"
#include "sus/iter/adaptors/inspect.h"
#include "sus/iter/adaptors/map.h"
#include "sus/iter/adaptors/map_while.h"
#include "sus/iter/adaptors/peekable.h"
#include "sus/iter/adaptors/reverse.h"
#include "sus/iter/adaptors/scan.h"
#include "sus/iter/adaptors/skip.h"
#include "sus/iter/adaptors/skip_while.h"
#include "sus/iter/adaptors/step_by.h"
#include "sus/iter/adaptors/take.h"
#include "sus/iter/adaptors/take_while.h"
#include "sus/iter/adaptors/zip.h"
// IWYU pragma: end_exports
