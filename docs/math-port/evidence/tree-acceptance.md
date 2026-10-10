# P7 root acceptance: generic Node and Wrapper

Status: Node.ts and Wrapper.ts are accepted locally under revision 3 of
`../tree-contracts.md`. The complete native TeX/MML, fonts, layout, scene, SVG
and Folia replacement remain unaccepted. No application behavior changes.

Root froze interface storage, lifecycle, per-file responsibilities and actual-JS
expectations before dispatch. The node and wrapper agents owned two disjoint
implementation/ledger pairs; a third agent audited concrete usage and reviewed
the frozen contracts. Root owned declarations, fixtures, CMake, scope, builds,
mutations and acceptance. Every executable member and field is mapped in
`p7-node.md` and `p7-wrapper.md`; E1 and E10 record source dependencies and
required future MML/Common specializations.

## Architecture and source review

Node<Product> holds one factory reference, parent and replaceable child-array
handle. AbstractNode<Product,Base> and AbstractEmptyNode<Product,Base> allow
normal and empty MML classes to implement the same future MmlNode interface.
Only generic operations are implemented; no flat/custom math AST was invented.
Objects are fully allocated/adopted before initializeNode performs virtual
property/child phases. An abrupt setter leaves the adopted object alive with
the source's partial property state, without attaching later children.

Parent disagreements, duplicate/null slots, replacement identities and old
array references remain observable. Generic copy skips null slots, while later
MML copy must preserve its different policy. Iteration captures array identity
but reads live lengths; Array.join captures length as well. Visitor data uses
shared native handles, and all child traversal/copy dispatch stays virtual.
The base Wrapper starts with absent child storage and drops wrap extras; the
wrapper factory carries extras through a virtual WrapperServices bridge.
Common's parent-aware override is a future packet, not an implicit behavior here.

An agent correctly identified contradictory root expectations for __proto__
copy. TS object spread is emitted as __assign/Object.assign in pinned CJS and
Object.assign in MJS. Root verified Folia's js/ package exports resolve to CJS,
pinned emitted-file hashes, revised the contract and fixed an invalid fixture
comparison of an object against a numeric variant. Actual runtime expected
results did not change. Native copy now performs fresh assignment, including
the destination prototype setter effect.

Root rejected toString's live/re-targeted loop and null-check-before-kind order.
Actual Array.join witnesses require kind first, captured array/length, current
indexed values, ignored appends, preserved empty separators after removals,
recursive-array suppression and cleanup after abrupt child conversion. A
per-call-chain TreeJoinGuard implements only active array identity, separate
from TreeCallGuard's stack resource bound; it is not a visited-node cache.

The emitted CJS Factory lowers its arrow creator to a normal function as well.
After __proto__ registration, inherited lookup can reach its own prototype
object, a truthy non-callable value. Root added the TypeError witness, preserved
the existing independent Map/creator architecture and reran factory acceptance.
The pinned resolved Factory hash is now checked alongside TS provenance.

## Independent verification

- `tree-oracle.cjs --check` verifies clone/package TS equality, hashes, actual
  js/ export resolution and pinned emitted CJS hashes. Fifteen JS cases generate
  140 assertions, independently of the native implementation.
- Native suite passes **150 assertions in 17 tests**. Ten additional native
  assertions test 512-call walk/copy/toString and wrapper-walk boundaries,
  over-limit RangeError and recovery. Expected resource capacity is the frozen
  literal 512, not read from the implementation's own limit constant.
- `tree-mutation-test.cjs` passes an isolated baseline, then detects fifteen
  changes that compile and fail unchanged assertions: bypassed virtual setter,
  cached setChildren length, lost property handle, lost null partial push, wrong
  replace return, retained foreign parent, source-spread instead of emitted
  assignment, wrong copied kind, lost child virtual traversal, re-targeted join
  array, live join length, missing join cycle suppression, premature resource
  limit, forwarded dropped extras and absent-wrapper failure. Results/logs stay
  under ignored build/math-port-tree-mutations. Modules and fixtures are copied
  for isolation; the runner rejects overlapping runs and changed working fixtures.
- A mutation initially exposed an unsafe root fixture dereference after an
  expected partial push was missing. Root guarded the empty array and leaked
  pointer checks without changing behavioral expectations, then completed the
  final run with assertion exits rather than crashes/debug dialogs.
- Updated factory suite passes **116 assertions in ten tests**, including the
  emitted-function prototype witness; all eight factory mutations still fail.
- `cmd /c build_test.bat 2>&1` builds and passes the entire core suite.
- All 64 full reference snapshots and fresh-process observer-neutrality checks
  still pass, along with eleven reference mutations. This validates the oracle,
  not a native renderer that is not yet implemented.

## Adaptations and unaccepted dependencies

Native services use typed compiled class predicates, scalar PropertyList and
synchronous std::any callback handles. No arbitrary JS evaluation, descriptor/
proxy/custom iterator API or VM object constructor is provided. Split native
initialization and immobile owning objects preserve dynamic dispatch and stable
borrowed addresses. The emitted-source mapping is specific to the pinned runtime.

The 512-call limit is a documented native resource adaptation rather than the
VM's stack capacity or permission to shrink supported syntax. Later MML and
Common overrides must share the resource service and match their own method
semantics. Wrapper source nodes must outlive wrappers; session ownership remains
explicit. Generic acceptance cannot satisfy missing text/XML copies, attributes,
operator dictionaries, TeX-class rules or Common layout by approximating them.

Ten of 246 inventoried sources now have local acceptance. The other 236 and
external imports keep full G0 closed. Complete G1–G7 are still closed, and the
existing single QuickJS production backend remains in place.
