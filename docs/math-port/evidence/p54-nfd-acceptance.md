# P54 native Unicode NFD acceptance

Current component check (2026-10-20): the existing implementation is restored
to `folia_math`. `unicode_normalization.cpp` now loads the generated corpus TSV
at runtime; only 85 sequence cases and 17 plane digests remain in a small include.
The previous generated 20,077-line test translation unit and its obsolete test
runner are removed. The mathematical implementation and Unicode data are unchanged.
The source/data generators pass `--check`; the component build passes, and
`FoliaMathTests.exe "*unicode normalization*"` passes 99,928 assertions in three
tests (`build/math-port/unicode-normalization-focused.log`). The extra assertion
checks the exact corpus row count. Coverage remains 99,825 exact corpus outputs,
85 exact sequence outputs and 17 full-plane fingerprints. The historical test
counts and runner references below apply to the previous test organization.
This checks the NFD dependency; it does not accept French style dispatch,
ParserConfiguration wiring, the complete TeX pipeline or Folia integration.

The portable Unicode16Normalization body supplies the actual NFD operation
required by Source ParseOptions French math style and ParseUtil character
classification. It is stateless and consumes the hash-pinned Unicode 16 data;
no Windows API, restored ICU dependency or JS engine is involved.

Root froze data, original gold and a compiling failing stub before Luna's
implementation (cb3007f). A second Luna independently audited both gold and
the stable body. Root inspected canonical recursive decomposition, Hangul
range/formula and optional final Jamo, stable non-starter runs, class-zero
boundaries, UTF-16 decoding/re-encoding and isolated surrogate/NUL preservation.
No blocking review finding remains. The original oracle enforces Unicode 16.0;
its Node 24.13.0 and ICU 77.1 versions are observed reference metadata.

`build/p54-nfd-baseline.log` passes 99927 assertions in 103 tests, exit 0:

- 99825 exact outputs across 19965 official five-column normalization rows.
- 85 exact UTF-16 sequence outputs, including deterministic mixed sequences.
- 17 full-plane FNV64 output fingerprints over all 1114112 codepoints. These
  fingerprints are additional evidence, not direct per-scalar comparisons.

All nine compiling faults in p54-nfd-mutations.json fail original assertions:
one-level decomposition, disabled Hangul, spurious empty-tail Jamo, descending
CCC order, changed equal-class order, crossing starters, separate valid surrogate
halves, replaced unmatched surrogates and NUL truncation. Frozen data and gold
stay byte-identical. The restored correct implementation passes again.

The parser string-key Map host also detects all four compiling faults recorded
in p53-string-map-mutations.json, with its 12 original observations passing
before and after faults. Those cover duplicate replacement, revived exhausted
iterator, snapshotted iterator storage and reporting an existing key as missing.
Independent Luna review checked the actual Configuration/Tags/ParseOptions and
enabled-package Map callsites and found no blocker for this string-key surface.
The native key iterator returns an optional key for native loops; it does not
claim an externally observable JavaScript iterator-result object API.

Generators reproduce with --check. All validation reuses build/core; no large
duplicated build tree is created. The complete build_test.bat core build and
regression suite passes, exit 0 (build/p54-full-core.log).

This accepts the NFD host and required Map host surface. It does not accept
the still-unimplemented ParseOptions algorithms, native TeX/package pipeline,
fonts, mathematical layout, SVG output or Folia integration. The next main
route is actual parser-family bodies over the committed Source declaration
graph, with module-lifetime metadata/registries retained by ParserRuntime.
