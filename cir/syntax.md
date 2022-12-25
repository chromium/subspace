## Instructions

```c
fn name@id(args) -> out {
  X
}
```
 - Denotes the function that the CIR is representing the body of a function
   named `name. The CIR appears at `X`.
 - Use `void` for no return type.

```c
let x: T
```
 - Declares storage for an object named `x` of type `T`.
 - The type `T` may be qualified by const or volatile, and may be a pointer, or
   pointer-to-pointers.
 - References are represented as pointers.
 - Each pointer is annotated by qualifiers: const, volatile, nullable, lifetimes.
 - Function and method pointers have the form `fn name@id`. For a method, the
   `name` includes the full type name (incl namespace), e.g `S<int>::operator=`.
 - Pointer symbols come before the type of the pointee. Annotations come after
   the thing they annotate: e.g. `*const int const`
 - _0 is the return type.
 - Arguments are represented as objects in this form.

```c
call x@id(p, a const, b nullable) -> c
```
 - Calls function/method with the given id. The `id` used to avoid overload
   resolution in CIR.
 - If x is a method, p is a pointer to the Object that will be in `this`.
 - The objects `p`, `a`, and `b` are passed as arguments to the function, and
   the return value is stored in the object `c`.
 - For each pointer argument, the pointer can be annotated by qualifiers if it
   will be received with additional qualifiers attached to it: const, nullable.
   - For pointer types, if they are not const-annotated, then the analysis
     should assume the function will overwrite data (including any pointers)
     accessible from them.
   - NOTE: A const pointer to a mutable pointer is possible in C++, and we need
     to assume [const-transitivity](../borrowck/DESIGN.md#const-contract) for an
     analysis to ignore pointers received-as-const. Otherwise it would need to
     look at the type and treat mutable pointers inside as if they've been
     modified as well.
 - The types of all objects must match the function signature of `x@id()`.
 - Note: The id is useful for humans, but can be internally represented
   through a pointer instead.
 - Builtin functions also appear as `call` instructions.

```c
convert(y, x)
```
 - Converts arithmetic type `x` to arithmetic type `y`, which can involve
   truncation or sign extension.

```c
reinterpret(y, x)
```
 - Converts arithmetic type `x` to arithmetic type `y` by reinterpreting the
   value of its bytes, and possibly growing the type with 0s at the front.
 - Note: This is the representation of CXXReinterpretCastExpr in the AST.

```c
deinit(x)
```
 - Lifetime ends for object x.

```c
alloc(p, b, a)
```
 - Allocate memory of size `b` and alignment `a` and store pointer in `p`.
 - Note: placement new is represented as a `call` to a constructor instead.

```c
free(p)
```
 - Deallocate memory pointed to by pointer `p`

```c
v = readp(p)
```
 - Dereference a pointer `p` to an arithmetic type, assign the value to `v`.
 - The types must match.

```c
writep(p, v)
```
 - Dereference a pointer `p` to an arithmetic type, assign the `v` to the
   object pointed to by `p`.
 - The types must match.

```c
p = &o
```
- Take the address of an object `o` and store it as a pointer in `p`.

```c
x = y
```
 - Copy arithmetic type `x` to arithmetic type `y`. The type must match.

```c
v = x + y
```
 - Add the arithmetic type `x` and `y`, store the result in `v`.
 - The types of all three objects must match.

```c
v = x - y
```
 - Subtract the arithmetic type `y` from `x`, store the result in `v`.
 - The types of all three objects must match.

```c
v = x * y
```
 - Multiply the arithmetic type `x` and `y`, store the result in `v`.
 - The types of all three objects must match.

```c
v = x / y
```
 - Divide the arithmetic type `x` by `y`, store the result in `v`.
 - The types of all three objects must match.

```c
v = x % y
```
 - Divide the arithmetic type `x` by `y`, store the remainder in `v`.
 - The types of all three objects must match.

```c
v = !x
```
 - Invert the value of the boolean type `x` and store the result in the boolean
   type `v`.

```c
v = x == y
```
 - Test if arithmetic type x is the same value as y, and store true or false in
   the boolean type `v`.
 - The types of `x` and `y` must match.

```c
v = x > y
```
 - Test if arithmetic type x is greater than y, and store true or false in the
   boolean type `v`.
 - The types of `x` and `y` must match.

```c
v = x >= y
```
 - Test if arithmetic type x is greater than or equal y, and store true or
   false in the boolean type `v`.
 - The types of `x` and `y` must match.

```c
v = x < y
```
 - Test if arithmetic type x is less than y, and store true or false in the
   boolean type `v`.
 - The types of `x` and `y` must match.

```c
v = x <= y
```
 - Test if arithmetic type x is less than or equal y, and store true or false
   in the boolean type `v`.
 - The types of `x` and `y` must match.

```c
v = ~x
```
 - Flip the bits of the arithmetic and integer type `x` and store the result in
   `v`.
 - The types of `x` and `v` must match.

```c
v = x & y
```
 - ANDs the bits of the arithmetic and integer types `x` and `y`, storing the
   result in `v`
 - The types of all objects must match.

```c
v = x | y
```
 - ORs the bits of the arithmetic and integer types `x` and `y`, storing the
   result in `v`
 - The types of all objects must match.

```c
v = x ^ y
```
 - XORs the bits of the arithmetic and integer types `x` and `y`, storing the
   result in `v`
 - The types of all objects must match.

```c
v = x << y
```
 - Shifts the bits of the arithmetic and integer types `x` to the left `y` bits,
   storing the result in `v`.
 - The types of `x` and `v` must match.
 - The type of `y` must be an integer object of pointer-size (size_t in the C++
   representation).

```c
v = x >> y
```
 - Shifts the bits of the arithmetic and integer types `x` to the right `y` bits,
   storing the result in `v`.
 - The types of `x` and `v` must match.
 - The type of `y` must be an integer object of pointer-size (size_t in the C++
   representation).

```c
goto -> label
```
 - Terminator. Moves control to the basic block named `label`.

```c
switch(x) -> [y: label1, z: label2, otherwise: label3]
```
 - Terminator. Moves control based on the value of the arithmetic type `x`.
 - If the value of `x` is `y`, control moves to the basic block named `label1`.
 - If the value of `x` is `z`, control moves to the basic block named `label2`.
 - Otherwise, control moves to the basic block named `label3`. It’s possible
   that the otherwise block is unreachable, and will denote that.

```c
unreachable
```
 - Terminator. Marks the basic block as unreachable. It should be the only
   instruction in that block.

```c
return
 - Terminator. Exits the current function, returning the value of the first
   variable defined.
```

```c
bbN: {
  …
}
```
 - A control flow block labeled `bbN` where `N` will be a number. The label
   is used from terminators of other control flow blocks to jump to this one.




## Type representation
 - Primitive (arithmetic) types
 - Structs (no classes)
 - Unions
 - Enums (are integers in CIR?)
 - Pointers, but no references (replaced by pointers)
    - Pointer symbols come first: **int instead of int**
    - Modifiers go to the right of each part of the pointer:
      - `*const * int` instead of int * const*.
      - `*const * int const` instead of `const int * const*`.
 - No pointers-to-members: replaced by a generated function for each type+field.
    - The function takes the type pointer, and returns the field pointer:
    - `fn access_type::field@id(* type) -> * field type`
    - Note that `:` and `<>` are a regular characters in CIR that can be part of identifiers,
      So the function may be named like
      `access_std::vector<int>::private_data_@fg7e(* std::vector<int>) -> * int`
 - All template parameters are substituted; uninstantiated templates are not represented
