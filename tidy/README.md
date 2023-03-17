# Things that we want a clang-tidy for

## Closure concept usage
* Templates restricted as Fn are received as const&-ref
* Templates restricted as Fn are invoked as lvalue `f()`
* Templates restricted as FnMut are received by value or as &-ref
* Templates restricted as FnMut are invoked as lvalue `f()`
* Templates restricted as FnOnce are received as &&-ref or by value
* Templates restricted as FnOnce are invoked as rvalue `sus::move(f)()`

## References to closures
* FnOnceRef, FnMutRef and FnRef types only appear as lvalues as ParamVarDecl,
  and are not returned.

## Long-lived closures
* Think about how to rewrite FnBox to allow lambdas with capture if we use a
  clang-tidy to ensure arguments are not stored as references carelessly.

## Mutable references
* Passing an lvalue to a function receiving an lvalue-reference should require
  the argument to be wrapped in `sus::mref()`. Probably exceptions for generic
  code.

## IteratorLoop
* It's not safe to use IteratorLoop outside of a ranged-for loop, and we should
  check that it's not happening. UB can result from getting to the end() and
  calling operator*().
