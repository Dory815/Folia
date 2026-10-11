# P21d — v-mode case-folding and class-set audit

Read-only audit of the ignored Unicode16 candidate. No source change or
replay was made for this report. The relevant observed boundaries are Node
`v24.13.0`, Unicode `16.0`, ICU `77.1`.

## Evidence and file identities

| File | SHA-256 | Responsibility examined |
|---|---|---|
| `build/math-port-regexp-unicode16-source/src/parse.rs` | `34a075eaf65c01f5c4f3fbe527018f7573cf3dc034072b419bcab82abcbd864a` | Class-set operands/operators and case-fold placement |
| `.../src/codepointset.rs` | `cb7b12546485c5af737f522d3dd976b6cfc047463f906ed4fec03bdb9869fee0` | Raw interval membership, inversion, intersection and removal |
| `.../src/unicode.rs` | `e4a9192ef5fcb116f5623394520e808c965b4298a0cd1fd6f41bd162864f308d` | Unicode fold closure and property result kind |
| `.../src/unicodetables.rs` | `475cb03589f0deb859b9a2ccfb34e3f33026a1e89500524024ce575a135e5daf` | Unicode16 fold ranges and word-fold table |
| `.../src/ir.rs` | `8742a4621573f6099ac704d7ac1be9f841676da62ade7c54099cac317c6bf0cc` | `Bracket` versus `StringSet` IR representation |
| `.../src/types.rs` | `2649c903fbf003731ebc6377163e2134d6c61405264359b6df0a90c4300a76a4` | `BracketContents { invert, cps }` |
| `.../src/emit.rs` | `52b2a9a2be8b4eb7b34713b84b15124f7349cb108eda1ec6b9d466be11c9e499` | IR lowering; no class-set folding |
| `.../src/matchers.rs` | `575b5c0cbbfbe97c528eaac64bfc768204e8565c5370ee5055f1f0a39465931b` | Runtime bracket membership and inversion |
| `.../src/optimizer.rs` | `693fa10913b24951bf67b9caf18a8592a7fc882710e077d09e3a672fca1c916b` | Bracket representation optimization |
| `.../src/pikevm.rs` | `084bd2c86a310896cdc36648b2379177e408c68a63e0f36d7b3a6a476912f9ba` | Runtime dispatch to bracket matcher |
| `.../src/classicalbacktrack.rs` | `e1dadeabf6e6090186b79d051b492bcb656f73e5f7dd80cfc91b773f1fc84360` | Alternate runtime dispatch to bracket matcher |
| `.../src/startpredicate.rs` | `5ec05872079d8cfae734bc110abd59bcad12c857efb80232dd22409f544e0d57` | Start-set derivation from bracket contents |

The original-corpus inventory is
`docs/math-port/evidence/p21c-regexp-remaining-patterns.json` (SHA-256
`110d5c087df9b815ac6cdf0073ce044d01a85354295021d8f0ade3140f673f7e`):
376 of 119,218 cases differ; it includes the concrete `vi` set-operation
witnesses below. The Unicode16 property replay is
`docs/math-port/evidence/p21c-unicode16-data-replay-result.json` (SHA-256
`47a0252911b0d4a66af3befa3c4a8ec21315c5c45c79525093ab6b6a400bb0e1`):
176 of 286,500 cases were fixed, none regressed, and 32,200 remain in the
literal-flags diagnostic adapter. The corresponding remaining-pattern report
`p21c-data-regexp-remaining-patterns.json` (SHA-256
`99acbd0f5cadca577af4ef617ede0daae8d435ed92ac237d1f1f1669b41085af`) narrows
the correct `v_implies_unicode` remainder to 274: 144 `WSpace` property-alias
constructor differences and 130 `vi` negated-property case-fold differences.
Those counts are distinct from the earlier original-corpus set-operation
witnesses; the 274 must not be attributed wholesale to folding.

## Parser ownership and operation order

For Unicode-set syntax, `ClassSet::node` at `parse.rs:75–93` is the shared
case-fold boundary. It calls `unicode::add_icase_code_points` on the completed
`codepoints` set at 75–80, then constructs `BracketContents` with the
outer-class `negate_set` bit at 82–85. If the result also contains strings,
the alternatives travel through `into_node(icase)` at 86–91 rather than
through the point-set operation.

The set algebra itself is performed before that call and on raw code points.
`union_operand` at 95–111 adds literal points, escapes, nested classes, and
their string alternatives. `intersect_operand` at 113–176 uses raw set
membership and interval intersection; `subtract_operand` at 178–223 removes
raw points/intervals and separately removes string alternatives. The parser
selects those operations in `consume_class_set_expression` at 1040–1175:
`&&` dispatches to intersection (1146–1160), and `--` dispatches to
subtraction (1162–1175). Thus any fold variants added by `ClassSet::node`
arrive after these decisions.

Nested negation takes another route. `consume_class_set_operand` reads `[^…]`
at 1192–1204, recursively builds the nested set, and immediately replaces its
raw code points with `inverted()` at 1200–1202 before the outer set algebra
continues. `\p` is returned as a raw `CharacterClassEscape` at 1281–1294;
`\P` is inverted at 1296–1305. Both can later flow through `ClassSet::node`
and its post-operation folding. At top level, the class parser calls
`.node(self.flags.icase, negate_set)` at 688–695.

`CodePointSet` supplies mathematical interval operations, not regex case
semantics: `inverted` complements the point universe at `codepointset.rs:250–275`,
`remove` subtracts sorted intervals at 277–313, and `intersect` computes
overlaps at 315–329. It has no `icase` input and no folding logic. The
relevant fold operation is `add_icase_code_points` in `unicode.rs:309–322`:
it folds intervals and adds all table-derived preimages. `fold_interval` and
`unfold_interval` are at 172–215 and 217–252. This is a closure expansion of
the input set, so applying it only after set algebra can add code points that
the preceding subtraction removed or that the preceding property membership
did not include.

Ordinary `u` property escapes are separate. `consume_atom_escape` handles
`\p`/`\P` at `parse.rs:1641–1671`; its comment at 1651–1653 says, “Per ES2024:
apply SimpleCaseFolding to the property set first,” and explicitly states for
`\P`: “complement before expansion.” The implementation at 1654–1659 folds
`cps.inverted()` for `\P` and folds `cps` for `\p`, producing a non-inverted
bracket. The Unicode-set parser uses the `ClassSetOperand` path instead. These
are not interchangeable branches.

## Node witnesses and the shared failure surface

I checked the following neighboring cases directly with Node 24.13.0; they
also appear in the P21c original-corpus evidence:

| Pattern / flags | Subject | Node result | Candidate result in the frozen inventory |
|---|---|---|---|
| `[\p{ASCII}--[a-z]]` / `vi` | `a` | no match | match |
| `[\p{ASCII}&&\p{Letter}]` / `vi` | `K` (U+212A) | no match | match |
| `\p{ASCII}` / `vi` | `K` (U+212A) | no match | match |
| `\P{Lowercase_Letter}` / `vi` | `a` | no match | match |
| `[a-z]` / `vi` | `K` (U+212A) | match | not in the mismatch inventory |

The first three point-set examples share the `ClassSet::node` post-operation
folding boundary: it widens Unicode-property and range results after the set
operations/property lookup have already determined membership. In subtraction,
that can re-add a folded equivalent after removal; in intersection, it can
expand a property intersection beyond the Node result; for a standalone
property it can widen the property set. The fourth is the negative-property
form of the same boundary: `\P` is inverted in the operand parser first, then
the complemented set is expanded at `ClassSet::node`, allowing a fold-equivalent
lowercase character back into the result. The 130 new-data residuals are this
`vi` `\P` family across property names and aliases.

The `[a-z]` neighbor is important: Node accepts Kelvin sign for this range in
`vi`, so globally disabling case folding for v-mode classes would be wrong.
Direct Node checks also distinguish the u branch: `\p{ASCII}` with `ui`
matches Kelvin sign and `\P{Lowercase_Letter}` with `ui` matches `a`, while
the corresponding `vi` checks reject them. That agrees with the source's
separate `consume_atom_escape` comment and cautions against changing its u-mode
ordering to repair the v-mode set path.

## Boundaries that are not implicated

The general interval primitives in `codepointset.rs` should not be altered to
encode ECMAScript case rules; the ordering decision belongs to parser/class
semantics. The ordinary `u` property branch at `parse.rs:1641–1665` and the
legacy-fold branch `add_legacy_icase_code_points` at `unicode.rs:324–340` have
distinct semantics and evidence; this report does not identify them as the
same defect.

String alternatives are a separate representation. `ClassSetAlternativeStrings::into_node`
at `parse.rs:256–265` produces `Node::StringSet` with an `icase` flag;
`ir::Node::StringSet` is defined separately from `Bracket` at `ir.rs:90–98`,
and `emit_string_set` expands each sequence in `emit.rs:130–165`. The remaining
`[\q{ab|a|}]` empty-string behavior and `\p{RGI_Emoji}` string-property
behavior therefore must not be folded into the point-set fix. `\B` reports
in the original inventory are word-boundary/indexing behavior, not class-set
folding.

The compiler also does not decide this set algebra. `emit.rs:294–306` lowers
`Node::Bracket` into an ASCII bitmap or stores its `BracketContents`; `matchers.rs:34–43`
checks `cps.contains(cp)` and applies `invert`. `pikevm.rs:361–366` and
`classicalbacktrack.rs:278–282, 351–355` dispatch bracket instructions to the
same matcher. `optimizer.rs:447–495` may replace a small bracket or swap its
stored set with its complement while toggling `invert`; that representation
optimization is separate from case-folding order. No runtime or emitter
special case is indicated by these witnesses.

This audit identifies where the mode-specific semantics diverge, not the
accepted repair algorithm. Root owns the subsequent semantic freeze and
independent Node corpus; no source, API, fixture, manifest, or build was
changed here.
