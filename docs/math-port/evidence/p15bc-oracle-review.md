# P15b/P15c oracle and contract review

Read-only review of the frozen actual-CJS TeXAtom/semantics oracle, native bindings, shared operator operation, and their contracts. No implementation, fixture, expected-data, interface, or build changes were made.

## Provenance and source agreement

`tools/math-port/atom-semantics-reference.cjs` hashes the pinned `TeXAtom.ts` (`bad114…f7028`) and `semantics.ts` (`e609df…b6e96`), verifies each matches the installed `@mathjax/src` source tree, loads the 4.1.3 package CJS classes, and asserts the effective source identity `TeXAtom.prototype.adjustTeXclass === Mo.prototype.adjustTeXclass`. The package lock pins `@mathjax/src` at 4.1.3 with an integrity hash. The frozen snapshot I initially reviewed contained 6,828 assertions / 63 cases / 924 raw class-state rows. Root subsequently expanded and regenerated it to 6,888 assertions / 74 cases.

The current `operator_class.cpp` algorithm is a mechanical receiver substitution for the accepted `MmlMo::adjustTeXclass` body: it uses the passed `MmlNode&` for the source `this` accesses and retains one `TreeCallGuard`. `MmlMo::adjustTeXclass` currently delegates to it; TeXAtom's declared virtual entry point is ready to delegate similarly. The helper does not downcast to Mo or TeXAtom. TeXAtom initialization also matches the source sequence in its phase hook: write ORD to the inherited backing field, call virtual `texClass()`, then virtual `setProperty("texClass", value)`. Annotation's native phase replaces the `properties` handle; its source class-field initializer similarly replaces the object after `super`.

The contract's explicit limitation on JS method-function identity is accurate. The source oracle checks that identity independently, while native expectations test shared receiver behavior and return identity through both class entry points. That is a defensible native adaptation given the present typed API, which carries no JS prototype-method objects.

## Original frozen-snapshot discrepancy: constructor callback traces

In the original frozen snapshot, JS `proxy(T)` instrumented setters for `prevClass` and `prevLevel`; its `attributes` setter only stored the value and did not log. The actual installed CJS constructor invokes those two instrumented accessors on the derived probe before `setChildren`, followed by TeXAtom's `tex.read` and `set/texClass` (the oracle additionally records `phase/...` inside `setChildren`). A separate read-only probe I wrote logged the `attributes` setter as `attributes.set`; that was extra instrumentation and was not an event in the original frozen expected trace.

At that snapshot, native `AbstractMmlNode::initializeMml` initialized previous-state fields directly and so could not produce the two expected setter events. Root has since changed initialization to issue source-ordered virtual `setPrevClass` / `setPrevLevel` writes and separately initialize the raw TeX class after generic initialization. Root also instrumented `attributes` assignment on both source and native sides consistently. Root reports the expanded oracle passes 6,888 assertions / 74 cases. Thus the original discrepancy has been dispositioned; this paragraph records why it was raised and clarifies the attributes instrumentation, rather than claiming the current frozen oracle still fails.

## Original frozen-snapshot coverage gaps, now dispositioned

1. The original `defaults_getters_class_identity` case directly checked distinct identity for TeXAtom versus `AbstractMmlBaseNode.defaults` and Annotation versus `MmlAnnotationXML.defaults`, but omitted `MmlSemantics.defaults !== AbstractMmlBaseNode.defaults` and `MmlAnnotationXML.defaults !== AbstractMmlNode.defaults`. Both are fresh spread objects in `semantics.ts:36–40, 77–84`. Root has since added both identity assertions.

2. The original logical-parent case exercised a real TeXAtom inferred row and checked the logical `Parent()` chain, but did not combine that chain with the shared BIN last-child walk. Root has added `real_shared_parent` cases for both receivers through semantics/TeXAtom trees, including presence/absence of an annotation and a trailing sibling. This addresses the witness gap for actual wrapper parent chains.

The remaining stated source observations are represented: TeXAtom's virtual constructor getter and `setProperty` ordering; child-first `setTeXclass(null)` with return discard, mutation/rebind/throw cases, and virtual adjust override; both receiver kinds across current/previous class and autoOP matrices; null/malformed parent and attribute failures; annotation's empty hook under normal/null-child/null-array states; annotation old-property identity and replacement; and logical Parent traversal through inferred row, TeXAtom, and semantics.

## Contract and implementation notes

The three classes' inheritance/default behavior is consistent with the pinned source: semantics has minimum-one arity and `notParent=true`; annotation-xml inherits the base default arity and has a genuinely empty child-inheritance hook; annotation inherits that hook and resets its properties object to `{isChars:true}` after the base constructor. The test-only source probes preserve subclass dispatch: `Dispatch.adjustTeXclass` is called virtually by TeXAtom, while the shared algorithm probes invoke the real upstream TeXAtom and Mo methods.

Root reports that it has resolved the constructor-trace discrepancy and both original witness gaps. This remains an evidence update only: I did not build, rerun the oracle, or accept the packet. No source, interface, test, or expected-data files were changed by this review.
