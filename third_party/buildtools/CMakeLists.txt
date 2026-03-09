FetchContent_Declare(
  buildtools
  GIT_REPOSITORY https://chromium.googlesource.com/chromium/src/buildtools.git
  GIT_TAG        136da69a1267b8db487354b96d44d0cc8add5aeb # 2026-02-26
  SYSTEM
  OVERRIDE_FIND_PACKAGE
)
