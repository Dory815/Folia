# Mermaid native Direct2D work — root ownership and acceptance

User added this objective alongside the native MathJax goal: add Direct2D to
mermaid-rs-renderer as a backend parallel to SVG, validate it independently,
then switch Folia's Mermaid drawing to the native result without SVG transit.
Both objectives remain required. The current goal API supports status changes
only, so this file records the added requirement rather than replacing or
prematurely completing the existing math goal.

Status: verified upstream vendoring, M1 local scene-invariant acceptance;
M2a typed paths, M2b explicit edge symbols, M2c complete ordinary node shapes
and M4a source text presentation helpers plus M2d edge-label geometry, M2e
Architecture icon presentation, M2f C4 style selectors and M2g ER/Sankey label
data accepted locally (34/70 inventoried
render functions); M3 native typed-scene consumer built
and checked with 11 actual WARP tests and 39 compiling mutations after local
M3b/M3c Source blend acceptance. Its 16 blend modes match 1,536 independent
Source pixel samples across four DPI values with maximum channel error 2;
the four nonseparable modes use a private native HLSL effect. Evidence is
evidence/m3bc-source-blend-acceptance.md. Full diagram
scene compilation, SVG and Direct2D backend acceptance, and Folia wiring remain
open. Exact implementation packets are frozen in implementation-contract.md,
m2a-path-contract.md, m2b-edge-symbols-contract.md, m2c-node-shapes-contract.md
and m3-direct2d-contract.md, followed by m4a-text-presentation-contract.md
and m2d-edge-label-geometry-contract.md. M2d evidence is
evidence/m2d-edge-label-geometry-acceptance.md. M2e contract is
m2e-architecture-icons-contract.md, with local evidence in
evidence/m2e-architecture-icons-acceptance.md. M2f contract is
m2f-c4-style-contract.md, with local evidence in
evidence/m2f-c4-style-acceptance.md. M2g contract is
m2g-label-data-contract.md, with local evidence in
evidence/m2g-label-data-acceptance.md.
M4a accepts source requests only; shared text preparation and completed scenes
remain open. Pinned shared-text findings and the next preparation
boundary are recorded in evidence/shared-text-source-audit.md.
The typed input/fork visibility and provenance proposals are in
evidence/m4b-typed-text-adapter-audit.md; that report remains a source audit,
not shared text implementation or acceptance. COLR callback/state findings
are in evidence/m4c-colr-typed-painter-audit.md; root's required Source appearance
and fallback decisions are in colr-source-parity-policy.md. Neither freezes
the next complete text API or accepts a text backend.
The actual OpenType SVG-resource path and independent default parsing options
are audited in evidence/m4d-font-svg-resource-audit.md; root verified all 12
source hashes and the referenced resource/selection boundaries. This remains
a prerequisite audit, with resource policy and complete text APIs still open.
Root owns architecture, file responsibilities, interfaces, scope and acceptance.
M3e locally accepts shared radial geometry preparation under
m3e-radial-geometry-contract.md: eight mapped Source geometry methods,
4,956 actual-Source cases / 23,173 native assertions, 21 effective compiling
mutants and 38 tests in the explicit native matrix (11 actual WARP tests).
Evidence: evidence/m3e-radial-geometry-acceptance.md. This packet does not
advance the 34/70 Mermaid rendering-function count or accept completed paints;
stop preparation, stage selection/sampling, GPU gradients and scene wiring
remain required separately.
The explicitly requested Sol agent is mermaid_dx2d. Its initial authorization
was source audit only. Root has since independently verified archive/source
identity and frozen exact scene/path implementation responsibilities. Sol may
write only each authorized packet's owned implementation/report files. Root
alone owns tests/expectations, build manifests, acceptance and Folia integration.

## Required architecture

Preserve mermaid-rs-renderer's parser, diagram model and layout responsibilities.
Extract placement/geometry currently embedded in SVG emission where needed;
produce one completed shared geometry/display-list result. SVG and Direct2D
consume that same result. Neither backend performs a second diagram layout.
Direct2D drawing must never serialize Mermaid SVG, parse SVG, invoke usvg or
the Folia SVG normalizer, nor silently drop unsupported draw commands.

The shared result must retain all existing supported diagram primitives,
ordered drawing, transforms, clipping, fills/strokes/markers, text/fonts/styles,
images where supported, dimensions and theme/options behavior. Diagram-specific
algorithms remain in their upstream per-diagram responsibilities; no substitute
parser or generic approximate layout engine is authorized.

Windows APIs remain in platform-specific renderer code and the WinUI app layer;
portable parser/layout/model modules remain platform-independent. The Folia shell
stays WinUI 3. Worker/cache/cancellation and result ownership are integration
responsibilities, not layout or SVG-parsing logic. SVG export remains a parallel
backend, while Folia's normal Mermaid drawing directly consumes native output.

## Required acceptance before Folia wiring

1. Pin upstream source/version and inventory every supported diagram family,
   option and rendering method; record source-to-target responsibilities.
2. Preserve SVG behavior and compare complete shared geometry/text/style/clip
   results for the existing feature set, including valid and invalid inputs.
   Smoke checks for nonempty SVG or one flowchart cannot prove equivalence.
3. Exercise actual Direct2D rendering with parity checks at multiple sizes, DPI
   and themes. Verify font/text behavior, drawing order, transforms, clips,
   markers and all supported primitives. Detect deliberate geometry/style/text
   omissions; no failure/crash counts as a passed rendering comparison.
4. Verify native resource lifetimes, device loss and recreation, concurrency,
   cancellation, caching, metrics and errors under Folia's actual usage.
5. Root independently reviews the diff, runs the relevant Rust/native tests and
   builds, and records explicit acceptance. Agent completion is not acceptance.
6. Only then integrate in Folia and verify editor drawing/previews, selection,
   theme/settings changes, export, tests and app build. Do not retain a Folia
   old/new feature flag or silently fall back to the former SVG drawing route.

Root baseline has 470 source observations over all 23 families, pinned Folia
dependencies and fingerprinted Windows font environment; --check reproduces it.
This initial matrix is not complete option/geometry/pixel acceptance. Local M1
evidence is evidence/m1-acceptance.md; native primitive evidence is
evidence/m3-warp-local-acceptance.md. No release gate is passed by this document.

M3e locally accepts shared radial geometry; M3f locally accepts its shared
radial stage parameter/order projection. Their Source observers, root-frozen
interfaces and effective mutations are recorded in evidence/m3e-radial-geometry-
acceptance.md and evidence/m3f-radial-stages-acceptance.md. M3f's final matrix has
39 passing tests including 11 WARP tests. These local responsibilities leave
34/70 rendering functions accepted and do not pass stop/sampling/pixel or full
backend/Folia gates. Both consumers must receive the same completed shared
preparation rather than independently recreating geometry or stage decisions.

M3g locally accepts six shared f32 stop/color preparation methods under
m3g-gradient-stops-contract.md. Root's actual Source oracle has 3,242 cases /
382,002 native assertions; all 22 compiling semantic mutants are detected.
The final thirteen-target matrix has 40 tests, including all 11 actual WARP
tests with the pinned Arial fixture. Evidence:
evidence/m3g-gradient-stops-acceptance.md. The initial missing-font failure is
retained separately. One authoritative stop preparation vector carries mode,
dual transforms and flags for both consumers. The 34/70 rendering-function
count stays unchanged; complete Gradient::push_stages wrapper, color expansion,
sampling/precision/masks, radial resources, complete SVG/D2D and Folia gates
remain open. Agent completion alone never passes a gate.

M3h locally accepts the complete generic gradient wrapper/color context shared
by both future consumers under m3h-gradient-program-contract.md. Root's actual
Source corpus has 13,988 programs/1,296 colors/8,010 powers; 2,130,397 assertions
and all 34 effective compiling semantic mutants pass the independent acceptance.
The fourteen-target matrix passes 43 tests including 11 WARP. Evidence:
evidence/m3h-gradient-program-acceptance.md. Original-color endpoint decisions,
raw f32 contexts/defaults/padding, warning/failure state and stage/mask order
are retained in one program. Actual sampling, whole-pipeline precision/color
conversion, scene resources and both consumers remain required. Rendering
acceptance stays 34/70 and no Folia gate is passed.

M3i1 locally accepts the whole-pipeline compiler control projection under
m3i1-pipeline-control-contract.md, using original compiled function-pointer
observations: 12,872 cases/64,552 assertions, all 27 compiling mutants detected,
final fifteen-target matrix 45 tests including 11 WARP. Evidence:
evidence/m3i1-pipeline-control-acceptance.md. One plan retains complete-list
precision/width/order/tail and default/moved context decisions. Stage execution,
complete shader/blitter context, actual sampling/GPU gradients and shared-scene/
SVG/D2D/Folia gates remain open; rendering-function acceptance remains 34/70.

M3i2 locally accepts executable resource-independent gradient sampling under
m3i2-sampling-contract.md: original wide provider and 35 highp/11 lowp stages
consume the same existing GradientProgram. Actual original compile/start/stage
observations cover 6,678 cases/15,750 captures; 1,334,130 native assertions and
all 66 effective compiling mutants pass. Final sixteen-target matrix passes
48 tests including 11 actual WARP. Evidence: evidence/m3i2-sampling-acceptance.md.
Raw register/mask parity does not accept actual Paint/blitter/destination pixel
execution or GPU resources. Complete scene/SVG/D2D/Folia gates stay open and the
render-function acceptance count remains 34/70. No Folia integration occurred.

M3i3 locally accepts borrowed destination/mask/coverage resource stages and
original mask termination under evidence/m3i3-resource-stage-acceptance.md:
3,377 original cases, 11,489,370 assertions and 41 effective compiling mutants.
M3i4 adds pure register blend stages under evidence/m3i4-blend-acceptance.md:
4,686 original cases, 1,691,703 assertions and 54 effective compiling mutants.
The final eighteen-target matrix passes 56 tests including 11 actual WARP.
Low HardLight/Overlay extreme-color debug overflow and release-profile behavior
remain outside that bounded corpus. Pattern sampling, whole pipeline/blitter,
paint/text/shared-scene parity and full SVG/D2D/Folia remain required; rendering
acceptance stays 34/70 and Folia is not connected.

M3i5 and M3i6 subsequently connect borrowed Pattern sampling and shader producers
to the sole PipelineBuilder/CompiledPipeline context owner. The old independent
GradientProgram context was removed by M3i6c; its mentions above describe the
historical packet, not the current ownership model. M3i6d accepts actual
Linear/Radial producer wrapper wiring, without constructor/variant acceptance.
See evidence/m3i6c-gradient-builder-acceptance.md and
evidence/m3i6d-gradient-shader-acceptance.md for exact gates.

M3i7a ports separate highp/lowp start control behind a thin compiled-precision
facade. Its bounded original complete-run gate passes 4,608 cases and 19 effective
control faults; all 24 integration targets / 67 tests pass, including 11 WARP.
This is not full executor acceptance. Gradient/radial/image complete runs and
context ownership faults remain required, followed by three independent blitter
contexts borrowing the same resources, Shader dispatch, complete scene and final
SVG/Direct2D/Folia gates. See evidence/m3i7a-executor-run-gate.md. Rendering-file
acceptance remains 34/70 and no Folia integration has been authorized by a passed
full gate.

M3i7b now closes controlled Gradient/Pattern producer-through-executor runs:
13,580 original complete observations, zero native mismatches, nine effective
compiling dispatch/image/context/allocation faults; 25 integration targets /
68 tests pass including 11 WARP. The surviving Gather/Bilinear fault required
an independent sampling supplement; the frozen main gold stays unchanged.
See evidence/m3i7b-shader-run-acceptance.md for exact source and build evidence.
Public Shader factories and variant dispatch still precede three separate
blitter owners and final shared-scene/SVG/Direct2D/Folia acceptance. Root's
next responsibility audit is evidence/m3i8-shader-dispatch-audit.md. This
bounded closure does not advance the historical whole-rendering tally.
