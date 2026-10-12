# P37 MmlMo core implementation report

Pinned Source: MathJax 4.1.3 `core/MmlTree/MmlNodes/mo.ts` at
`fb987178c2d279a99b0db5ea02e751691703955e`.

Implemented only the five assigned bodies in `mo_classes.cpp`. No build or
tests were run, as required.

## Method correspondence

- `coreParent`: starts from this Mo, obtains the registered `math` class
  from the factory, and walks raw parents while each node is embellished and
  its `coreMO()` is this exact node. Constructor identity is checked through
  `host::moInstance`, not a kind string. Returns the last qualifying
  embellished node, or this if none qualified.
- `coreText`: returns empty for nullish input; for embellished input it asks
  that node's core MO for text. Otherwise it unwraps singleton `mrow`,
  `TeXAtom`, `mstyle`, and `mphantom`, or any `munderover`, while slot
  zero is truthy. The final result is token text or empty text.
- `isAccent`: obtains the raw parent of `coreParent`, selects `accent` or
  `accentunder` by the Source mover/munder/munderover slot rules, and
  preserves the mover/munder quirk: a truthy slot core-MO selects the key,
  without checking that it is this operator. If the parent has an explicit
  key value (including explicit null), the result remains false; only absent
  explicit value falls back to this operator's `accent` attribute.
- `setTeXclass`: evaluates the `form,fence` getList in order, then checks
  missing `texClass` plus the virtual spacing predicate. The fence rewrite
  requires a truthy raw fence and REL class; exact `prefix`/`postfix`
  values write OPEN/CLOSE through the virtual setter before delegation.
- `adjustTeXclass`: delegates directly to the shared
  `adjustOperatorTeXclass(*this, prev)`; it adds no wrapper guard.

## API and review notes

- The implementation uses the Root-owned tagged helpers `moChild`,
  `moAttributes`, and `moInstance`; it introduces no helpers or declarations.
- `coreText` has to project the Source `getText()` call from `NodeValue` to
  `AbstractMmlTokenNode`, since the native base `MmlNode` interface does not
  declare `getText()`. The implementation raises a TypeError if an
  embellished core-MO is not represented by a token node.
- The C++ raw parent API distinguishes Null and Undefined, but the Source
  `if (node)` branch treats both as false; `isAccent` therefore uses the
  truthy node projection there. Child-slot dereferences retain tagged
  `requireNode` failures.
- No compile or oracle results are claimed. Root should independently verify
  compile/API compatibility and Source parity, especially exact errors for
  malformed non-token core-MO values and callback-driven mutation timing.
