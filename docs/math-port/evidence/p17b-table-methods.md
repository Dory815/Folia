# P17b table inheritance dispatch evidence

The pinned MathJax 4.1.3 sources are `MmlNodes/mtable.ts` (SHA-256 `8c701c9e8e55565c7ea905bd2d0e23e8a433547c5ed11bfb868f74e3089a3bda`) and `MmlNodes/mtr.ts` (SHA-256 `ddf536d1e1a615d542052cd60db6440ad1698c0a13ff2e2307097d05c1b0ed2f`). Implementations are [mtable.cpp](../../../src/math/core/mml_tree/mml_nodes/mtable.cpp) and [mtr.cpp](../../../src/math/core/mml_tree/mml_nodes/mtr.cpp), with MathJax Apache-2.0 attribution; see `src/math/LICENSE-MathJax.txt`.

## Source-to-native method ledger

| Source method | Source lines | Native mapping |
| --- | ---: | --- |
| `MmlMtable.setInheritedAttributes` | `mtable.ts:106–125` | Renamed to `MmlMtable::inheritAttributes` to avoid the public C++ facade's eager source defaults. Runs the raw indent-attribute pre-pass, then calls qualified `AbstractMmlNode::inheritAttributes` with the unchanged raw four values. |
| `MmlMtable.setChildInheritedAttributes` | `mtable.ts:134–176` | Retains its row repair, display selection, additions spread, row alignment loop, and child inheritance. Raw tuple reads use `host::get`; its indexed `rowalign[1]` write uses `host::set`. |
| `MmlMtr.setChildInheritedAttributes` | `mtr.ts:76–114` | Retains cell repair, raw inherited record, row/column/break alignment setup, tuple mutation, vertical-alignment write, and child calls. Indexed record access and writes use `host::get`/`host::set`. |

`setInheritedAttributes` is the nonvirtual facade that forwards raw missing/null/scalar values unchanged to `inheritAttributes`. The Mtable override itself does not call `inheritanceDefaults`; only the qualified base body performs base semantics after this source pre-pass. The qualified call avoids redispatching to Mtable and preserves the source `super` call.

## Evaluation order and identity notes

`MmlMtable::inheritAttributes` captures each current `indentAttributes[index]` string for that for-of iteration while the shared names vector remains live. It reads `attributes[name]` once for the truthiness test. For a truthy value it captures the current `this.attributes` receiver, re-reads `attributes[name]`, then reads tuple slot `"1"` and invokes `setInherited`. It next re-reads `this.attributes` for `hasExplicit`; if true, it re-reads the receiver again for `unset`. All parameter reads use `host::get` on the original Property graph; nullish receivers therefore fail where source property access fails, with no record copy or coercion. Only after the full pre-pass does it call the qualified base `inheritAttributes` with the original values.

In `MmlMtable::setChildInheritedAttributes`, source-created additions remain a fresh ordered record. `addInheritedAttributes` produces the source-spread result; each row iteration captures the current `rowalign` tuple receiver before shifting the alignment queue. A falsey shifted value causes a fresh read of the current `attributes.rowalign[1]`. The indexed write is through `host::set` on the captured tuple, so it preserves shared tuple identity and exposes strict receiver errors rather than silently converting a scalar/null value into a record.

In `MmlMtr::setChildInheritedAttributes`, cell repair uses the captured row child list, then reads current row and break attributes at their source positions. The rowalign/columnalign/data-break-align additions are a fresh ordered record and the `addInheritedAttributes` result replaces the local attributes handle. Each iteration captures the current `columnalign` tuple before shifting; falsey shift fallback re-reads the current tuple and index 1, then `host::set` mutates the captured tuple. The source reads `this.kind` before shifting break alignment; the fallback reads `data-break-align[1]` after the shift. The final `data-vertical-align` assignment uses the same returned attributes record and stores a fresh two-slot array `[kind,value]`. Child inheritance receives this updated record handle. Scalar/null writes throw using the root host's strict assignment behavior.

The prior tree/row verification and TeX-class methods are untouched. The root owns base and host implementations, declarations, fixtures, generators, tests, build/mutation checks, and acceptance. This report records method mapping and identity/order adaptations only; no build or test was run for this packet.
