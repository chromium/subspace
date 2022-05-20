# Copyright 2022 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cc_library(
    name = "subspace",
    hdrs = [
        "assertions/builtin.h",
        "assertions/check.h",
        "assertions/panic.h",
        "assertions/unreachable.h",
        "marker/unsafe.h",
        "mem/__private/relocate.h",
    ],
    linkopts = [
        "-lc",
        "-lm",
    ]
)

cc_test(
    name = "subspace_unittests",
    deps = [
        ":subspace",
        "//third_party/googletest:gtest_main",
    ],
    srcs = [
        "assertions/check_unittest.cc",
        "assertions/panic_unittest.cc",
        "assertions/unreachable_unittest.cc",
        "mem/__private/relocate_unittest.cc",
    ],
    copts = [
        "-fPIE"
    ],
    linkopts = [
        "-lstdc++",
    ],
)
