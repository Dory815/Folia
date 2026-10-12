# P33 MML error construction and spacing dispatch

Implemented only `AbstractMmlNode::mError` and
`AbstractMmlNode::hasSpacingAttributes` in `src/math/core/mml_tree/mml_node.cpp`.
The pinned MathJax 4.1.3 source is `MmlNode.ts`, SHA-256
`cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656`;
the pinned CJS SHA-256 is
`86c5eb572dcf2652c3c52a60b66c720d231e17f600cc9aeef3ace4aeafc81d45`.
The relevant source members are `hasSpacingAttributes` at lines 746–748 and
`mError` at lines 987–1016.

`hasSpacingAttributes` uses `TreeCallGuard`, returns false immediately when
virtual `isEmbellished()` is false, and otherwise calls virtual `coreMO()` once,
requires its tagged result for the `hasSpacingAttributes` access, then dispatches
that virtual method. It does not use `core()` or inspect attributes.

`mError` uses `TreeCallGuard` and first reads the virtual parent for the
short-circuit truthiness check. If it is truthy, the source expression reads
the virtual parent again for the `isKind` member access; the implementation
preserves this second read and its tagged receiver check. A parent whose
virtual `isKind("merror")` succeeds returns null before allocation or options
access. Otherwise it creates `merror`, obtains its
attributes through the virtual getter, and writes `data-mjx-message` before
reading `options.fullErrors`. Missing and Null options produce their respective
fullErrors property-read errors at that point, after those earlier effects.

The implementation preserves the repeated raw options reads. In the
full-errors/short branch it creates `mtext` then `text`, resolves the text
receiver before evaluating the subsequent fullErrors read, selects the message
or current virtual `kind()`, then checks the native `TextNode` capability and
calls its virtual `setText`. A product without that method raises the source
TypeError text `text.setText is not a function`. It appends text to mtext,
mtext to merror, reads
the current parent and replaces this node, and then reads fullErrors again. If
that value is falsey, it freshly obtains merror's attributes and writes the
title. In the other branch it replaces this node first, then appends this node
to merror. Append/replace return values are ignored; all earlier mutations
remain visible if a later operation throws.

No other source body, declaration, helper, fixture, or oracle was changed. No
build or test was run; root owns independent verification and acceptance.
