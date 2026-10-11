# P20b isolated legacy casefold implementation

Status: implementation delivered for root review. This is an isolated `regress` 0.12.0 experiment only; no engine adoption or compatibility claim is made. Per packet, I did not build, run tests, execute the corpus/oracle, modify generated data, or touch production files.

## Changed files and anchors

| File | SHA-256 | Source behavior implemented |
|---|---|---|
| `build/math-port-regexp-casefold/src/unicode.rs` | `e4a9192ef5fcb116f5623394520e808c965b4298a0cd1fd6f41bd162864f308d` | `fold_code_point` now dispatches legacy mode to the root-frozen canonicalizer (`143–148`); `unfold_uppercase_char` enumerates all preimages from the same mapping (`281–295`); new `add_legacy_icase_code_points` forms source set + canonical image + all table preimages (`327–343`). |
| `build/math-port-regexp-casefold/src/parse.rs` | `901843e501c545b8b9c4a479797c3222650919b64f22ad6f3c35f9bf556a219b` | `make_bracket_class` now receives the Unicode-mode bit and chooses Unicode simple-fold closure or legacy canonical closure before its complement (`342–357`); ordinary bracket closure uses the same `u || v` mode while retaining `result.invert` (`856–866`); `\\d`, `\\s`, `\\w` and their complements pass `u || v` into the helper (`1608–1640`). |

The root-owned `legacy_case.rs` mapping payload is unchanged (SHA-256 `48ab41597e445b62e4c3e530a5fe919fae4cf2b1513f568cd2c36073dd9dc9dc`). It contains the contract's 1,166 nonidentity BMP UTF-16 mappings from Node 24.13.0 full uppercase, after the full-result-length and ASCII rules. Its `canonicalize(unit)` binary-searches that table and returns identity when absent (`legacy_case.rs:1172+`).

## Branch and path ledger

- Scalar matching and backreference folding use one operation: Unicode mode remains `fold(cu)`; legacy mode calls `legacy_case::canonicalize(cu)`. The previous candidate `TO_UPPERCASE` implementation was removed from this path so there is no second simple-uppercase route. This reaches `CharProperties::fold` / `InputIndexer::fold_equals` without changing those modules.
- Literal expansion continues through `expand_code_point` and `unfold_uppercase_char`. That inverse starts with the input, adds its canonical target, adds every mapped source whose target equals that canonical target, then sorts and deduplicates. Identity inputs, non-BMP scalar inputs, and table targets therefore share the same exact relation as runtime comparisons.
- Legacy set closure starts with the original `CodePointSet`, adds canonical targets of contained mapped sources to a canonical-image set, unions that image back into the result, then adds all mapping sources whose target is in that image. The mapping table is finite, so wide ranges and non-BMP values are retained as intervals rather than walked code point by code point. Targets are fixed points per the frozen data contract.
- `make_bracket_class` receives `unicode || unicode_sets`; `i` selects the existing Unicode fold closure for `u`/`v`, or the new canonical-image closure otherwise. Optional `\\D`, `\\S`, and `\\W` negation is applied afterward.
- Ordinary bracket parsing preserves `result.invert`; at closing `]`, it expands the positive source set with Unicode closure only for `u` or `v`, otherwise with legacy closure. This keeps range endpoints and mixed class members in the same mode-aware set operation.
- `ClassSet::node` for `/v` remains on the existing Unicode fold path. `\\p`/`\\P` property folding and its `/\\P/iu` complement order are unchanged. `src/parse.rs` also retains the two earlier root-prepared parser repairs; this packet's additions are only the mode argument and closure selection described above.

## Boundaries and validation

The implementation uses the private root-generated mapping table and generic `CodePointSet` operations. It does not recognize Kelvin, long s, Greek, a pattern spelling, or any other individual examples. The root's independent real-RegExp witnesses, unchanged 119,218-case replay, regression comparison and upstream test run remain pending and are the acceptance gate. Existing candidate differences in Unicode version/properties, `/v`, UTF-16 start behavior, stateful API semantics, and budgets are outside this bounded change and remain unresolved.
