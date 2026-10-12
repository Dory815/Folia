# P39 MmlMfenced implementation report

Pinned Source: MathJax 4.1.3 `core/MmlTree/MmlNodes/mfenced.ts`, SHA-256
`64b3f4d89d47765eb83034b480b843b2769d8de12e31d7571c50e5c4bb2bd397`.

Implemented only the four assigned method bodies in `mfenced.cpp`. Root's
`childAt` and `requiredText` helpers were left unchanged. No build or tests
were run.

## Method correspondence

- `setTeXclass`: calls `getPrevClass` first; processes current `open` and
  tagged slot zero; captures child count after those callbacks. Each loop
  iteration reads current `separators[i-1]` then current `childNodes[i]`.
  It processes current `close`, then uses current delimiter fields and slot
  zero for fallback `updateTeXclass`. Each child callback result is passed
  onward and the final result is returned.
- `setChildInheritedAttributes`: calls virtual `addFakeNodes`, builds a
  concat-style snapshot of current open, close, then separator slots, and
  dispatches inheritance on truthy node slots with the original arguments.
  A null separators handle contributes one null value, which is skipped.
  The qualified base hook runs afterward on its own current real-child list.
- `addFakeNodes`: reads open/close/separators once in source order and
  validates/cleans all three strings before creating nodes. Cleaning removes
  only space, tab, LF and CR UTF-16 code units. It creates open first, pads
  separators using current child length and the final UTF-16 unit, then takes
  a sparse slice snapshot. Only truthy children consume separator units.
  Each push receiver is captured and checked before the virtual `fakeNode`
  call; the result is appended to that retained array after the callback.
  Close creation is last. Existing delimiter/separator state is not cleared.
- `fakeNode`: only Missing arguments receive empty-PropertyList/Null defaults.
  It creates text with no arguments, calls the validated virtual text setter,
  uses that setter's returned pointer as the only tagged `mo` child, and
  forwards properties and children in the two factory argument slots. It
  writes TeX class through the virtual setter before assigning the raw parent;
  nullable Mo results fail through `requireNodeForWrite` at the class write.

## Typed API notes

The implementation uses the existing sparse `ChildArray` handle for indexed
reads, slice snapshots, sparse concat-style snapshots and retained pushes.
`NodeValue` keeps Missing, Null and node slots distinct; UTF-16 strings retain
the source's code-unit length/index behavior. Factory calls retain their raw
nullable product results and callback order. Arbitrary JavaScript named array
properties/reflection remain outside this typed host contract. Root owns the
independent Source comparison and acceptance gate; this report makes no parity
or build claim.
