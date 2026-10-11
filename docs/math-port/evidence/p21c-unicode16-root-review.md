# P21c root review — candidate Unicode data version alignment

This isolated data-only successor is not production engine, JS host, TokenMap,
TeX/Common/SVG or Folia acceptance. Local MathJax Source acceptance remains
53/246. All original Source/predecessor files remain byte-protected.

Root froze input identity and responsibility in regexp-unicode16-experiment-
contract.md before preparing a successor of P21b. The original gen-unicode
algorithm modules remain byte-exact. Root changes only main's input/output
paths and standalone output-only rustfmt calls. The generated tests are ignored
evidence and cannot overwrite any original suite or Node expectations.

Unicode's pinned final 16.0.0 UCD and Emoji 16.0 files supply data. Every
extracted UCD file is checked against the hash-pinned archive; both separate
emoji files are hash-pinned and checked against their copied generator inputs.
The manifest p21c-unicode16-generation.json records original/adapted generator,
unchanged algorithm modules, archive/input identities and generated hashes.
A separate fresh successor reproduces table and test hashes. The original
Unicode 17 table and every P21b parser/legacy-case algorithm remain protected.
Production adoption still requires the separate Unicode data license.

Root's four original actual-Node corpora total 266,556 cases. Correct adapter:

| Corpus | Cases | Equal | Remaining | Newly fixed | Regressions |
|---|---:|---:|---:|---:|---:|
| Original | 119,218 | 118,842 | 376 | 1,528 | 0 |
| Legacy | 111,818 | 111,800 | 18 | 0 | 0 |
| Lookahead | 4,032 | 4,032 | 0 | 0 | 0 |
| Unicode escape | 31,488 | 31,488 | 0 | 0 | 0 |

Both adapter modes remain recorded. The original corpus now has 20 remaining
pattern/flag groups, no constructor differences, and zero remaining cases in
its Unicode-version group. Surviving v empty-string sets, casefold set algebra,
emoji string matching and astral zero-width behavior remain open.

Root added **286,500** independent actual Node 24.13.0 / Unicode 16 / ICU 77.1
property observations. Inputs come from primary UCD aliases/ranges/simple
casefolds and Emoji sequence rows (range endpoints for range rows); expected constructor/match/range/lastIndex
come only from actual Node, never generated Rust tables or native output.
Source corpus SHA256:
03d1dd3cc23955f107d1a2265f04720497743bdb30b83d5ea804ba8adbb8094e.
It covers all Script/Script_Extensions aliases, all general categories,
binary/rejected property aliases, every C/S casefold mapping and all seven
string-property classes on the explicit Emoji strings and range endpoints.

Correct adapter: **286,226 equal / 274 different**, **352 newly fixed / 0
regressions**. All script and simple-casefold/Emoji families match in this
corpus. Of the 274 differences, 144 are WSpace alias constructor mismatches
and 130 are v+i property complement mismatches. These are retained in the full
34-pattern inventory, not described as Unicode table acceptance. The corpus
samples property membership boundaries; it does not exhaust every code point.

The complete unchanged locked/offline upstream utf16 suite with --no-fail-fast
returns exit 101: **495 pass / 56 fail / 1 ignored**. Three failures are the
already recorded legacy braced-escape conflicts. All 53 newly failing original
Unicode 17 fixture tests have independent actual-Node 16 counter-witnesses:
four constructor conflicts and 49 positive-membership conflicts. Root's tool
reads the unchanged original positive-input shapes (ranges, escaped strings
and scalar arrays) and records an actual Node conflict for each. The initial
observer assumed only range arrays and failed; root added exact support for
the two other Source fixture representations before producing evidence.
No failed execution or unsupported fixture is counted as a witness.

No original test was changed or waived; the upstream gate remains failed.
Generated version-16 tests are not substituted to create a passing claim.
A reduced mismatch count is insufficient for adoption. Evidence:
p21c-unicode16-regexp-repair-result.json, p21c-unicode16-data-replay-result.json,
p21c-unicode16-upstream-witnesses.json and both remaining-pattern inventories.
Logs/native results remain ignored under build. Root next freezes independent
v casefold/set-algebra responsibility; aliases, empty strings and JS UTF-16
state protocol are distinct required responsibilities.
