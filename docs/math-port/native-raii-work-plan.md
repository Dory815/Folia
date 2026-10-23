# Native RAII work allocation

Root owns the architecture in `native-raii-architecture.md` and acceptance in
`native-raii-acceptance.md`. These are implementation constraints, not a request
for agents to choose separate architectures. All older Context retirement,
prototype-fidelity and runtime-lifecycle packets are withdrawn from promotion.
They may remain as ignored historical evidence; they are not dependencies.

Current execution state and next action are only in HANDOFF.md. The allocations
below are historical; they do not supersede later architecture locks or the
accepted Renderer/WinUI worker. Remaining cleanup must account for each inactive
file and preserve its independent mathematical/owner tests before retirement.

## Historical first implementation wave

Agents write complete candidate files/patches in separate ignored packets.
They must target the canonical paths below and use the current worktree bytes
as their base. Root alone promotes files, changes CMake, builds, runs native
executables and commits. No agent writes another agent's formal or draft files.

| Work packet / owner | Exact implementation scope | Deliverable and acceptance |
|---|---|---|
| MML foundation / Luna | `core/mml_tree/node_kind.ixx`, `attributes.ixx/.cpp`, `mml_node.ixx/.cpp`, `mml_document.ixx/.cpp` | typed attributes, nonmovable document arena, document-local links, complete focused UT; no old tree/property imports |
| Session and parser contracts / Luna | `input/tex/session.ixx/.cpp`, typed parser/NodeFactory/ParseOptions/package-registration interface proposals | real macro/label/tag state implementation + UT, exact ownership/signatures, Source persistence cases; no Context |
| Scene foundation / Luna | `output/scene/types.ixx`, `scene.ixx/.cpp`, `builder.ixx/.cpp` | owned immutable primitives/metrics/metadata, complete builder/freeze implementation + UT; no MML/JS imports or SVG strings |
| SVG consumer / Luna | `output/svg/writer.ixx`, `writer.cpp`, `node_metadata.cpp` | serializer of the Scene contract, ordering/escaping/numeric-format tests and Source provenance; no layout calculations |
| Common/font dependency audit / Luna | current `output/common`, `fonts`, pure math utilities and Source SVG-placement routines | exact per-file keep/rewrite/delete and placement ownership mapping; identify reusable pure algorithms with transitive dependency evidence |
| Full graph inventory / Luna | `tools/math-port/native-raii-inventory.cjs`, proposed `docs/math-port/native-raii-files.json` | enumerate every current source/module/import and disposition; list runtime/test/target retirement without deleting mathematical coverage |

## Locked cross-packet boundaries

- MML packet defines `folia.math.core.mml_tree.node_kind`, `.attributes`,
  `.mml_document`; namespace `folia::math::mml`. `mml_node.ixx` is its exported
  `:node` partition: mutually referencing node/document classes belong to the
  same named module. Consumers import `.mml_document`. Other agents do not
  invent alternate nodes, handles or attribute values.
- Session packet defines `folia.math.input.tex.session`; namespace
  `folia::math::tex`. It does not implement a second parser. Parser interface
  proposals remain proposals until Root reconciles them with MML.
  `TexEquationState` is a request-owned value; persistent session state does
  not contain current-equation scratch. Macro expansion belongs to ParseUtil.
- Scene packet defines `folia.math.output.scene.types` and
  `folia.math.output.scene`; namespace `folia::math::output::scene`.
  A separate `builder` module may import both. It publishes its concrete
  primitive/header contract first for the SVG agent to consume.
- SVG packet defines `folia.math.output.svg.writer`; namespace
  `folia::math::output::svg`. It consumes Scene by const reference and cannot
  change Scene ownership. Coordinate/number serialization is SVG-specific;
  glyph positions and sizes arrive already calculated.
- Audit packets do not implement bridges into the withdrawn runtime. Reuse
  means removing transitive simulation dependencies, not wrapping them.

## Following waves

### Correction before further implementation (2026-10-20)

The architecture's Source structure correction supersedes the standalone
`common/placement/fraction` and `radical` allocations. Pause further family
promotion until Root has mapped the upstream methods, canonical owning files,
current callers, inactive duplicate implementations and removal/rewiring sites.
Use `common/wrappers/mfrac`, `msqrt` and `mroot` as the family owners; do not
create another hierarchy because an agent packet is independently assignable.
The numerical code now replaces those canonical files in the existing Common
primary module's :mfrac/:msqrt/:mroot partitions. The separate placement files
and three obsolete Scene family pairs/registrations were removed. This is a
preparatory arithmetic consolidation, not completed CommonWrapper orchestration
or a layout-to-Scene consumer. Those remain the next implementation boundary.

The user's subsequent review also rejects treating the numerical free-function
surface as the final design. Restoring paths alone is insufficient. Implement
the Source CommonWrapperFactory/CommonWrapper family and the
CommonMfrac/CommonMsqrt/CommonMroot class responsibilities, inheritance and
calls before claiming this layer complete. Root must resolve the actual base
state, constructor order, typed services and child/font/stretch consumers first;
do not promote additional independently exported mathematical helper modules.

Use the existing C++ implementation as the migration baseline. For the Common
fraction/root classes, the pre-consolidation implementation is available at
b030b874; the other original wrapper files are still in the tree. Preserve
their classes, methods and mathematical bodies while replacing the JS-runtime
type/ownership boundary. Do not translate the same algorithms afresh from TS
when their C++ implementation already exists. Keep specific Source discrepancies
revealed by the newer paired tests fixed (for example the radical temporary
BBox reset); reusing the old code does not mean undoing verified corrections.
Every assignment must identify the existing C++ methods retained and the exact
Context/Property/ObjectRef or ownership dependency being removed.

Root also rejects the generated 4,241/3,954-line test form in `b030b874`.
Retain all 288/192 Source case observations as data, with a reproducible witness
and compact runner. The witness candidates are drafts; no cleanup or native
test acceptance is claimed until they are reviewed, promoted and executed.

Each subsequent assignment must name the Source files/methods, canonical write
scope, producer/consumer contract, deleted/replaced responsibilities, ownership
and focused acceptance. An unconnected component remains explicitly incomplete.

After Root reviews the foundation contracts, separate Luna packets port typed
TeX parser/stack/NodeFactory, MML node families, package families, CommonWrapper/
font services, and per-family placement. Each has an exact file set and paired
Source mathematical cases. No packet may claim full support from a toy slice.
Root maintains missing-feature rows until the original profile is complete.

The build graph is switched deliberately to the reviewed native foundation;
it does not keep both native implementations selectable. Obsolete simulation
sources and JS-only tests are removed in responsibility-sized commits. The
application switch happens only after full SVG and ownership acceptance.
