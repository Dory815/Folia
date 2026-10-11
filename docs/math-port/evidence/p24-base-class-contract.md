# Root frozen base TeX-class propagation packet

Implement only three existing stub bodies in
`src/math/core/mml_tree/mml_node.cpp`: AbstractMmlNode::setTeXclass,
updateTeXclass and getPrevClass. Their signatures and responsibility stay at
the original base MML boundary. Write p24-base-class-implementation.md.
No declarations, additional functions, other production files, helpers, tests,
gold, observer tools, pins/locks, build wiring or Folia changes. No build/test,
commit or delegation. Root owns independent acceptance.

Original MmlNode.ts SHA-256 is
`cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656`;
CJS MmlNode.js is
`86c5eb572dcf2652c3c52a60b66c720d231e17f600cc9aeef3ace4aeafc81d45`.
Methods are at original TS 697-728. The already established NodeValue boundary
preserves Node, Null and Undefined and does not implicitly cast to a pointer.
Use that boundary and current virtual property accessors, not old dense storage
or an alternative class-state cache.

setTeXclass must virtually call getPrevClass before reading the current raw
texClass. Select this when current class is non-nullish (including false, zero,
NaN, strings and property objects); otherwise return the original tagged prev.
getPrevClass does nothing for nullish prev. For a node, read prev.texClass then
write this.prevClass; then read prev.attributes once, call get('scriptlevel')
and write the raw result to this.prevLevel. A null attributes error occurs after
the previous-class write and before the level write. Preserve virtual order.
updateTeXclass skips nullish core. Otherwise copy core.prevClass then prevLevel
through virtual reads/writes; original chained assignment clears core.prevLevel
before core.prevClass; then read core.texClass and write this.texClass. Do not
preload all fields or normalize their values. Existing shared native recursion
budget is a resource boundary, not a reason to alter Source dispatch.

Root independently reran the original class-state oracle --check: 29 raw values,
1,682 spacing rows, 2,523 level rows, 13,441 assertions in nine tests. The fixture
is unchanged, SHA-256
`4effd17780045c90b8c65ac26c55cee3aed15ee4e3585f1838f37f283178d007`;
the original oracle SHA-256 is
`5c79bbb340c3fc99a1053f34f39ef220ccf3469805c95eb7e4557d8df05871c6`.
This existing witness observes raw getter/setter trace, null guards, transfer
order and partial attribute failure; much of it tests already accepted spacing
and storage, so it is not 13,441 independently red new-method cases.

Root additionally ran three original protected-method boundary calls and froze
19 assertions / three tests. Source snapshot SHA-256:
`216b672fdeee6cb334a510ce057a71ba681dcfe86c6713ad84f204c28f77d35d`.
Its native emitter catches and names actual errors while preserving expected
results. The first red harness run using only the old fixture was rejected:
Boost's generic what() text did not prove the actual Unimplemented error name.
Root added the separately sourced boundary observer, then compiled and executed
the full 90-interface/implementation graph. The boundary observes all three
real stubs as Unimplemented, and the full gate fails assertions. Original
spacing/storage rows still pass. This is real runtime red before delegation.

Root requires unchanged Source and fixture checks, all 13,460 assertions / 12
tests, supplementary Null/Undefined and callback/partial-effect witnesses,
effective compiling semantic mutants, relevant existing regressions and the
repository build. Keep mismatches visible; do not fix tests to fit the port.
This closes three base methods, not Parent/childPosition, subclass class
propagation, full TeX parsing, layouts, backends or Folia. No whole-file tally
advance follows without that file's complete acceptance.
