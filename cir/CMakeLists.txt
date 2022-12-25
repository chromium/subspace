add_library(cir_lib STATIC
    "lib/syntax/let.h"
    "lib/run.cc"
    "lib/run.h"
    "lib/visit.cc"
    "lib/visit.h"
)

add_executable(cir
    "cir_main.cc"
)

add_executable(cir_unittests
    "tests/cir_test.h"
    "tests/smoke_unittest.cc"
)

# cir_lib
set_target_properties(cir_lib PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(cir_lib PUBLIC ..)

find_package(LLVM REQUIRED)
find_package(Clang REQUIRED)
llvm_config(cir_lib)
target_include_directories(cir_lib PUBLIC ${LLVM_INCLUDE_DIRS})
target_link_directories(cir_lib PUBLIC ${LLVM_LIBRARY_DIRS})

target_link_libraries(cir_lib
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

# cir_unittests
set_target_properties(cir_unittests PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(cir_unittests PUBLIC
    ..
    ../third_party/googletest
)
target_link_libraries(cir_unittests
    cir_lib
    gtest_main
)

if(MSVC)
    target_compile_options(cir_unittests PUBLIC /W4 /WX)
else()
    target_compile_options(cir_unittests PUBLIC -Werror)
endif()

gtest_discover_tests(
    cir_unittests
)

# cir binary
set_target_properties(cir PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(cir cir_lib)
