# P38 menclose and mpadded implementation

Implemented the two P38-owned `setTeXclass` bodies from the pinned MathJax
4.1.3 methods.

- `MmlMenclose::setTeXclass`: classes live child slot 0 with the incoming
  previous value, rereads the current live slot 0 for virtual
  `updateTeXclass`, and returns the first child's callback result.
- `MmlMpadded::setTeXclass`: dispatches falsey `vbox` to the qualified
  `AbstractMmlLayoutNode::setTeXclass` implementation. For truthy `vbox`, it
  calls virtual `getPrevClass`, writes ORD through `setTexClass`, classes the
  current slot 0 with Null, and returns itself without an update callback.

Both entry points use `TreeCallGuard`; child method dispatch uses the tagged
`arithmeticChild` access. No API or fixture changes were needed. No build or
tests were run, per the P38 instruction.
