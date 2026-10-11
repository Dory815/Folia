# P23e Root MML sole interface freeze

Root has now written the chosen declarations, rather than merely proposing
them. Twenty declaration files use the sole NodeValue alias, and four existing
child-helper modules have typed real stubs for the next algorithm red gate.
All 89 production math declaration modules plus static family contracts compile
as a closed C++23 static library. No MML semantic test was executed by this
build, no stub is accepted as implemented, and the full core graph still needs
the implementation and observer migration. There is no old/dense runtime path.

The controlling architecture is p23b-node-binding-decision.md and its Source
call-value audit. All Parent/core/coreMO/setTeXclass overrides are tagged;
appendChild/replaceChild follow the same generic value type;
getPrevClass/updateTeXclass/inheritAttributesFrom consume NodeValue.
Every notParent override returns Property (Boolean/Missing/Null). MmlMo and
TeXAtom adjustTeXclass and the shared free function use the same tagged input
and result. Root additionally chose NodeValue for Mo.coreText's argument: its
default body may explicitly project falsey values to empty text, preserving
the tag through virtual dispatch up to that source guard.

Source-filtered firstNonSpace/lastNonSpace and mError keep MmlNode* with nullptr
explicitly meaning Source Null, never Undefined. selected/coreParent and fresh
factory/copy/fakeNode returns stay pointers under the existing owned-product
factory contract. Fenced open/close have the Source Node|Null field domain.
These limited projections do not apply to direct indexed child Get or parent.

Required-child helpers take an explicit member name at the original dereference
point; no default hides it. moChild/tableSlot/multiscriptSlot return NodeValue.
Table creation and multiscript insertion receive NodeValue and must preserve
array mutation before the later parent write. Their frozen stubs are explicit
Unimplemented exceptions. Algorithms may not invoke a forgiving pointer helper,
insert an implicit pointer/bool conversion, or globally convert holes to Null.

Root owns declarations, helpers, fixtures, manifests, builds and mutations.
No Luna implementation authorization follows from this declaration build.
Before dispatch Root must bind original Source299/return32 to the actual native
registry, obtain a compiling real semantic red, freeze helper/member/order
contracts and precise file ownership, then authorize algorithm bodies. Root
must inspect actual Source-driven failures and all affected overrides/observer
call sites before whole-graph green acceptance.

The initial one-time script counted 21 declaration files rather than the actual
20. That inventory assertion was corrected and writes were moved after the
inventory check. It was not red/acceptance evidence. The read-only Luna audit
also had an incorrect mfenced source hash; Root found it and requested complete
hash verification/correction. Its old native header hashes are historical
pre-freeze fingerprints, not hashes of the current declarations.

Reproduce: freeze-mml-declarations.cjs --check and mml-declaration-build.cjs.
Ignored build/math-port-mml-declarations/baseline-build.log SHA256:
3b487b08b4d5a345247325781be42823923d8b77e9cc8737febaecfcf1ba59b5.
compile-result.json SHA256:
32ca6f656b834a3afbc36750bd6a55cf24d37acd0c7760141da5403284077b8b.
The result explicitly records semantic_tests_executed=false and unchanged bytes.
