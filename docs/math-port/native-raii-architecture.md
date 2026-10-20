# Native C++ math architecture

Decision: 2026-10-19, following the user's instruction to remove the JS object
semantic simulation. This supersedes the Context/ObjectRef implementation and
its ownership-repair plans. The original MathJax version, 38-package profile,
fonts and rendering requirements in `scope.json` remain in scope. No runtime
or container retirement patch is part of this redesign.

## Pipeline and responsibilities

```text
TeX string + RenderOptions
    -> TeX input jax / package handlers / parser stack
    -> request-owned MML document
    -> CommonWrapper + BBox + font metrics + placement
    -> immutable mathematical Scene
    -> SVG writer
    -> existing Folia SVG path
```

The future Direct2D consumer takes the identical completed Scene. It performs
no TeX parsing, stretching, mathematical spacing, baseline calculation, table
layout or script placement. SVG performs none of those calculations either.
There is one layout pass. Backend-specific XML spelling, escaping, attribute
order and numeric formatting belong to SVG serialization.

MathJax's TeX packages, NodeFactory, MmlFactory/MmlNode/Attributes, CommonWrapper,
BBox, font providers and SVG responsibilities remain recognizable. C++ classes,
enums, value records and typed callbacks replace JS prototypes, mixins and
reflective dispatch. Source mathematical algorithms remain the reference;
arbitrary JS mutation behavior is no longer a native API requirement.

## Source structure correction (2026-10-20)

The upstream file responsibilities are the starting architecture, not merely
algorithm references. Removing JS object semantics does not justify replacing
the Common/MML/TeX family structure with a new collection of helper modules.
This section supersedes the earlier `output/common/placement/` allocation and
the fraction/radical packet paths. The numerical portions now replace those
canonical Common family files. The standalone placement files and corresponding
old Scene family files/registrations have been removed.

Review of `b030b874` found two problems: the new fraction/radical services had
only test callers, and their equations coexisted on disk with the superseded
Common and Scene family implementations. Duplicate family arithmetic has now
been consolidated. The former scalar APIs were replaced in place by actual
CommonMfrac/CommonMsqrt/CommonMroot classes in `95a119b1`; they now participate
in the native factory, MML, font, BBox and recursive linebreak collaboration.
The renderer facade and complete TeX input remain outside the active graph;
it cannot render TeX end to end. A successful component build does not resolve
that integration gap.

| Pinned upstream responsibility | Native owning files | Required boundary |
|---|---|---|
| `output/common/Wrappers/mfrac.ts` | `output/common/wrappers/mfrac.ixx/.cpp` | Fraction attributes, child metrics, ruled/atop/bevel measurement, slash stretching and width reconciliation |
| `output/common/Wrappers/msqrt.ts` | `output/common/wrappers/msqrt.ixx/.cpp` | Shared radical logic, surd stretching, p/q and base measurement |
| `output/common/Wrappers/mroot.ts` | `output/common/wrappers/mroot.ixx/.cpp` | Degree-specific dimensions/height; retain its relationship to msqrt |
| Numerical parts of SVG `Wrappers/mfrac.ts`, `msqrt.ts`, `mroot.ts` | The corresponding Common family above | Calculate child origins and rule geometry once, preserving separate Source measurement and painting quantities |
| SVG elements, attribute order, grouping and number spelling | `output/svg/` | Consume completed layout; no second mathematical placement calculation |
| Completed owned geometry | `output/scene/` | Store/freeze values; no node-family wrapper hierarchy or layout equations |
| `core/MmlTree/MmlNodes/mfrac.ts`, `msqrt.ts`, `mroot.ts` | Corresponding `core/mml_tree/mml_nodes/` files | MML structure, inheritance and semantics, distinct from output layout |

The numerical placement adjustment is necessary for the requested shared
SVG/Direct2D result: upstream SVG wrappers currently contain mathematical
positions as well as SVG construction. Move only the mathematical part to its
Common family. Preserve the rest of the upstream responsibility separation.
This is not a reason to create a separate placement family beside Common.

Replace canonical family files in place and retire superseded implementations
and their registrations in the same responsibility-sized change. Do not merely
move a free-function experiment into a wrapper-named file and claim the wrapper
is complete. MML/font/child/stretch orchestration and actual callers must be
reviewed too. Existing inactive consumers require explicit rewrite/removal
decisions; absence from CMake alone is not source cleanup.

File names describe their MathJax responsibility. Do not add `native_`, `typed_`,
`new_` or equivalent suffixes to distinguish a replacement from its original
owner. Rewrite the owning module and remove superseded code and registrations
within the reviewed change. `util/function_list.ixx` owns FunctionList;
`tests/math/function_list.cpp` owns its behavior tests. A source file left out
of CMake is still unfinished cleanup when remaining consumers reference it.

### Tag formatting ownership (2026-10-20)

`tagformat/tag_format_configuration.*` remains the single package owner. It
registers defaults and a priority-10 configuration callback. The selected Tags
creator is captured by value in a factory owned by ParserConfiguration; its
product remains uniquely owned by ParseOptions. The local catalog entry is
`configTags`, without a process-wide counter or permanent class registration.

`tags.*` retains numbering, equation/session state, tag MML construction and
default formatting. Its public formatting dispatch applies the configured
callbacks before the selected class's default formatting hooks, preserving the
Source outer-subclass precedence without synthesizing classes or prototypes.
Formatting borrows the input-jax TexOptionValues record and reads its current
TagFormatOptions on each call; it never borrows a temporary optional payload.
The same original default algorithms supply both ordinary Tags and the package
defaults. No second tag builder, numbering algorithm or MML owner is introduced.

### Setoptions and typed option access (2026-10-20)

`setoptions/set_options_configuration.*` retains the original package's three
filters, macro and configuration callback. Its private field dispatch maps
Source names to the existing finite C++ option members. It owns no mirrored
option dictionary and exposes no arbitrary property access. ParseOptions only
adds a const borrow of its ConfigurationHandler for the package-name check.

The TeX input domain is boolean/string/RegExp. Boolean fields use the incoming
primitive's truth value; regular-expression fields require an actual RegExp;
`digits` retains its existing string/RegExp union. Resource limits are numeric
comparison thresholds stored as native doubles, including fractions, negative
values, NaN and infinities. Counters and allocation sizes remain unsigned
integers. The setter only converts the numeric input; ParseUtil and the array
parser perform their original comparisons. It must not duplicate those
comparisons by projecting thresholds to integer capacities in the setter. Strings
remain strings, with `noundefined`'s false styling projected to an empty string
to preserve its disabled-style output. Arrays, maps and function callbacks are
configured through their existing typed C++ fields, not synthesized from TeX
strings. The nullable reference formatter may be cleared by false/empty input,
retaining its native callback type. Incompatible writes produce InvalidValue
rather than reviving arbitrary JS type mutation. Permission and field-presence
checks still precede conversion,
and successful earlier writes remain when a later key fails, as in Source.

### Empheq table transformations (2026-10-20)

The original `EmpheqUtil.ts` remains `empheq/util.*`: option parsing, allowed
environments, column counting, copied table/first-row phantoms, side cells and
table adjustment. `EmpheqConfiguration.ts` remains `empheq/configuration.*`:
Empheq environment entry/exit, token commands, delimiter commands and maps.
There is no separate Empheq item class, method module or table-layout service.

Utilities receive MmlNode references and TexParser references; the existing MML
document owns every node. Nested TeX parsers are automatic local objects that
borrow the current ParseOptions and copy the current environment. Snapshot an
inferred row's child pointers before reparenting them. Table copies use the
original ParseUtil::copyNode, preserving node-list registration. The outer
phantom in topRowTable uses the MML factory as in Source, while other new nodes
use the configured parser creators. BeginItem owns only its optional left/right
TeX strings and existing nestStart flag. StackGlobal.empheq and the environment
closing marker retain their existing owners; tags use the existing equation
state. No universal property store or parser/node identity registry is added.

False and empty side values disable that side; false is represented by the
empty native TeX string. Source bare true sides fail in TexParser.trim, and
equation* with only a right side fails on missing columnspacing. Native calls
reject these inputs with invalid_argument, without emulating JS exception
objects. Record them separately from successful MML comparisons and verify
the following numbered equation and label state recover.

Acceptance must compare actual Source MML for all six permitted environments
and their starred forms, left/right/both sides, labeled/ragged/empty tables,
all 32 fixed symbols and four delimiter commands, labels/tags, option parsing,
environment argument splitting, nesting/errors and the next parse. Direct
utility tests also check copied-node independence, parent links and all moved
inferred children. These package checks do not prove final Scene/SVG integration.

The original `input/tex/ParseMethods.ts` belongs at `input/tex/parse_methods`,
not under `base/`. Its ten compiled methods handle variables, numbers, control
sequences, mapped character tokens and the environment entry. BaseMethods,
BaseMappings and BaseConfiguration retain their separate upstream duties.
ParseUtil owns getFontDef and Unicode-normalized Latin/Greek classification;
TexParser passes the existing NodeDefinition through its configured creators.
Properties such as autoOP must reach that creator before construction, whereas
mathchar0mo sets fixStretchy and registers the node after construction, as in
Source. Token attributes remain owned values, with absent versus empty explicit.
TexEnvironment additionally carries the specific multi-letter RegExp reference
and noAutoOP flag. Shared expression state models the actual configured pattern;
it contains no parser, frame or document owner and creates no object registry.

BaseMethods keeps all 75 upstream members in the original base/base_methods
module. The 70 character/macro members are compiled MacroHandler functions
`DispatchOutcome(TexParser&, MacroInvocation const&)`; Array, AlignedArray,
IndentAlign, Equation and EqnArray are EnvironmentBodyHandler functions returning
StackEntry with a borrowed parser, real begin FrameHandle and TexArgument span.
Definition arguments use the existing finite TexArgument types, never Property,
any, FunctionRef or a runtime method object. Original mathematical routines are
adapted in place, not retained beside a new dispatcher. ParseUtil owns its
shared text/under-over/alignment/equation/expansion helpers. BaseMappings owns
the pinned table data and direct binding of those compiled functions; package
configuration and filters remain BaseConfiguration responsibilities. Review
checks all original methods and real configured-parser trees/errors against
the pinned source, with no success stubs for pending members.

The input-jax compilation service uses the original core/input_jax owner with
compile-time Options/Data parameters. Its filter record borrows MathItem,
MmlDocument and the concrete ParseOptions; filters execute synchronously and
retain no record identity. TeX owns its configuration, ParseOptions and session;
the compiled registries/callbacks and current MML factory are borrowed services.
The caller supplies the document owning each compilation's nodes and keeps its
factory alive until rebinding or jax destruction. TeX never accumulates MML in
a session arena. Its compilation executes the original package filters and seven
FilterUtil passes before Common sees a tree. FilterUtil has its own original
module, rather than continuing as free functions mixed into tex_jax.cpp.
HTML delimiter discovery/DOM adaptor methods are outside this string compile
service and remain explicit pending owners; they are not replaced by fake DOM
results or called through the withdrawn object layer.

The token creator takes `optional<u16string_view>` through TexParser and
NodeFactory. Explicit null text creates no TextNode; an explicitly empty text
and the omitted-text default create an empty TextNode. This is the original creator collaboration,
including the configured boldsymbol token creator, rather than a second node
construction path for hidden delimiters. Boldsymbol owns its command mapping,
scoped argument parsing, token marking and priority-5 variant rewrite in its
original configuration module. Its scope is restored before pushing the result.

C++ module boundaries follow owners and dependencies. A packet, individual
function or test seam does not automatically need its own named module.
Family implementation details and numerical intermediates remain private
unless a real production consumer needs them. Native ownership changes remain
typed values, unique owners and explicit borrowed references within a request.

The Source class collaborations are requirements too, not only the file paths:
`CommonWrapperFactory` owns request-local wrappers; `CommonWrapper` owns common
wrapper state and algorithms; `CommonMfrac` specializes fractions;
`CommonMsqrt` specializes shared radical behavior; `CommonMroot` extends
`CommonMsqrt` with degree behavior. Preserve their Source method responsibilities
and initialization/call order using native C++ classes and RAII. Do not replace
this collaboration with a public collection of scalar-input/result functions.
The numerical consolidation in b23b2b46 does not implement these classes and
does not satisfy wrapper architecture acceptance. Reusable equations may become
private implementation details of their owning family during that port; their
current exported input/result/function surface is not the final wrapper API.

## Ownership

### CSS stylesheet dependency for Common (2026-10-20)

Keep `StyleJsonSheet` in existing `util/style_json.ixx/.cpp`; no Common-local
stylesheet implementation. Its native CSS tree is `StyleEntry {u16string name;
variant<u16string,double,vector<StyleEntry>> value;}` and `StyleJson` is the
ordered vector of entries. Leaves are CSS strings/numbers; groups are selector
definitions including nested rules. This is not a general runtime object value.
Copying owns a value snapshot. Merging updates named declarations in place,
appends new names, and replaces nested definitions shallowly as Source does.
Preserve constructor/addStyles/removeStyles/clear/cssText/getStyleString/
getStyleRules/getStyleDefString and exact Source whitespace/numeric spelling.
Use overloads for omitted getStyleRules input, not a Missing/null sentinel.
Acceptance compares actual pinned StyleJson.ts execution for replacement order,
removal/reinsertion, nested @media/@font-face, numbers, empty groups and UTF-16.
Arbitrary JS alias/prototype/coercion tests remain withdrawn; CSS output and
merging behavior remain required. Common shared declarations are governed by
`common-native-implementation.md`; drafts do not establish compiled acceptance.

### Typed factory and actual MML family decision (2026-10-20)

The generic `Tree/Factory` and `Tree/Wrapper` class roles remain. Replace their
reflective records in the existing files; do not bypass these bases in Common.
`FactoryServices<Product, Kind, Arguments>` and
`AbstractFactory<Product, Kind, Arguments>` use one ordered registry of
`FactoryClass<Product, Arguments>` values. Each record contains only a typed
`unique_ptr<Product>(Arguments const&)` creator and a typed instance predicate.
There is no second creator object/cache, universal argument vector, class-object
identity, source-call bridge or lifecycle lease. Replacing an entry retains its
position; deleting and re-registering appends it. `getNodeClass` returns an
optional record value, avoiding a registry reference that could be invalidated.
An explicit optional default kind preserves requested-kind/default-kind
selection; absence of both is an error. `nodeIsKind` uses the registered C++
instance predicate, preserving subclass checks rather than comparing kind names.

The generic factory constructs and passes the unique owner to a concrete
`adopt(unique_ptr<Product>)` hook. Common's concrete factory owns the wrapper
arena. Node factories transfer ownership to their document. A null creator or
product is an error, never a placeholder node. Product initialization and MML
indexing remain the concrete factory's responsibility after ownership transfer.

`WrapperServices<Product, Source>::wrap(Source&, Product* parent)` and
`AbstractWrapperFactory<Product, Source, Arguments>` retain the source-kind
dispatch and delegate to this same registry. The wrapper-factory base calls a
typed `creationArguments(Source&, Product*)` override; concrete Common arguments
contain its actual factory reference, MML node reference and borrowed parent.
`Wrapper`/`AbstractWrapper` retain `kind`, `wrap` and preorder `walkTree` in their
existing files. The base stores the one borrowed source pointer and one vector
of borrowed wrapper children. A native visitor captures any typed caller state;
it does not take `std::any` or a simulated JS argument/array. Common must not add
a parallel child collection or creator registry.

For MML, retain the current nonmovable document arena, typed attributes,
properties, intrinsic state and tree invariants. Restore the original C++
`MmlNode`/`AbstractMmlNode`/token/layout/base/empty classes and concrete family
inheritance as actual polymorphic arena objects. `MmlDocument` keeps one
`vector<unique_ptr<MmlNode>>`; its destructor uses the virtual node destructor.
The former final structural `MmlNode` has been replaced by actual polymorphic
family objects in the document arena. Do not
add a separate semantic façade, detached family object, NodeKind switch in each
consumer, or a fallback implementation of missing semantic methods.

Reuse the original C++ methods and replace their typed inputs/ownership in
place. Node-kind identity becomes `NodeKind`; family/class tests retain the
Source subclass relation. Parent and child storage have one authority, and
structural mutations use the document's existing ownership/cycle checks. The
MML family owns core/embellished/spacing, TeX classification and inherited
attribute algorithms. Common calls these real methods. The document/node named
module may keep the shared base declarations in its existing `:node` partition;
concrete family modules import that primary, and document implementation units
may import the families for construction without creating interface cycles.

Acceptance of this boundary requires typed factory registration/replacement/
fallback/subclass/ownership tests paired with pinned Source, and real factory
construction of family objects with Source semantic cases. Passing generic
factory tests alone is not Common/MML pipeline acceptance. Native MML family
signatures and the tree-node ownership interface must be reviewed before family
implementation assignments; do not independently invent these in each packet.

The existing `tree/node_types.ixx` and `tree/node.ixx` retain `Node`,
`AbstractNode` and `AbstractEmptyNode`. Their native template shape is
`Node<Product, Kind>` and `AbstractNode<Product, Kind, Base=Node<Product,Kind>>`
(likewise Empty). The one protected `Product* parent_` and
`vector<Product*> children_` live in `Node`, not again in MML. A borrowed
`NodeOwner<Product,Kind>` interface supplies document operations: `setChildren`,
`append`, `replace(parent, oldChild, replacement)`, `remove`, `clone(source,
keepIds)`, `nodeIsKind` and `kindName`. The base node's protected `treeOwner()`
returns that owner; `MmlDocument` implements the interface using its existing
checked tree algorithms. This is a typed ownership boundary, not a second tree
implementation or a lifetime registry. Source per-node methods delegate to it.

`AbstractNode` retains typed `isKind`, `setChildren`, `appendChild`,
`replaceChild(newChild, oldChild)`, `removeChild`, `childIndex`, `copy`,
`findNodes`, `walkTree` and `toString`. Subclass checks delegate to the same
registered native class predicates. Missing-child replace/remove remain Source
no-ops; document ownership/cycle checks still apply to actual edits.
`AbstractEmptyNode` retains its Source no-child overrides. Generic JS property
methods are retired; mathematical property values remain in the existing typed
MML `NodeProperties`, and cloning them belongs to the document. No `std::any`,
property bags, source readers/writers or callable receiver abstraction remain.

`MmlDocument` owns its one `MmlFactory` and exposes it by borrowed reference;
TeX `NodeFactory` and callers borrow that factory. Raw document construction
delegates to `MmlFactory::allocateRaw`, the single concrete-class registry, and
document adoption. `MmlFactory::make(kind, attributes, children)` keeps current
validation/default/arity responsibilities and calls `allocateRaw` directly.
It must not call back into `document.make`. Raw construction continues to omit
defaults, preserving the existing low-level API; ordinary parser construction
uses `MmlFactory::make`. Exact `cloneSubtree` and Source `node.copy(keepIds)`
retain their different, already identified contracts.

Inherited attribute inputs are explicit records `{name, origin: NodeKind,
value: AttributeValue}`. Origin is required for the Source `noInherit` rules.
Plain attribute additions have no origin until `addInheritedAttributes` stamps
the current node kind, excluding displaystyle/scriptlevel/style. Inheritance
display/level/prime inputs are native bool/double/bool with false/0/false
defaults. Verification omission means no verification, so the typed API keeps
an explicit absent-options call form distinct from `VerifyOptions{}`.

Foreign XML/HTML keeps its existing `XmlObject`, `XmlAdaptor`, `XMLNode`,
`HtmlAdaptor` and `HtmlNode` responsibilities; do not invent another DOM or an
unrelated snapshot API. `XmlRef` owns an immutable, XML-domain payload;
`XMLNode` retains a shared const typed adaptor owner. This protocol has only
clone/serialization and the actual HTML string-attribute/content operations,
with native strings, optional strings and vectors of XML references replacing
Property/PropertyList. Adaptors retain no MML/parser/wrapper state or references.
This is a foreign-content value boundary; Common layout does not construct DOM
nodes. Token text reads real `TextNode`/`HtmlNode` virtual methods rather than
reflective `textContent` fields. SVG consumes serialized content and measured
runs through its existing foreign-content contract.

| Lifetime | Owner | Owned contents | Permitted references |
|---|---|---|---|
| Process/profile | immutable `FontRepository`, `PackageRegistry` | font tables/outlines, native handler registration | shared const data; finite declared cache bounds |
| Renderer | `Renderer::State` / `TexSessionState` | macros, labels, tag counters, explicit package session state | value data or independently owned persistent mathematical data |
| One render | `RenderRequest` | `MmlDocument`, parser stack, package scratch, wrapper arena, layout scratch | references into this same request; no persistence |
| Returned result | `RenderResult` | SVG bytes and immutable Scene | owned values and shared const font resources only |

`MmlDocument` is a non-copyable, non-movable typed node arena, holding
`vector<unique_ptr<MmlNode>>`. Its root, parent and child links borrow stable node
addresses within that document. It owns no generic object records. Nodes cannot
be copied/moved independently, cross-document links are rejected, and tree
operations preserve the declared parent/child invariants. Detached construction
nodes still die with the one document. This avoids per-node retirement lists.

The parser's detached nodes and stack items belong to this request. Wrappers
borrow MML nodes and are destroyed before the MML document. Scene freezing
copies the required geometry/metadata and retains no MML, parser, wrapper,
request, mutable package-state or DOM pointer. Exceptions, cancellation and
early returns unwind the same RAII owners; cleanup has no retry protocol.

Persistent macros/labels are explicit semantic state. They cannot capture a
request through `std::function`, borrowed pointers, an untyped payload or an
identity registry. If a package genuinely retains mathematical content, its
typed session state owns a deep value copy or an independent MML document.
Source effects before a TeX error remain governed by MathJax's behavior;
RAII cleanup must not invent a rollback of persistent semantic changes.

## Native interfaces

Public `folia.math::Renderer`, `RenderOptions`, `RenderResult` and the plain C
UTF-16 bridge keep their external roles. `math.cpp` only assembles services and
orchestrates the pipeline. It implements no TeX commands or layout rules.

The foundation contracts are:

- `mml::NodeKind`: explicit mathematical node kinds, including token, inferred
  row, table, scripts, annotations and HTML content required by the profile.
- `mml::AttributeValue`: `variant<AttributeNull, bool, double, u16string>`.
  Attributes are ordered mathematical name/value data with explicit own/default/
  inherited lookup. There are no executable getters, prototypes or callables.
- `mml::MmlNode`: typed node kind, token text, attributes, node properties and
  document-local tree links. Node-family algorithms remain family modules.
- `mml::MmlDocument::make(NodeKind) -> MmlNode&`; `setRoot`, `append`, `insert`,
  `replace`, `remove`, `cloneSubtree` act on nodes owned by that document.
- `tex::TexSessionState`: owns macros, labels and tag counters. Its phase-one
  module is `folia.math.input.tex.session`, independent of the parser facade.
- `tex::ParseOptions`: references typed request/session services; owns no global
  object registry. `NodeFactory` borrows its document's `MmlFactory` and typed arguments.
- Package handlers have named typed signatures such as
  `void(TexParser&, ControlSequence const&)`; configuration is typed native
  registration, including explicitly typed lifecycle callbacks where required.
- `scene::Scene`: immutable, owned grouping/transform/glyph/rule/path/text/clip/
  interaction/foreign-content values with layout metrics and semantic metadata.
- `svg::write(Scene const&, SvgWriteOptions const&) -> string`: serialization
  only. It cannot import the parser, MmlFactory or CommonWrapper.

Source options are represented by explicit option structs. Package-specific
extensible data may use a closed data-only variant with owned strings, arrays
and maps; it cannot contain functions, prototypes, object handles, arbitrary
`std::any`, property traps, or a universal invocation API. Mathematical attribute
maps and macro dictionaries are legitimate domain data, not JS objects.

`parse_options.ixx` contains the finite `TexOptionValues` schema and the original
ParseOptions service declaration. It covers the 42 option fields observed in
the pinned profile through explicit scalar, callback and package-specific
records. Each optional distinguishes absence from false, zero or an empty
string. Source callback options are fixed C++ function pointers; macro and
environment options reuse the existing owned definition values. The schema is
not a parser, a package implementation or proof that these callbacks are bound.

`mergeDefaults` follows Options.defaultOptions: later present fields replace
earlier ones, package records and named dictionaries merge recursively, and
arrays are copied/replaced. It does not mean "fill missing fields", and it does
not implement userOptions warnings or the string-facing setoptions dispatcher.
Permission records retain the distinction between a denied package and a
per-option permission map. Real RegExp state remains shared where Source keeps
the same regular expression; no general runtime object model is involved.

The input jax must own one effective option record. Requests borrow it and the
math-style function captured at input-jax construction. Later mathStyle option
updates do not select a new resolver. `ParseOptions::getVariant` implements the
four actual Source functions, including Unicode 16 NFD for French style. Its
constructor and configured factory/Tags/ColumnParser collaboration are connected
to the real TeX input jax. The actual parser primary exports the declaration/value
and map families; it does not export or call the withdrawn Context runtime.

`macro.*`, `token_map.*` and `map_handler.*` keep the original Macro, token-map,
registry and subhandler responsibilities in that same parser primary. Macro
arguments are owned TeX values. `MacroCallbacks` copies two fixed typed function
tables at input-jax creation and has no rebinding/registration API; definitions
use stable one-based IDs and can recover a compiled pointer or its binding.
The actual 38-package callback table still has to be supplied by its native
configuration code. An invalid binding fails instead of selecting an erased
callable or returning a fabricated parse result.

MapHandler owns only each name's current real TokenMap. Selected SubHandlers
share those map owners, so a replacement does not invalidate an existing
configuration; after its last real owner releases it, the old map is destroyed.
Do not keep a vector of all past replacements. The ignored early declaration
packet used that retention scheme and is superseded here. A lookup key remains
separate from the stored token, as needed by let aliases. In-flight callback
arguments use value snapshots so replacement does not leave dangling borrows.
Subhandler lookup/composition, map replacement, real TexParser callback execution
and CommandMap's currentCS wrapper are Source-checked. CommandMap retains its
original wrapper responsibility rather than being replaced by MacroMap.

Newcommand keeps its configuration, mappings, methods, utility and BeginEnvItem
in the original five module owners. TexSessionState owns the sole command,
delimiter and environment definitions; session-backed TokenMap instances borrow
that data and are used only while their input jax is alive. Definition snapshots
protect in-flight calls from redefinition. Plain parameter counts are doubles,
without allocating a delimiter array for each declared argument; only a def
template requires at most nine sequential parameters and matching delimiters.
An explicit parameter declaration records whether substitution runs, including
[0] with a default argument. Suppressed commands invoke the configured fallback;
suppressed delimiters retain a null character and produce an empty mo. Global
definitions use the existing BegingroupStack over the same owned definition
layers. This does not claim that the begingroup package methods are connected.

Node construction retains the original four creator slots: node, token, text,
error. `NodeCreators` contains optional typed C++ function pointers, and the
factory merges only provided slots. Default createToken calls the configured
text then node slots; createError does likewise. Only the public node route
registers a completed result, after its creator returns. Null text differs from
an empty text node. These rules stay in NodeFactory. `NodeDefinition` and the
texClass/movablelimits/property application rules belong to the original
NodeUtil, not to the factory or renderer.

`NodeFactoryConfiguration` is the borrowed native ParseOptions protocol for the
current mathematical environment and kind/named node registration. It contains
no arbitrary keys, payloads or invocation service. The boldsymbol owner keeps
its configured token creator and variant rewrite; the token creator delegates
to the default token builder before marking/registering fixBold. Its mtext
branch does not read the parser environment. Its span-based rewrite, TexFilterData
entry point, live-node filtering, package initialization and scoped macro parsing
are connected and tested with the real TeX input jax, in addition to creator unit tests.

## File responsibility map

| Files | Responsibility |
|---|---|
| `src/math/math.ixx`, `math.cpp`, `native_renderer_bridge.cpp` | public facade, request orchestration and C ABI |
| `src/math/input/tex/tex_jax.*` | TeX input-jax lifecycle and persistent-state boundaries |
| `src/math/input/tex/session.ixx`, `session.cpp` | native macros, labels, counters, explicit session ownership |
| `src/math/input/tex/tex_parser.*`, `stack*`, `parse_options.*` | token consumption, parser scopes, stack items, request references |
| `src/math/input/tex/node_factory.*` | TeX node/token/text/error construction |
| `src/math/input/tex/<package>/` and existing package-named units | their pinned package syntax and algorithms |
| `src/math/core/mml_tree/node_kind.ixx`, `mml_node.*`, `mml_document.*` | typed node vocabulary, document ownership and tree operations |
| `src/math/core/mml_tree/attributes.*`, `mml_factory.*` | mathematical attribute inheritance and typed node construction |
| `src/math/core/mml_tree/mml_nodes/` | node-family validation, inheritance, TeX classes, operator semantics |
| `src/math/output/common/wrapper.*`, `wrapper_factory.*`, `wrappers/` | one upstream-aligned wrapper family hierarchy; BBox, stretching and shared numerical placement; fraction/sqrt/root remain in their canonical family files |
| `src/math/output/common/font*`, `src/math/fonts/` | typed font lookup, metrics, stretch data and bounded immutable resources |
| `src/math/output/scene/types.ixx`, `scene.ixx`, `scene.cpp`, `builder.*` | owned completed layout representation and freezing; no second layout hierarchy |
| `src/math/output/svg/writer.ixx`, `writer.cpp`, `node_metadata.cpp` | Source-compatible SVG serialization and metadata emission |
| `src/math/util/bbox.*`, numeric/string/length/Unicode utilities | pure mathematical/text operations with typed inputs |
| `src/platform/`, `src/app-winui/` | platform rendering, worker, cache, cancellation wiring |

## Removal and reuse rules

Remove the runtime object graph: `option_types`, `option_storage`, `option_object`,
`option_array*`, `option_iterator`, `context_property_bindings`, `native_identities`,
generic property/callable graphs, JS class/prototype services, Context module
registries, and their Context/bindings/host projection units. Remove imports of
these from algorithm modules rather than renaming or hiding them behind adapters.
The exact file inventory must account for every current math source file.

Existing Scene `ChildOwner` sparse arrays, Source identity projections and
`scene/wrappers` duplicate placement are not retained in the new Scene model.
An SVG DOM/Lite-adaptor simulation cannot sit between layout and SVG writing.
HTML/annotation support uses explicit owned content and measured runs, while
preserving the required Source output; it cannot reintroduce generic JS objects.

Pure algorithms and pinned font/Unicode datasets may be reused after their
transitive imports and ownership are inspected. A file is not reusable merely
because its name resembles a native utility. Source provenance and licenses
remain recorded. Node/CJS scripts may generate data and provide development-time
oracles; no JS engine or generated JS bundle executes in native math rendering.

## Integration policy

### TeX request and stack implementation (2026-10-20)

The original `StackItem`/`MmlStack`, `StackItemFactory`, `Stack`, `TexParser`,
`ParseOptions`, `Tags`, `base/BaseItems` and `ColumnParser` responsibilities now
have native implementations in their corresponding files. The obsolete flat
`input/tex/base_items.cpp` implementation is removed; `base/base_items.cpp` is
the sole owner of the 26 base frame algorithms. Factory construction uses the
existing typed class registry and returns activation handles without retaining
a product history. Stack owns the actual activation slots and keeps the top
and incoming handles alive through a check/pop/replay transition.

`TexScanner` remains the sole owner of the parser's immutable source snapshot,
UTF-16 cursor and current control sequence. `TexParser` delegates its lexical
methods to that owner and preserves the original dispatch, stack and LaTeX
provenance algorithms. Its private scope member registers before constructing
Stack and unregisters the exact parser on exceptions or normal `mml()` exit.
It never retains a parser in a registry after that activation ends.

`ParseOptions` borrows the effective input-jax options, package configuration
and session. It owns its factories, column parser, tagging service and
equation scratch. Its node lists contain document-owned MML borrows, preserve
registration duplicates and filter by the current root's parent chain. The
finite `in_lists` node property records additional postprocessing memberships;
it is copied by the original MML clone operation. It does not represent a
second node owner or a generic property bag. `clear()` clears only the original
ephemeral fields and tag scratch, retaining session definitions and options.

`Tags` uses the one equation record and the existing session counter/labels/ids
APIs. Numbering, nested environments, references and equation recompile state
remain on those owners. Formatted tag content invokes the actual TexParser;
there is no replacement text parser inside Tags. Source `enTag` table creation
uses the configured NodeFactory. Tag creators and the default name belong to
the actual ParserConfiguration instead of a process-wide JS class registry.

TeX assembles script nodes by physical child slot before input-jax postfilters
select the final single/double script family. `MmlDocument::setChildSlot` owns
this operation, validating document ownership and parent links. An absent slot
is a null mathematical child, not a generated placeholder or a JS array object.
Exact cloning preserves these slots. Completed layout must receive the result
of the original TeX postfilters; this addition does not authorize incomplete
script nodes to flow into Common layout.

The original ParseUtil fence/palette/initial-operator helpers and NodeUtil
child/text/operator helpers are implemented on their original owners. Token
text delegates existing MML virtual methods so HTML content retains its
meaning. No BaseItems-local copies of those helpers are introduced.

These implementations are component progress. Native BaseMethods, package
mappings, the input-jax compile/filter phases and full nested tag parsing are
still required. Tests using a small configured callback map exercise the real
TexParser collaboration, not the complete selected package profile. Source
full-profile parses are reference evidence only until the corresponding native
pipeline produces and compares their results. Full TeX-to-SVG/Folia acceptance
remains open.

### TeX frame construction and transfer (2026-10-20 Root lock)

Keep the existing generic Tree Factory registry and the actual StackItemFactory
role. MML and Common factories return document/job-owned borrowed products;
StackItemFactory must instead return a `unique_ptr<BaseItem>` to the caller.
It must not retain every constructed frame in a request arena or shared_ptr
registry. Stack owns accepted frames and releases discarded frames naturally.

Extend the existing `FactoryServices` and `AbstractFactory` templates with a
fourth result type parameter, defaulting to `Product&`. The registry still
constructs one `unique_ptr<Product>` using the same typed FactoryClass and
calls `adopt`; both `create` and `adopt` return the selected result type.
StackItemFactory selects `shared_ptr<BaseItem>` and its adopt operation converts
the newly constructed unique owner once to a frame-activation handle. Existing document/wrapper factories keep their
current reference-returning ownership contract. This is a compile-time native
ownership signature, not a runtime migration switch, duplicated registry or
compatibility path. Construction, registered order, fallback and predicates
remain the already verified Factory algorithms.

The earlier unique-owner-only frame draft is superseded by new concrete
Source evidence: CasesMethods.NumCases calls BaseMethods.EqnArray, which pushes
the begin frame, then pushes that same frame again. The actual stack contains
two simultaneous slots for one CasesBeginItem; its mutable fields, environment
and globals are shared. Cloning, deduplicating or moving away the sole handle
would change this configured package. Root replayed the actual Source methods
in the formal `tools/math-port/tex-frame-activation-source.cjs`, with explicit argument/
tag hook limits. This finding concerns activations, beyond the earlier review
of checkItem replacement lists; it does not invalidate those observations.

Use `FrameHandle = shared_ptr<BaseItem>` only for actual frame activations.
Factory callers, Stack slots and transition locals may share that handle;
frames never own their parser, factory, outer Stack or document. There is no
factory product registry, request frame arena, raw-pointer adoption or
shared_from_this bridge. The last slot/local release destroys the frame by
normal RAII. MML remains document-owned and environment remains a finite value.

For Stack transitions the active top stays alive during checkItem, which takes
`FrameHandle const& incoming`. ProofTree passes a handle into its inner Stack
at the actual Source call site before returning Drop; outer Drop releases only
its local reference. A typed replacement list contains newly created handles,
reuse-top/reuse-incoming tokens and borrowed MML. Stack first executes the
actual Pop/environment transition, then resolves those tokens with the original
handles still available, then recursively pushes entries in order before
continuing the outer input sequence. Aliased slots still see the same frame's
mutations. Do not add a universal object ownership or identity service.

The actual Source observations are in ignored `tex-stack-transition-source/`:
24 cases and 88 hash-locked files, replayed by Root. Left/middle reuses top;
Over/Style/Position/Prime/Nonscript can reuse incoming; Prime also returns bare
MML. Source `delete item.env` does not clear BaseItem's inherited accessor
backing `_env`. Thus a surviving replayed/popped frame's environment cannot be
cleared based on the source statement alone. The rest of parser/ParseOptions
declarations remain to be frozen separately.
This contract does not claim a native parser or the full package profile works.

Use the existing closed `TexEnvironment` value record for environment contents.
Stack and BaseItem share a `shared_ptr<TexEnvironment>` only where Source has
one active environment scope: a non-open accepted frame aliases that scope;
an open frame starts with a fresh empty record and overlays present fields from
the previous scope when copyEnv is true. ArrayItem has copyEnv=false. Start
receives the already copied parser seed. BaseItem.clearEnv clears the existing
record in place (`*env = TexEnvironment{}`), so existing scope aliases observe
the cleared fields. The distinct explicit `this.env = {}` in Left/middle
installs a fresh record before replay; do not conflate those operations.
Pop restores the remaining top's scope or a
fresh empty scope; it does not erase the popped frame's record. This finite
domain value contains declared bool/number/string optionals and the specific
multi-letter regular-expression reference, with no
parser/frame/MML owner, callbacks or universal property lookup, so it cannot
form an ownership cycle. Copy operations copy the values rather than sharing
the new open scope. StackGlobal is a separate typed stack-owned record borrowed
by its frames; it is not an extension namespace in TexEnvironment.

Transition results distinguish Accept, Drop and Replay explicitly. An empty
Replay still pops the old top and is different from Accept. Source errors are ordinary TexError exceptions;
checkItem throws before Stack Pop, so the original owners and active scope
remain valid while the caller handles the error. Native tests must exercise
success and throw cleanup, scoped environment aliasing/copying, same-top replay,
incoming replay and bare-node replay against the actual Source observations.
They must also check actual CasesBeginItem aliases, shared mutations, and final
destruction after the last slot/local releases, including nested-stack Drop.

TexParser remains the owner of source positions and stamping StackItem.startStr,
startI and stopI at the actual Push call. Its existing TexScanner is the sole
owner of the current UTF-16 source through `shared_ptr<const u16string>` and
exposes a `sourceSnapshot()` value for those immutable frame snapshots. There
is no parallel current string in TexParser. `replaceSource` installs one new
immutable buffer and retains its existing cursor semantics (no implicit reset);
old frame snapshots remain valid until their last owner releases them. Cursor
reset is the explicit parser expansion operation, not a scanner side effect.
The buffer contains text only, never parser/MML/frame owners or callbacks.

There will be one native math implementation and one `folia_math` target.
No `native2`, old/new selector, runtime compatibility adapter or legacy fallback
is introduced. Replacements use the canonical file paths/module responsibilities.
During reconstruction, incomplete layers are reported as incomplete; they cannot
silently call the withdrawn Context implementation or return fake SVG success.
Root switches the target to the reviewed native dependency graph as a deliberate
source change, and removes obsolete runtime files/tests in reviewable batches.

Folia retains its current math worker until the complete native SVG path passes
the new acceptance gates. Then it switches once through the C ABI. Mermaid is
unchanged and paused. An additional D2D pixel comparison is not a gate for this
SVG integration; the future D2D backend remains a separate consumer of Scene.
