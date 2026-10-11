# P16c root contract — complete MmlMfenced semantic source file

Root independently read complete pinned ts/core/MmlTree/MmlNodes/mfenced.ts,
SHA256 64b3f4d89d47765eb83034b480b843b2769d8de12e31d7571c50e5c4bb2bd397.
This packet can proceed while MathChoice's different public argument boundary
is being designed. It must not modify that boundary or normalize its arguments.
Scope is the complete 10-member MmlMfenced class: defaults, four initialized
fields, kind, setTeXclass, child inheritance, addFakeNodes and fakeNode.

## Frozen ownership and interface

Root owns declaration/generated bindings, source member ledger, CMake,
actual-CJS oracle/expectations, tests, builds, mutation checks and acceptance.
math_fonts Luna may write only src/math/core/mml_tree/mml_nodes/mfenced.cpp and
docs/math-port/evidence/p16c-mfenced-methods.md. No other edits, builds, tests,
commits, delegation, reference changes, exclusions, guessed primitives or
renderer logic. Missing prerequisites must be reported before implementing.

Root declaration: MmlMfenced derives AbstractMmlNode. It has source-public
Children separators and MmlNode* open/close; initializeClassFields resets its
shared separator-array handle, both null pointers and INNER class after the
accepted base initialization phase. Defaults are a fresh shallow base spread
plus open='(',close=')',separators=','. kind returns exact 'mfenced'. Root's
generated bindings own all defaults/accessors/class field phase.

Agent implements:

- MmlNode* setTeXclass(MmlNode*) override
- protected void setChildInheritedAttributes(AttributeList,Property,Property,Property) override
- protected virtual void addFakeNodes()
- protected virtual MmlNode* fakeNode(std::u16string,
  Property properties=Missing{},Property texClass=Missing{})

The virtual fakeNode defaults are raw Missing, because omitted arguments must
reach an overridden callback unchanged. The source base fakeNode itself then
normalizes properties Missing to a fresh PropertyList and texClass Missing to
PropertyNull; explicit null/other raw values are not defaulted. This keeps
default ownership in the method, without C++ callsite pre-normalization.
Factory create receives the same raw properties graph. Use a PropertyList
record view in std::any when available; otherwise retain raw Property in
std::any and let the factory binding diagnose unsupported constructor values.
No source-level special case or broad factory-interface change is authorized.

## Complete method obligations

setTeXclass calls virtual getPrevClass(previous), then current open's class hook,
current child0, captures child count only after those calls, then for i=1..m-1
reads current separators[i-1] and current childNodes[i] separately. Each child
result becomes the next previous value. After current close's hook, if either
current delimiter is absent, invoke virtual updateTeXclass on current
open || child0 || close. Keep live reads and callback order; captured count
does not grow during loop callbacks. Missing slots skip, null arrays throw at
the same source access. Do not create fake nodes during this class pass.

setChildInheritedAttributes invokes virtual addFakeNodes first, then captures
the source concat snapshot [current open,current close,...current separators].
For nonnull pointers call inheritance with the original attribute handle and
raw display/level/prime, in that order, then qualified superclass propagation
over actual current children. Callback changes to fake arrays do not change
the concat snapshot; changes to real children remain visible to the superclass.

addFakeNodes invokes current attributes.getList once with open,close,separators
in source order, then performs all three .replace operations before creating
any fake. Only native strings bind the source string method; nonstrings throw
TypeError, without String(...) coercion. Remove only U+0020/U+0009/U+000A/U+000D,
preserving all other Unicode whitespace and UTF-16 code units.

Create open first only if cleaned open is nonempty, with fence=true/form=prefix
and OPEN. If separator string is nonempty, repeat its final UTF-16 code unit
until its length is at least current childNodes.length-1. Preserve the source
length reads and null-array failures. Capture childNodes.slice(1) only after
padding; iterate that snapshot. Skip null children and increment the separator
index only for nonnull children; charAt past end returns empty string.

For each separator, capture the current separators push receiver before
calling virtual fakeNode(c,Missing,Missing). Push the returned pointer (even
null) into that captured array after the callback. If a callback rebinds the
field, it must not redirect the push. Null push receiver throws. Finally create
close if nonempty with fence=true/form=postfix and CLOSE. Do not clear prior
open/close/separator state; repeated calls deliberately retain/append it.
Preserve partial state when an intermediate callback/factory throws.

fakeNode applies its own defaults, calls factory.create('text'), invokes that
returned TextNode's virtual setText(c), uses the returned text pointer as the
single child of factory.create('mo',properties,[text]), calls that node's
virtual setTexClass with raw normalized texClass, then sets node.parent=this.
Return exactly that mo pointer. Capture receivers at source evaluation points;
setText may return another text pointer or null; factory failures and null/
missing method receivers throw rather than produce a fake success. Required
text method binding is TextNode's existing virtual API. Node lifetime belongs
to the accepted factory/session, not fake fields or real child array ownership.

Use accepted TreeCallGuard/MML host scalar-string operations and standard
native containers. Do not add a VM, renderer/font selection, expression evaluator,
normalization of delimiters, inferred child row, fake child-tree attachment,
source-input exception, compatibility mode or fallback. Source has independent
fake storage deliberately; actual childNodes must not gain delimiter nodes.

## Root acceptance obligations

Run actual pinned CJS methods and compare defaults/class field phase, all10
members, empty/one/many/null children, custom open/close/separators, ASCII-only
removal, non-ASCII spaces, astral/lone-surrogate separator units, padding and
repetition, null skips/indexing, repeated invocation retaining state, fake
pointer parent/tex class/properties/text, live mutations/concat/slice snapshots,
captured push receivers, return chaining/count capture/update selection, partial
failure order and raw default argument callbacks. Include real Mo/TextNode
integration plus independent witnesses around virtual/factory boundaries.
Expectations must come from source, never this CPP. Deliberate compiling method/
fake/UTF-16/ordering omissions must fail assertions. Root runs focused checks
and core build before recording local source-file acceptance. Common/SVG wrappers,
full MmlFactory registration, complete TeX parsing/layout/rendering and G0-G7
remain required and cannot be claimed complete by this semantic packet.
