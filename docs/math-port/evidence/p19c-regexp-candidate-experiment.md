# P19c native regexp feasibility experiment — not accepted

Root reran `node tools/math-port/regexp-candidate-probe.cjs` against the
unchanged regress 0.12.0 checkout at
771e6c8fa1e472ef86c45b5206b37fa4e4494e25. The harness verifies five candidate
file hashes and the actual MathJax BaseConfiguration CJS hash before executing.
Node v24.13.0 / Unicode 16.0 / ICU 77.1 supplies expectations independently;
the Rust process receives only raw UTF-16 pattern/input, flags and start.
No expected outcome is given to the candidate. Generated large input/results
stay in ignored build/math-port-regexp-candidate.

The 119,218 distinct cases include the four actual base default patterns,
additional flag combinations on those patterns, targeted syntax/captures/
lookaround/backreferences, surrogate inputs and starts, all 256 subsets of
dgimsuvy, invalid flags and changed Unicode assignments. Single-exec captures
and named ranges, constructor outcome and numeric lastIndex are compared.
Node's d flag is added for capture observation only after the original flags
have independently passed constructor validation. Object-key order is ignored;
capture and named-group array order is preserved.

| Experimental adapter | Equal | Different |
| --- | ---: | ---: |
| Candidate flags literally | 112,371 | 6,847 |
| Also set unicode when v is present | 116,871 | 2,347 |

Both configurations validate JS flags before compilation, ingest legacy
patterns as code units and choose find_from_ucs2 for legacy input. Unicode
patterns decode pairs while retaining lone surrogates, and use find_from_utf16.
The low-surrogate start adjustment is an experimental adapter hypothesis;
zero-width assertion differences inside pairs remain unresolved. These results
measure the candidate plus this adapter, not the crate in isolation.

The normalized-v differences comprise 333 added-v variants of initialDigit,
1,894 targeted syntax/capture/Unicode cases and 120 Unicode-version cases.
The original unmodified four default regexps passed these stimuli; the 333
cases must not be reported as failures of their original default flags.

Representative boundaries independently examined by root:

- `[0-9.,]` with v is valid in Node but rejected by the candidate's
  consume_class_set_character: it treats two different reserved punctuation
  characters as a forbidden doubled punctuator. No source repair was applied.
- Legacy case-insensitive word/letter classes differ for Kelvin sign and long
  s. Unicode case-fold expansion cannot substitute for legacy canonicalization.
- Escaped surrogate pairs are combined by candidate parse.rs even in legacy
  mode; raw literal pattern ingestion alone does not fix this.
- v set subtraction/complement case folding, empty class strings and RGI_Emoji
  string matches differ. A few successful ordinary character classes do not
  establish the v feature.
- Candidate Unicode 17 data accepts newer scripts/assignments absent from the
  actual Unicode 16 oracle. Dataset identity is a separate acceptance boundary.
- Zero-width `\B` inside an astral pair needs explicit reference investigation;
  a generic pair alignment rule is not yet justified for every assertion.

This harness does not implement repeated exec/test, String.match/replace/split,
callback replacements, full JS array shapes, budgets/cancellation or a native
C ABI. Constructor diagnostics compare error category only. No production
dependency, host API, subset restriction or fallback is selected. The candidate
cannot be adopted from these results. Full dynamic regexp behavior remains
required because the enabled setoptions package can construct patterns from
TeX input.

The separate p19c-regexp-callsite-audit JSON is a source inventory, not an
acceptance ledger. Root verified all 246 recorded file hashes and inspected
TokenMap.test, ParseMethods' live String.match, FindTeX's three lastIndex writes,
SetOptions' constructor path and mhchem's callback replacement. The alias
inference is approximate; the external mhchem parser is outside this TS
inventory. The small experiment-result JSON records reproducible case identity,
runtime, pinned files and aggregate failures.
