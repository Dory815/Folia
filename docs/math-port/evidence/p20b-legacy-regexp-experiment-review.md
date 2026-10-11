# P20b root review — isolated legacy canonicalization

This is a prerequisite experiment, not regexp-engine adoption, TokenMap
acceptance or a MathJax Source-file acceptance. Production code is unchanged.
The candidate remains regress 0.12.0 at
771e6c8fa1e472ef86c45b5206b37fa4e4494e25, under its MIT OR Apache-2.0 license.

Root froze ../regexp-legacy-experiment-contract.md, copied tracked Source into
an ignored directory, and generated a separate legacy mapping from Node
24.13.0 / Unicode 16 / ICU 77.1 full uppercase conversion. This data generator
does not run RegExp and cannot derive the expected match results. Root's
separate actual-RegExp oracle freezes 111,818 constructor/match/capture/UTF-16
range/lastIndex observations across 2,431 relevant BMP units. The generated
table has 1,166 nonidentity mappings, 102 full-uppercase expansion exclusions
and two non-ASCII-to-ASCII exclusions. Native host semantics remain unaccepted.

Root reviewed Luna's unicode.rs/parse.rs against Source. Legacy scalar folding,
literal equivalence and bracket/range closure now share the same mapping;
closure occurs before negation. Unicode u/v folding and property-class rules
retain their separate Source path. Previous parser fixes remain present.
No pattern-spelling or individual-code-point exception was introduced.

The original 119,218 cases improve by 60, with no regressions, in both recorded
flag adapters. The additional 111,818 cases improve by 421 with no regressions
under the correct v-implies-Unicode adapter: 111,798 equal, 20 different.
The literal-flags diagnostic adapter records 421 fixes and 28 regressions;
it incorrectly treats v as independent of Unicode and is not an acceptable
host contract. Both metrics are retained in p20b-legacy-regexp-repair-result.json.
The original correct-adapter corpus still has 1,904 differences: 1,784 syntax /
capture / Unicode-mode cases and 120 Unicode-version cases.

The 20 new differences include two surrogate-escape parser failures and 18
Unicode word-boundary traversal cases. These remain failures, not waived
expectations. All upstream regress tests with utf16 pass: 551 passed, zero
failed, one ignored doctest. Log: build/p20b-casefold-upstream-tests.log.

Root's reproducible prepare tool applies committed patches to the original
pinned tracked files, adds the root mapping module and uses the committed
experiment workspace lock. --fresh-check reproduced an independent copy.
That check exposed differing CRLF/LF mixtures in patch output; changed-body
checks now normalize line endings and compare identical Source text hashes.
Original Source byte hashes remain strict, and result reports retain actual
changed-file byte hashes. Existing experiment files are never reset.
Build manifests, public API, indexing and Unicode tables are byte-guarded.
No mathematical layout or Mermaid backend gate is passed here.
