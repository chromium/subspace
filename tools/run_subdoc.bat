out\subdoc\subdoc -p out --out docs ^
    --include-file-pattern /sus/ ^
    --exclude-file-pattern /third_party/ ^
    --css subdoc-test-style.css ^
    --copy-file subdoc/gen_tests/subdoc-test-style.css ^
    --copy-file web/logo.png ^
    --favicon "logo.png;image/png" ^
    --project-md sus/project.md ^
    --project-name Subspace ^
    --ignore-bad-code-links ^
    subspace/sus/num/uptr ^
    subspace/sus/error ^
    subspace/sus/boxed ^
    subspace/sus/iter/compat_ranges_unittest ^
    subspace/sus/choice 
