# P9 concrete MML row, root, token and style nodes — root contract revision 1

Owner: root. Scope: every member of pinned MmlNodes/{mrow,math,mi,mn,mtext,ms,
mspace,mstyle,merror,mphantom}.ts. No node may be replaced by a generic switch.
MML.ts/MmlFactory and their full registration table remain a later dependency
gate; a partial registry must never be exported as the completed MML factory.

## Bindings frozen before dispatch

Root owns each module interface and *_bindings.cpp, leaf_defaults.ixx,
leaf_host.ixx, class_binding.ixx, oracle, fixtures, CMake and acceptance. Agents
own source method bodies in the assigned .cpp files only. All methods use the
accepted MmlNode tree and Attributes stores; no parallel AST or attribute graph.

classBinding<T> adopts before virtual constructor initialization and captures
the actual class metadata independently of later factory registration. Live
defaultsBinding reads T::defaults(), while initializeMml still reads the
currently registered kind's defaults as upstream does. constructorDefaults()
models this.constructor.defaults. Never replace it by factory.getNodeClass(kind()).
initializeClassFields() runs after the source super constructor; initializeTexClass
writes the underlying texclass field without dispatching its virtual setter.
Mrow similarly resets its _core field at that phase. Empty classes use the
accepted empty constructor. No class may create inferred rows itself except
through the accepted superclass constructor's arity dispatch.

Family defaults initialize together before a registration is exposed, using
source Object.assign copy semantics. All ten per-kind tables are fresh shallow
copies; only InferredMrow aliases Mrow defaults. Pattern callbacks and NONSPACELIKE
lists remain live reference bindings. Fixed mi patterns use source non-Unicode
UTF-16 regex semantics; no wchar regex or codepoint substitution. Root's host
functions implement those fixed grammars, whitespace, parseInt and primitive
addition; agents must use them and cannot invent character/number shortcuts.
There is no arbitrary RegExp parser/evaluator in the raw-TeX API.
Pinned TS search finds no assignment to mi patterns beyond their declarations;
MmlVisitor and mi only read them. Callable native replacements stay live; an
empty native callback is an invalid binding TypeError, not a JS undefined regex.
Whitespace uses the accepted numeric-input service's shared ECMAScript table.

MmlError ErrorOptions distinguishes an empty map from null. Reading options
from null throws at the source read, after any earlier error-node allocation
and message write. A nested math verifyTree(null) must retain this behavior.
Existing non-null option calls retain the same record identity.
Source default parameters apply to explicit Missing as well as omitted arguments.
Mi, Mstyle and the common superclass normalize display/level/prime at their
declared method entry, before any writes; protected no-default methods preserve
their incoming values. Root's partial-write witness prevents a later superclass
normalization from masking a missing subclass entry normalization.

## Exact writable packets

| Agent | Sole source files | Sole evidence file |
| --- | --- | --- |
| math_layout | mml_nodes/mrow.cpp; mml_nodes/math.cpp | evidence/p9-row-math.md |
| tex_parser | mml_nodes/mi.cpp; mn.cpp; mtext.cpp; ms.cpp; mspace.cpp | evidence/p9-tokens.md |
| math_fonts | mml_nodes/mstyle.cpp; merror.cpp; mphantom.cpp | evidence/p9-style.md |

Every source member must be mapped to a body, root binding or explicit native
host adaptation. Agents cannot edit declarations/defaults/host operations,
tests/oracles, CMake, scope, production, another packet, or commit/build. Report
an interface gap rather than patching around it. No dummy methods or fallback.

## Source-order acceptance obligations

Mrow has different null behavior across isSpacelike, isEmbellished and nonspace
queries. Its _core persists when later scans fail; core/coreMO call virtual
isEmbellished first and reread the current child array. for-of captures the array
with live length, lastNonSpace instead rereads this.childNodes each iteration.
Inferred toString joins a captured array with captured length, null slots,
recursive-array suppression and shared tree resource guard; it does not call kind().
Delimiter properties use nullish checks, not truthiness; normal class propagation
updates from the first current child only after the live child traversal.

Math preserves mode/display/displaystyle read order, truthiness versus strict
string equality, unchanged incoming prime, live constructor scriptlevel fallback,
full inherited explicit-attribute augmentation, and nested-tag errors after base
verification. Mi uses raw UTF-16 length, its exact static patterns, incoming
mathvariant tuple truthiness, and autoOP/texClass Missing checks. Explicit
mathvariant still overrides the inherited italic value; no feature shortcuts.

Mtext/mspace NONSPACELIKE checks explicit presence, even null/false/zero values.
Mspace checks width via String(value).trim().charAt(0), newline values strictly,
and leaves previous class state untouched. Mstyle preserves .toString versus
String coercion, parseInt grammar, relative primitive += (including string
concatenation), strict true display, prime resets and raw data-cramped override,
and exactly child0 dispatch. Its notParent getter is adapted to bool because all
source callers use truthiness; null/undefined return states normalize to false.

Root requires actual pinned executable fixtures, complete member review,
filtered/full core build, native resource recovery, and compiling mutations
that fail unchanged assertions. Full G1/G2 and renderer/integration gates remain
closed until every required concrete node and subsequent dependency is accepted.
