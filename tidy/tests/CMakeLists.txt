# Copyright 2023 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

enable_testing()

function(tidy_test check_name)
  add_test(NAME "RunClangTidy.${check_name}" COMMAND ${CMAKE_COMMAND}
    "-DCLANG_TIDY_COMMAND=$<TARGET_FILE:clang-tidy>"
    "-DCLANG_TIDY_MODULE=$<TARGET_FILE:subspace_clang_tidy_module>"
    "-DCHECK_NAME=${check_name}"
    "-DRunClangTidy_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/run_clang_tidy.cmake"
    )
endfunction()

tidy_test(subspace-smoke)
