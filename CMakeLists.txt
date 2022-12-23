cmake_minimum_required(VERSION 3.16)
project(subspace)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(MSVC)
    add_compile_options(/Zc:preprocessor /wd5105)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Werror)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

enable_testing()

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(third_party/googletest)

add_subdirectory(subspace)
