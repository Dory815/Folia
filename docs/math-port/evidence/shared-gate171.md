# Gate171: SVG, Scene, layout and remaining semantic differences

All 57 targets compile with 2,078 captured inputs unchanged through build and
evaluation. Complete public SVG/error comparison passes 150/150 against the
pinned clean MathJax Source capture; API passes 9/9. The public executable
SHA-256 is 694990c27740c340a178a5e766c2164f0334863045ac12db7ec664b02606556c.
The full SVG comparison report SHA-256 is
19336b78bd4b0e7c6e126a286835caa92d2fefdb3cdefcbcaf9032e98848c042.

The independently captured paired route has 146 equal successful complete
frozen Scenes and four equal error pairs. All 150 Scene-only records report
zero SVG materialization and zero XML serialization. Executable and input
hashes remain unchanged; probe SHA-256 is
ffc4eda0cff2cc358e74b8a07e6d9d658d4e727aace4f91f63ce121d05355198.
This checks native route equality; Source parity also requires the separate
Source observations and tests.

Common layout observations pass 75/75. The temporary XML foreignObject shell
now uses Builder::adoptExisting; only the actual jax.svg helper records paint.
The XML fixture now supplies the actual production Scene creator service and
its comparison passes. Eighteen of twenty checkpoint comparisons pass, and
all 25 package captures exit 0. Parser, Regexp and LinkedList suites pass.

Core, Host and Foundation still fail. The existing document helper test has
one false assertion: replacing children[0] through the raw child array must
leave boldIdentity.parent pointing to section. An isolated copy with
diagnostic assertion reporting locates this at scene_attribute_parity_g145.cpp
line 527; the expected behavior has not been changed. The Common factory test
still fails when a live field descriptor is replaced. Frozen factory-null and
NodeFactory direct-call cases still have different error behavior. The newly
registered post-filter metadata test has four false assertions and needs
actual field diagnostics before a correction is chosen.

The two remaining checkpoint groups expose missing Source-visible MmlFactory
fields (defaultKind, node, nodeMap) and StackItem fields (_nodes, _properties).
These require actual live owner bindings rather than empty objects added to
satisfy own-key observations. Candidate fixes remain under review.

Filter record deletion/replacement and RegExp clear with an externally held
owner now pass their tests. Gate170's private Context API compile failure was
resolved using weak references to fields created by RegExpBindings itself.
No new public Context internals were needed.

All evaluation processes are terminal. The final audit records zero changed
inputs and evaluation exit 1. Folia still uses math QuickJS, and Mermaid remains
paused. No complete renderer acceptance or Folia integration is claimed.
