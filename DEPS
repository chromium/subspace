use_relative_paths = True

vars = {
  'chromium_git': 'https://chromium.googlesource.com',
  'external_gh': '/external/github.com',

  'buildtools_revision': 'edbefcee3d2cc45cdb0c60c2b01b673f8ba728bc',
  'googletest_revision': 'ec25eea8f8237cf86c30703f59747e42f34b6f75',
}

deps = {
  'third_party/buildtools':
    Var('chromium_git') + '/chromium/src/buildtools@' +
    Var('buildtools_revision'),

  'third_party/googletest':
    Var('chromium_git') + Var('external_gh') + '/google/googletest@' +
    Var('googletest_revision'),
}

hooks = [
  # Pull clang-format binaries using checked-in hashes.
  {
    'name': 'clang_format_win',
    'pattern': '.',
    'condition': 'host_os == "win"',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'third_party/buildtools/win/clang-format.exe.sha1',
    ],
  },
  {
    'name': 'clang_format_mac_x64',
    'pattern': '.',
    'condition': 'host_os == "mac" and host_cpu == "x64"',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'third_party/buildtools/mac/clang-format.x64.sha1',
                '-o', 'third_party/buildtools/mac/clang-format',
    ],
  },
  {
    'name': 'clang_format_mac_arm64',
    'pattern': '.',
    'condition': 'host_os == "mac" and host_cpu == "arm64"',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'third_party/buildtools/mac/clang-format.arm64.sha1',
                '-o', 'third_party/buildtools/mac/clang-format',
    ],
  },
  {
    'name': 'clang_format_linux',
    'pattern': '.',
    'condition': 'host_os == "linux"',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'third_party/buildtools/linux64/clang-format.sha1',
    ],
  }
]

