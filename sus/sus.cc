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
module;
#include "sus/assertions/check.h"
#include "sus/assertions/debug_check.h"
#include "sus/assertions/panic.h"
#include "sus/assertions/unreachable.h"
#include "sus/boxed/__private/string_error.h"
#include "sus/boxed/box.h"
#include "sus/boxed/boxed.h"
#include "sus/boxed/dyn.h"
#include "sus/choice/__private/all_values_are_unique.h"
#include "sus/choice/__private/index_of_value.h"
#include "sus/choice/__private/index_type.h"
#include "sus/choice/__private/nothing.h"
#include "sus/choice/__private/ops_concepts.h"
#include "sus/choice/__private/pack_index.h"
#include "sus/choice/__private/storage.h"
#include "sus/choice/__private/type_list.h"
#include "sus/choice/choice.h"
#include "sus/choice/choice_types.h"
#include "sus/cmp/__private/void_concepts.h"
#include "sus/cmp/cmp.h"
#include "sus/cmp/eq.h"
#include "sus/cmp/ord.h"
#include "sus/cmp/reverse.h"
#include "sus/collections/__private/sort.h"
#include "sus/collections/array.h"
#include "sus/collections/collections.h"
#include "sus/collections/compat_deque.h"
#include "sus/collections/compat_forward_list.h"
#include "sus/collections/compat_list.h"
#include "sus/collections/compat_map.h"
#include "sus/collections/compat_pair_concept.h"
#include "sus/collections/compat_queue.h"
#include "sus/collections/compat_set.h"
#include "sus/collections/compat_stack.h"
#include "sus/collections/compat_unordered_map.h"
#include "sus/collections/compat_unordered_set.h"
#include "sus/collections/compat_vector.h"
#include "sus/collections/concat.h"
#include "sus/collections/iterators/array_iter.h"
#include "sus/collections/iterators/chunks.h"
#include "sus/collections/iterators/drain.h"
#include "sus/collections/iterators/slice_iter.h"
#include "sus/collections/iterators/split.h"
#include "sus/collections/iterators/vec_iter.h"
#include "sus/collections/iterators/windows.h"
#include "sus/collections/join.h"
#include "sus/collections/slice.h"
#include "sus/collections/vec.h"
#include "sus/construct/__private/into_ref.h"
#include "sus/construct/cast.h"
#include "sus/construct/construct.h"
#include "sus/construct/default.h"
#include "sus/construct/from.h"
#include "sus/construct/into.h"
#include "sus/construct/safe_from_reference.h"
#include "sus/env/env.h"
#include "sus/env/var.h"
#include "sus/error/compat_error.h"
#include "sus/error/error.h"
#include "sus/fn/__private/signature.h"
#include "sus/fn/fn.h"
#include "sus/fn/fn_concepts.h"
#include "sus/fn/fn_dyn.h"
#include "sus/iter/__private/into_iterator_archetype.h"
#include "sus/iter/__private/is_generator.h"
#include "sus/iter/__private/iter_compare.h"
#include "sus/iter/__private/iterator_archetype.h"
#include "sus/iter/__private/iterator_end.h"
#include "sus/iter/__private/range_begin.h"
#include "sus/iter/__private/step.h"
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
#include "sus/iter/adaptors/moved.h"
#include "sus/iter/adaptors/peekable.h"
#include "sus/iter/adaptors/reverse.h"
#include "sus/iter/adaptors/scan.h"
#include "sus/iter/adaptors/skip.h"
#include "sus/iter/adaptors/skip_while.h"
#include "sus/iter/adaptors/step_by.h"
#include "sus/iter/adaptors/take.h"
#include "sus/iter/adaptors/take_while.h"
#include "sus/iter/adaptors/zip.h"
#include "sus/iter/compat_ranges.h"
#include "sus/iter/empty.h"
#include "sus/iter/extend.h"
#include "sus/iter/from_iterator.h"
#include "sus/iter/generator.h"
#include "sus/iter/into_iterator.h"
#include "sus/iter/iterator.h"
#include "sus/iter/iterator_concept.h"
#include "sus/iter/iterator_defn.h"
#include "sus/iter/iterator_impl.h"
#include "sus/iter/iterator_loop.h"
#include "sus/iter/iterator_ref.h"
#include "sus/iter/once.h"
#include "sus/iter/once_with.h"
#include "sus/iter/product.h"
#include "sus/iter/repeat.h"
#include "sus/iter/repeat_with.h"
#include "sus/iter/size_hint.h"
#include "sus/iter/size_hint_impl.h"
#include "sus/iter/successors.h"
#include "sus/iter/sum.h"
#include "sus/iter/try_from_iterator.h"
#include "sus/iter/zip.h"
#include "sus/lib/__private/forward_decl.h"
#include "sus/lib/lib.h"
#include "sus/macros/__private/compiler_bugs.h"
#include "sus/macros/arch.h"
#include "sus/macros/assume.h"
#include "sus/macros/builtin.h"
#include "sus/macros/compiler.h"
#include "sus/macros/eval_and_concat.h"
#include "sus/macros/eval_macro.h"
#include "sus/macros/for_each.h"
#include "sus/macros/inline.h"
#include "sus/macros/lifetimebound.h"
#include "sus/macros/no_unique_address.h"
#include "sus/macros/nonnull.h"
#include "sus/macros/pure.h"
#include "sus/macros/remove_parens.h"
#include "sus/marker/empty.h"
#include "sus/marker/unsafe.h"
#include "sus/mem/__private/data_size_finder.h"
#include "sus/mem/__private/ref_concepts.h"
#include "sus/mem/addressof.h"
#include "sus/mem/clone.h"
#include "sus/mem/copy.h"
#include "sus/mem/forward.h"
#include "sus/mem/move.h"
#include "sus/mem/never_value.h"
#include "sus/mem/relocate.h"
#include "sus/mem/remove_rvalue_reference.h"
#include "sus/mem/replace.h"
#include "sus/mem/size_of.h"
#include "sus/mem/swap.h"
#include "sus/mem/take.h"
#include "sus/num/__private/check_integer_overflow.h"
#include "sus/num/__private/float_ordering.h"
#include "sus/num/__private/int_log10.h"
#include "sus/num/__private/intrinsics.h"
#include "sus/num/__private/literals.h"
#include "sus/num/__private/primitive_type.h"
#include "sus/num/cast.h"
#include "sus/num/float.h"
#include "sus/num/float_concepts.h"
#include "sus/num/float_impl.h"
#include "sus/num/fp_category.h"
#include "sus/num/integer_concepts.h"
#include "sus/num/num_concepts.h"
#include "sus/num/overflow_integer.h"
#include "sus/num/signed_integer.h"
#include "sus/num/signed_integer_impl.h"
#include "sus/num/try_from_int_error.h"
#include "sus/num/try_from_int_error_impl.h"
#include "sus/num/types.h"
#include "sus/num/unsigned_integer.h"
#include "sus/num/unsigned_integer_impl.h"
#include "sus/ops/range.h"
#include "sus/ops/range_literals.h"
#include "sus/ops/try.h"
#include "sus/option/__private/is_option_type.h"
#include "sus/option/__private/is_tuple_type.h"
#include "sus/option/__private/marker.h"
#include "sus/option/__private/storage.h"
#include "sus/option/compat_option.h"
#include "sus/option/option.h"
#include "sus/option/option_iter.h"
#include "sus/option/state.h"
#include "sus/prelude.h"
#include "sus/ptr/as_ref.h"
#include "sus/ptr/copy.h"
#include "sus/ptr/nonnull.h"
#include "sus/ptr/subclass.h"
#include "sus/ptr/swap.h"
#include "sus/result/__private/is_result_type.h"
#include "sus/result/__private/marker.h"
#include "sus/result/__private/result_state.h"
#include "sus/result/__private/storage.h"
#include "sus/result/ok_void.h"
#include "sus/result/result.h"
#include "sus/string/__private/any_formatter.h"
#include "sus/string/__private/bytes_formatter.h"
#include "sus/string/__private/format_to_stream.h"
#include "sus/string/compat_string.h"
#include "sus/test/behaviour_types.h"
#include "sus/test/ensure_use.h"
#include "sus/test/no_copy_move.h"
#include "sus/tuple/__private/storage.h"
#include "sus/tuple/tuple.h"

export module sus;

export namespace sus {
  // Functions
  using sus::cast;
  using sus::clone_into;
  using sus::clone;
  using sus::dyn;
  using sus::err;
  using sus::empty;
  using sus::forward;
  using sus::into;
  using sus::move_into;
  using sus::move;
  using sus::none;
  using sus::some;
  using sus::try_into;
  using sus::err;
  using sus::ok;
  using sus::tuple;
  using sus::size_of;
  using sus::data_size_of;

  // Classes
  using sus::Array;
  using sus::Box;
  using sus::Option;
  using sus::Result;
  using sus::Slice;
  using sus::Vec;
  using sus::Choice;
  using sus::Slice;
  using sus::SliceMut;
  using sus::Err;
  using sus::Ok;
  using sus::Tuple;
  using sus::None;
  using sus::Some;

  // Type Aliases
  using sus::f32;
  using sus::f64;
  using sus::i16;
  using sus::i32;
  using sus::i64;
  using sus::i8;
  using sus::isize;
  using sus::u16;
  using sus::u32;
  using sus::u64;
  using sus::u8;
  using sus::uptr;
  using sus::usize;
}

export namespace sus::assertions {
  // Classes
  using sus::assertions::PanicLocation;
}  // namespace assertions

export namespace sus::boxed {
  // Classes
  using sus::boxed::Box;
  using sus::boxed::Dyn;

  // Functions
  using sus::boxed::dyn;

  // Concepts
  using sus::boxed::DynConcept;
}  // namespace boxed

export namespace sus::choice_type {
  // Classes
  using sus::choice_type::CanConvertToStorage;
  using sus::choice_type::Choice;

  // Concepts
  using sus::choice_type::ChoiceValueIsVoid;

  // Exposed to facilitate `sus_choice_types`.
  namespace __private {
  using sus::choice_type::__private::find_choice_storage_mut;
  using sus::choice_type::__private::find_choice_storage;
  using sus::choice_type::__private::TypeList;
  using sus::choice_type::__private::MakeStorageType;
  using sus::choice_type::__private::StorageIsSafelyConstructibleFromReference;
  using sus::choice_type::__private::AllValuesAreUnique;
  }
}  // namespace choice_type

export namespace sus::cmp {
  // Classes
  using sus::cmp::Reverse;

  // Functions
  using sus::cmp::clamp;
  using sus::cmp::max;
  using sus::cmp::max_by;
  using sus::cmp::max_by_key;
  using sus::cmp::min;
  using sus::cmp::min_by;
  using sus::cmp::min_by_key;

  // Concepts
  using sus::cmp::Eq;
  using sus::cmp::ExclusiveOrd;
  using sus::cmp::ExclusivePartialOrd;
  using sus::cmp::ExclusiveStrongOrd;
  using sus::cmp::Ord;
  using sus::cmp::Ordering;
  using sus::cmp::PartialOrd;
  using sus::cmp::StrongOrd;
}  // namespace cmp

export namespace sus::collections::compat {
  // Concepts
  using sus::collections::compat::Pair;
}  // namespace collections::compat

export namespace sus::collections {
  // Classes
  using sus::collections::Array;
  using sus::collections::ArrayIntoIter;
  using sus::collections::Chunks;
  using sus::collections::ChunksExact;
  using sus::collections::ChunksExactMut;
  using sus::collections::Drain;
  using sus::collections::RChunks;
  using sus::collections::RChunksExact;
  using sus::collections::RChunksExactMut;
  using sus::collections::RSplit;
  using sus::collections::RSplitMut;
  using sus::collections::RSplitN;
  using sus::collections::Slice;
  using sus::collections::SliceIter;
  using sus::collections::SliceIterMut;
  using sus::collections::SliceMut;
  using sus::collections::Split;
  using sus::collections::SplitInclusive;
  using sus::collections::SplitInclusiveMut;
  using sus::collections::SplitMut;
  using sus::collections::SplitN;
  using sus::collections::SplitNMut;
  using sus::collections::Vec;
  using sus::collections::VecIntoIter;
  using sus::collections::Windows;
  using sus::collections::WindowsMut;

  // Functions
  using sus::collections::get;

  // Concepts
  using sus::collections::Concat;
  using sus::collections::Join;

  // Function aliases
  using sus::collections::begin;
  using sus::collections::end;

  namespace __private {
    using ::sus::collections::__private::Storage;
  }
}  // namespace collections

export namespace sus::construct {
  // Classes
  using sus::construct::CastImpl;

  // Functions
  using sus::construct::cast;
  using sus::construct::into;
  using sus::construct::move_into;
  using sus::construct::try_into;

  // Concepts
  using sus::construct::Cast;
  using sus::construct::Default;
  using sus::construct::From;
  using sus::construct::Into;
  using sus::construct::SafelyConstructibleFromReference;
  using sus::construct::TryFrom;
  using sus::construct::TryInto;
}  // namespace construct

export namespace sus::env {
  // Concepts
  using sus::env::VarError;

  // Functions
  using sus::env::set_var;
  using sus::env::var;
}  // namespace env

export namespace sus::error {
  // Classes
  using sus::error::DynError;
  using sus::error::ErrorImpl;

  // Functions
  using sus::error::error_display;
  using sus::error::error_source;

  // Concepts
  using sus::error::Error;
}  // namespace error

export namespace sus::fn {
  // Classes
  using sus::fn::Anything;
  using sus::fn::DynFn;
  using sus::fn::DynFnMut;
  using sus::fn::DynFnOnce;
  using sus::fn::NonVoid;

  // Functions
  using sus::fn::call;
  using sus::fn::call_mut;
  using sus::fn::call_once;

  // Concepts
  using sus::fn::Fn;
  using sus::fn::FnMut;
  using sus::fn::FnOnce;

  // Type aliases
  using sus::fn::Return;
  using sus::fn::ReturnMut;
  using sus::fn::ReturnOnce;
}  // namespace fn

export namespace sus::iter {
  // Classes
  using sus::iter::ByRef;
  using sus::iter::Chain;
  using sus::iter::Cloned;
  using sus::iter::Copied;
  using sus::iter::Cycle;
  using sus::iter::Empty;
  using sus::iter::Enumerate;
  using sus::iter::Filter;
  using sus::iter::FilterMap;
  using sus::iter::FlatMap;
  using sus::iter::Flatten;
  using sus::iter::FromIteratorImpl;
  using sus::iter::Fuse;
  using sus::iter::Generator;
  using sus::iter::Inspect;
  using sus::iter::IteratorBase;
  using sus::iter::IteratorOverRange;
  using sus::iter::IteratorRange;
  using sus::iter::IterRef;
  using sus::iter::IterRefCounter;
  using sus::iter::Map;
  using sus::iter::MapWhile;
  using sus::iter::Moved;
  using sus::iter::Once;
  using sus::iter::OnceWith;
  using sus::iter::Peekable;
  using sus::iter::Repeat;
  using sus::iter::RepeatWith;
  using sus::iter::Reverse;
  using sus::iter::Scan;
  using sus::iter::SizeHint;
  using sus::iter::Skip;
  using sus::iter::SkipWhile;
  using sus::iter::StepBy;
  using sus::iter::Successors;
  using sus::iter::Take;
  using sus::iter::TakeWhile;
  using sus::iter::Zip;

  // Functions
  using sus::iter::begin;
  using sus::iter::empty;
  using sus::iter::end;
  using sus::iter::from_generator;
  using sus::iter::from_iter;
  using sus::iter::from_range;
  using sus::iter::once;
  using sus::iter::once_with;
  using sus::iter::repeat;
  using sus::iter::repeat_with;
  using sus::iter::successors;
  using sus::iter::try_from_iter;
  using sus::iter::zip;

  // Concepts
  using sus::iter::DoubleEndedIterator;
  using sus::iter::ExactSizeIterator;
  using sus::iter::Extend;
  using sus::iter::FromIterator;
  using sus::iter::IntoIterator;
  using sus::iter::IntoIteratorAny;
  using sus::iter::Iterator;
  using sus::iter::IteratorAny;
  using sus::iter::Product;
  using sus::iter::Sum;
  using sus::iter::TrustedLen;

  // Type Aliases
  using sus::iter::IntoIteratorOutputType;
  using sus::iter::Option;

  // Concept Aliases
  using sus::iter::Into;
  using sus::iter::TriviallyRelocatable;

  namespace __private {
    using sus::iter::__private::IteratorArchetype;
    using sus::iter::__private::IntoIteratorArchetype;
    using sus::iter::__private::TrustedLenMarker;
    using sus::iter::__private::Step;
  }
}  // namespace iter

export namespace sus::marker {
  // Classes
  using sus::marker::EmptyMarker;
  using sus::marker::UnsafeFnMarker;

  // Variables
  using sus::marker::empty;
  using sus::marker::unsafe_fn;
}  // namespace marker

export namespace sus::mem {
  // Classes
  using sus::mem::NeverValueConstructor;

  // Functions
  using sus::mem::addressof;
  using sus::mem::clone;
  using sus::mem::clone_into;
  using sus::mem::clone_or_forward;
  using sus::mem::data_size_of;
  using sus::mem::forward;
  using sus::mem::move;
  using sus::mem::replace;
  using sus::mem::size_of;
  using sus::mem::swap;
  using sus::mem::swap_nonoverlapping;
  using sus::mem::take;
  using sus::mem::take_and_destruct;
  using sus::mem::take_copy_and_destruct;

  // Concepts
  using sus::mem::Clone;
  using sus::mem::CloneFrom;
  using sus::mem::CloneOrRef;
  using sus::mem::Copy;
  using sus::mem::CopyOrRef;
  using sus::mem::CopyOrRefOrVoid;
  using sus::mem::IsMoveRef;
  using sus::mem::Move;
  using sus::mem::MoveOrRef;
  using sus::mem::MoveOrRefOrVoid;
  using sus::mem::NeverValueField;
  using sus::mem::TrivialCopy;
  using sus::mem::TriviallyRelocatable;

  // Type Aliases
  using sus::mem::remove_rvalue_reference;

  namespace __private {
    using sus::mem::__private::HasCloneFromMethod;
  }
}  // namespace mem

export namespace sus::num {
  // Classes
  using sus::num::f32;
  using sus::num::f64;
  using sus::num::i16;
  using sus::num::i32;
  using sus::num::i64;
  using sus::num::i8;
  using sus::num::isize;
  using sus::num::OverflowInteger;
  using sus::num::TryFromIntError;
  using sus::num::u16;
  using sus::num::u32;
  using sus::num::u64;
  using sus::num::u8;
  using sus::num::uptr;
  using sus::num::usize;

  // Enums
  using sus::num::FpCategory;

  // Concepts
  using sus::num::Add;
  using sus::num::AddAssign;
  using sus::num::BitAnd;
  using sus::num::BitAndAssign;
  using sus::num::BitNot;
  using sus::num::BitOr;
  using sus::num::BitOrAssign;
  using sus::num::BitXor;
  using sus::num::BitXorAssign;
  using sus::num::Div;
  using sus::num::DivAssign;
  using sus::num::Float;
  using sus::num::Integer;
  using sus::num::IntegerNumeric;
  using sus::num::IntegerPointer;
  using sus::num::Mul;
  using sus::num::MulAssign;
  using sus::num::Neg;
  using sus::num::PrimitiveEnum;
  using sus::num::PrimitiveEnumClass;
  using sus::num::PrimitiveFloat;
  using sus::num::PrimitiveInteger;
  using sus::num::Rem;
  using sus::num::RemAssign;
  using sus::num::Shl;
  using sus::num::ShlAssign;
  using sus::num::Shr;
  using sus::num::ShrAssign;
  using sus::num::Signed;
  using sus::num::SignedPrimitiveEnum;
  using sus::num::SignedPrimitiveEnumClass;
  using sus::num::SignedPrimitiveInteger;
  using sus::num::Sub;
  using sus::num::SubAssign;
  using sus::num::Unsigned;
  using sus::num::UnsignedNumeric;
  using sus::num::UnsignedPointer;
  using sus::num::UnsignedPrimitiveEnum;
  using sus::num::UnsignedPrimitiveEnumClass;
  using sus::num::UnsignedPrimitiveInteger;

  namespace __private {
    using sus::num::__private::float_is_nan_quiet;
  }
}  // namespace num

export namespace sus::ops {
  // Classes
  using sus::ops::Range;
  using sus::ops::RangeFrom;
  using sus::ops::RangeFull;
  using sus::ops::RangeTo;
  using sus::ops::TryImpl;

  // Functions
  using sus::ops::range;
  using sus::ops::range_from;
  using sus::ops::range_to;
  using sus::ops::try_from_default;
  using sus::ops::try_from_output;
  using sus::ops::try_into_output;
  using sus::ops::try_is_success;
  using sus::ops::try_preserve_error;

  // Concepts
  using sus::ops::RangeBounds;
  using sus::ops::Try;
  using sus::ops::TryDefault;
  using sus::ops::TryErrorConvertibleTo;

  // Type Aliases
  using sus::ops::TryOutputType;
  using sus::ops::TryRemapOutputType;
}  // namespace ops

export namespace sus::option {
  // Classes
  using sus::option::Option;
  using sus::option::OptionIter;

  // Functions
  using sus::option::none;
  using sus::option::some;

  // Function Aliases
  using sus::option::begin;
  using sus::option::end;
}  // namespace option

export namespace sus::prelude {
  using sus::collections::Vec;
  using sus::marker::unsafe_fn;
  using sus::num::f32;
  using sus::num::f64;
  using sus::num::i16;
  using sus::num::i32;
  using sus::num::i64;
  using sus::num::i8;
  using sus::num::isize;
  using sus::num::u16;
  using sus::num::u32;
  using sus::num::u64;
  using sus::num::u8;
  using sus::num::uptr;
  using sus::num::usize;
  using sus::option::Option;
}  // namespace prelude

export namespace sus::ptr {
  // Classes
  using sus::ptr::NonNull;

  // Functions
  using sus::ptr::as_ref;
  using sus::ptr::copy;
  using sus::ptr::copy_nonoverlapping;
  using sus::ptr::swap_nonoverlapping;

  // Concepts
  using sus::ptr::SameOrSubclassOf;
}  // namespace ptr

export namespace sus::result {
  // Classes
  using sus::result::OkVoid;
  using sus::result::Result;

  // Functions
  using sus::result::err;
  using sus::result::ok;

  // Type Aliases
  using sus::result::Once;
  using sus::result::ResultState;
  using sus::result::StoragePointer;

  // Function Aliases
  using sus::result::begin;
  using sus::result::end;

  // Concept Aliases
  using sus::result::IsTrivialCopyAssignOrRef;
  using sus::result::IsTrivialCopyCtorOrRef;
  using sus::result::IsTrivialDtorOrRef;
  using sus::result::IsTrivialMoveAssignOrRef;
  using sus::result::IsTrivialMoveCtorOrRef;
  using sus::result::VoidOrEq;
  using sus::result::VoidOrOrd;
  using sus::result::VoidOrPartialOrd;
  using sus::result::VoidOrWeakOrd;
}  // namespace result

export namespace sus::tuple_type {
  // Classes
  using sus::tuple_type::Tuple;

  // Functions
  using sus::tuple_type::get;
  using sus::tuple_type::tuple;
}  // namespace tuple_type

#if !defined(SUS_PRELUDE_NO_IMPORT) || SUS_PRELUDE_NO_IMPORT
export using sus::prelude::Vec;
export using sus::prelude::unsafe_fn;
export using sus::prelude::f32;
export using sus::prelude::f64;
export using sus::prelude::i16;
export using sus::prelude::i32;
export using sus::prelude::i64;
export using sus::prelude::i8;
export using sus::prelude::isize;
export using sus::prelude::u16;
export using sus::prelude::u32;
export using sus::prelude::u64;
export using sus::prelude::u8;
export using sus::prelude::uptr;
export using sus::prelude::usize;
export using sus::prelude::Option;
#endif

export using ::operator""_f32;
export using ::operator""_f64;
export using ::operator""_i16;
export using ::operator""_i32;
export using ::operator""_i64;
export using ::operator""_i8;
export using ::operator""_isize;
export using ::operator""_r;
export using ::operator""_rs;
export using ::operator""_u16;
export using ::operator""_u32;
export using ::operator""_u64;
export using ::operator""_u8;
export using ::operator""_usize;

export using ::sus::panic;
export using ::sus::unreachable;
export using ::sus::unreachable_unchecked;
