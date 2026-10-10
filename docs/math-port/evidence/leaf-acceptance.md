# Root acceptance audit of P9 concrete MML nodes

Status: local root acceptance passed. The ten source files are mrow,
math, mi, mn, mtext, ms, mspace, mstyle, merror and mphantom. They contain
62 inventoried members, including 39 executable members. Root's p9-members
ledger maps every field, default table, pattern, collection and method.

Root froze the declarations, source/native class metadata and field phases,
primitives, static/default responsibilities and actual-runtime fixtures in
commit 5f50819 before dispatch. Three agents had disjoint source-method files
and evidence files. Root alone owns shared code, contracts, expected values,
CMake, source review, mutations, builds, scope and acceptance.

## Required semantics and corrections

These are actual concrete subclasses of the accepted MML hierarchy, not
test-only nodes or one switch-based engine. Source imports/types remain
traceable. MML.ts and its complete factory table are separate unfinished work.
Test-only Probe subclasses inject callbacks into actual concrete methods; they
are never exported as production MML nodes or substitute for missing mo rules.

Root rejected private-field writes in Mi's semantic class assignment; only
post-super protected field initialization bypasses virtual setters. A derived
setter witness proves the distinction. Root corrected a collapsed third
attributes getter in Math's display expression and froze a seven-read/value
witness with independently varied attribute handles.

The first differential run failed three assertions because explicit undefined
arguments did not receive source defaults. Root fixed the common host/MML
boundary, then required Mi/Mstyle's own entry normalization before superclass
dispatch. A partial-write/throw witness prevents later superclass normalization
from masking this defect. Null and other raw inputs remain distinct. Protected
methods without defaults preserve incoming arguments, including string +=.

Class binding preserves actual constructor metadata after registry replacement,
and reads its defaults live. Registered kind defaults remain a separate source
lookup. Defaults are shallow copies, except InferredMrow's alias. Mrow preserves
captured live iterators, fresh reverse-index reads, current-array core access,
null differences, class propagation and join capture/cycle behavior. Token
methods preserve UTF-16 rules, explicit attribute presence and mutable bindings.
Mstyle/Math preserve inherited records, strict versus truthy decisions and error
allocation/partial mutation order. Nested Math with null verify options still
creates/messages the error node before the options read fails.

## Validation

- p9-source-lock.json pins both clone source and installed resolved CJS hashes.
  `leaf-oracle.cjs --check` verifies source equality and executable resolution.
- 26 actual-CJS cases generate 663 assertions independently of native output;
  six native assertions freeze 512-call row/join overflow and recovery.
- Five complete 65,536-unit bitset comparisons validate mi single-character,
  combining suffix, high-surrogate prefix, operator-name and ECMA whitespace
  rules. The fixed non-Unicode source patterns operate on UTF-16 units.
- Filtered native build passes 669 assertions in 27 cases. Full core build and
  suite pass. The updated MML base suite passes 1,084 assertions in 43 cases,
  and all twenty prior base mutations are still detected after the shared fix.
- The isolated leaf mutation runner compiles its baseline, then detects all
  29 deliberate regressions with assertion exit 1: surrogate prefix, combining
  ranges, whitespace, virtual operator setter, incoming variant tuple,
  autoOP presence, delimiter nullishness, live row traversal, reverse-array
  reads, core predicate order, both join snapshots, explicit linebreak,
  width trimming, newline variants and constraints, argument entry defaults,
  relative string addition, prime reset, strict display, raw cramped value,
  literal child0, actual-constructor defaults, initializer setter/phase order,
  null verification options, inferred defaults alias, resource capacity and
  separate display attributes reads. No mutation is counted by crashing.
  Copies, output lock and byte-equality checks preserve working sources.
  Logs/results stay ignored under build/math-port-leaf-mutations.

Root corrected two mutation-tool defects before the final complete run: an
array-retarget mutation originally changed a different branch, and the all-child
mutation originally moved its arguments during repeated dispatch and crashed.
The final mutations target the intended behavior and retain valid argument
bindings; all 29 are detected through unchanged assertions.

## Explicit native boundaries

Source class constructors are compiled, typed descriptors with one owning
factory/session. Node field initialization is a post-adoption/post-super phase
because C++ constructor dispatch cannot perform the source virtual calls.
Defaults and NONSPACELIKE collections remain live; pattern callbacks are typed
compiled matchers. All pinned source references to mi patterns are reads after
initialization. This is not a JS RegExp evaluator or arbitrary JS value union;
an empty callback is an invalid native binding, not undefined regex coercion.

The source's falsey null/undefined notParent returns normalize to bool where
all callers test truthiness. Ordinary scalar records and known prototype
method identities retain their accepted binding. Arbitrary JS proxies,
descriptors and evaluated functions are not exposed. Compiled virtual node
accessors/native callbacks are supported and preserve source operation order.
Whitespace reuses the accepted numeric-input ECMA table rather than a second
classification table. Native resource capacity is not a VM stack-capacity claim.

No complete TeX parser, font resolver, Common/scene layout, SVG backend or Folia
replacement is established by these semantic methods. Full gates stay closed.
Twenty-one of 246 inventoried sources now have local acceptance; the other
225 sources and external dependencies remain unreviewed. G0 and full G1–G7
remain closed. The single production QuickJS backend has not been replaced.
