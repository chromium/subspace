#!/bin/sh

# Set the following env vars to control this script.
#
# General:
# * USE_DEBUG=[true|false] to enable debug symbols or optimizations.
#   Default: false
# * TESTS=[true|false] to build tests.
#   Default: true
# * BENCHMARKS=[true|false] to build benchmarks.
#   Default: true
#
# Requires LLVM:
# * SUBDOC=[true|false] to build subdoc. This requires an LLVM installation.
#   Default: false
# * LLVM_ROOT=/path/to/llvm/install where there should exist `lib/cmake/llvm`
#   and `lib/cmake/clang`. Needed if SUBDOC=true or if USE_LLVM_CLANG=true.
# * USE_LLVM_CLANG=[true|false] to use clang found in the LLVM_ROOT. Otherwise
#   it uses the standard CMake compiler detection.
#   Default: false
#
# Requires compiling with Clang:
# * USE_LLD=[true|false] to link with lld linker instead of the system linker.
#   Clang must have been built with `lld` in the cmake config variable
#   `LLVM_ENABLE_PROJECTS`.
#   Default: false
# * USE_ASAN=[true|false] to build against Address Sanitizer. Clang must have
#   been built with `compiler-rt` in the cmake config variable
#   `LLVM_ENABLE_PROJECTS`.


# Defaults.
if [[ $USE_DEBUG == "" ]]; then USE_DEBUG=false; fi
if [[ $TESTS == "" ]]; then TESTS=true; fi
if [[ $BENCHMARKS == "" ]]; then BENCHMARKS=true; fi
if [[ $SUBDOC == "" ]]; then SUBDOC=false; fi
if [[ $LLVM_ROOT == "" ]]; then LLVM_ROOT=$HOME/s/llvm/install; fi
if [[ $USE_ASAN == "" ]]; then USE_ASAN=false; fi
if [[ $USE_LLD == "" ]]; then USE_LLD=false; fi
if [[ $USE_LLVM_CLANG == "" ]]; then USE_LLVM_CLANG=false; fi

# System detection.
if [[ $OSTYPE == darwin* ]]; then IS_MAC=true; else IS_MAC=false; fi

# Figure out our command line parameters.
if [[ $USE_DEBUG == true ]]; then 
    BUILD_TYPE=Debug
else
    BUILD_TYPE=Release
fi
if [[ $USE_ASAN == true ]]; then
    CXXFLAGS="$CXXFLAGS -fsanitize=address";
fi

if [[ $USE_LLD == true ]]; then
    LDFLAGS="$LDFLAGS -fuse-ld=lld"
fi
if [[ $SUBDOC == true ]]; then
    SUBSPACE_BUILD_SUBDOC=ON
else
    SUBSPACE_BUILD_SUBDOC=OFF
fi
if [[ $TESTS == true ]]; then
    SUBSPACE_BUILD_TESTS=ON
else
    SUBSPACE_BUILD_TESTS=OFF
fi
if [[ $BENCHMARKS == true ]]; then
    SUBSPACE_BUILD_BENCHMARKS=ON
else
    SUBSPACE_BUILD_BENCHMARKS=OFF
fi

function set_env() {
    echo Set env: "$@"
    export "$@"
}

function run() {
    echo Running: "$@"
    "$@" || exit 1
}

if [[ $USE_LLVM_CLANG == true ]]; then
    set_env CC="$LLVM_ROOT/bin/clang"
    set_env CXX="$LLVM_ROOT/bin/clang++"
fi

if [[ $IS_MAC == true && $USE_ASAN == true ]]; then
    if [[ $CXX == "" ]]; then CLANG=clang++; else CLANG=$CXX; fi
    RESOURCE_DIR=$($CLANG --print-resource-dir)
    run codesign -f -s - "$RESOURCE_DIR/lib/darwin/libclang_rt.asan_osx_dynamic.dylib"
fi

set_env LLVM_DIR="$LLVM_ROOT/lib/cmake/llvm"
set_env Clang_DIR="$LLVM_ROOT/lib/cmake/clang"
run cmake -B out \
        -DSUBSPACE_BUILD_TESTS=$SUBSPACE_BUILD_TESTS \
        -DSUBSPACE_BUILD_SUBDOC=$SUBSPACE_BUILD_SUBDOC \
        -DSUBSPACE_BUILD_BENCHMARKS=$SUBSPACE_BUILD_BENCHMARKS \
        -G Ninja \
        -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
        -DCMAKE_CXX_FLAGS="$CXXFLAGS" \
        -DCMAKE_EXE_LINKER_FLAGS="$LDFLAGS" \
        -DCMAKE_SHARED_LINKER_FLAGS="$LDFLAGS"
