# Copyright 2023 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

add_library(subdoc_lib STATIC "")
add_library(subdoc::lib ALIAS subdoc_lib)
target_sources(subdoc_lib PUBLIC
    "lib/gen/files.h"
    "lib/gen/generate_function.cc"
    "lib/gen/generate_function.h"
    "lib/gen/generate_head.cc"
    "lib/gen/generate_head.h"
    "lib/gen/generate_namespace.cc"
    "lib/gen/generate_namespace.h"
    "lib/gen/generate_record.cc"
    "lib/gen/generate_record.h"
    "lib/gen/generate.cc"
    "lib/gen/generate.h"
    "lib/gen/html_writer.h"
    "lib/gen/options.h"
    "lib/database.h"
    "lib/friendly_names.h"
    "lib/parse_comment.h"
    "lib/path.h"
    "lib/record_type.h"
    "lib/run_options.h"
    "lib/run.cc"
    "lib/run.h"
    "lib/type.h"
    "lib/unique_symbol.h"
    "lib/visit.cc"
    "lib/visit.h"
)

add_executable(subdoc
    "subdoc_main.cc"
)

# subdoc_lib
subspace_default_compile_options(subdoc_lib)

find_package(LLVM REQUIRED)
find_package(Clang REQUIRED)
llvm_config(subdoc_lib)
target_include_directories(subdoc_lib PUBLIC ${LLVM_INCLUDE_DIRS})
target_link_directories(subdoc_lib PUBLIC ${LLVM_LIBRARY_DIRS})

target_link_libraries(subdoc_lib
    subspace::lib
    clangAnalysis
    clangAnalysisFlowSensitive
    clangAnalysisFlowSensitiveModels
    clangAPINotes
    clangARCMigrate
    clangAST
    clangASTMatchers
    clangBasic
    clangCodeGen
    clangCrossTU
    clangDependencyScanning
    clangDirectoryWatcher
    clangDriver
    clangDynamicASTMatchers
    clangEdit
    clangExtractAPI
    clangFormat
    clangFrontend
    clangFrontendTool
    clangHandleCXX
    clangHandleLLVM
    clangIndex
    clangIndexSerialization
    clangInterpreter
    clangLex
    clangParse
    clangRewrite
    clangRewriteFrontend
    clangSema
    clangSerialization
    clangStaticAnalyzerCheckers
    clangStaticAnalyzerCore
    clangStaticAnalyzerFrontend
    clangSupport
    clangTooling
    clangToolingASTDiff
    clangToolingCore
    clangToolingInclusions
    clangToolingInclusionsStdlib
    clangToolingRefactoring
    clangToolingSyntax
    clangTransformer
)

# subdoc binary
subspace_default_compile_options(subdoc)
target_link_libraries(subdoc subdoc::lib)

if(${SUBSPACE_BUILD_TESTS})
    add_executable(subdoc_unittests
        "gen_tests/gen_tests.cc"
        "tests/fields_unittest.cc"
        "tests/functions_unittest.cc"
        "tests/include_exclude_unittest.cc"
        "tests/inherit_unittest.cc"
        "tests/macros_unittest.cc"
        "tests/markdown_unittest.cc"
        "tests/methods_unittest.cc"
        "tests/namespaces_unittest.cc"
        "tests/records_unittest.cc"
        "tests/styles_unittest.cc"
        "tests/types_unittest.cc"
        "tests/subdoc_gen_test.h"
        "tests/subdoc_test.h"
    )

    # subdoc_unittests
    subspace_test_default_compile_options(subdoc_unittests)
    target_link_libraries(subdoc_unittests subdoc::lib)

    gtest_discover_tests(subdoc_unittests)
endif()

