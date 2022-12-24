# Lifetimes
- Crubit [lifetime annotations](https://github.com/google/crubit/blob/main/docs/lifetime_annotations_cpp.md)
- We also need to infer [elided
  lifetimes](https://github.com/google/crubit/blob/main/docs/lifetime_annotations_cpp.md#lifetime-elision-lifetime-elision)
  where there are no anotations.

# Exclusive mutability
- To enforce exclusive mutability, any type with a mutable pointer inside
  must be non-copyable.
  - Otherwise it's trivial to escape exclusive mutability:
  ```cpp
  struct S { i32* i; };
  auto i = 0_i32;
  const auto cs = S(&i);  // Const access to `i`.
  auto ms = cs;  // Mutable access to `i`.
  ```
  - Copying could just count as a borrow, and trip in the borrow checking logic,
    but copying implies 2 aliasing mutable pointers so it is always illegal to
    do. It may(?) be easier to check the AST of the type and determine it can't
    be copied, though it may require some computation from
    [CXXRecordDecl](https://clang.llvm.org/doxygen/classclang_1_1CXXRecordDecl.html).
    The more I type here the more I think we should just treat the copy as a
    borrow (for all pointers always anyway) and catch it that way.
- To enforce exclusive mutability through local knowledge and simple rules, we
  need to uphold the const contract transitively. See [Const
  contract](#const-contract).

# Const contract
- Const must be transitive through objects, which is not currently enforced in
  C++. Otherwise it requires non-local reasoning to know if a const object will
  cause mutable access of its members.
  - May only call const methods on a const object's member pointers. This
    applies to methods of the const object as well, which can be verified by
    applying the same to const methods.
  - May not assign through member pointers of a const object (`**c.a = 2`). This
    applies to methods of the const object as well, which can be verified by
    applying the same to const methods.
  - Mutable pointers in a const object can be used in a mutable way.
