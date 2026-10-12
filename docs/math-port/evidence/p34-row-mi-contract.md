# P34 Root: restore row and identifier semantics on the tagged tree

Root assigns Luna only the five stub bodies nonSpaceLength, firstNonSpace,
lastNonSpace, setTeXclass, MmlInferredMrow::toString in
src/math/core/mml_tree/mml_nodes/mrow.cpp and the stub MmlMi::setTeXclass in
src/math/core/mml_tree/mml_nodes/mi.cpp, plus p34-row-mi-implementation.md.
Do not modify other methods, declarations, helpers, tests/expectations,
manifests, Folia or build/commit/spawn. Root owns verification and acceptance.

Use pinned original MathJax 4.1.3 ts/core/MmlTree/MmlNodes/{mrow,mi}.ts and the
existing architecture/protocol. These are per-kind semantic node methods;
do not merge them into a common switch or introduce another child model.
NodeValue and ChildArray are the single tagged coordinate.

Forward row queries and class assignment capture the Source for-of array once,
use live length and indexed prototype Get, and preserve callback mutation and
short-circuit/error order. Row queries skip falsey null/undefined children;
class assignment invokes all children's virtual setTeXclass, so nullish slots
throw. lastNonSpace captures the initial length but re-reads this.childNodes for
every backward indexed Get; it must not reuse a captured array after callbacks.

Row class assignment must retain the != null open/close checks (not truthiness),
getPrevClass and previous=null behavior of a delimited row, virtual raw texClass
read/write, and INNER only for Null/Missing. In a normal row, thread each child
return into the next call and return the final previous; after iteration re-read
childNodes[0] twice as Source does, calling virtual updateTeXclass with its
current value when truthy. Preserve mutations/throws and short-circuit accesses.

Inferred row toString follows raw Array.join semantics: one current array,
captured length, live indexed Get/prototypes, empty fields for holes/U/Null,
virtual child toString, the existing array-identity TreeJoinGuard and shared
TreeCallGuard. Do not call Generic::toString and strip a prefix or skip array
slots. Null child array fails at the reached join access.

Mi class assignment calls virtual getPrevClass, reads virtual UTF-16 getText,
uses the existing operatorName callback in the original short-circuit order,
then current attributes.get('mathvariant'), raw property reads requiring exact
Undefined, virtual texClass setter and setProperty. Do not replace the regex
service, normalize raw class values, use truthiness for property presence or
count UTF-8 bytes. Use existing exact nullish member checks and shared guard.

Root's leaf-oracle.cjs already freezes the original methods, callbacks, row
array rebinding/live growth/backward access, property presence and virtual class
assignment. Root will reproduce gold, inspect the pre-implementation failure,
review the six bodies and run actual reference_leaf tests with deliberate
faults and source/fixture guards. Remaining parser/layout/SVG work stays open.

Root extension after the actual normal build/run: the six restored bodies leave
three existing leaf tests failing through MmlMstyle::setChildInheritedAttributes
at its explicit stub. Root now additionally assigns exactly that body in
src/math/core/mml_tree/mml_nodes/mstyle.cpp to the same Luna, with its report.
No other write scope expands. Read the original mstyle.ts method. Preserve raw
Property level += parseInt string/number addition, nullish explicit presence,
methodString/relativeLevel/integer host operations, exact displaystyle===true,
raw cramped value, resets of prime, fresh virtual attribute reads, virtual
addInheritedAttributes and the single literal current childNodes[0] dispatch.
Do not use a loop/embellished core, normalize Property level/prime, or add a new
regex/parser service. Keep exact reached nullish get/setInherited/member errors
and the shared TreeCallGuard. Original leaf gold is unchanged.
