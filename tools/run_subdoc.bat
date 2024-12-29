out\subdoc\subdoc -p out --out docs ^
    --include-file-pattern /sus/ ^
    --exclude-file-pattern /third_party/ ^
    --exclude-file-pattern /test/ ^
    --exclude-file-pattern test.cc ^
    --include-macro-prefix sus_ ^
    --include-macro-prefix SUS_ ^
    --css subdoc-test-style.css ^
    --copy-file subdoc/gen_tests/subdoc-test-style.css ^
    --copy-file "web/logos/logo-512px/subspace unpadded.png;logo.png" ^
    --favicon "logo.png;image/png" ^
    --project-md sus/project.md ^
    --project-logo logo.png ^
    --project-name Subspace ^
    --project-version 0.1.2 ^
    --ignore-bad-code-links ^
    --remove-source-path-prefix %cd% ^
    --add-source-path-prefix .. ^
    --source-path-line-prefix L ^
    subspace/sus/num/uptr_uni reverse_uni
