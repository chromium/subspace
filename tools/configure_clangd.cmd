call vcvarsall x64

@set LLVM_DIR=C:/Users/Admin/Documents/s/llvm/build/install/lib/cmake/llvm
@set Clang_DIR=C:/Users/Admin/Documents/s/llvm/build/install/lib/cmake/clang
@cmake -B out/clangd -GNinja ^
    -DCMAKE_C_COMPILER="C:/Users/Admin/Documents/s/llvm/build/install/bin/clang-cl.exe" ^
    -DCMAKE_CXX_COMPILER="C:/Users/Admin/Documents/s/llvm/build/install/bin/clang-cl.exe" ^
    -DSUBSPACE_BUILD_SUBDOC=ON ^
    -DSUBSPACE_BUILD_TESTS=ON
