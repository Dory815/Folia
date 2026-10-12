# Root borrowed blitter dependency contract

Root owns the types, responsibilities, original records, real stubs, transport,
faults and acceptance. Sol mermaid_geometry may implement exactly eight method
bodies in scene/prepare/blitter/surfaces.rs and three helper bodies in
scene/prepare/blitter/surface_size.rs, plus its implementation report. Types,
signatures, imports, module registration, tests, gold, observers, manifests,
original copies and build files are frozen. No builds/tests/generation/commits
or further delegation by the implementer.

surface_types.rs owns the raw original representations: BorrowedPixmapMut
borrows the entire contiguous RGBA byte slice; OwnedMask owns its exact Vec;
LogicalTarget reborrows mutable bytes with IntSize and parent real_width;
LogicalMaskRef carries an immutable borrow, IntSize and parent real_width.
Public fields expose these representations to the native host/observer. Actual
constructor methods validate the original size/storage rules; do not add a
second validation policy to raw views. A8-versus-RGBA comes from the original
consumer, which uses the same SubPixmapMut representation. There is no SVG,
Direct2D, pipeline Context, cloned target buffer or three-owner allocation here.

surfaces.rs owns exactly the original PixmapMut::from_bytes/as_subpixmap/
subpixmap and Mask::from_vec/as_submask/submask/as_subpixmap/subpixmap algorithms.
surface_size.rs owns the original min_row_bytes, compute_data_len and
data_len_for_size functions from pixmap.rs. Preserve their call layering:
from_bytes calls data_len_for_size, which calls min_row_bytes and then
compute_data_len. Do not inline a different arithmetic/validation policy.

The original tiny-skia 0.12.0 pixmap.rs and mask.rs pins are recorded in the
Source freeze. Original rectangle intersection and IntSize stay the existing
tiny_skia_path dependency; no replacement rectangle implementation is needed.
Subviews intersect the requested rectangle, preserve the parent stride and
entire post-offset tail, and borrow storage without copying. RGBA offset uses
four bytes per pixel; both mask projections use one. from_bytes allows extra
storage; from_vec requires exact length and moves the Vec. Preserve the original
i32 row-byte limit and checked usize arithmetic in all three size helpers.

Root's immutable surface fixture contains 3,240 complete original records:
240 successful views, 2,700 constructor rejections, 300 absent rectangles,
zero panics. SHA256:
03935ed7e9791f1765461c38ee9626f79eda113863217b068edec13ab85b986c.
The independent scalar supplement observes all three original private helpers
without allocating huge buffers: 567 complete records over u32 size boundaries
and checked 64-bit arithmetic, SHA256:
2f06cea5d2bf2d92a9e8462fff02a57615b3dbf9a8ec3e1d4838a651145da91a.
Original usize results are kept as original JSON integer tokens; they are never
rounded by a JS parse/re-encode. Native serde compares exact usize values.

Root compiled the actual production declarations and real stubs, then ran both
native transports. Surface red: 2,754 stub/errors/mismatches, plus eight actual
direct stub calls. Size red: 448 stub/errors/mismatches, plus three actual direct
stub calls. The remaining invalid-IntSize rows deliberately return before those
methods, as original transport does. Both tests fail assertions with exit 101;
compile errors/crashes do not count as red evidence.

Acceptance requires unchanged original --check replays, exact full records,
all eleven direct-body witnesses, effective compiling faults including huge
arithmetic and wrong stride/offset/tail/ownership, and all registered integration
targets including real WARP. This accepts only borrowed surface dependencies.
Blitter constructors/three independent contexts, optional mask distinctions,
sparse AlphaRun/callbacks, full scene/backend and Folia gates remain required.
