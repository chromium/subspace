if(SUBSPACE_BUILD_SUBDOC)
  FetchContent_Declare(
    md4c
    GIT_REPOSITORY https://github.com/mity/md4c.git
    GIT_TAG        e9ff661ff818ee94a4a231958d9b6768dc6882c9 # 2024-02-25
    PATCH_COMMAND
      git am
        "${CMAKE_CURRENT_SOURCE_DIR}/0001-Add-same-document-links-from-headers-to-themselves.patch"
        "${CMAKE_CURRENT_SOURCE_DIR}/0002-Mention-what-this-fork-does-with-header-links.patch"
        "${CMAKE_CURRENT_SOURCE_DIR}/0003-Move-md_html-callbacks-into-a-structure.patch"
        "${CMAKE_CURRENT_SOURCE_DIR}/0004-Add-codelinks-with-as-the-first-character-in-the-URL.patch"
        "${CMAKE_CURRENT_SOURCE_DIR}/0005-Allow-code-links-and-self-links-callbacks-to-report-.patch"
        "${CMAKE_CURRENT_SOURCE_DIR}/0006-Prevent-openers-with-negative-next-field-to-avoid-me.patch"
        "${CMAKE_CURRENT_SOURCE_DIR}/0007-Enable-MD_ASSERT-so-as-to-catch-memory-safety-bugs.patch"
        "${CMAKE_CURRENT_SOURCE_DIR}/0008-Assert-that-dest_size-is-not-negative.patch"
        "${CMAKE_CURRENT_SOURCE_DIR}/0009-Add-assertions-on-subtractions-that-become-unsigned.patch"
        "${CMAKE_CURRENT_SOURCE_DIR}/0010-Add-more-asserts-on-subtractions-looking-for-underfl.patch"
        "${CMAKE_CURRENT_SOURCE_DIR}/0011-Fix-README-typo.patch"
        "${CMAKE_CURRENT_SOURCE_DIR}/0012-Fix-narrowing-and-sign-conversion-warnings-and-asser.patch"
        "${CMAKE_CURRENT_SOURCE_DIR}/0013-Bump-the-cmake-version-to-3.16-matching-subspace.patch"
    SYSTEM
    OVERRIDE_FIND_PACKAGE
  )
endif()
