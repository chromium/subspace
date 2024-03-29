// This tests a struct in the global namespace, and a struct in a private
// namespace. The former is generated and the latter is not.

/// The <code>summary</code> has <b>html tags</b> in it.
struct S {};

/// The summary has
/// newlines in
/// it.
struct N {};

/// A code block with syntax highlighting.
/// ```
/// A "comment";  // At the end of the line.
/// // At the start of the line.
///   // And after some whitespace with "a string inside".
/// A 'c' char  // Not a 'c' char.
/// A "st\"\\ring" or '\''.foo();
/// auto int; return keywords;
/// not_auto a keyword nor auto_;
/// []() { punctuation[0] + -0; }
/// ```
struct Syntax {};
