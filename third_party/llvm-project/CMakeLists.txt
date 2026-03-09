set(LLVM_BUILD_TOOLS               No      CACHE BOOL "")
set(LLVM_ENABLE_PROJECTS          "llvm;clang" CACHE BOOL "")
set(LLVM_ENABLE_WARNINGS           No      CACHE BOOL "")
set(LLVM_INSTALL_BINUTILS_SYMLINKS No      CACHE BOOL "")
set(LLVM_INSTALL_CCTOOLS_SYMLINKS  No      CACHE BOOL "")
set(LLVM_TARGETS_TO_BUILD         "Native" CACHE BOOL "")
set(LLVM_TOOLS_INSTALL_DIR        "bin"    CACHE STRING "")

FetchContent_Declare(
  llvm-project
  GIT_REPOSITORY https://github.com/llvm/llvm-project.git
  GIT_TAG        llvmorg-17.0.6
  GIT_SHALLOW    Yes
  SOURCE_SUBDIR  llvm
  SYSTEM
  OVERRIDE_FIND_PACKAGE
)
