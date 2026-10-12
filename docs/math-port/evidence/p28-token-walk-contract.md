# Root token traversal implementation contract

The authorized implementation is exactly the body of
AbstractMmlTokenNode::walkTree in src/math/core/mml_tree/mml_branches.cpp.
Luna math_layout owns that body and p28-token-walk-implementation.md. Root owns
architecture, original observations, observer code, real stub, build/gate,
mutation testing and acceptance. No other production bodies, declarations,
helpers, fixtures, oracle, inputs, expected values, scope/pins or build files
may change. No test/build/generation, commit or delegation by the implementer.

The original is MathJax 4.1.3 MmlNode.ts (SHA256
cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656)
and its pinned actual CJS. Root's original oracle contains 780 complete
observations, including 224 original errors. Frozen Source JSON SHA256:
646bba252a86677fdab47a598d1cd75a828ad9c3e2f275b741898b6a238c6e71.
Each record includes exact exception, returned data identity/type, data mutation,
current and captured array lengths/own/Has/Get slots and complete callback trace.
Arrays include holes, explicit Undefined/Null, prototypes, Text nodes and
AbstractMmlNode children. Root/child callbacks grow/delete/shrink/rebind arrays,
replace prototypes, throw, and override child traversal return/throw behavior.

The original visits the receiver first, then captures its current child array.
For-of uses that array's live length and indexed Get, including inherited slots.
Only values that are instanceof AbstractMmlNode receive their virtual walkTree
call; Text nodes and nullish values are skipped. Child return values are ignored.
The original data is passed to every callback/child and returned unchanged in
identity. Capturing before the receiver callback, filtering a dense copy,
re-reading childNodes per iteration, fixed initial length, visiting Text nodes,
or returning a child's replacement data changes semantics.

Use the established ChildArray/NodeValue, dynamic AbstractMmlNode type boundary
and existing Error/host services. Preserve the actual original null-array error
from the frozen records. Include TreeCallGuard for the established Native
512-frame traversal limit and exception recovery; this is explicitly a Native
host boundary, independently witnessed by four assertions after the original
780-record test. Do not alter Generic::walkTree or introduce another tree walk.

Root independently compiled the full current 90-module graph and ran the real
stub baseline with mml-token-walk-gate.cjs --red. It exits 1 on assertions/actual
Unimplemented calls, without crashes, and final working-file guards pass.
The production stub remains unchanged at delegation. After implementation,
Root requires unchanged original --check, 784 assertions in two tests,
effective compiling faults, relevant existing regressions and normal core build.
This method packet cannot accept other MML consumers, full TeX/layout/SVG/D2D
or Folia integration.
