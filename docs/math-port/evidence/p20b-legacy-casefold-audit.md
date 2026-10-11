# P20b: Legacy `ignoreCase` canonicalization audit

Read-only audit of the pinned `regress` candidate and the existing Node v24.13.0 differential/repair experiment. No source, expectation, corpus, or dependency was changed. This report proposes a general algorithm direction; it does not implement or accept it. `src/math` currently has no production regexp matcher; “candidate” below means the Rust `regress` engine called by the ignored probe harness, not a C++ production implementation.

## Evidence pins

The ignored candidate is `regress` commit `771e6c8fa1e472ef86c45b5206b37fa4e4494e25` (the manifest `.reference/regress/Cargo.toml` declares package version 0.12.0; this checkout has no tag at `HEAD`). The relevant original source hashes are:

| Source | SHA-256 |
|---|---|
| `.reference/regress/src/unicode.rs` | `df896bd48e36ba4d08812bc68a378b8e1ad32a069ad67f5cf68bcc2d6ba26e12` |
| `.reference/regress/src/parse.rs` | `591243258a598d58e9ed9c4998bff3d75004ceb5dc15bb40e2f35b9c2e813833` |
| `.reference/regress/src/matchers.rs` | `575b5c0cbbfbe97c528eaac64bfc768204e8565c5370ee5055f1f0a39465931b` |
| `.reference/regress/src/indexing.rs` | `8df75d23395147e16a06112aba619d8305eba6a0455046c2fe3291ed303ab5f4` |
| `.reference/regress/src/literal.rs` | `549e45cf31d07fc3db8af3bc28f0ba15a4fa6150dc25e0607e5a28d108072f99` |
| `.reference/regress/src/optimizer.rs` | `693fa10913b24951bf67b9caf18a8592a7fc882710e077d09e3a672fca1c916b` |
| `.reference/regress/src/scm.rs` | `b44e8ee83eadda29309977bd3508157e776164d6ba9d02854a9ff097e41f49b9` |
| `.reference/regress/src/pikevm.rs` | `084bd2c86a310896cdc36648b2379177e408c68a63e0f36d7b3a6a476912f9ba` |
| `.reference/regress/src/emit.rs` | `52b2a9a2be8b4eb7b34713b84b15124f7349cb108eda1ec6b9d466be11c9e499` |
| `.reference/regress/src/unicodetables.rs` | `e2f63d08fbe2f0b523986ad29dd87e5e9c24a3e4389632f043b2e1185d859849` |
| `.reference/regress/gen-unicode/src/uppercase.rs` | `a71d813afbf0bb5eed76fb4b171d788dc4f5d7c58d1427e7fc856b6769874b2b` |
| `.reference/regress/gen-unicode/src/main.rs` | `f420ac195dc831b3aadf9ba65cbcac5047d6974b8f07c1cdd617ceb7fff20bd0` |

The generated differential corpus has source hash `5b03d68995413da826ebb481d5e8057fa1f248eaf240913ffeacc7087e1d1195`, recorded in `build/math-port-regexp-candidate/report.json` (file SHA-256 `ac48697a8d14c6567821120f434cb4d4371945ddb7f9155a4d80dde1df2cd560`). The root-owned parser-repair summary is `docs/math-port/evidence/p19c-regexp-parser-repair-result.json` (SHA-256 `40feb1045af849054d93b13ae15dfff792855d26631b3d915fb964c89d623a90`).

The repair experiment changes only `parse.rs` in an ignored build copy (new `/v` reserved-punctuation check and Unicode escape/surrogate handling). It does not touch `unicode.rs`, case folding, matching, or optimizer behavior. It reports 119,218 comparisons; with `v` normalized to imply Unicode, 117,254 match and 1,964 differ. These counts describe the complete current experiment, not a casefold-only benchmark.

## Current candidate behavior

`unicode::fold_code_point` (`unicode.rs:140–148`) selects `fold()` when Unicode mode is on and `uppercase()` otherwise. The legacy path is uppercase based, but it uses a generated **simple** uppercase mapping and lacks both ECMAScript legacy restrictions: full uppercase results whose UTF-16 length is not one must leave the input unchanged, and a non-ASCII input whose single-unit uppercase is ASCII must also remain unchanged. The same omissions are present when inverting that mapping: `unfold_uppercase_char` (`unicode.rs:303–323`) adds preimages of the simple-uppercase target without checking either restriction. `expand_code_point` (`unicode.rs:325–334`) uses that inverse for legacy literals.

This matters for the two requested case families:

| Pattern/text | Node v24.13.0 legacy `i` | Candidate legacy `i` | Unicode `iu` |
|---|---:|---:|---:|
| `k` against `K` (Kelvin sign) | no match | no match for literal; class `[k]` incorrectly matches | match |
| `s` against `ſ` (long s) | no match | match for literal and class | match |
| `[a-z]` against either `K` or `ſ` | no match | both incorrectly included | both included |
| `[^a-z]` against either `K` or `ſ` | match | both incorrectly excluded | neither matches |

The JS spec's legacy `Canonicalize` behavior is the key distinction: it computes the full Unicode uppercase string first, preserves the source if that string's UTF-16 length is not one, and then preserves a non-ASCII source whose one-unit uppercase is ASCII. Thus neither Kelvin sign nor long s collapses into the ASCII `K`/`S` equivalence class in legacy mode. Unicode-aware ignore-case uses Unicode simple folding and does include these counterparts. The Kelvin family exposes a class-expansion error even though the current literal uppercase path happens to leave `K` distinct from `k`; long s exposes the missing ASCII guard in both literal and set paths.

There is a separate full-uppercase counterexample. Node v24.13.0 reports `/ᾀ/i.test('ᾈ') === false`; JavaScript full uppercase of `ᾀ` is `ἈΙ` (UTF-16 length 2), so `Canonicalize` must preserve `ᾀ`. The candidate's generated UnicodeData *simple* uppercase mapping maps this character to a single code point and its literal matcher returns a match. This shows that replacing only the ASCII guard would still be incomplete.

I ran a read-only, 216-case probe against the existing candidate executable and Node v24.13.0, with 18 patterns (`k`, `K`, `K`, `[k]`, `[^k]`, `[a-z]`, `[^a-z]`, `s`, `S`, `ſ`, `[s]`, `[^s]`, `[A-Z]`, `[^A-Z]`, `\w`, `\W`, `\b`, `\B`), six single-character inputs (`k`, `K`, `K`, `s`, `S`, `ſ`) and flags `i`/`iu`. It found 20 mismatches, all legacy `i` mismatches. The direct Node outputs confirm `/[k]/i` does not match `K`, `/[s]/i` does not match `ſ`, `/[a-z]/i` excludes both, while `/[a-z]/iu` includes both. The generated 119,218-case corpus also includes `Kſİı` as text; it reports candidate-vs-Node legacy differences for `\w+`, `\W+`, `[a-z]`, `[^a-z]`, and `[a-z-]` under `i`/`ims` for each start position. These are concrete candidate observations; the ad hoc 216-case probe was not added to the frozen corpus.

## Exact implementation paths

**Character comparison and backreferences.** `CharProperties::fold` delegates to `unicode::fold_code_point` for UTF-16 `u32` elements (`matchers.rs:69–78`). `InputIndexer::fold` and `fold_equals` use the same path (`indexing.rs:60–70`). Case-insensitive backreferences compare their captured/input elements with `fold_equals` (`matchers.rs:91–110`). Correcting the scalar canonicalizer should therefore repair direct comparison/backreference semantics in this path, provided every user of `fold_code_point` receives the correct Unicode-mode bit.

**Literal lowering and opcodes.** The UTF-16 parser's `char_node` returns a literal `Node::Char` when `i` is absent, otherwise it calls `unicode::expand_code_point(c, icase, unicode)` and emits `Node::CharSet` for a multi-character equivalence set (`parse.rs:464–474`; `unicode.rs:325–334`). In the UTF-16 build the byte-oriented `literal.rs` lowering is compiled out; it has a separate legacy/Unicode unfold split (`literal.rs:41–79`) and must not become a divergent second implementation if that build path is used. `emit.rs:408–416` lowers `Node::CharSet` to `Insn::CharSet`. The Pike VM checks a `CharSet` by direct membership (`pikevm.rs:129–132`); classical backtracking has corresponding `CharSet` dispatch (`classicalbacktrack.rs:301–303`, `366–368`). These opcodes carry a precomputed set, so their semantics come from parser expansion rather than runtime `i` flags.

**Classes, ranges and complements.** `make_bracket_class` expands the positive set using `unicode::add_icase_code_points` when `i` is set and only then inverts for `\D`, `\S`, or `\W` (`parse.rs:334–351`; callers at `1590–1620`). For bracket syntax, the parser collects code points/ranges in `result.cps`, preserves a separate `result.invert`, and expands `result.cps` at the closing bracket (`parse.rs:833–852`; ranges are inserted at `875–888`). This is the right broad order for complement semantics—close the positive set first, then apply complement—but the closure currently uses Unicode `FOLDS` and is not mode-aware. The `/v` `ClassSet::node` similarly folds before building the class node (`parse.rs:60–93`); it is a Unicode-sets path and should retain its separate Unicode simple-fold rules.

Unicode property escapes are already an explicit separate path: `\p`/`\P` are parsed only in Unicode or Unicode-sets mode, and on `i` the code applies simple-fold closure; for `\P`, the positive property is complemented before closure as documented in the source comment (`parse.rs:1623–1646`). A legacy-only algorithm change must not accidentally replace that Unicode algorithm.

**Matching opcodes and optimization.** `Insn::Bracket` uses `CharProperties::bracket`, which checks `bc.cps` membership then applies `bc.invert` (`matchers.rs:34–43`; `scm.rs:45–57`; `pikevm.rs:361–368`). `WordBoundary` stores a Unicode-ignore-case bit in the IR (`ir.rs:71–72`), while the parser's `\w`/`\W` currently route through `make_bracket_class` (`parse.rs:1614–1620`); that helper receives only `icase`, not `unicode`, so it applies full Unicode fold closure even for legacy `i`. This explains the `\w`/`\W` legacy errors in the differential corpus, separate from `\b` runtime handling.

After parser-side expansion, optimizer `try_reduce_bracket` converts a non-inverted set of at most four code points into `Node::CharSet` (`optimizer.rs:447–474`); `simplify_brackets` may also toggle the stored set and complement when a complemented interval representation is smaller (`optimizer.rs:476–494`). Emit and start-predicate construction can further specialize classes (`emit.rs:28–42, 294–301`; `startpredicate.rs:118–210`). If the correction remains a compile-time exact set closure, the existing `CharSet`/`Bracket` opcodes can remain, but the optimizer must preserve parity for both reduced and unreduced classes, and both bitmap and general-bracket start predicates need witness coverage. A runtime fold opcode is a different design: it would require carrying mode/flags through `Insn` and updating both VM dispatch paths, backtracking optimizations and start predicates.

## General algorithm direction (not implemented)

Use two explicitly distinct equivalence operations rather than one generic case-fold closure:

1. Unicode and Unicode-sets modes retain the existing simple Unicode fold, with the already explicit spec-specific property-complement behavior.
2. Legacy mode defines `LegacyCanonicalize(cp)` per ECMAScript: compute the **full Unicode default uppercase string** of the code point; if its UTF-16 length is not exactly one, return the original code point; otherwise take that single UTF-16 code unit, and if the original is non-ASCII while the result is ASCII, return the original; else return the resulting code unit. This handles Kelvin/long-s, multi-character uppercase mappings, and the wider non-ASCII-to-ASCII class without naming characters in implementation code. Primary normative source: [ECMA-262 2025, Runtime Semantics: Canonicalize](https://tc39.es/ecma262/2025/multipage/text-processing.html#sec-runtime-semantics-canonicalize-ch).
3. Make literal equivalence sets and class/range closure derive from that exact relation. For positive class set `S`, the semantic input set is `{ x | LegacyCanonicalize(x) ∈ { LegacyCanonicalize(s) | s ∈ S } }`; apply an enclosing negation only after forming the positive closure. A range should be transformed as a whole using the legacy map's interval/range data, not special-cased by endpoint characters. Ensure word-class rules remain exact: legacy `\w` stays ASCII-only; the extra non-ASCII characters that fold to ASCII word characters belong only to Unicode ignore-case `\w`/word-boundary behavior.

This direction is an inference from the candidate source and Node behavior. It needs independently frozen Node witnesses for direct literals, both case-family directions, singleton/range classes, complements, `\w`/`\W`, word boundaries, backreferences, optimized and non-optimized opcode paths, representative non-ASCII-to-ASCII mappings, and multi-unit full uppercase results before any adoption. The data provenance confirms the gap: `gen-unicode/src/uppercase.rs:7–15` reads `simple_uppercase_mapping` only (SHA-256 above), while `gen-unicode/src/main.rs:24` pins its generator input path to `/tmp/ucd-17.0.0`. The generated `TO_UPPERCASE` table therefore is neither full uppercase nor Node 24.13's Unicode 16 data. The tables also contain `FOLDS` and `TO_UPPERCASE` ranges (`unicodetables.rs:13199`, `25392`), but neither table alone implements legacy `Canonicalize`.

Likely candidate files to change if root chooses this repair: `src/unicode.rs` for canonicalize/inverse and set closure; `src/parse.rs` for literal/class and `\w`/`\W` call-site mode selection; then tests or validation at `src/optimizer.rs`, `src/startpredicate.rs`, `src/emit.rs`, `src/pikevm.rs`, and `src/classicalbacktrack.rs` to prove optimized/fallback opcode equivalence. `src/matchers.rs` and `src/indexing.rs` already centralize scalar case comparison, but their callers and flags need audit. `src/literal.rs` is a separate byte-engine path and should share the same rule if that backend remains in scope. This list is impact analysis, not file-change authorization.

## Limits

The `regress` crate and its tables are only a candidate, not an accepted ECMAScript engine. This audit does not change the matcher, regenerate frozen expectations, alter configuration scope, or resolve the broader Unicode/version and syntax differences. Root owns the repair experiment and any decision on adoption; V8/native full-engine investigation remains unresolved.
