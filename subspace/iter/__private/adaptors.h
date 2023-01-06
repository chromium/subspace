#pragma once

#include "subspace/option/option.h"

namespace sus::result {
template <class T, class E>
class Result;
}

namespace sus::iter {
template <class Item>
class Once;
template <class ItemT>
class IteratorBase;
template <class I>
class Iterator;
namespace __private {
template <class T>
constexpr auto begin(const T& t) noexcept;
template <class T>
constexpr auto end(const T& t) noexcept;
}  // namespace __private
}  // namespace sus::iter

namespace sus::iter::__private {

using ::sus::result::Result;

template <class Item, class U, class E,
          ::sus::fn::callable::CallableReturns<Result<U, E>, Item&&> MakeResult>
struct Unwrapper : public ::sus::iter::IteratorBase<U> {
  Unwrapper(::sus::iter::IteratorBase<Item>&& iter, Option<E>& err,
            MakeResult make_result)
      : iter(iter), err(err), make_result(make_result) {}

  Option<U> next() noexcept final {
    Option<Item> try_item = iter.next();
    if (try_item.is_none()) return Option<U>::none();
    Result<U, E> result = make_result(
        ::sus::move(try_item).unwrap_unchecked(::sus::marker::unsafe_fn));
    if (result.is_ok())
      return Option<U>::some(
          ::sus::move(result).unwrap_unchecked(::sus::marker::unsafe_fn));
    err.insert(
        ::sus::move(result).unwrap_err_unchecked(::sus::marker::unsafe_fn));
    return Option<U>::none();
  }

  ::sus::iter::IteratorBase<Result<U, E>>& iter;
  Option<E>& err;
  MakeResult make_result;
};

template <class Item, class MakeResult, int&...,
          class R = std::invoke_result_t<MakeResult, Item&&>>
Unwrapper(::sus::iter::IteratorBase<Item>&& iter,
          Option<typename R::ErrType>& err, MakeResult make_result)
    -> Unwrapper<Item, typename R::OkType, typename R::ErrType, MakeResult>;

}  // namespace sus::iter::__private
