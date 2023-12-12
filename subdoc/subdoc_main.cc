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

#include <filesystem>
#include <fstream>
#include <unordered_set>

#include "lib/gen/generate.h"
#include "lib/run.h"
#include "subdoc/llvm.h"
#include "sus/prelude.h"

int main(int argc, const char** argv) {
  llvm::InitLLVM init(argc, argv);
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();

  llvm::cl::OptionCategory option_category("SubDoc options");

  llvm::cl::opt<std::string> option_project_name(
      "project-name",
      llvm::cl::desc("The name of the project, which will appear in the "
                     "generated output."),
      llvm::cl::cat(option_category));

  llvm::cl::opt<std::string> option_out(
      "out",
      llvm::cl::desc("Where to generate the docs. Defaults to `./out/docs`"),
      llvm::cl::init("out/docs"),  //
      llvm::cl::cat(option_category));

  llvm::cl::opt<std::string> option_project_md(
      "project-md",
      llvm::cl::desc("A markdown file containing an overview of the project, "
                     "to insert into the project root"),
      llvm::cl::cat(option_category));

  llvm::cl::list<std::string> option_css(
      "css",
      llvm::cl::desc(
          "A CSS file to include in the generated HTML header. May be "
          "specified multiple times for multiple files.\n"
          "\n"
          "When rendering the HTML, a <link> tag will be added\n"
          "with each path to a CSS file that is specified. For\n"
          "example: \"../main.css,other.css,/top.css\"."),
      llvm::cl::cat(option_category));

  llvm::cl::opt<std::string> option_project_logo(
      "project-logo",
      llvm::cl::desc("The path (on the website) to the project logo image."),
      llvm::cl::cat(option_category));

  llvm::cl::list<std::string> option_favicon(
      "favicon",
      llvm::cl::desc(
          "The path (on the website) to an icon to act as the favicon and its "
          "mime type, separated by a semicolon."
          "May be specified multiple times for multiple files in which case "
          "the first is used as the primary and the others as alternates.\n"
          "\n"
          "When rendering the HTML, a <link> tag will be added\n"
          "with each path to an icon file that is specified. For\n"
          "example: \"favicon.png;image/png,favicon-vec.svg;image/svg+xml\"."),
      llvm::cl::cat(option_category));

  llvm::cl::list<std::string> option_copy_files(
      "copy-file",
      llvm::cl::desc("A file to be copied into the output directory. May be "
                     "specified multiple times for multiple files."),
      llvm::cl::cat(option_category));

  llvm::cl::list<std::string> option_include_paths(
      "include-file-pattern",
      llvm::cl::desc(
          "A path pattern for which documentation should be included in the "
          "generated HTML. May be specified multiple times for multiple "
          "patterns. This is required."),
      llvm::cl::cat(option_category));

  llvm::cl::list<std::string> option_exclude_paths(
      "exclude-file-pattern",
      llvm::cl::desc(
          "A path pattern for which documentation should be excluded from the "
          "generated HTML. May be specified multiple times for multiple "
          "patterns."),
      llvm::cl::cat(option_category));

  llvm::cl::list<std::string> option_include_macro_prefixes(
      "include-macro-prefix",
      llvm::cl::desc(
          "Macros are only included in the generated output if they match a "
          "prefix specified by --include-macro-prefix. May be specified "
          "multiple times for multiple prefixes."),
      llvm::cl::cat(option_category));

  llvm::cl::opt<std::string> option_remove_path_prefix(
      "remove-source-path-prefix",
      llvm::cl::desc("A path prefix to remove from all source code links."),
      llvm::cl::cat(option_category));

  llvm::cl::opt<std::string> option_add_path_prefix(
      "add-source-path-prefix",
      llvm::cl::desc(
          "A path prefix to add to all source code links, after any prefix "
          "specified by `--remove-source-path-prefix` is removed."),
      llvm::cl::cat(option_category));

  llvm::cl::opt<bool> option_no_source_links(
      "no-source-links",
      llvm::cl::desc("Avoid generating links to source code."),
      llvm::cl::init(false),  //
      llvm::cl::cat(option_category));

  llvm::cl::opt<bool> option_ignore_bad_code_links(
      "ignore-bad-code-links",
      llvm::cl::desc("Ignore bad code links, don't generate an error. Useful "
                     "for generating partial docs."),
      llvm::cl::init(false),  //
      llvm::cl::cat(option_category));

  llvm::Expected<clang::tooling::CommonOptionsParser> options_parser =
      clang::tooling::CommonOptionsParser::create(argc, argv, option_category,
                                                  llvm::cl::ZeroOrMore);
  if (!options_parser) {
    llvm::WithColor::error() << llvm::toString(options_parser.takeError());
    return 1;
  }

  auto& comp_db = options_parser->getCompilations();

  std::vector<std::string> paths = options_parser->getSourcePathList();
  if (paths.empty()) {
    fmt::println(stderr, "Error: no input files specified.");
    llvm::cl::PrintHelpMessage(/*Hidden=*/false, /*Categorized=*/true);
    return 1;
  }

  // These are the files available to run the tool against.
  std::vector<std::string> comp_db_files = comp_db.getAllFiles();
  // These are the files we choose to run the tool against. We use fuzzy
  // matching on the input arguments to pick them.
  sus::Vec<std::string> run_against_files;

  for (const std::string& input_path : paths) {
    bool found = false;
    for (const std::string& s : comp_db_files) {
      // Canonicalize the path to use `/` instead of `\`.
      auto canonical_path = std::string(s);
      std::replace(canonical_path.begin(), canonical_path.end(), '\\', '/');

      if (canonical_path.find(input_path) == std::string::npos) {
        continue;
      }
      run_against_files.push(sus::move(canonical_path));
      found = true;
    }
    if (!found) {
      fmt::println(stderr, "Unknown file, not in compiledb: {}", input_path);
      return 1;
    }
  }

  auto paths_to_regex = [](const auto& paths) {
    std::ostringstream s;
    bool add_or = false;
    std::string escaped;
    for (const std::string& p : paths) {
      if (add_or) s << "|";
      if (size_t pos = p.rfind('\\'); pos != std::string::npos) {
        // Turn `\` into `\\` so it is not read as an escape sequence in the
        // regex.
        escaped = p;
        while (pos != std::string::npos) {
          escaped.replace(pos, 1, "\\\\");
          if (pos == 0u)
            pos = std::string::npos;
          else
            pos = escaped.rfind('\\', pos - 1u);
        }
        s << escaped;
      } else {
        s << p;
      }
      add_or = true;
    }
    return std::regex(sus::move(s).str());
  };

  if (option_include_paths.empty()) {
    fmt::println(
        stderr,
        "Error: Missing --include-file-pattern. Without this, subdoc would "
        "generate docs for every library used from the source files. "
        "Specify which path pattern(s) to generate docs for.");
    return 1;
  }

  auto run_options = subdoc::RunOptions();
  run_options.include_path_patterns = paths_to_regex(option_include_paths);
  if (!option_exclude_paths.empty())
    run_options.exclude_path_patterns = paths_to_regex(option_exclude_paths);
  if (option_project_md.getNumOccurrences() > 0) {
    auto markdown_file =
        std::ifstream(std::filesystem::path(option_project_md.getValue()));
    if (markdown_file.is_open()) {
      std::ostringstream stream;
      std::string line;
      while (std::getline(markdown_file, line)) stream << line << "\n";
      run_options.project_overview_text = sus::move(stream).str();
    }
  }
  run_options.macro_prefixes =
      sus::iter::from_range(option_include_macro_prefixes)
          .cloned()
          .collect<sus::Vec<std::string>>();
  run_options.generate_source_links = !option_no_source_links.getValue();
  if (option_remove_path_prefix.getNumOccurrences() > 0) {
    // Canonicalize the path to use `/` instead of `\`.
    std::string canonical_path = option_remove_path_prefix.getValue();
    std::replace(canonical_path.begin(), canonical_path.end(), '\\', '/');
    run_options.remove_path_prefix = sus::some(sus::move(canonical_path));
  }
  if (option_add_path_prefix.getNumOccurrences() > 0) {
    run_options.add_path_prefix = sus::some(option_add_path_prefix.getValue());
  }

  auto fs = llvm::vfs::getRealFileSystem();
  auto result = subdoc::run_files(comp_db, sus::move(run_against_files),
                                  sus::move(fs), sus::move(run_options));
  if (result.is_err()) {
    fmt::println(stderr, "Error occurred. Exiting.");
    return 1;
  }

  subdoc::Database docs_db = sus::move(result).unwrap();

  auto gen_options = subdoc::gen::Options{
      .output_root = std::filesystem::path(option_out.getValue()),
      .stylesheets = sus::empty,
      .favicons = sus::empty,
      .copy_files = sus::empty,
      .ignore_bad_code_links = option_ignore_bad_code_links.getValue(),
  };
  if (option_project_name.getNumOccurrences() > 0) {
    gen_options.project_name = option_project_name.getValue();
  }
  if (option_project_logo.getNumOccurrences() > 0) {
    gen_options.project_logo = option_project_logo.getValue();
  }
  if (option_css.empty() && option_copy_files.empty()) {
    // Defaults to pull the test stylesheet, assuming subdoc is being run from
    // the source root directory.
    gen_options.copy_files.push("subdoc/gen_tests/subdoc-test-style.css");
    gen_options.stylesheets.push("subdoc-test-style.css");
  } else {
    for (std::string s : sus::move(option_css))
      gen_options.stylesheets.push(sus::move(s));
    for (std::string s : sus::move(option_copy_files))
      gen_options.copy_files.push(sus::move(s));
  }
  for (std::string s : sus::move(option_favicon)) {
    auto favicon_result = subdoc::gen::FavIcon::from_string(s);
    if (favicon_result.is_err()) {
      fmt::println(stderr, "ERROR: {}", favicon_result.as_err());
      return 1;
    }
    gen_options.favicons.push(sus::move(favicon_result).unwrap());
  }

  fmt::println("Generating into '{}'", gen_options.output_root.string());
  sus::result::Result<void, sus::Box<sus::error::DynError>> r =
      subdoc::gen::generate(docs_db, sus::move(gen_options));
  if (r.is_err()) {
    fmt::println(stderr, "ERROR: {}", r.as_err());
    for (sus::Option<const sus::error::DynError&> source =
             sus::error::error_source(r.as_err());
         source.is_some();
         source = sus::error::error_source(source.as_value())) {
      fmt::println(stderr, "  note: {}", source.as_value());
    }
    return 1;
  }
  return 0;
}
