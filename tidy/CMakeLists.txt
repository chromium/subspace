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

add_library(subspace_clang_tidy_module MODULE "")
add_library(subspace::clang_tidy ALIAS subspace_clang_tidy_module)
target_sources(subspace_clang_tidy_module PUBLIC
    "llvm.h"
    "module.cc"
    "smoke_check.cc"
    "smoke_check.h"
)

target_link_libraries(subspace_clang_tidy_module
    subspace::lib

    clangTidy

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

find_package(Clang REQUIRED)
llvm_config(subdoc_lib)
target_include_directories(subspace_clang_tidy_module PUBLIC ${LLVM_INCLUDE_DIRS})
target_link_directories(subspace_clang_tidy_module PUBLIC ${LLVM_LIBRARY_DIRS})

# Subspace clang-tidy module.
subspace_default_compile_options(subspace_clang_tidy_module)

add_subdirectory(tests)
