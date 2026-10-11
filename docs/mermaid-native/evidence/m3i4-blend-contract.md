# M3i4: pure register blend stages

Root owns architecture, public APIs, original Source observers, fixtures, builds and acceptance. SVG and Direct2D remain consumers of the same prepared scene. This packet only fills register operations needed by shared paint preparation; it does not certify the whole blitter or either complete backend.

## Fixed responsibilities

`sampling/blending/mod.rs` exports `blend_high_stage(PipelineStage,&mut HighRegisters)->Result<(),SamplingError>` and its low equivalent. Each call executes one pure stage and returns; the caller owns continuation, precision, resources and program order. The original register order is r,g,b,a,dr,dg,db,da. Unsupported stages return the exact error before mutating anything. High supports ordinals 18–44; low supports those except 32,33,40–44. No graphics API, SVG, pixmap, mask, alternate scalar/SIMD provider or independent blend-mode selection belongs here.

Existing `sampling/wide` is the pinned original provider. Highp must use its actual f32x8 arithmetic, comparisons, min/max, reciprocal and square root behavior, preserving expression and channel write order. Lowp uses the original u16x16/div255 operation order with the current debug/release semantics, not widened/saturating/branch-lazy replacement formulas. The pure blend stage Source is distinct from existing scene-level Direct2D compositing.

## Agent scope

Sol may modify only `src/scene/prepare/sampling/blending/highp.rs` and `lowp.rs` under `third_party/mermaid-rs-renderer`, and write `m3i4-blend-implementation.md` beside this contract. Root's module export, function signatures, register/error types, original provider, fixtures, manifests and observers are fixed. No agent builds, tests, commits or delegation. Read the adjacent Source audit, the actual pinned highp.rs/lowp.rs stage functions and helpers, then transplant their algorithms with a ledger of exact original functions. Do not infer from W3C or Direct2D formulas; preserve even unusual original helper expressions.

## Actual Source and red gate

`node tools/mermaid-native/resource-stage-oracle.cjs --blend --check` executes a separate append-only original tiny-skia 0.12.0 copy through real compile/start/function pointers. It records 4,686 cases: 2,970 high, 1,716 low, 20,188 setup/continuation captures. Compact register/cursor fixture SHA256 `644c643d3e848bd7d6741e8fd71e6207008cca8697a171ad2d2fa9cd70495c4a`. Before compacting, every actual original capture and final destination buffer is checked against the original input bytes; repetitive resource bytes are then removed because the native API has no resource access. The original full byte projection is retained in the ignored Source build directory.

These cases are bounded premultiplied inputs. Low HardLight/Overlay are restricted to avoid eager candidate checked intermediate overflow; full valid-color domain/debug overflow acceptance remains a later gate. No claim of all malformed/raw u16 inputs or release-profile parity follows from this packet. High zero-alpha cases still retain exact Source-produced raw register bits. Single stages and multi-stage compositions cover both precision selection outcomes, lane widths, tails and multiple invocations.

The actual native callable stubs were compiled and executed with `cargo test --locked --offline --no-default-features --manifest-path third_party/mermaid-rs-renderer/Cargo.toml --target-dir build/mermaid-native-target --test blend_stage_parity -- --nocapture`. Result: 3 tests, 2 bound-stage assertion failures and 1 unsupported-stage success (333 assertions). Log: `build/m3i4-blend-red.log`. This is actual execution after a successful compile, not a missing test target or compiler diagnostic. Root rejected the earlier manifest omission and fixed explicit test registration before establishing this red.

Root must independently pass original raw-bit comparisons and effective compiling semantic mutations, then run the native matrix including actual WARP. Existing resource/terminal golds reproduce byte-for-byte after this observer extension. Do not advance whole-renderer function tallies or integrate Folia based on pure-stage acceptance: pattern sampling/tiling, complete pipeline, original blitter paths and skips/memset, paint/text/scene parity and final SVG/D2D acceptance remain open.
