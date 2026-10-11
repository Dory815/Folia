# P15a multiscript contract review

Read-only comparison of the root-frozen contract and native host/declarations against the complete pinned MathJax source. This report does not change expected behavior or authorize implementation.

## Source identity and member inventory

Pinned file: `.reference/MathJax-src/ts/core/MmlTree/MmlNodes/mmultiscripts.ts`  
SHA-256: `bb8d8d2180ef483b86b0b1570b919ba424ed54db1f30ef6e0e697cfab0bf5cf7`  
MathJax 4.1.3, Apache-2.0 header at lines 1–16.

The 13 class members divide as the contract states: three fresh `defaults` object initializers and six `kind`/`arity` accessors (lines 37–54, 154–169, 198–213), plus four executable methods: `MmlMmultiscripts.setChildInheritedAttributes` (65–105), `MmlMmultiscripts.verifyChildren` (112–142), `MmlMprescripts.verifyTree` (177–186), and `MmlNone.verifyTree` (221–230). The binding file supplies the nine declarations/static members; the four method definitions are the bounded implementation unit.

## Contract correspondence

The main ordering obligations in `docs/math-port/multiscript-contracts.md` match the pinned source:

* `setChildInheritedAttributes` first calls the base child's `setInheritedAttributes` with the original four inputs (71–76), then performs a live loop whose condition and child reads both use the current `this.childNodes` (78–79). It changes `prescripts` only at the first marker, inserts a zero-argument `none` before an even-positioned first marker, assigns that inserted node's parent only after splice, and increments local `i` so the inserted marker isn't processed as a script (80–89). Other children receive `attributes`, `false`, JavaScript `level + 1`, and raw `prime || (n % 2 === 0)`, then increment `n` (90–94). Final parity is recomputed from the then-current array; a created `none` is passed through virtual `appendChild`, then the last child from the current array receives inherited attributes with the original `prime` (96–104).
* `verifyChildren` reads `options['fixMmultiscripts']` once before the live indexed scan (112–116). The duplicate-marker message evaluates `child.kind` before `this.kind` (117–123). Even first-marker and final parity diagnostics use the captured `fix`, and the qualified superclass call is last (125–142). A returning `mError` permits later work; a thrown one interrupts it.
* Both marker classes call the base `verifyTree(options)` before checking their parent, and null options do not suppress the later parent check (177–186, 221–230). Their arity is zero, so the inherited exact-arity check is distinct from the `mmultiscripts` arity of one, which is a minimum base-child count, not a fixed count (source `MmlNode.ts:906–927`).

The frozen native representation matches the described supported boundary: `Children` is an identity-bearing shared vector; `multiscriptChildren` captures the current handle, `multiscriptSlot` exposes a nullable raw pointer, and `multiscriptInsert` mutates that handle with positive-index splice semantics without setting the parent (`multiscript_host.ixx`). `MmlMmultiscripts::arity()==1` and marker/none arity zero appear in `mmultiscripts_bindings.cpp`; the base verifier uses the same minimum-one predicate (`mml_node.cpp:473–479`). `multiscriptCreate` retains a possible null factory result for the source's later operation, and `multiscriptErrorOptions` can carry absent options as null `ErrorOptions`.

## Two details to make explicit in the implementation ledger

1. The source marker check is against the **actual `parent` field**, not the logical `Parent` accessor. `Node.ts:164–166` declares `parent` as a plain stored edge; `MmlNode.ts:572–580` defines `Parent` as a traversal that skips `notParent` nodes. These are observably different when a marker/none is directly under an inferred row inside an `mmultiscripts`: source checks and rejects that direct parent; using native `Parent()` could skip the inferred row and accept the logical ancestor. The contract says “current parent”; record that as direct `parent` when freezing the call mapping, not the logical accessor.

2. Preserving null through `multiscriptErrorOptions` also preserves the subsequent **partial `mError` failure** for invalid-parent marker verification. Source `MmlNode.mError` first checks whether the current parent is already `merror` (992–994); otherwise it creates `merror` and writes `data-mjx-message` (995–996), then evaluates `options.fullErrors || short` (997). With null options, that options read throws after the error node has been created and annotated but before the child replacement at 1003/1008. The contract correctly forbids replacing null with `{}`, but its nullable-parent-verification obligations should include this partial state and its parent-already-`merror` early return, so a port does not silently convert it into either a completed error tree or an earlier failure. Native `AbstractMmlNode::mError` follows this order at `mml_node.cpp:524–535` and `ErrorOptions::get` throws for null in `mml_node.ixx:17–26`.

These are source-derived clarifications, not proposed behavioral changes. The rest of the reviewed host API and contracts are consistent with the inspected source; I made no source, interface, test, oracle, or build changes and make no acceptance claim.
