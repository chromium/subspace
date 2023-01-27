namespace n {

/// Function with overloads
void multiple();
void multiple(int);
/// #[doc.overloads=0]  // This is merged with the default overload set.
void multiple(float);

/// Separated overload
/// #[doc.overloads=1]
void multiple(unsigned);

}  // namespace n
