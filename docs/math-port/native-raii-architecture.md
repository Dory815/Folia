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
been consolidated, but the numerical APIs still have only component-test
callers. Complete CommonWrapper orchestration and the renderer facade remain
outside the active graph; it cannot render TeX end to end. A successful
component build does not resolve that integration gap.

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
The current final structural `MmlNode` is not the final representation. Do not
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
  object registry. `NodeFactory` takes `MmlDocument&` and typed arguments.
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
