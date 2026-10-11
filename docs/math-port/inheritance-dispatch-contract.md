# Root P17b — one raw MML inheritance dispatch path

Root inspected pinned MmlNode.ts public/default/body/child/add methods and all
concrete public override and child-hook callsites before defining this boundary.
MathChoice and Mtable public overrides have no defaults; Base/Mi/Mo/Mstyle own
all four defaults; Empty ignores arguments without defaulting. The native
static-callsite C++ defaults must not run before the selected source method.
This corrects the argument carrier, not the source algorithm or editor model.

## Frozen API

AttributeList becomes the accepted Property carrier, retaining Missing/null,
scalars, lossless strings, arrays and shared records. There is one public
nonvirtual MmlNode::setInheritedAttributes(AttributeList=Missing{},
Property=Missing{},Property=Missing{},Property=Missing{}) forwarding unchanged
to virtual inheritAttributes(AttributeList,Property,Property,Property).
The body hook has no C++ defaults. Qualified source super calls use the
qualified base inheritAttributes body, not the facade (which would redispatch).
Base/Mi/Mo/Mstyle apply host::inheritanceDefaults(attributes,display,level,prime)
inside their own body: only Missing attributes becomes a fresh PropertyList;
only Missing scalars become false,0,false. Mtable/MathChoice/Empty do not call it.
All child hooks and addInheritedAttributes use raw AttributeList; its additions
argument also uses raw Property. Forwarding preserves raw identity and values.
Only a source spread makes a fresh record. No old overload, alternate dispatcher,
legacy carrier, feature flag, parser stub or broad JS interpreter is introduced.

Root owns all interfaces, source member mapping, base body/host primitives,
generators/witness adaptation, oracles, unchanged expectations, mutations,
builds, review and acceptance. Qualified body renaming is a documented native
dispatch adaptation of the same source member, not a second implementation.

## Host primitive ownership

Root adds host::ownKeys(Property) with exact Object.keys nullish errors, shared
record/array enumerable-key ordering and UTF16 indexed string keys. Scalars
have no own enumerable keys. host::objectAssign(Property) is the pinned CJS
Object.assign({},source) lowering: nullish is ignored, strings enumerate UTF16
indices, records retain shared nested identity and __proto__ setter semantics.
Arithmetic copy reuses that single operation. No record conversion occurs
before a source Object.keys/spread/property read.
Tuple destructuring accepts native arrays and strings (Unicode code-point
iteration), returning raw origin/value including Missing. The pinned CJS __read
lowering returns a noniterable receiver unchanged: subsequent indexed reads
accept boxed scalars/records and throw for nullish receivers. This emitted
behavior is required for the actual CJS comparisons, including partial state.
noInherit/stopInherit property access uses existing typed Property lookup and
nullish optional-chain rules, retaining source property-key conversion.
Explicit source {} constructions stay PropertyList{} after the alias changes.

## Luna ownership after root interfaces land

math_fonts: only mi.cpp, mo_scripts.cpp, mstyle.cpp and
docs/math-port/evidence/p17b-defaulting-methods.md. Rename their own public
source body to inheritAttributes, own four defaults, qualified super body;
Mi reads ORIGINAL normalized parameter via host::get after super. Leave all
other source behavior/order intact. No other edits/builds/commits/delegation.

tex_parser: only mtable.cpp, mtr.cpp and
docs/math-port/evidence/p17b-table-methods.md. Rename Mtable body, preserve its
NO-default indent pre-pass and qualified super. Raw attribute reads use existing
host::get. After source addInheritedAttributes yields a fresh object, record
writes use root host::set(Property,key,value) with original strict setter rules
(record/array handles retain identity, nullish/scalar receivers throw TypeError).
Keep all tuple aliases/receiver captures and callbacks/order intact. No other
edits/builds/commits/delegation.

Root handles Base/Empty, host operations, explicit fresh source records and
all declarations. Unchanged child forwarding bodies require no new algorithms.
MathChoice has a separately frozen complete-source packet; its implementation
must use this same raw virtual body and cannot normalize missing values.

## Acceptance

Actual pinned CJS comparisons must exercise defaulted and no-default methods
through base pointers; omitted versus explicit Missing/null/scalars/objects;
string/array enumeration and object spread; raw tuple origins; superclass and
child callbacks; partial state and errors. Mstyle writes before a base null
error; Mtable fails at its first raw indexed read before base defaulting.
Retain existing expected observations while mechanically adapting witnesses
and generator C++ signatures to this API. Do not rewrite JS source witnesses
or change source expected values to accommodate the implementation.
Compile deliberate default/copy/dispatch/normalization mutants and require
unchanged assertion failures; focused regression and full core build/test
must pass before accepting. Existing complete source gates remain incomplete.
