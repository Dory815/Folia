# SVG helper identity fixture boundary correction (G160)

This candidate changes only `tests/math/scene_attribute_parity_g145.cpp`.

The helper’s extra rectangle is now appended to the exact `foreignObject` returned by the wrapped real SVG helper. The scene importer can then discover its identity while recursively importing that returned node’s actual children. The test retains the live geometry check: after typesetting and import, it changes the original rectangle attributes object from width `12` to `19`, freezes the mapped live Scene node through a fresh `Builder`, and checks width `0.019`.

The prior fixture created the rectangle as an unrelated side effect, then immediately asked `SceneContextNodes::sourceNode(rectValue)` for a mapping before the returned foreignObject was imported. That lookup cannot succeed: `sourceNode` finds only identities registered by `associateSourceNode`, and the ordinary `SvgXmlNode.toSVG` path uses only the node returned by `this.svg(...)` as the foreignObject appended to its parent. A side-effect node outside the returned subtree is not part of that import. This correction preserves the geometry assertion and makes its mapping boundary real.

Evidence: pinned MathJax Source 4.1.3 `ts/output/svg/Wrappers/semantics.ts` (`SvgXmlNode.toSVG`) constructs and appends the return value of `this.svg('foreignObject', ..., [this.getHTML()])`; native `src/math/output/svg/output_jax.cpp::createXmlSceneNode` imports the returned DOM node; `src/math/output/svg/scene_context_nodes.cpp::State::SourceChildOwner::snapshot` recursively resolves children; `associateSourceNode` registers source identities.

Apply with `git apply --check tools/math-port/candidates/scene-helper-return-boundary-g160/scene-helper-return-boundary-g160.patch`. This is candidate-only; it was not applied to formal files or built.

The separately reported G160 `options::Exception` during typesetting remains under investigation. This fixture correction addresses a demonstrably invalid side-effect-node lookup but does not claim to explain or conceal that earlier exception.
