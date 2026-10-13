# RegExp legacy ignore-case profile

This extension implements the ECMAScript non-Unicode `i` flag for the UTF-16
code-unit regex profile used by the MathJax TeX scanners. `g` and `y` continue to
control the same `lastIndex` state; other flags remain explicitly unsupported.
This is not a claim of complete ECMAScript RegExp coverage.

SRELL's stock `icase` uses Unicode simple case-folding. That differs from
ECMAScript legacy `Canonicalize`, notably for non-ASCII characters that fold to
ASCII and for multi-code-unit uppercase mappings. The host therefore compiles
SRELL with a narrowly enabled policy hook, leaving default SRELL behavior
unchanged for other consumers. The hook routes literal, class, sequence,
backreference, and input matching through one generated equivalence relation.

The two policy instances must also have different C++ namespaces. Otherwise
SRELL's inline, non-template case-folding methods violate the one-definition
rule when `folia_math` and `folia_core` link into the same executable. Gate 22
exposed that integration defect although the standalone math regex tests passed.
`SRELL_NAMESPACE` keeps the math instance in `folia_math_legacy_srell`; ordinary
editor search retains the default `srell` Unicode policy. The joint search/math
regression observes Kelvin-sign and long-s matching in both engines.

`tools/math-port/regexp-ignore-case-oracle.cjs` derives the canonical
equivalence classes by applying Node 24.13.0 / Unicode 16.0 `String#toUpperCase`
to each individual UTF-16 code unit, preserving the original unit when the
uppercase result is not one unit or when a non-ASCII unit would map into ASCII.
It independently verifies every generated class member with Node's actual
non-`u` `/i` RegExp, locks the resulting table hash, and emits both the vendored
table and native differential fixture. For each of 1,141 nontrivial BMP
classes, the oracle runs a global `/i` expression against one string containing
all 65,536 UTF-16 code units in order. The native fixture compares the complete
match sequence, including code-unit positions, matched contents, each
`lastIndex`, and the terminal failure/reset. This gives positive and negative
membership evidence across the entire BMP without separately allocating a
regex match for every input unit. Global and sticky state traces and flag
reflection are also emitted from actual Node results, alongside boundary,
class, backreference, capture, and unsupported-flag cases.

Observed boundaries include `É` matching `é` and final sigma matching sigma;
Kelvin sign, long s, dotted/dotless I, and capital sharp s do not fold into
their ASCII or multi-code-unit counterparts under legacy `/i`. Astral input
continues to be processed as UTF-16 code units by the no-`u` matcher.

Regenerate and verify the source artifacts with:

```powershell
node tools/math-port/regexp-ignore-case-oracle.cjs --check
```
