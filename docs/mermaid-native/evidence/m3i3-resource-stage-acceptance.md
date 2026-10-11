# M3i3a Root resource-stage local acceptance

Root independently accepts the bounded resource-stage bodies in the frozen
M3i3 contract. This does not accept a full paint/blitter pipeline, shared scene
compilation, all SVG/Direct2D behavior or Folia integration. The 34/70 original
render-function tally is unchanged; this packet closes subordinate execution
dependencies rather than counting unrelated tiny-skia stages as Mermaid source
functions. User's full parallel SVG/Direct2D consumer goal remains open.

## Original Source, review and execution

Root's append-only Source observers execute pristine tiny-skia 0.12.0 original
compiler, full/tail function slices and start/stage bodies. No native equation
produces expected values. Profile remains std+simd x64 MSVC; nine original wide
files and BSD license verify unchanged except established cfg profile bindings.

The main corpus has 2,959 cases / 13,281 captures, 1,276 highp and 1,683 lowp.
Root added a separate byte-guarded Source copy whose observer also reads
registers/destination after original continuation returns. This adds 418 cases
/ 2,299 captures (209 per precision), observing MaskU8 Stop before register
mutation. Without terminal observation a stage could alter its local registers
then return Stop, and a subsequent-capture-only observer would miss the change.
Original primary gold remains unchanged; both oracles --check reproduce.

Main fixture SHA256:
281b3ecf96096f0a8f4bf810feedf6dcce359c8dd0cacc309aec3e537999a122.
Terminal fixture SHA256:
3bd553cc6d54c90ef5ba9a92b0ed25ef057e037b68f441355cfc6ed4c8ca7c99.

Five native tests pass with 11,489,370 assertions: 4,856,600 high main,
4,944,885 low main, 940,025 high terminal, 747,175 low terminal and 685
unsupported assertions. These compare every register bit/cursor and every
destination byte, preserve context/mask state, and exercise full/tail RGBA/A8
stride, two-byte AA, clip termination, float/byte coverage and fused
destination read/blend/write. Root reviewed the owned Source branches and
helpers directly. Registers and contexts retain the shared precision split;
no scalar substitute, SVG/platform call, resource owner, or second color cache
was introduced. Allocation/invalid internal-buffer panic domains are not
accepted as normal paint behavior by this bounded API.

Forty-one effective mutations compile and fail assertions: all 21 owned
precision/stage omissions, unsupported acceptance, pixel/byte stride,
AA offset/second byte/tail, zero-mask continuation and pre-Stop register
mutation, load factor, store rounding, inactive tail access/store and lowp
coverage/div255 quantization. The isolated runner rejects build failures,
overflow/bounds/process crashes and verifies unchanged working sources/golds.
Summary is m3i3-resource-stage-mutations.json.

The final named matrix passes 17 targets / 53 tests, including all 11 actual
Direct2D WARP tests. Arial fixture is pinned C:/Windows/Fonts/arial.ttf SHA256
b3658eadae55e682b5f69eb64c439c1ecc8f196c0bb8d4756d145d13bc86476a.
Root's first matrix omitted FOLIA_SCENE_TEST_FONT; the real font test failed.
That failed run is preserved in build/m3i3-matrix-missing-font-env.log and is
not accepted. Root restored the exact pinned font environment and reran the
entire named matrix. No production relaxation was used.

## Raw artifacts and remaining gates

| Ignored artifact | SHA256 |
|---|---|
| build/m3i3-resource-terminal-green-test.log | dc268b6846012191a534e1190d0731b36bef92025a58517b8d6e9e75c83523db |
| build/mermaid-resource-stage-mutations/results.json | f309298fb0a0d3e7d3833be4ddd6a14d5e8bffeec2dc4ec37f64ccb18ab6cab3 |
| build/m3i3-final-native-matrix.log | 8b2febf22bce77e400baf63365967a7eb490566247ff5e1c8ab2d13969513079 |

Reproduce resource-stage-oracle.cjs --check, the same with --terminal --check,
sampling-numeric-vendor.cjs --check, resource-stage-mutations.cjs, and the
seventeen explicit Cargo test targets with --no-default-features --features
direct2d and the pinned FOLIA_SCENE_TEST_FONT environment.

Blend ordinals 18–44, pattern pixmap/sampler/gather/filter execution, actual
whole-sequence resource invocation, Source three-path blitter plus memset/skip
paths and A8 construction, complete valid-paint pixels, shared resource/text
preparation and full backend acceptance still remain. Resource-stage acceptance
alone does not authorize calling an unsupported stage, skipping its error or
connecting Folia. The app integration was not changed.
