# P16a root contracts — five scalar MML semantic classes

Root read complete pinned menclose, mpadded, maligngroup, malignmark and mglyph
sources. Source hashes are pinned in simple-mml-reference.cjs from scope.json,
checked against clone and package source; actual CJS and full executable closure
are pinned by the accepted reference loader. Inventory: 22 source members.

## Boundaries and ownership

Each class retains its upstream base, fresh shallow defaults, scalar accessors
and derived field phase. Root generates those exact declarations/bindings with
TypeScript AST and actual CJS constants. This layer owns semantic inheritance,
class assignment and verification, not TeX commands, enclosure/glyph/padding
geometry, font/image resources or rendering. Output wrappers remain required.
malignmark has only generated members and requires no invented algorithm CPP.

Root owns interfaces, generators, CMake, source/member mapping, actual-CJS
observations, expected fixtures, builds, mutations and acceptance. After freeze:

| Luna agent | Only writable CPP files | Only writable report |
| --- | --- | --- |
| tex_parser | menclose.cpp, mpadded.cpp | evidence/p16a-enclose-padding-methods.md |
| math_fonts | mglyph.cpp | evidence/p16a-glyph-methods.md |
| math_layout | maligngroup.cpp | evidence/p16a-align-methods.md |

CPP files are under src/math/core/mml_tree/mml_nodes; reports under docs/math-port.
No other edits, builds/tests/commits/delegation, changed expectations, fallbacks
or scope decisions. Missing prerequisites must be reported to root.

## Method obligations

menclose.setTeXclass calls the required current child0 with original previous,
keeps its result, rereads current child0 after the callback, calls this receiver's
virtual updateTeXclass with that reread pointer, and returns the saved result.
Do not cache the first child across the callback or forward the update result.
The second indexed value may be missing/null: updateTeXclass accepts null and
does nothing. A null child array still throws on that second indexed access.
First required access must throw for null/missing child rather than return early.

mpadded.setTeXclass first calls virtual getProperty(vbox). Falsey values invoke
qualified AbstractMmlLayoutNode::setTeXclass. Truthy values call virtual
getPrevClass(previous), then virtual setTexClass(ORD), then required current
child0.setTeXclass(null), discard its result and return this. Preserve raw
truthiness (empty arrays/objects are truthy) and callback order/partial state.
Use accepted primitive host operations; do not add a new source special case.

Both setTeXclass overrides own one existing TreeCallGuard for native recursive
dispatch. No duplicate algorithm, alternate tree or cached child is added.
Required child access uses arithmeticChild. A source optional second indexed
read needs only a nullable dense-array read; it must not turn undefined into a
required-child error. Dense nullable native child vectors are the established
explicit host adaptation, not arbitrary JS array properties/proxies.

mglyph.verifyAttributes captures this receiver's attributes once, calls its
getList with src,fontfamily,index in that order, and performs strict empty-string
tests. If src==empty and either family==empty or index==empty, call virtual
mError with exact source message, same options handle, short=true and stop.
Otherwise call qualified AbstractMmlTokenNode::verifyAttributes(options).
Do not coerce false/null/zero/undefined to an empty string. Do not run superclass
verification before or after the invalid-glyph branch. No image/font loading.
This nonrecursive method needs no additional TreeCallGuard.

maligngroup.setChildInheritedAttributes reads current attributes and their live
getAllAttributes record, calls virtual addInheritedAttributes(original input,
that record), then qualified AbstractMmlLayoutNode::setChildInheritedAttributes
with the returned record and unchanged display/level/prime. Child callbacks must
see that returned handle, not a clone or the original input. Preserve excluded
style/displaystyle/scriptlevel keys and inherited tuple origins through the
accepted superclass helper, not a locally copied merge algorithm. Recursive
child inheritance is budgeted by existing setInheritedAttributes; no extra
local guard is needed in this hook.

## Root acceptance

Compare actual pinned CJS output for all fresh defaults/getters/field phases,
real inferred rows, strict glyph attribute raw matrix, invalid/valid superclass
verification order and options identity, raw vbox truthiness, source class/read/
write order, child and receiver callbacks/rebinding/throws, original return and
update arguments, nullable/malformed arrays and partial state, merged attribute
aliasing, raw inheritance values, real error trees and outer inheritance. Root
must review every source member, reproduce generators, run focused and complete
core tests and detect compiling assertion-failing mutants. Passing this semantic
packet cannot pass font/layout/SVG/Folia gates.
