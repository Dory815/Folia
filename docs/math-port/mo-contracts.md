# P12c complete MmlMo — root contract revision 1

Source: pinned MathJax 4.1.3 mo.ts, hashes locked in p12b-source-lock.json.
All 32 AST-inventoried members, including 20 executable members, require an
individual mapping. P12b accepted only the five default pattern predicates.
This packet supplies real MmlMo semantics over the established MML tree; no
parser, custom AST, common wrapper, partial factory or production registration.

Root owns all declarations, typed host bindings, generated source statics,
oracles/fixtures, CMake, mutations, scope, review and acceptance. Freeze them
before dispatch. Agents own only the listed implementations and evidence;
no shared-file edits, builds/commits, altered expectations or omitted branches.

| Agent | Only source | Only evidence | Methods |
| --- | --- | --- | --- |
| tex_parser | mml_nodes/mo_lookup.cpp | evidence/p12c-mo-lookup.md | texClass getter, hasSpacingAttributes, getOperatorDef, checkOperatorTable, getForms, handleExplicitForm |
| math_layout | mml_nodes/mo_classes.cpp | evidence/p12c-mo-classes.md | coreParent, coreText, isAccent, setTeXclass, adjustTeXclass |
| math_fonts | mml_nodes/mo_scripts.cpp | evidence/p12c-mo-scripts.md | setInheritedAttributes, checkPseudoScripts, checkPrimes, checkMathAccent, isMathAccent, isMathAccentWithWidth |
| Root | mo.ixx, mo_types/host, mml_node/class_binding extension, mo_bindings.cpp and all tools/tests/build/data/acceptance | Root review | source fields/statics, kind, isEmbellished, setter, phase initialization and constructor services |

## Fields and actual constructor services

_texClass, lspace and rspace are raw Property fields. They start Missing during
the superclass's initializeMml, then initialize null and 5/18 after that phase.
The inherited source texclass field is a different source field, never a Mo
fallback or mirrored canonical state. Getter tests strict null only, calls
getOperatorDef(getText())[2] without caching or otherwise returns the raw field.

defaults is a one-time shallow own-property spread of token defaults plus exact
source overrides. Class OPTABLE/MMLSPACING are module-initialized aliases of the
dictionary handles; class and module rebinding remain separate. remapPrimes is
a fresh ordinary numeric-key object. Five replaceable shared Pattern handles
preserve source regex object identity/lifetime and use P12b pristine predicates.
These bind compiled native callbacks, not arbitrary JS regex evaluation.

Factory metadata captures the actual constructor independently of later
registry replacement. Generic staticServices transports one family-owned
MoClassServices handle; T::staticBinding<T>() installs live getters for the
actual class's table, spacing, remap and pattern fields. Each source static read
calls its provider at the source position, never eagerly snapshots all fields
or consults the current factory registry. Pattern reads retain shared handles;
replacement does not change already captured handles. Missing bindings raise
typed errors, never silently select base statics.

## Method fidelity and call order

Preserve every virtual read/write and short circuit; stage C++ argument reads
in JS order. All recursive/traversal hooks use the shared TreeCallGuard. Parent,
child-array and attribute failures raise FactoryError TypeError before later
reads/writes rather than native UB. Host helpers only bind calls/indexing and
source Object.keys/array-or-string iterators, not feature decisions.

getOperatorDef returns raw Property because an inherited prototype-name hit can
be a function/string/object, not only an OperatorDef array. Set inherited form
before constructor/static reads. Retain ordered truthy form lookups and exact
hit identity; misses write noDictDef before limits/pattern checks. getRange uses
module-level data even when constructor OPTABLE is rebound. Fallback spacing
destructures its live array or source string iterator and produces fresh OPDEF.
checkOperatorTable reads def[3] anew per attribute after reading its receiver,
enumerates own keys in source order, and divides raw slots by 18 left then right.

getForms returns fresh arrays and retains parent versus Parent differences.
handleExplicitForm returns its original handle unless form is set; filtering
uses strict raw Property inequality, skips absent slots but retains inherited
slots, and prepends the raw explicit form without normalization.

coreParent uses actual registered math instanceof, not kind equality. coreText
uses source singleton/unwrapping and embellished short circuit. isAccent returns
raw Property despite its TS boolean annotation; an explicit parent value returns
the initial false rather than that value. setTeXclass reads form/fence together,
retains the spacing early return and source fence/class getter count. Class
adjustment preserves raw prev/level values, autoOP mutations, BIN rules, parent
walk and partial states on failed last-child access.

setInheritedAttributes applies source entry defaults then superclass, reads text
once and calls the four checks in exact order. Prime remapping uses codepoint
iteration and live dictionary reads; missing mappings become NaN to the accepted
unicodeString RangeError path. checkMathAccent intentionally compares
under.coreMO() in its isOver expression: preserve the pinned source behavior.
Accent methods only evaluate getText when the optional argument is omitted;
explicit empty text is preserved. Do not repair source quirks or add sample
special cases.

Root review also binds native-function prototype metadata when malformed form
names address Object.prototype functions: actual function prototype/constructor
and call/apply/bind/toString identities, name/length/prototype and restricted
caller/arguments access. This is a typed metadata binding, not JS execution.
Snapshot the complete accent child tuple before callbacks; evaluate the
attributes receiver before reading a definition value; complete all prime
mapping reads before coercion; preserve right-associative class-write order.

## Required verification

Root actual-CJS fixtures cover every source member, pristine static aliases and
default copy, constructor capture/rebinding, lookup identity/freshness, raw null
versus Missing, form/position and class matrices, inherited/explicit spacing,
parent/accent quirks, prime remapping, text/default arguments and check order.
Root extends gaps using unchanged upstream behavior, never weakened expected
results. All retained branches need witnessed tests and a member ledger.
Require reproducible data/oracles, complete core regression and compiling,
assertion-failing isolated mutants for algorithm/state/call-order errors.
Native code is not release accepted until root independently passes these;
complete SVG/layout/Folia G0–G7 remain separate obligations.
