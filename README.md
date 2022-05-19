# Subspace Library

An experimental take on a safer, simpler C++ standard library.

Please don't use this library. This is an experiment and we don't yet know where
it will take us. There will be breaking changes without warning, and there is no
stable version.

# Copyright

All source files must include this header:
```
// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
```

# Getting started

1. Install [Bazelisk](https://github.com/bazelbuild/bazelisk) to install Bazel,
   and [set it up with your IDE](https://bazel.build/install/ide).
1. Install [LLVM](https://releases.llvm.org/download.html).
1. Build and run tests: `bazel test :all`
