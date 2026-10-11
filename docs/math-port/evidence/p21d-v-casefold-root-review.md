# P21d root review — v point-set operand folding

Locally verified ignored candidate repair; no engine, native JS host, TokenMap,
MathJax file or Folia route is accepted. MathJax Source count remains 53/246.
Root froze typed property metadata, file responsibilities and independent
actual-Node expectations in 2d87b13 before Luna changed the two permitted files.

Root reviewed every changed branch against the frozen responsibility:
literal/range closure precedes algebra; positive d/s/w closure precedes
complement; ordinary Unicode properties close before v complement; intrinsic
ASCII/Any remain their catalog-defined ranges. Removing final whole-point
closure prevents removed points from being reintroduced. Ordinary u/legacy
paths, string alternatives, tables, flags/indices adapters and matching stay
outside the packet. No full-pattern/text special case was added.

Reproducible tracked patches reconstruct both files from the protected P21c
copy. A fresh copy verifies normalized-LF identities and every other copied
file byte-for-byte. Original `.reference/regress` and predecessor are retained.
Parser LF SHA256 is db9a0588975f072671dfecbe309b156bb480dff6bb842e495b5c6b5a42c3e54a.
Unicode LF SHA256 is 27a4c473cbbf7990d55f2a8aedc06dc845d0bd165b87b3c3ea6370eaf67b3892;
its agent-edit raw hash is ead02aef680de46559032fc6b19f926bb431ad5c87e6b774899a530debcac62b.
The first prepare attempt compared an LF file to the raw Unicode pin and failed;
root separated raw replay identities from normalized patch-reconstruction pins.
That failed check is not counted as reproducibility.

Actual Node 24.13.0 / Unicode16 / ICU77.1 is the sole behavior oracle. Root's
six-corpus replay retains both adapters; the correct `v_implies_unicode` results:

| Corpus | Cases | Equal | Remaining | Newly fixed | Regressions |
|---|---:|---:|---:|---:|---:|
| Original | 119,218 | 118,925 | 293 | 83 | 0 |
| Legacy | 111,818 | 111,800 | 18 | 0 | 0 |
| Lookahead | 4,032 | 4,032 | 0 | 0 | 0 |
| Unicode escape | 31,488 | 31,488 | 0 | 0 | 0 |
| Unicode16 properties | 286,500 | 286,356 | 144 | 130 | 0 |
| v folding/algebra | 227,712 | 227,684 | 28 | 7,669 | 0 |

Every remaining case is inventoried by pattern/flag, constructor, match and
lastIndex disagreement in p21d-v-casefold-remaining.json. The original 293 are
250 empty-string q alternatives, 28 astral non-word-boundary observations and
15 RGI_Emoji matches. The legacy 18 and property WSpace alias 144 remain.
The new 28 are exclusively ui ordinary-class controls of `[\W&&\W]` and
`[\W\W]`; the v/vi point-algebra observations all agree. These ordinary-u
failures remain required engine work rather than being waived or relabeled green.

Independent semantic mutants exposed missing bare-singleton intersection and
later union-range coverage in the initial corpus. Root did not change the
implementation or expected outcomes to kill them: it generated more actual
Node observations for generic operand/range syntax. The original 227,712-case
corpus stays immutable. Supplemental 72,768 observations / 1,516 patterns have
SHA256 13f819f34f2c4a9c828fc166beb30b9a3a2727b4b763c09c20c20f66a98f5736.
The 101,232-case v/vi mutation baseline (all original selected v/vi/offset-zero
cases plus all supplemental cases) has zero differences. All **11** final
semantic mutants compile, complete normally without native panic, and differ
from actual Node. Surviving first attempts are coverage failures, not passes.

The unchanged complete upstream utf16 suite remains exit 101:
**495 passed / 56 failed / 1 ignored**. It retains the prior three legacy
braced-escape conflicts and 53 Unicode17 fixtures already counter-witnessed
against actual Node16 in P21c. No test is changed, replaced or waived. This
packet is insufficient to adopt the engine; remaining host/profile/data/license
and whole-TeX/Common/SVG/backend/Folia gates stay open.

Evidence: p21d-v-casefold-{replay-result,remaining,mutations,operand-source}.json.
Logs/results are ignored under build/p21d-v-casefold-*.log and their named
source-harness/mutation directories. P22 parser and TokenMap reports remain
read-only dependency audits, not a constructor/storage/API freeze.
