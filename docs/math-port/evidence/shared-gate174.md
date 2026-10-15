# Gate174: current rendering parity and unresolved semantic tests

All 57 targets compile. Gates172 and 173 failed compilation: the parent
projection used Value instead of ObjectRef for Context::define and range-for
on ChildList; one new factory UT also named OrderedMapAccess in the wrong
namespace. These are corrected using the existing APIs. Their build input
audits report no changes; no runtime acceptance is claimed for those gates.

Gate174's first API capture was invalidated when an agent accidentally edited
src/math/core/tree/child_array.ixx after compilation. No build ran during that
edit. The complete edited file and diff are retained in the candidate packet;
the formal file was restored to the exact frozen SHA-256
66c88460063ecb1b7bc90b00b7a0d46fec19a4e52d327de35a516abf05d3fe8f.
All 2,081 inputs then match the original build manifest. The interrupted API
capture and provenance remain separately archived, and shared-build-state
records the restart and build/math-port/g174-input-recovery.json. The complete
evaluation below starts after recovery and closes with no input changes.

Fresh public Renderer SVG/error comparison passes 150/150 against the clean
pinned Source capture. API passes 9/9 and Common layout passes 75/75. Public
SVG executable SHA-256 is
dc29db1221e608f58ae48ed2652eb81a3130db6c1cde3c02f94e80d97f8d684c;
the full comparison report SHA-256 is
956d505b8f3cd43a57338244dd1db36f45537d4bba99c311dd232d13e8a99733.

The paired Scene route has 146 equal complete successful frozen layouts and
four equal error pairs. All 150 Scene-only records have zero materialize and
zero XML serialization calls. Its executable SHA-256 is
ed483dd6ccc09977fa9396f443b0a795d1a6c3a8116f665b8bbb212002f185f5.
Source geometry is checked separately; native route equality alone does not
establish it. All 25 package captures exit 0; 18/20 checkpoint comparisons
pass. The remaining two groups expose MmlFactory and StackItem live fields.

Parser, Regexp and LinkedList suites pass. Core, Host and Foundation fail.
The corrected eight-filter metadata test now passes, and the frozen null
factory/direct NodeFactory cases no longer appear among Core failures.
Two factory fixture issues remain: an omitted newly-required direct callback,
and an owner identity query performed after live descriptor replacement.
An isolated, unchanged-test diagnostic locates the latter at the final
nodeMapAccess()->identity() assertion in common_factory_live_fields.cpp.
Candidate corrections make the callback mandatory and retain the actual Map
adapter while the native data field is still installed, checking that retained
adapter after provider release instead of performing a Source read then.
An isolated executable built against the stable Gate174 IFCs/library now
passes both corrected fixture files: 73 assertions in two Common tests and
52 assertions in eight factory-access tests. This does not yet verify removal
of the constructor's default argument in a rebuilt module interface.

Parent-related tests still have a helper assertion, a Scene live-field
assertion and several operation exceptions. The new findIndex test also
extends its thisArg object rather than its array, so its length assertion
does not test the intended behavior. These are being diagnosed against the
actual Source operations; existing expectations are not relaxed to accept
different native behavior. The MML factory bad-kind/children-consumption test
also fails in Foundation and is required in the pending raw argument bridge.
The combined ignored diagnostic confirms all three parent/target exceptions
come from Scene child deletion being unsupported. Correcting the findIndex
fixture and the direct-array-push parent expectation matches pinned Source.
Additional before/after diagnostics show bold already has a missing parent
before raw slot replacement; its import/clone field ownership needs correction
rather than inferring a parent from the mutated child array.

The complete evaluation is terminal with exit 1 and all 2,081 restored inputs
unchanged throughout that complete run. Folia remains on math QuickJS;
Mermaid remains paused. No renderer acceptance or Folia integration is claimed.
