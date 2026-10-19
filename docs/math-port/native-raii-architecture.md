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

## Ownership

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
| `src/math/output/common/wrapper.*`, `wrapper_factory.*`, `wrappers/` | one wrapper family hierarchy, BBox and mathematical measurements |
| `src/math/output/common/placement/` | placement extracted from Source SVG wrappers; one implementation per family |
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
