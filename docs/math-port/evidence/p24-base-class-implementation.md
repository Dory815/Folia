# P24 base TeX-class propagation implementation

Implemented the three assigned `AbstractMmlNode` method bodies in
`src/math/core/mml_tree/mml_node.cpp` against pinned `MmlNode.ts` lines 697–728
(TS SHA-256 `cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656`).

- `setTeXclass` calls `getPrevClass(previous)` first, then reads the current
  virtual `texClass()` once. A non-nullish value returns this node as a tagged
  `NodeValue`; otherwise it returns the original `previous` tag.
- `getPrevClass` skips Null/Undefined. For a node, it reads `texClass()` and
  writes `this.prevClass` before reading `attributes()`. It reads the
  `scriptlevel` property once and writes the raw value to `prevLevel`. Null
  attributes raise a TypeError at the `get` step, after the previous-class
  write and before the level write.
- `updateTeXclass` skips Null/Undefined. It performs each virtual read/write in
  source order: transfer `prevClass`, transfer `prevLevel`, clear the core's
  `prevLevel` and then `prevClass` (the source chained assignment order), then
  transfer `texClass`.

Each method retains the shared `TreeCallGuard<MmlNode>` boundary. No other
production files or interfaces were changed. No build or tests were run; Root
owns the independent Source/native comparison and acceptance.
