# P3b root acceptance

Status: Options prerequisite accepted locally for the defined raw-TeX API and
documented host adaptations. Complete TeX/MML, font services, Common layout,
shared scene, SVG and the Folia switch remain unaccepted.

Root designed the Context-owned option graph and froze declarations, ownership,
behavior and initial JS fixtures before dispatch. Options uses distinct values
and object/symbol identities, not the scalar MathML Property model. Source:
MathJax 4.1.3 `ts/util/Options.ts`, SHA256
`b810ffc09864eb8ed9295d829b229194d7e38aaac6c8f01525bc1a4b190a61e3`.
Root reviewed all twelve functions, globals and Expandable identity against
source and the individual object/array/Options ledgers. The private representation
and module boundaries contain no interpreter, JS evaluation or platform APIs.

## Review and corrections

The initial implementation cached constructor and APPEND/REMOVE getter values,
changing source evaluation order. Root added upstream-generated cases and
required repeated reads, left-spread-before-right-get, and abrupt completion
when recursive copy removes a later descriptor. Other corrections restored
descriptor-conversion order, array truncation's partial
rollback/non-writable flag, foreign Symbol checks and intrinsic metadata order.
Root also verified getter receiver identity against actual inherited accessors.

The first recursive implementation crashed with Windows STATUS_STACK_OVERFLOW
before reaching the frozen 512 depth limit. Root rejected it. Copy and insert
now use one explicit traversal-frame stack with source key snapshots, child
continuations and staged descriptor definitions; parent mutations survive abrupt
completion as upstream does. A 512-object copy succeeds and a self-cycle raises
the specified RangeError without consuming recursive native call frames.

An exported inline self-referencing diagnostics initializer failed to link on
MSVC 19.50. The approved ABI adaptation defines the exported OPTIONS object
once in the module interface object. It preserves global mutable identity,
default warn/fatal behavior and callback replacement.

Final source review found that the default stderr encoder emitted invalid UTF-8
for a trailing unpaired high surrogate. Root corrected it and added byte-exact
tests from a fresh pinned MathJax process for a valid surrogate pair and three
unpaired-surrogate positions. The UTF-16 diagnostic sink still retains code units.

Mutation testing exposed a defect in the root fixture: a missing descriptor or
shortened vector could be dereferenced after an assertion failed, causing a
Windows Debug assertion dialog. Root added guards after the failing checks;
expected values and comparisons did not change. Every mutation now ends with
the required assertion failure rather than a crash or a blocked process.

## Independent verification

- `options-oracle.cjs --check` verifies clone/package source equality and all
  frozen expectations generated from actual pinned Options.js and native JS
  object/array operations. No native implementation generates expected data.
- The native fixture passes **593 assertions in 21 tests**: all functions,
  constructor shadowing, missing/undefined, UTF-16 string boxing, ordered keys,
  symbols, descriptors/accessors, alias identity, sparse arrays, exact sentinels,
  NaN strict equality, warnings/fatal partial writes, inherited selectors,
  remainder order, live getter-driven spread/filter, conversion order, abrupt
  copy mutation, default diagnostic UTF-8 bytes, Context release/isolation and
  the depth boundary.
- `options-mutation-test.cjs` first passes an isolated baseline, then detects
  eight changes that each compile and fail unchanged assertions: dropped
  non-enumerable symbols, wrong inherited getter receiver, missing writable
  rollback, cached spread length, SameValue indexOf, ignored inherited indices,
  missing constructor re-read and remainder returned last. Logs/results remain
  under ignored `build/math-port-options-mutations`; working sources never mutate.
- `cmd /c build_test.bat 2>&1` builds and passes the complete core suite.
  No production/platform integration changed in this packet.
- The separate reference pipeline still passes all 64 pinned snapshots and
  fresh-process observer-neutrality checks, plus all eleven reference mutations.
  These are oracle checks, not proof that a native renderer exists yet.

## Adaptations and limits

Revision 2 of `../options-contracts.md` is authoritative. Context owns records
until engine/session destruction and releases cycles via non-owning handles.
Native callbacks bind C++ services; this is not a general JS callback API.
The 512-frame budget is per Options invocation, not arbitrary callback recursion.
Any actual ported callback needing re-entry requires a separate session contract.

Ordinary-array spread/filter/indexOf are native adapters. Custom iterator/species,
method overrides, proxies, cross-realm behavior, generic fromIndex/object
coercion, arbitrary builtin bodies and direct JS iterator objects are outside
this raw-TeX API. E8 checks the actual source profile for witnesses but does not
prove all font-package or future host behavior. Explicit MathJax diagnostics
remain exact; VM property-error wording and stack resource limit are adaptations.
No general JavaScript compatibility or full source-closure acceptance is claimed.

Options raises the locally reviewed source count to five of 246. The remaining
241 sources and external imports keep full G0 closed. G1–G7 remain closed for
complete parser/layout/SVG/native app acceptance; QuickJS has not been replaced.
