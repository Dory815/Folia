# P20d root review — speculative surrogate escape rollback

This bounded experiment changes only a generic parser rollback point in a
new ignored successor copy. Production engine adoption remains open. Root
froze ../regexp-lookahead-experiment-contract.md and the independent 4,032
actual-Node observations before applying the repair.

When a high surrogate is followed by a possible second Unicode escape, Source
saved the input cursor after consuming that escape's prefix. If the following
value was not a valid low surrogate, rollback lost the prefix. The repair saves
the cursor before the trial prefix, allowing the complete following escape to
be parsed normally after a failed trial. It retains successful pair decoding,
prior legacy-mode separation and the existing named-capture grammar boundary.
No concrete pattern or code point is special-cased.

All three corpora are replayed against the immediate P20b predecessor:

| Corpus | Correct v-implies-Unicode equal / total | Fixed | Regressed |
|---|---:|---:|---:|
| Original | 117,314 / 119,218 | 0 | 0 |
| Legacy canonicalization | 111,800 / 111,818 | 2 | 0 |
| Speculative escapes | 4,014 / 4,032 | 532 | 0 |

The original corpus still has 1,904 differences; the legacy corpus still has
18. The new corpus's 18 remaining differences use legacy braced Unicode
escapes, whose existing candidate behavior differs from JavaScript. No result
is declared equivalent merely because the upstream suite passes. The separate
literal-flags diagnostic adapter records 358 fixes and eight regressions in
the new corpus, and two fixes without regression in the legacy corpus; its
incorrect v treatment remains unaccepted. Both adapters' complete counts and
samples are retained in p20d-lookahead-regexp-repair-result.json.

Upstream utf16 tests complete: 551 passed, zero failed, one ignored doctest.
Log: build/p20d-lookahead-upstream-tests.log. The prepare script verifies the
predecessor, reproduces the sole parser text change and checks all protected
boundaries. The original Source and predecessor remain unchanged.
MathJax Source acceptance stays 53/246; no concrete TokenMap/TeX/Common/SVG
or Folia gate is passed by this experiment.
