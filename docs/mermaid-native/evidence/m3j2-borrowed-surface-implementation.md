# M3j2 borrowed-surface implementation handoff

The eight Root-frozen surface methods and three size helpers have been filled. This handoff awaits Root's independent comparison with the 3,240 original surface records and 567 private-helper records. It does not establish blitter constructor, callback, scene, backend or Folia acceptance.

| Original tiny-skia 0.12.0 source | Native body | Preserved operation order and boundary |
| --- | --- | --- |
| `pixmap.rs:467–475`, `PixmapMut::from_bytes` | `blitter/surfaces.rs`, `BorrowedPixmapMut::from_bytes` | `IntSize::from_wh` → `data_len_for_size` → `data.len() < data_len` rejection → borrow the entire supplied slice, including extra bytes. |
| `pixmap.rs:532–539`, `PixmapMut::as_subpixmap` | `BorrowedPixmapMut::as_subpixmap` | Reborrow the entire mutable slice, retain the logical size and set `real_width` to its width. |
| `pixmap.rs:541–554`, `PixmapMut::subpixmap` | `BorrowedPixmapMut::subpixmap` | Intersect with the zero-origin size rectangle, compute parent RGBA row bytes as width×4, compute top/left offset in bytes, borrow the complete post-offset tail, retain parent `real_width` and the intersection's logical size. |
| `mask.rs:95–105`, `Mask::from_vec` | `OwnedMask::from_vec` | Require exact width×height byte count and move the supplied `Vec`; no clone or alternate storage check. |
| `mask.rs:140–146`, `Mask::as_submask` | `OwnedMask::as_submask` | Immutable whole-storage borrow, full logical size and parent width. |
| `mask.rs:148–158`, `Mask::submask` | `OwnedMask::submask` | Original rectangle intersection, one byte per pixel for row/left offset, full post-offset borrowed tail, intersection size and parent real width. |
| `mask.rs:160–166`, `Mask::as_subpixmap` | `OwnedMask::as_subpixmap` | Mutable whole A8-storage borrow in the shared logical target representation. |
| `mask.rs:168–178`, `Mask::subpixmap` | `OwnedMask::subpixmap` | Original rectangle intersection and A8 offset, mutable post-offset tail, intersection size and parent real width. |
| `pixmap.rs:590–597`, `min_row_bytes` | `blitter/surface_size.rs`, `min_row_bytes` | Convert width to `i32`, checked multiply by four, then `NonZeroUsize::new`. This retains the original 31-bit row-byte limit. |
| `pixmap.rs:599–607`, `compute_data_len` | `compute_data_len` | `height.checked_sub(1)`, checked `usize` multiplication by supplied row bytes, checked width×4, then checked addition. The supplied row-byte argument is not replaced with width×4. |
| `pixmap.rs:609–612`, `data_len_for_size` | `data_len_for_size` | Call `min_row_bytes` first and pass its result to `compute_data_len`; no inlined alternative path. |

The pinned Source hashes are `pixmap.rs` `484d49d1f4c3c3c8fb3fab3e1b1535292de828c0f258c3cab9bda02c7d9626fc` and `mask.rs` `5c6b8d615f219f78ceebc714a56e9916745eeb93fc9213b87c96b3f00f55792b`. Native representation names differ from Source names, but the type fields, mutable/immutable borrows, and byte-offset arithmetic follow the corresponding source methods. `tiny_skia_path::IntSize` and `IntRect` continue to provide Source rectangle behavior.

No declarations, imports, registrations, tests, fixtures, gold, manifests or other production files were changed. No build or test was run. I found no missing API for these 11 bodies. Optional mask identity, sparse AlphaRun, three pipeline contexts and actual blitter execution remain future responsibilities.
