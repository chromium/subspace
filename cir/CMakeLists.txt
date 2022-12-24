add_library(cir_lib STATIC
    "lib/run.cc"
    "lib/run.h"
    "lib/visit.cc"
    "lib/visit.h"
)

add_executable(cir
    "main.cc"
)

set_target_properties(cir PROPERTIES LINKER_LANGUAGE CXX)

# Subspace config.
target_include_directories(cir_lib PUBLIC ..)
target_link_libraries(cir_lib subspace)
target_link_libraries(cir cir_lib)

# Clang/LLVM config.
find_package(LLVM REQUIRED)
find_package(Clang REQUIRED)

target_include_directories(cir_lib PUBLIC ${LLVM_INCLUDE_DIRS})
target_link_directories(cir_lib PUBLIC ${LLVM_LIBRARY_DIRS})

if(MSVC)
    # This depends on if the LLVM build was debug or release:
    if($ENV{LLVM_DEBUG})
        set_property(TARGET cir_lib PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL")
        set_property(TARGET cir PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL")
    else()
        set_property(TARGET cir_lib PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
        set_property(TARGET cir PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
    endif()
endif()

llvm_config(cir)

target_link_libraries(cir
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
