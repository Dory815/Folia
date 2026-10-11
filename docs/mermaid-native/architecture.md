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
