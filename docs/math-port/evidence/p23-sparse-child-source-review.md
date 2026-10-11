# P23 Root original Node/NodeUtil observations

Root's architecture choice is `../p23-child-storage-design.md`: one typed sparse
array, factory-owned products and borrowed node identity. This evidence prepares
the upcoming concrete API/red tests and 31-file migration; it accepts no native
storage, NodeUtil or full parser/layout/backend behavior. Local Source-file
acceptance stays 54/246. Existing dense-node evidence does not establish holes.

`node tools/math-port/sparse-child-oracle.cjs --check` executes actual pinned
MathJax 4.1.3 CJS AbstractNode methods and NodeUtil.setChild/copyChildren. Source
is never patched and native code contributes no outputs. The CJS pins are:

* core/Tree/Node.js `94174004b89f19bf3c98e8eec5fabdd0bcbf1e13e0b740963fa69554ad8a12a7`
* input/tex/NodeUtil.js `4a55841fd22167d25f505e8f41adea072715ef024e00ea5ac6df3f199f2cb809`

The corpus has **111 cases**, including **27 actual Source TypeErrors**. It
records array length, own and inherited presence, tagged indexed values, exact
registered node identities and parent identities after normal return or error.
Cases include every ordered pair of hole/Undefined/Null/two node identities,
NodeUtil copy, AbstractNode set/copy/append/replace/remove/indexOf, growing writes,
superscript-first hole then subscript fill, copied hole becoming own Undefined,
live source growth/deletion, captured source handle after rebind, inherited
numeric slots and target capture before a parent setter rebinds childNodes.
The superscript-shaped observation exercises the actual NodeUtil operations;
it is not a claim to have executed the unported TeX parser or complete MML path.

Source snapshot SHA256:
`0a91ef40e88691fc5ae57f1406a635c4cab925d86650c1c1923fd07cb3c2e743`.
The initial uncommitted adapter mistakenly requested a named NodeUtil export
instead of the actual default export, generating 47 adapter errors alongside
27 real node errors. Root rejected that output, retained it in ignored
`build/p23-sparse-child-rejected-adapter.json`, bound the actual export and added
preconditions that both methods exist, all copyChildren calls reach Source and
every observed node identity is registered. Only the corrected reproduced
111-case/27-Source-error corpus is tracked. No failed adapter result passes a gate.

Concrete native declarations, compiled red tests, original full MML sparse cases,
all consumer/test migrations and mutation/core acceptance remain mandatory.
Luna implementation is not authorized by this Source-only evidence.
