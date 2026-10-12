# Root original borrowed surface observations

Root independently inspected the original PixmapMut/Mask subview methods and
current executor resource types after the Sol read-only audit. This freezes
original observations before Native declaration or implementation assignment.
It does not accept any Native surface or blitter behavior.

The append-only observer calls eight actual original methods:
PixmapMut::from_bytes/as_subpixmap/subpixmap and
Mask::from_vec/as_submask/submask/as_subpixmap/subpixmap. Original source bytes
are pinned and every Rust source file in the copied crate is guarded as its
unchanged original prefix plus the exact observer suffix. The transport only
serializes records; it does not compute constructor/view outcomes.

There are 3,240 complete records: 240 successful views, 2,700 constructor
rejections, 300 absent/invalid/intersection-free rectangles and zero panics.
Each includes logical dimensions, parent real width, byte offset from original
storage, entire retained slice length, slice before/after controlled byte writes
and all backing bytes afterward. The stimulus includes zero/huge dimensions,
short/exact/extra buffers, nonzero region origins, clipping of negative origins,
zero/overflow rectangle dimensions, immutable mask references, A8 mask targets
and RGBA targets. Writes at both the first and backing-tail index witness the
borrow across subviews; read-only views remain unchanged.

Original PixmapMut accepts extra backing bytes and keeps the entire post-offset
slice. Mask::from_vec requires exact length. Logical subview width is distinct
from parent real width; RGBA offset uses four bytes per pixel while A8 uses one.
These differences must survive the next Native boundary. Do not infer a logical
size from a slice length or truncate the retained subview to its logical area.
Premultiplied pixel interpretation, pixels_mut casts, mask allocation/conversion,
blitter construction, optional-mask policy, sparse AlphaRun consumers, renderer
and Folia are outside this Source-only freeze.

Fixture SHA256:
03935ed7e9791f1765461c38ee9626f79eda113863217b068edec13ab85b986c.
Reproduce with `node tools/mermaid-native/borrowed-surfaces-oracle.cjs --check`.
Next Root work is exact types/file ownership, real Native stubs and a compiled
red transport before implementation assignment to Sol.
