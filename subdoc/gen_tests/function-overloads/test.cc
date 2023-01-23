namespace n {

/// Function with overloads
void multiple();
void multiple(int);
void multiple(float);

/// Separated overload
/// #[doc(overloads=1)]
void multiple(unsigned);

}  // namespace n
