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

add_library(subdoc_lib STATIC
    "lib/database.h"
    "lib/run.cc"
    "lib/run.h"
    "lib/visit.cc"
    "lib/visit.h"
)

add_executable(subdoc
    "subdoc_main.cc"
)

add_executable(subdoc_unittests
    "tests/subdoc_test.h"
    "tests/smoke_unittest.cc"
)

# subdoc_lib
set_target_properties(subdoc_lib PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(subdoc_lib PUBLIC ..)

find_package(LLVM REQUIRED)
find_package(Clang REQUIRED)
llvm_config(subdoc_lib)
target_include_directories(subdoc_lib PUBLIC ${LLVM_INCLUDE_DIRS})
target_link_directories(subdoc_lib PUBLIC ${LLVM_LIBRARY_DIRS})

target_link_libraries(subdoc_lib
    subspace
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

# subdoc_unittests
set_target_properties(subdoc_unittests PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(subdoc_unittests PUBLIC
    ..
    ../third_party/googletest
)
target_link_libraries(subdoc_unittests
    subdoc_lib
    gtest_main
)

if(MSVC)
    target_compile_options(subdoc_unittests PUBLIC /W4 /WX)
else()
    target_compile_options(subdoc_unittests PUBLIC -Werror)
endif()

gtest_discover_tests(
    subdoc_unittests
)

# subdoc binary
set_target_properties(subdoc PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(subdoc subdoc_lib)
