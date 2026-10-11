# Mermaid native Direct2D work — root ownership and acceptance

User added this objective alongside the native MathJax goal: add Direct2D to
mermaid-rs-renderer as a backend parallel to SVG, validate it independently,
then switch Folia's Mermaid drawing to the native result without SVG transit.
Both objectives remain required. The current goal API supports status changes
only, so this file records the added requirement rather than replacing or
prematurely completing the existing math goal.

Status: prerequisite source audit; implementation boundary is not yet frozen.
Root owns architecture, file responsibilities, interfaces, scope and acceptance.
The explicitly requested Sol agent is mermaid_dx2d. Its initial authorization
is source audit only: docs/mermaid-native/source-audit.md and optional
source-inventory.json, with a verified pinned reference checkout. It may not
change implementation, tests/expectations, build manifests or Folia integration
until root reviews the source and freezes the exact implementation packet.

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

Concrete file ownership and fixed expected evidence follow the source audit.
No release gate is passed by this architecture document.
