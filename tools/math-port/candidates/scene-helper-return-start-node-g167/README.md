# Native scene helper-return fixture start-location correction (G167)

This candidate fixes the direct `MathItem` test fixture to match the location setup performed by the pinned `MathDocument.convert()` implementation: it assigns `start.node = adaptor.body(document)` before typesetting. The default `end` location is intentionally retained because Source `convert()` assigns only `start.node`.

The G167 exception stack ended in `CommonXmlNode::getFontStyles()` while resolving `math.start.node`; `getHTML()` had not yet run, so the foreignObject geometry and PaintOwner assertions were not reached. The patch changes only the fixture helper in `tests/math/scene_attribute_parity_g145.cpp`.

Apply check (from repository root):

```powershell
git apply --check tools/math-port/candidates/scene-helper-return-start-node-g167/scene-helper-return-start-node-g167.patch
```

No build or test was run for this candidate.
