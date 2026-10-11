# P21b root review — isolated Unicode escape grammar repair

This is an ignored candidate-engine experiment, not native host, TokenMap,
complete parser, MathJax Source file or Folia acceptance. Native MathJax local
Source acceptance remains 53/246. The original .reference/regress stays pinned
and unchanged; every predecessor remains protected.

Root froze responsibility and actual Node corpus in f07714e before dispatch.
Luna changed only the successor parse.rs and its report. Root reviewed two
separate grammar booleans, ordinary versus named-identifier callers, the
legacy cursor restore, all three ASCII HexDigit paths and retained P20d rollback.
Only this generic grammar responsibility changes; no pattern-specific fix,
flags adapter, matching, indexing or Unicode/case table changed.

The four independently pinned actual-Node corpora total 266,556 cases.
In the correct v-implies-Unicode adapter:

| Corpus | Cases | Equal | Remaining | Newly fixed | Regressions |
|---|---:|---:|---:|---:|---:|
| Original | 119,218 | 117,314 | 1,904 | 0 | 0 |
| Legacy | 111,818 | 111,800 | 18 | 0 | 0 |
| Lookahead | 4,032 | 4,032 | 0 | 18 | 0 |
| Unicode escape | 31,488 | 31,488 | 0 | 2,380 | 0 |

Both adapter modes and all retained differences are recorded in
p21b-unicode-escape-regexp-repair-result.json. Literal-flags mode is diagnostic,
not a proved JS protocol. It is not used to claim equivalence.

All eight effective mutants compile and finish normally; actual Node observes
differences for each. They cover braced grammar, ordinary pair behavior,
HexDigit checks, named grammar/pairs and code-point range. The preliminary
widened-range mutant produced a caught native panic and is excluded; root
replaced it with narrowing and reran the complete set. Panics cannot pass.
Final results: p21b-unicode-escape-mutations.json.

The complete unchanged locked/offline upstream utf16 suite with --no-fail-fast
returns exit 101: **548 pass / 3 fail / 1 ignored**. The three failures expect
braced Unicode escapes without Unicode flags to match a code point. Root's
separate actual Node witnesses contradict the first expectation in each test;
p21b-upstream-escape-witnesses.json records source hash and observed ranges.
No upstream tests or expectations were changed, waived or counted as passing.
The upstream gate remains failed and the candidate cannot be adopted.

Root's full remaining-pattern inventory has 48 original pattern/flag groups;
1,504 constructor differences and many membership differences concern the
candidate's Unicode 17 data versus pinned Node's Unicode 16 data. The isolated
P21c data experiment handles that next. Empty-string v sets, v ignore-case set
algebra, string properties and astral zero-width protocol remain open. Logs and
native outputs stay under ignored build directories; reproducible Source
preparation, replay and mutations are root-owned tools.
