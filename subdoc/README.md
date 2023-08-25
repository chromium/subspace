# Subdoc

[Subdoc](subdoc/) is a documentation generator from inline C++ documentation
in the spirit of
[Rustdoc](https://doc.rust-lang.org/rustdoc/what-is-rustdoc.html).

Subdoc collects and parses doc comments in source code and generates a
set of HTML pages from it. Doc comments are typically denoted by `///`
(or `/** */` inside macros). The text within the doc comments is parsed
as [markdown]($https://www.markdownguide.org/) with support for:
* Tables
* Strikethrough
* Inline HTML markup
* [Code links](#code-links)
* [Grouping Overloads](#grouping-overloads)
* [Hiding symbols](#hiding-symbols)

There are many examples throughout the [Subspace C++ library](
https://github.com/chromium/subspace/tree/main/sus).

## Code links

Subdoc uses an extension to links in markdown. If the link URL starts with
a `$` then the text is parsed as a C++ symbol. If it can be found in the
source code that Subdoc is generating docs for, a link to the correct HTML
page will be inserted. Otherwise, an error is generated.

For example the markdown link `[Option]($sus::option::Option)` will generate
a link to the page about the `Option` type found in the `sus::option`
namespace. And `[Option::unwrap]($sus::option::Option::unwrap)` will link to
the `unwrap` method for that same type.

## Grouping overloads

By default all overloads of a function/method are grouped together and get a
single doc comment applied to them all. If more than one comment is found,
Subdoc will generate an error stating that the comments have collided.

To generate different documentation for overloads, use the
`#[doc.overloads=group]` doc-attribute. All overloads that have the same
`group` name will be grouped together and get the same documentation applied
to them. This can be used to group overloads in arbitrary ways and into an
arbitrary number of groups.

For example:
```cpp
/// This function receives no arguments.
/// #[doc.overloads=void]
void my_function();

/// This function receives an i32 argument.
/// #[doc.overloads=i32]
void my_function(i32);
```

To link to a specific overload group, place the `group` name in the link after
a `!` character.

```cpp
/// A simple example function.
///
/// Calls [`my_function`](#my_function!i32), passing an i32.
void with_i32() { my_function(2); }
```

# Hiding symbols

To document a symbol in the source code but hide it from the generated documentation,
use the `#[doc.hidden]` attribute.

For example:
```cpp
/// This function is omitted from the generated docs.
/// #[doc.hidden]
void secret() {}
```

# Running Subdoc

```
subdoc --project-name NAME [--include-file-pattern PATH] [--exclude-file-pattern PATH]
    [--css PATH] [--copy-file PATH] [--project-md PATH]
    FILES...
```

```
--project-name NAME
```
The `NAME` will be inserted into the title and overview section of the generated
HTML pages.

## Include/exclude symbols

Subdoc uses Clang to parse C++ code, so it sees every symbol that your 
code sees, including the standard library, C library, etc. To avoid generating
docs for everything, Subdoc has command line flags that control what is
included or excluded.

```
--include-file-pattern PATH
```
Symbols defined in files whose path contains the `PATH` as a substring
are included.

```
--exclude-file-pattern PATH
```
Symbols defined in files whose path contains the `PATH` as a substring are
excluded. This overrides symbols that would have been included by
`--include-file-pattern`.

By default Subdoc also excludes symbols in a namespace named `__private` or
`test`, or any namespace nested within one.
TODO: Make this a command line flag.

Subdoc also excludes symbols in an anonymous namespace, or that are marked
protected or private.
TODO: Make this a command line flag.

```
FILES...
```
Subdoc can only see the symbols from the source files that you ask it to read.
Typically it is ideal to point Subdoc at your unit test files, for two reasons:
* Subdoc requires build rules in the compile database to know what flags to use
  when running Clang. The unit test files are part of the compile database.
* Subdoc (TODO: make this happen) reads `static_assert()` statements to
  determine if a type satisfies a concept. Those can be placed in your unit
  test files and Subdoc can pick them up from there.

## Additional files

A number of other flags allow including or using files with the generated
output.

```
--css PATH
```
The `PATH` is used as a path to a css file on the server, and an HTML reference
is added to it within every page. This can be the name of a file included in the
output with `--copy-file`.

```
--copy-file PATH
```
Copies the file named by `PATH` into the output directory when generating docs.
Useful for images, css stylesheets, or anything else linked to from a doc
comment.

```
--project-md PATH
```
Inserts the markdown text in the file named by `PATH` as the description of the
global namespace. This provides a good place to write a project overview that
links to various parts of the project's documentation.

## Example execution

See the
[subdoc.yml](https://github.com/chromium/subspace/blob/8be259f818684490e161eb1e4cb0420d362e18ca/.github/workflows/subdoc.yml#L152-L162)
file for an example of how the Subspace C++ Library documentation is generated.
