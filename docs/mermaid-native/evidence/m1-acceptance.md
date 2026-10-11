# Root M1 local acceptance

M1's immutable scene structures and structural invariants are locally accepted.
Backend extraction, SVG parity, Direct2D drawing and Folia integration are not.
Root froze source/file responsibilities in implementation-contract.md before
Sol's constrained five-file implementation. Root reviewed all five files,
corrected missing font/glyph and visible-text coverage requirements during
review, and wrote the independent scene_contracts Rust tests.

Final build is locked/offline/no-default-features, using Cargo resolutions seeded
from Folia's existing lock. Windows APIs/usvg/SVG parsing are absent from these
scene modules. All 13 root tests pass in build/mermaid-m1-pinned-test.log; terminal
exit is 0. The earlier test also passed, but its newly resolved dependency
versions were replaced with existing Folia versions before final acceptance.
Font fixture: C:/Windows/Fonts/arial.ttf, SHA256
b3658eadae55e682b5f69eb64c439c1ecc8f196c0bb8d4756d145d13bc86476a.

Checks cover Send+Sync/immutable empty output, affine composition order, painter
order/zero-length geometry, typed quadratic/cubic/arcs, invalid geometry/numbers,
gradient refs/stop order/opacity, clips, metadata shape, unresolved visible text,
empty/hidden text, font bytes/face/glyph bounds, UTF-8 and incomplete source
coverage, owned image/resource references and deep rejected/abandoned trees.
Two 20,000-level trees are disposed on a 128 KiB stack without overflow; the
128-level accepted-depth budget is enforced by iterative validation/cleanup.

This validates structural completeness and ownership, not correct font shaping,
image decoding, diagram geometry, transformed curve extrema, device behavior
or pixels. Root test image bytes intentionally test owned reference lifetime;
they are not decoding evidence. These later obligations remain open. Source SVG
baselines are independent and root-owned; an agent report cannot approve a
backend. No production Folia dependency or drawing route was changed.
