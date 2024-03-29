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

namespace sus {

/// Composable external iteration.
///
/// If you've found yourself with a collection of some kind, and needed to
/// perform an operation on the elements of said collection, you'll quickly run
/// into 'iterators'. Iterators are heavily used in idiomatic Rust code, so
/// it's worth becoming familiar with them.
///
/// The use of iterators with collections is described in [the collections
/// documentation]($sus::collections#iterators).
///
/// TODO: Write lots more here.
///
/// All iterators implement the [`Iterator`]($sus::iter::Iterator) concept. Part
/// of implementing that requires inheriting from [`IteratorBase`](
/// $sus::iter::IteratorBase) which provides a large number of methods for
/// filtering or transforming the items produced by the iterator. All of these
/// methods are "lazy" in that they construct a new iterator but do not touch
/// the source which generates items until iteration begins.
///
/// Iterators can [interact with standard ranges]($sus::collections#ranges) as
/// well.
namespace iter {}
}

// IWYU pragma: begin_exports
#include "sus/iter/iterator_defn.h"
#include "sus/iter/iterator_impl.h"

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
