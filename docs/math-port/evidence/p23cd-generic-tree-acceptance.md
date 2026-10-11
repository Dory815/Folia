# P23c/d Root generic tree local acceptance

This accepts the bounded generic Node/Wrapper bodies after the sole sparse
storage migration. It does not accept the MML consumer graph, NodeUtil, a whole
Source-file tally, TeX/Common/SVG, Direct2D or Folia. Full core compilation is
still pending while MML declarations/consumers migrate; the previous full-core
178,465-assert result is historical and is not a result for this working graph.

## Independent checks and review

Root selected all 64 generic cases from actual pinned Source111. Length, own/has
presence, Undefined/Null/borrowed node identity, all registered parents, return
values and exact errors match. The remaining 47 cases require actual NodeUtil.
Root then migrated only emitted C++ observer bindings in tree-oracle.cjs;
original executable Node/Wrapper computations and expected values remain the
source. Fifteen original cases now have 143 assertions, including three newly
checked exact null-array TypeError messages. Native resource and entry-point
checks add 13 assertions. This is 156 assertions in 18 tests.

The six-method Source64 baseline and the traversal baseline both pass. Nine
six-method and thirteen traversal/property/resource semantic mutants compile
and fail assertions. Final native factory-family checking adds three assertions
in one test and one compiling error-classification mutation, separately from
Source equivalence. Total local verification: 223 assertions in twenty tests;
23 effective compiling mutations. All runners verify unchanged working source
and expected-value bytes. Original sparse and tree oracles and freezes --check
reproduce unchanged expectations.

Root rejected the initial reuse-input-array mutation because live for-of plus
self-append did not terminate; it was not counted as a semantic assertion kill.
The replacement mutant retains the old destination array and is detected by
actual Source fingerprints. Root also rejected the initial red log because
exception::what() hid the stub name; explicit actual entry-point probes exposed
Unimplemented before dispatch. The final red compiled and exited 1 through
the real frozen bodies. Source64's earlier red had 64 assertions; traversal's
red ran 99 assertions before stubs ended some bodies. Neither was a build-error
substitute for real red.

Root reviewed Source ordering: current array lookup after virtual childIndex,
push/write/splice before parent receiver errors, getParent/setParent dispatch,
copy creation/property replacement before child-handle capture, live for-of
length/Get, kind-before-join capture, fixed join length and array-identity cycle
unwind. Source Empty returns are preserved. No dense array bridge, mirrored
parent, pointer conversion, runtime selector, or earlier behavior fallback exists.

Root caught and replaced a delivered unchecked downcast in copy. Native factory
products outside the current AbstractNode storage family now throw TypeError.
The focused native test uses a genuinely different template base family and
checks this boundary before undefined behavior. This is not arbitrary-JS-object
equivalence: JavaScript can assign a writable properties member on foreign
objects; native typed families must expose the actual storage subobject.
The initial test helper collided with Boost.UT's type variable and failed to
compile; it was renamed before the successful run, with no production change.

Root also caught inaccurate null-array error wording in the delivered traversal
and six-method setChildren body. Actual locked CJS/Node 24.13.0 emits
`object null is not iterable (cannot read property Symbol(Symbol.iterator))`.
Root corrected the three paths and added Source-produced exact assertions;
all thirteen traversal mutants were rerun with the final 156-assert fixture.
The agent reports describe pre-acceptance delivery; this Root report supersedes
their runtime-message adaptation note.

## Reproduction and raw evidence

Run freeze-sparse-node.cjs --check, sparse-child-oracle.cjs --check,
tree-oracle.cjs --check, sparse-node-mutations.cjs,
sparse-tree-mutations.cjs and tree-family-binding-test.cjs in tools/math-port.
These are closed C++23/module graphs with the repository Boost.UT module.

| Ignored raw artifact | SHA256 |
|---|---|
| sparse-tree-red/baseline-build.log | fa363b6fa9651a4e816657211f6636ed5232214498eaea7ae59c17976208651b |
| sparse-tree-red/baseline-test.log | 94646f9b860ffcb8f1a59f76f569c4b9394662d06cd2cb99321912207d3b31fe |
| sparse-node-mutations/baseline-test.log | 2d036b3511c3bb00cca6695fdf89d124742c84b239bd51a66caebbb2bc546d641 |
| sparse-node-mutations/results.json | 5f88d0192b84c478fdcfdd6f6ec70517032b50c322371a942e1a60b5b965d604 |
| sparse-tree-mutations/baseline-test.log | 93ec6baef0c5e520d237dd96533a180035b2558818b18672236d24e34c42988f |
| sparse-tree-mutations/results.json | 9b3b6ec80e40b1ad52d155950acd50f339e3f4b6223f3342bb9fb3fc03f73712 |

The raw paths are under build/math-port-*. Tracked mutation summaries are
p23c-generic-node-mutations.json and p23d-tree-traversal-mutations.json.

## Remaining acceptance

Actual MML Source299 and return32, NodeUtil Source47 plus its whole static-method
family, sparse parent/copy/delegate mutations, every MML implementation/observer
and final full core graph remain required. A passing generic tree is a
dependency, not evidence that the native mathematical rendering path is ready.
