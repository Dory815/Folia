# P8b: MmlNode semantic hierarchy — revision 2

Root owns this contract, declarations, bindings, expectations, builds and
acceptance. Source is MathJax 4.1.3 commit
fb987178c2d279a99b0db5ea02e751691703955e, MmlNode.ts SHA256
cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656;
the resolved pinned CJS MmlNode.js SHA256 is
86c5eb572dcf2652c3c52a60b66c720d231e17f600cc9aeef3ace4aeafc81d45.
Every one of its 111 inventoried members (85 executable members) must be mapped.
This packet ports the real semantic hierarchy, not a custom AST or TeX parser.

## Root declarations and native adaptations

`mml_node.ixx` declares MmlNode over the accepted Node<MmlNode> storage, and
AbstractMmlNode, AbstractMmlTokenNode, AbstractMmlLayoutNode,
AbstractMmlBaseNode, AbstractMmlEmptyNode, TextNode and XMLNode. Normal and empty
branches share exactly one parent/factory/child-array representation.
texSpacing returns Property: valid strings versus source undefined after an
invalid column index remain distinct; invalid row access raises TypeError.
Getter/setter functions replace JS properties, including read-only empty-node
setter TypeErrors. TeX classes preserve null via optional<double>; previous level
preserves raw Property, with PropertyNull distinct from Missing and every scalar.
Property-valued display/level/prime arguments preserve erased-TS-cast behavior.
Missing arguments at methods declaring defaults receive false/0/false even when
explicitly supplied, as emitted JavaScript default parameters do. Null and other
raw values remain unchanged. Protected methods without defaults do not normalize.
Attributes remain shared four-layer Attributes handles, never flattened.

Root extended Property with null and a typed object-handle accessor; null
prototype assignment preserves its distinction from Object.prototype. Property
strings use lossless WTF-8 internally so generic Node.toString can retain JS
UTF-16 text, including lone surrogates. TextNode itself stores u16string.
`mml_node_host.ixx` supplies scalar truthiness, Number/string operations using
accepted numeric/string services, known builtin identity coercions, SameValueZero
and the explicit WTF-8 bridge. It provides no arbitrary JS callbacks/evaluation.
Foreign dynamic objects must have a reviewed native binding; do not stringify
them into plausible values. Native PropertyList objects retain their ordinary
prototype conversion behavior. General Options objects remain in their own
accepted Context graph, never silently cast to PropertyList.

AttributeList is the existing PropertyList graph. Root's attributeTuple builds
a shared two-slot PropertyArray (0=origin string, 1=Property), and unpackAttribute
reads that actual array. P11 replaces the former typed-record adaptation with
array tags/slots in the SAME PropertyStorage graph; there is no record fallback.
Inherited lists still use assignment/prototype semantics
and own-key ordering, including __proto__ assignment. Foreign iterator/getter
behavior is not part of this native tuple binding. Missing/corrupt bindings raise
TypeError. No parser-produced origin/value may be discarded or normalized.

P9 adds root-owned constructor metadata binding without a second attribute
store. Registered class defaults and actual-constructor defaults are distinct
source lookups: initializeMml reads the registry, later this.constructor.defaults
reads the captured class with a live defaults resolver. Per-kind protected field
initializers run after the virtual superclass constructor phase, and use direct
storage writes; semantic assignments still invoke virtual setters. ErrorOptions
represents non-null records and null; a null options read throws after preceding
mError allocation/message effects, as required by nested math verification.

Registered class staticData carries shared MmlClassInfo with optional live
defaults. Missing own defaults is the Object.assign empty-source case; missing
math/global defaults raises TypeError (Object.create(undefined)). Read own and
math classes in source evaluation order. A null initial attribute source is
ignored, as Object.assign does. Initial children default fresh when omitted,
but explicit null reaches the actual setChildren branch and its partial states.
Other invalid std::any argument shapes raise typed binding TypeErrors.

Creators allocate/adopt complete native objects before initializeMml. The generic
base starts with empty properties/children; initializeMml performs MML constructor
body in source order. Future concrete packets must model their field initializers
after this phase, and cannot rely on C++ constructor virtual dispatch. An absent
normal attribute object during partial initialization is a null native handle;
empty nodes expose their read-only null handle. Both source failed attribute
dereferences map to TypeError. XML holds an opaque XmlRef plus borrowed XmlAdaptor;
the adaptor must outlive the node. Clone/serialize dispatch are explicit services,
not fake XML rendering. XML copy deliberately omits the adaptor on its result.
Optional TextContentBinding models property presence followed by invocation:
absent means skip; engaged empty callback means present/non-callable TypeError.

TEXCLASS/TEXCLASSNAMES are read-only compiled tables: full TS search has no writes
and the product exposes raw TeX, not JS module-export mutation. MATHVARIANTS and
indentAttributes retain their mutable identity; policy/default PropertyLists are
live. Token defaults copy base defaults; Layout/Base defaults alias them.
Root's defaults reference functions initialize all four default records together
in source order, avoiding cross-TU dynamic initialization while retaining alias
versus copied identities and later independent reference replacement.

## Exact implementation packets

| Agent | Sole writable files | All assigned source responsibility |
| --- | --- | --- |
| math_layout | core/mml_tree/mml_node.cpp; evidence/p8b-normal.md | All AbstractMmlNode executable members, fields/policy defaults, TEXSPACE/TEXSPACELENGTH, MATHVARIANTS/indentAttributes; default record supplied by root |
| tex_parser | core/mml_tree/mml_branches.cpp; evidence/p8b-branches.md | All Token, Layout and Base executable members; root supplies copy/alias default records |
| math_fonts | core/mml_tree/mml_empty.cpp; evidence/p8b-empty.md | All Empty/Text/XML members and fields |

Root alone writes declarations, mml_node_host, mml_bindings, Property bridge,
oracle/fixtures, mutation tests, CMake, scope and acceptance. Agents cannot build
or commit, edit another file, invent concrete MathML nodes, implement a parser,
add font/layout formulas, add fallback/stubs, or wire production. A missing
interface/prerequisite must be reported before dependent work proceeds.

## Required semantic witnesses

Preserve inferred routing versus flattening/conversion, parent side effects and
attribute aliasing; forEach snapshots length while for-of remains live. Copy
uses current registrations, drops only root id unless keepIds, never forwards
keepIds to child copies, copies only explicit attrs and shallow properties, and
preserves its distinct null-child behavior. Logical parent and childPosition
skip notParent nodes according to the source's different loops.

TeX spacing uses the complete 8x8 signed table, nullable classes and raw-level
JS comparisons, including the negative space entries that survive scriptstyle.
updateTeXclass copies then clears core state in source order. Inheritance obeys
own-default/always checks, origin-specific noInherit, copied stopInherit removal,
explicit Missing distinctions, sticky prime, arity trim/pad, six indent values
and unchanged incoming display/level/prime at child dispatch. Trim does not
detach old parents. addInheritedAttributes excludes only displaystyle,
scriptlevel/style and performs emitted Object.assign plus assignment.

Verification preserves ordered attributes, data-* and class/style/id/href
exceptions, SameValueZero variant membership, ignore-variant truthiness,
verifyTree(null) no-op, arity rules and traversal after error replacement.
mError preserves create/message/parent-replace/append ordering, full versus
short versus wrapping errors and parent-merror suppression. Detached-root
errors may fail after allocation; do not repair source behavior.

Token methods use concrete class identity, not kind text. Empty/Text/XML are
excluded from token inheritance/walk; null is skipped there but token getText
throws on null. Text get/set/copy and XML clone/serialization remain virtual.
Layout delegates to child0, Base treats base and snapshot scripts differently.
Missing/null children must fail exactly at the corresponding source operation.
All recursive overrides share the accepted TreeCallGuard budget/recovery.

## Root acceptance

The root oracle loads actual pinned CJS classes, verifies TS/runtime hashes,
and generates reference_mml_node.cpp before dispatch. Test-only registered
subclasses exercise base virtual dispatch; they are not production replacements
for mrow/math/mo etc. Root tests constructor traces, partial states, identities,
copy, spacing, inheritance, errors, visitors, text/XML and mutation witnesses.
Root reviews every method/field against source, runs oracle --check, filtered
and full core builds, and requires compiling mutations to fail unchanged
expectations. Numeric spacing/classes/strings compare exactly here.
Concrete nodes, operator dictionary, TeX, Common, fonts, scene/SVG and integration
remain separate blocked-by-dependency packets; P8b alone cannot pass full G1.
