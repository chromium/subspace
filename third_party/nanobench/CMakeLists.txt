FetchContent_Declare(
  nanobench
  GIT_REPOSITORY https://github.com/martinus/nanobench.git
  GIT_TAG        e4327893194f06928012eb81cabc606c4e4791ac # 2024-10-06
  PATCH_COMMAND  git am "${CMAKE_CURRENT_SOURCE_DIR}/0001-updates-CMake-version.patch"
  SYSTEM
  OVERRIDE_FIND_PACKAGE
)
