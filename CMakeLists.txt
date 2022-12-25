cmake_minimum_required(VERSION 3.16)
project(subspace)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(MSVC)
    add_compile_options(/Zc:preprocessor /wd5105)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(third_party/googletest)
include(GoogleTest)
enable_testing()

add_subdirectory(subspace)

if (NOT DEFINED ENV{SUBSPACE_NO_BUILD_CIR})
    add_subdirectory(cir)
endif()
