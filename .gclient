# Copy this file to <subspace clone dir>/.gclient to bootstrap gclient in a
# standalone checkout of Subspace.

solutions = [
  { "name"        : ".",
    "url"         : "https://github.com/chromium/subspace",
    "deps_file"   : "DEPS",
    "managed"     : False,
  },
]
