# M2d edge-label geometry submitted for root review

Implemented the pinned renderer's `LabelRect::from_center` and ten consecutive
edge-label geometry functions in
`third_party/mermaid-rs-renderer/src/scene/compile/edge_label_geometry.rs`.
The authority is pristine `mermaid-rs-renderer` 0.3.1 `render.rs` lines
1770–1966, SHA256
`026efceab06b86752789ea2b5c37ecac569b8cd020f97c776f444bec0a2166b4`.
The source range maps line for line to target lines 3–199:

| Function or type | Source lines | Target lines |
| --- | ---: | ---: |
| `LabelRect` and `from_center` | 1770–1796 | 3–29 |
| `EdgeLabelKind` | 1798–1803 | 31–36 |
| `edge_label_background_visible` | 1805–1831 | 38–64 |
| `polyline_rect_gap` | 1833–1846 | 66–79 |
| `segment_rect_gap` | 1848–1863 | 81–96 |
| `point_rect_distance` | 1865–1886 | 98–119 |
| `point_segment_distance` | 1888–1904 | 121–137 |
| `segment_intersects_rect` | 1906–1925 | 139–158 |
| `point_in_rect` | 1927–1932 | 160–165 |
| `segments_intersect` | 1934–1955 | 167–188 |
| `orient` | 1957–1959 | 190–192 |
| `on_segment` | 1961–1966 | 194–199 |

Only API visibility and the contracted `PartialEq` derive differ from the
source lines. `LabelRect` and its fields, `from_center`, `EdgeLabelKind`, and
all ten functions are public. A line-by-line comparison of the 197 source and
target lines found only these declarations and derive differences; every
function body remains identical, including `f32` operation order, early
returns, corner and segment candidate order, comparisons, and nonfinite
behavior. The API uses `crate::ir::DiagramKind` directly.

No doubts or missing prerequisites were found. I did not build, test, edit
manifests or exports, add callers or fixtures, or commit. Root owns the
unchanged-source oracle, parity and mutation tests, builds, review, and
acceptance. This submission alone does not establish Mermaid backend parity.
