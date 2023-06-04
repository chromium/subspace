# Copyright 2023 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.16)
project(subspace)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# For subdoc: Add system includes to compile_commands.json so that subdoc can
# find them. This hasn't shown to be needed on Windows for whatever reason but
# it is needed on Linux.
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/tools/cmake")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
    message(STATUS "No build type selected, default to Debug")
    set(CMAKE_BUILD_TYPE "Debug")
endif()

include(OptionIf)

option_if_not_defined(SUBSPACE_BUILD_TESTS "Build subspace tests" ON)
option_if_not_defined(SUBSPACE_BUILD_CIR "Build CIR (requires LLVM)" ON)
option_if_not_defined(SUBSPACE_BUILD_SUBDOC "Build subdoc (requires LLVM)" ON)
option_if_not_defined(SUBSPACE_BUILD_TIDY "Build clang-tidy plugin (requires LLVM)" NOT WIN32)

message(STATUS "Build CIR: ${SUBSPACE_BUILD_CIR}")
message(STATUS "Build subdoc: ${SUBSPACE_BUILD_SUBDOC}")
message(STATUS "Build clang-tidy plugin: ${SUBSPACE_BUILD_TIDY}")

function(subspace_default_compile_options TARGET)
    if(MSVC)
        target_compile_options(${TARGET} PUBLIC /Zc:preprocessor /wd5105)
    else()
        # LLVM is built without RTTI by default, and we need to match it.
        target_compile_options(${TARGET} PUBLIC -fno-rtti)
    endif()

    set_target_properties(${TARGET} PROPERTIES LINKER_LANGUAGE CXX)
    target_include_directories(${TARGET} PUBLIC ${PROJECT_SOURCE_DIR})
endfunction()

function(subspace_test_default_compile_options TARGET)
    subspace_default_compile_options(${TARGET})

    target_include_directories(${TARGET} PRIVATE
        ${PROJECT_SOURCE_DIR}/third_party/googletest
    )

    if(MSVC)
        target_compile_options(${TARGET} PUBLIC /W4 /WX)
    else()
        target_compile_options(${TARGET} PUBLIC -Werror)
    endif()

    target_link_libraries(${TARGET} gtest_main)
endfunction()

if(${SUBSPACE_BUILD_TESTS})
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    add_subdirectory(third_party/googletest)
    include(GoogleTest)
    enable_testing()
endif()

add_subdirectory(third_party/fmt)

add_subdirectory(subspace)

if(${SUBSPACE_BUILD_CIR})
    add_subdirectory(cir)
endif()

if (${SUBSPACE_BUILD_SUBDOC})
    add_subdirectory(subdoc)
endif()

if(${SUBSPACE_BUILD_TIDY})
    add_subdirectory(tidy)

    #set(CMAKE_CXX_CLANG_TIDY 
    #    clang-tidy;
    #    -header-filter=.;
    #    -checks=-*,subspace-*;
    #   -warnings-as-errors=*;
    #)
endif()
