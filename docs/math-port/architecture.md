# Native MathJax 4 port: architecture and release gates

Status: architecture specification; the complete native renderer is not accepted.
The authoritative current progress and remaining requirements are in status.md.
Accepted method packets and original observations are recorded under evidence/.
Their bounded results do not accept a complete upstream file or a working renderer.

The normal FoliaTests target builds and links. Restored tagged-tree consumers
currently include P33-P38; mfenced's remaining consumers are the next packet.
Remaining consumers, the complete TeX/package pipeline,
common wrappers, fonts, scene, SVG and Folia integration still block release.
Folia currently uses its existing QuickJS math path. The historical 54/246 ledger
and 90 compiled module interfaces are not completion percentages.

Root owns architecture, scope, interfaces, exclusions and acceptance. Luna agents
implement only assigned existing responsibilities after contracts and bindings
are frozen. Agents cannot narrow the required profile or certify their own work.
Mermaid is paused; its checkpoint and remaining work are in ../mermaid-native/status.md.

## 1. Baseline and scope

The reference is MathJax-src **4.1.3**, commit
`fb987178c2d279a99b0db5ea02e751691703955e`, cloned into
`.reference/MathJax-src`. Font packages are the locked NewCM, bbm, bboldx, and
dsfont packages at 4.1.3. The reference configuration is the Folia configuration
in `tools/mathjax-bundle/entry.cjs`, including every enabled TeX package,
`fontCache: none`, `exFactor: 0.5`, display line breaking and inline fragments.
The package list is a compatibility obligation, not a menu an agent may shrink.

`scope.json` records the reachable upstream source inventory, source hashes,
proposed target files and review status. Reachability is evidence of a
dependency, not automatic authorization to omit or port a file. Root must
classify each item as **port**, **generate data**, or **host adaptation**, with
the adaptation's behavior and reason recorded. Unreviewed items block release.
Browser DOM search, network loading, accessibility UI and CHTML output are
outside the requested raw-TeX-to-SVG API; any reachable dependencies on them
require an explicit host adaptation, not a stub returning plausible results.

Unsupported syntax and missing implementation are implementation failures.
An error rendering or source fallback does not satisfy a required feature.
Invalid-input behavior must itself match the reference profile.

## 2. Required responsibility boundaries

```text
UTF-8 TeX + render options
    ↓
input/tex: configuration + handler maps + parser + stack reduction
    ↓
core/mml_tree: semantic MathML tree, attributes, operator rules, TeX classes
    ↓
input/tex postfilters, in upstream priority order
    ↓
output/common: per-MML-node wrappers, BBox, scale, spacing, stretch negotiation
    ↓
output/scene: immutable positioned geometry and rendering semantics
    ├── output/svg: SVG document/fragment emission
    └── platform Direct2D consumer (a later stage)
```

Fonts are services/data of the common and scene layers; they do not own TeX
commands. A TeX command map is not a font table. The Folia worker is only an
adapter for scheduling, cancellation, caching and result delivery. It does
not parse TeX, assign mathematical attributes, compute formula geometry, or
parse SVG to recover geometry.

Upstream MathJax does **not** already expose a complete backend-neutral scene:
some placement and delimiter assembly is in SVG wrappers. The one intentional
architectural adaptation is to extract those decisions into `output/scene`
with documented one-to-one upstream method correspondence. Common wrappers
remain responsible for measurement. Scene builders remain responsible for
placement/assembly. Backends consume the same completed result and perform
no second mathematical layout. Merely adding two emitters to a custom TeX AST
does not preserve MathJax's architecture.

## 3. File mapping and ownership

Target names follow the upstream tree, translated to snake_case. Each upstream
behavior-bearing file has an individually reviewable target and provenance.
Small registration/data-only files can be generated together only when the
manifest records every source and generation validates every entry. Semantic
nodes and layout wrappers must not be merged into a general switch engine.

| Target files under `src/math/` | Upstream source | Sole responsibility |
| --- | --- | --- |
| `util/options.ixx`, `prioritized_list.ixx`, `lengths.ixx`, `bbox.ixx` | `util/Options.ts`, `PrioritizedList.ts`, `lengths.ts`, `BBox.ts` | Option composition, priorities, lengths, exact BBox math; no parser or backend decisions |
| `core/tree/node.ixx`, `node_factory.ixx`, `wrapper.ixx`, `wrapper_factory.ixx` | `core/Tree/*` | Generic ownership, traversal and registration abstractions |
| `core/mml_tree/attributes.ixx` | `core/MmlTree/Attributes.ts` | Explicit, inherited, per-kind default and global attributes; preserve lookup precedence and missing values |
| `core/mml_tree/mml_node.ixx`, `mml_factory.ixx`, `operator_dictionary.ixx` | corresponding `core/MmlTree` files | Semantic node contract, arity, embellished core, class assignment, dictionary lookup |
| `core/mml_tree/mml_nodes/<kind>.ixx` | corresponding `MmlNodes/<kind>.ts` | That node's defaults, inheritance, verification and TeX-class rules |
| `input/tex/token.ixx`, `token_map.ixx`, `map_handler.ixx`, `configuration.ixx`, `parse_options.ixx` | corresponding `input/tex` files | Configurable character/regex/command/delimiter/environment handlers, priorities and state |
| `input/tex/stack.ixx`, `stack_item.ixx`, `stack_item_factory.ixx` | corresponding files | Push/check/reduce protocol and group/environment ownership |
| `input/tex/tex_parser.ixx`, `parse_methods.ixx`, `parse_util.ixx`, `node_util.ixx`, `filter_util.ixx`, `tex_error.ixx` | corresponding files | Parser driving, token creation, parameter reading, MML helpers, postfilters and diagnostics |
| `input/tex/base/base_{configuration,mappings,methods,items}.ixx` | `input/tex/base/Base*.ts` | Base package registration, tables, command implementations and deferred stack reductions |
| `input/tex/ams/ams_{configuration,mappings,methods,items,tags}.ixx` | matching `input/tex/ams` files | AMS registration, structures, tables, alignment and tags |
| `input/tex/<package>/<file>.ixx` | each enabled extension's matching files | That extension through the shared registration mechanism; never a parser command-name branch |
| `output/common/font_data.ixx`, `direction.ixx` | `output/common/FontData.ts`, `Direction.ts` | Variant graph, SMP pointers, remaps, parameters, delimiters, size/stretch resolution and extension merge semantics |
| `fonts/newcm/common.ixx`, `svg_data.ixx`, `dynamic/*`; `fonts/extensions/<package>/*` | pinned font package files | Source-derived metrics, outlines and configuration data; no heuristic lookup algorithm |
| `output/common/wrapper.ixx`, `wrapper_factory.ixx` | matching common files | Wrapper lifecycle, lazy/cached BBox, inherited scale/variant/spacing, invalidation and dispatch |
| `output/common/wrappers/<kind>.ixx` | matching common wrappers | Exact measurement and layout constraints for that MML node |
| `output/common/line_bbox.ixx`, `linebreak_visitor.ixx` | matching common files | Break opportunities, line constraints, fragments and bounding boxes |
| `output/scene/types.ixx`, `builder.ixx`, `wrapper.ixx`, `wrappers/<kind>.ixx` | explicitly mapped methods in `output/svg/Wrapper.ts` and `Wrappers/<kind>.ts` | Shared completed placements, rules, assemblies, clipping and paint semantics; no TeX recognition |
| `output/svg/{document,serializer,font_cache,output}.ixx` | corresponding SVG emission/cache behavior | Emit the completed scene to SVG and fragment metadata; no bbox or placement calculations |
| `math.ixx`, `session.ixx` | scoped `input/tex.ts`, `output/common.ts`, `output/svg.ts` host orchestration | Thin API and lifecycle orchestration; no feature-specific formulas |

Factory modules register constructors only. Dependencies point from wrappers
to semantic interfaces, and from per-kind registrations to concrete classes.
Dependencies that would form C++ module cycles are broken with injected
factory/service interfaces, not global mutable singletons or a second model.
TypeScript mixins can become C++ inheritance/composition; their method
ownership, dependencies and invariants must remain traceable.

## 4. Contracts root must freeze before implementation

1. **Input and positions.** API input is UTF-8. Parser string-index behavior
   follows upstream UTF-16 semantics; a single explicit UTF-8/UTF-16 mapping
   converts diagnostic locations at the boundary. MML TextNode retains UTF-16
   text; glyph/character iteration uses Unicode codepoints only at the source's
   codePointAt/Array.from operations. This is separate from, and cannot change, the editor's
   authoritative block-local source selection.
2. **Attribute values.** Missing, explicit, inherited and default values stay
   distinct. A typed value must preserve false, zero and empty strings. Style
   is expressed through actual MML `displaystyle`, `scriptlevel`, `mathvariant`
   and related attributes; a four-state style enum is not a replacement for
   scriptlevel, prime style, relative values or inherited attributes.
3. **MML nodes.** Preserve real kinds (`mi`, `mn`, `mo`, `mrow`, `mfrac`,
   `msqrt`, `mroot`, `msub`, `msup`, `msubsup`, `munder`, `mover`, `munderover`,
   `mtable`, `mtr`, `mtd`, etc.), parent ownership, arity and source properties.
   Matrices keep rows and cells; column specifications are never discarded.
4. **Parser configuration.** Maps register handlers with upstream priority.
   Stack items perform reductions and retain environment state. Base and
   extension methods receive the parser/configuration services they actually
   require. No built-in `if (command == ...)` feature router replaces this.
5. **Normalization.** Run the upstream filters in their exact order:
   cleanSubSup (-7), setInherited (-6), checkScriptlevel (-5), moveLimits (-4),
   cleanStretchy (-3), cleanAttributes (-2), combineRelations (-1), plus any
   enabled package filters at their declared priorities.
6. **Font access.** Port the effective variant inheritance/link graph,
   pointers/remaps, parameters and extension integration. Missing direct data
   is resolved by the upstream rules, never an unconditional roman fallback.
   Generated direct records alone do not establish this contract.
7. **BBox.** Preserve `w`, `h`, `d`, `ic`, `sk`, `L`, `R`, `scale`, `rscale`,
   padding/outer bounds and percentage-width behavior. Preserve lazy
   recomputation and stretch-driven invalidation; do not collapse all of this
   into width/height/depth.
8. **Layout.** Common wrappers negotiate row/table stretch and apply per-node
   attributes. Shared scene wrappers preserve the placement equations from
   SVG wrappers. Constants and branch conditions need an upstream method
   reference. A visually plausible approximation is rejected.
9. **Scene.** Record ordered glyph identities/resolved variants/outlines,
   affine transforms, rules, complete clip stacks and nested viewport
   semantics, paints/opacity, semantic annotations and fragments. Coordinates
   are documented em units with exact baseline conventions. The result owns
   or safely references immutable font data. XML node order and mathematical
   draw order must not be conflated.
10. **Results.** Preserve inline/display mode, all fragments, dimensions,
    baseline/vertical alignment, inter-fragment spaces and break opportunities,
    diagnostic classifications and cancellation. Error scenes must follow the
    reference behavior; inventing a red message is not parity.
11. **Lifecycle.** No JS engine, subprocess or reference-oracle invocation in
    the native runtime. A bounded per-request session carries native state,
    cancellation and upstream limits. Node.js is permitted only for development
    generation and the reference oracle.

## 5. Acceptance gates

Root owns the oracle, corpus, expected results and final acceptance. An agent
cannot alter those to make its implementation pass.

| Gate | Required evidence | Failure condition |
| --- | --- | --- |
| G0 — provenance/scope | pinned hashes, reviewed dependency closure, enabled package registry, source-to-target and method-to-method ledger | unreviewed dependency, missing required behavior, invented approximation or unreported adaptation |
| G1 — core semantics | canonical MML including explicit/inherited/default attrs, node kinds, classes, embellished cores, properties, diagnostics | tree shape merely looks similar; operator/style/inheritance behavior differs |
| G2 — parser mechanisms | handler priorities, group/stack transitions, macro/environment state and postfilter snapshots match upstream | sample-based parser, missing `over`/prime/middle/array/default-argument or scope behavior |
| G3 — fonts | every direct record, effective variant query, map, parameter, delimiter size/stretch slot and extension merge compared to instantiated upstream data | direct path copying is used as evidence for a heuristic resolver |
| G4 — common layout | per-wrapper BBox/style/scale/spacing/stretch snapshots and selected algorithm branches match upstream | only outer width or nonempty SVG checked; copied formulas run on incompatible semantics |
| G5 — scene/SVG | every fragment; complete commands, paints, transforms, viewport/clip inheritance, geometry and serialized semantics | only first fragment or path+rect subset compared; linebreaking disabled; relevant metadata omitted |
| G6 — native rendering | complete SVG through Folia's SVG normalizer and Direct2D, with raster comparisons at multiple em sizes and DPI | SVG parses but is blank/clipped/wrong; normalized render hides differences |
| G7 — Folia switch | all prior gates, core/editor/build suites, cancellation/cache/settings/PDF integration, no QuickJS references in runtime/build/package | production switches early, JS fallback/feature flag remains, existing required features silently regress |

Discrete semantics and data must compare exactly. For numerical layout,
compare full-precision upstream common BBoxes/placements before SVG rounding,
using a documented machine-precision bound (initially absolute 1e-10 em plus
relative 1e-10); each exception requires root's written justification. For
serialized SVG geometry, compare after applying the same upstream rounding.
A blanket 0.002 em tolerance is not acceptable. Raster comparisons are an
additional gate, not permission to relax semantic/layout comparisons.

The oracle uses the **same** package profile and options as Folia. It must
check errors and all fragments, and cannot discard nested viewBox transforms,
group clips or unsupported renderable elements. It must first pass deliberate
mutation tests: missing second fragment, wrong variant, shifted glyph, wrong
rule thickness, altered clip, wrong macro scope and dropped attributes must
all be detected. Test expectations are committed independently of a task's
implementation and cannot be updated by that task's agent.

The corpus covers every enabled package/registered handler and every retained
node/wrapper branch, valid and invalid input, explicit/default/inherited
attributes, nested combinations, macro/environment scope, stretch threshold
boundaries, multiple script levels, Unicode, widths, line breaks and inline/
display modes. Add seeded bounded generation for composition and invalid-input
robustness. Positive SVG dimensions do not count as behavior coverage.

## 6. Execution and integration rules

The rejected prototype is archived at
`build/math-port-rejected-prototype`. It is not linked into Folia, not an
accepted implementation and not an algorithm source for the new port.
Copied font records can be reused only after G3 validates the regeneration.

Implementation proceeds in dependency-ordered waves with concrete task packets
in `tasks.md`. Root freezes each wave's interfaces and expected tests before
dispatch. Agents own disjoint listed files, may read dependencies, and must
report any needed contract change to root instead of editing shared contracts.
Root performs source review and independent verification after every packet.
Only root runs builds against a shared build directory and accepts commits.

Each reviewed subsystem is committed separately with upstream provenance and
test evidence. No bulk dump of hundreds of files. A rejected gate returns the
packet for correction; it does not trigger a weaker comparator or a fallback.
The app retains its existing single backend until G7 permits replacing it;
there is no runtime old/new switch, compatibility path or JS fallback.

Windows Debug hook recursion is validated in a 16 MiB test-process stack so
the native 512-call guard precedes the OS stack boundary. Caller stack capacity
is a host prerequisite, not configured by the portable math module. Before G7,
configure and verify math worker stack availability in Folia; the test executor
reservation does not validate existing application/thread integration.

P22a locally accepts native callable Property identity and sparse slice under
evidence/p22a-function-property-acceptance.md. The Missing/object/callable leaf
dependency direction avoids a Property/function import cycle; raw configuration
and typed calls retain one FunctionRef/body. This host packet accepts no new
complete MathJax Source file. Concrete maps and complete rendering/integration
remain gated, and the parallel Mermaid shared-scene SVG/Direct2D work remains
required before its separate Folia integration acceptance.

P23 fixes the sparse-child ownership and storage boundary in
`p23-child-storage-design.md`: factory/session-owned products, borrowed tagged
node identity, one sparse child-array store, exact alias/rebind and partial-write
semantics. Every one of the 31 existing child consumers must migrate together;
the dense-vector evidence does not accept Source-reachable holes. Generic tree
operations and traversal now have Source acceptance; the complete real MML
registry graph compiles, and P23f locally restores constructor/child policy
against 88 constructor cases and 62 of the original Source299 cases, with 11
effective compiling mutations. The remaining 237 cases, return32 and other MML
consumers still require migration and independent acceptance. No Source
count or engine/parser/renderer acceptance advances from this design decision.
RegExp is separately an owned canonical Property value with real object state
and JS operation/coercion protocols; the audited gaps and candidate mismatches
remain required work. Sparse node slots must not introduce fake shared ownership.

P23j subsequently closes the frozen inheritance chain with original sparse
observations and effective compiling faults; see
`evidence/p23j-inheritance-acceptance.md` for its exact bounded scope. Historical
dense whole-file evidence still does not certify all sparse consumers. P23k-n
repair generated observer bindings without changing Source expectations. Script,
leaf, HtmlNode and MathChoice now independently compile over the actual 90
interfaces; their Source profiles reproduce. The Root -k 0 repository inventory
in `evidence/p23n-mathchoice-observer-acceptance.md` found 12 stale observer
translation units. P24 independently repairs and compiles three operator and
registry observers, leaving nine in the current keep-going compiler inventory;
see `evidence/p24-operator-registry-observer-acceptance.md`. No core tests execute
yet. P24 also accepts three base TeX-class propagation bodies, including raw
values, virtual callback order and partial failure effects: 13,892 assertions,
11 effective compiling mutations and 1,765 existing regression assertions.
See `evidence/p24-base-class-acceptance.md`. Full parser, wrappers, shared
layout, SVG/D2D and Folia gates remain open; 54/246 remains a historical tally.

P25 accepts four parent/class-propagation methods over the actual graph:
12,960 Parent assertions with ten faults and 6,480 Layout/Base assertions with
twelve faults. Existing base/inheritance gates also pass. The full build still
has the same nine observer compilation failures. See
`evidence/p25-mml-method-acceptance.md`; no whole-file tally or rendering gate
advances from these narrow method results.

P26 independently compiles the simple/mfenced/maction observers against all
90 interfaces with original data intact. The current repository inventory
has six remaining observer compilation failures; semantic runtime acceptance
does not follow from this compile repair. See
`evidence/p26-observer-repair-acceptance.md`.

P27 closes the six remaining stale observer compile failures against all 90
interfaces with original profiles, value tables and raw matrices intact. The
normal repository runner now builds and links, then exits 1 on mathematical
runtime exceptions/assertion failures. Compile repair does not accept those
consumer algorithms or any TeX/layout/rendering integration. See
`evidence/p27-observer-repair-acceptance.md`; 54/246 remains historical.

P28 accepts only AbstractMmlTokenNode::walkTree: 780 original complete records,
four Native host assertions and eleven effective compiling faults. The actual
core filtered runner passes 784 assertions in two tests; inherited regressions
also pass. Other MML runtime failures remain. See
`evidence/p28-token-walk-acceptance.md` for the exact bounded evidence.

P29 accepts the base template inheritance method and Empty no-op override over
9,344 original complete records and thirteen effective compiling faults. The
normal core filtered runner, 445 Attributes assertions and existing inheritance
regressions pass. See `evidence/p29-inherit-from-acceptance.md`; verification,
replacement/error consumers and full TeX/layout/output/Folia remain open.
