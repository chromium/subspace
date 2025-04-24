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

# compile_commands.json is needed for Subdoc's documentation generation.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/tools/cmake")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
    message(STATUS "No build type selected, default to Debug")
    set(CMAKE_BUILD_TYPE "Debug")
endif()

include(OptionIf)

option_if_not_defined(SUBSPACE_BUILD_TESTS "Build tests" OFF)
option_if_not_defined(SUBSPACE_BUILD_SUBDOC "Build subdoc (requires LLVM)" OFF)
option_if_not_defined(SUBSPACE_BUILD_BENCHMARKS "Build benchmarks" OFF)

message(STATUS "Build tests: ${SUBSPACE_BUILD_TESTS}")
message(STATUS "Build subdoc: ${SUBSPACE_BUILD_SUBDOC}")
message(STATUS "Build benchmarks: ${SUBSPACE_BUILD_BENCHMARKS}")

function(subspace_default_compile_options TARGET)
    if(MSVC)
        target_compile_options(${TARGET} PUBLIC /Zc:preprocessor /wd5105)
    else()
        # LLVM is built without RTTI by default, and we need to match it.
        target_compile_options(${TARGET} PUBLIC -fno-rtti)

        if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR
           CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
          # Any use of _Nonnull causes every pointer to need annotation, which
          # we have not done.
          target_compile_options(${TARGET} PUBLIC -Wno-nullability-completeness)
        endif()
    endif()

    set_target_properties(${TARGET} PROPERTIES LINKER_LANGUAGE CXX)
    target_include_directories(${TARGET} PUBLIC ${PROJECT_SOURCE_DIR})
endfunction()

function(subspace_binary_default_compile_options TARGET)
    subspace_default_compile_options(${TARGET})
    if(MSVC)
        target_compile_options(${TARGET} PUBLIC /W4 /WX)
    else()
        target_compile_options(${TARGET} PUBLIC -Werror)
    endif()
endfunction()

function(subspace_test_default_compile_options TARGET)
    subspace_binary_default_compile_options(${TARGET})

    target_include_directories(${TARGET} PRIVATE
        ${PROJECT_SOURCE_DIR}/third_party/googletest
    )
endfunction()

if(${SUBSPACE_BUILD_TESTS} OR ${SUBSPACE_BUILD_BENCHMARKS})
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    # Prevent googletest from including libc++abi's cxxabi.h, since it has a
    # conflicting declaration of the __cxa_init_primary_exception symbol with
    # libstdc++. It's very unclear why googletest unconditionally includes this
    # header when the build is not set up to use libc++abi otherwise. We could
    # add detection here to keep it when using libc++abi but we currently
    # don't. See https://github.com/llvm/llvm-project/issues/121300
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGTEST_HAS_CXXABI_H_=0")
    add_subdirectory(third_party/googletest)
    include(GoogleTest)
    enable_testing()
endif()

if(${SUBSPACE_BUILD_BENCHMARKS})
    add_subdirectory(third_party/nanobench)
    set_property(TARGET nanobench PROPERTY CXX_STANDARD 20)
    add_subdirectory(bench)
endif()

add_subdirectory(third_party/fmt)

add_subdirectory(sus)

if (${SUBSPACE_BUILD_SUBDOC})
    add_subdirectory(third_party/md4c)
    add_subdirectory(subdoc)
endif()
