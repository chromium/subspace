call vcvarsall x64

@set LLVM_DIR=C:/Users/Admin/Documents/s/llvm/build/install/lib/cmake/llvm
@set Clang_DIR=C:/Users/Admin/Documents/s/llvm/build/install/lib/cmake/clang
@cmake -B out -GNinja ^
    -DSUBSPACE_BUILD_SUBDOC=ON ^
    -DSUBSPACE_BUILD_TESTS=ON
