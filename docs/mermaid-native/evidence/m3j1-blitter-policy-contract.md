# Root blitter policy implementation packet

Sol may fill only five new stub bodies: RasterBlendMode::should_pre_scale_coverage
and to_stage in blend_policy.rs; ColorSpace::expand_dest_stage/compress_stage
in gradient_color_space.rs; PremultipliedStopColor::to_color_u8 in
gradient_stops.rs. Files are under third_party/mermaid-rs-renderer/src/scene/prepare.
Declarations, enum ordering/default, ALL_RASTER_BLEND_MODES, existing numerical
methods, module/manifest wiring, tests/oracles/gold/pins and other bodies are
Root-owned. No scene CSS blend reinterpretation, blitter implementation,
new wrapper/fallback, SVG/backend/Folia change, build/test/commit or delegation.
Sol writes m3j1-blitter-policy-implementation.md as a mapping report only.

Root original gold contains 3,445 records, SHA-256
d4b941e3840104a0a7a11cc31cf2e302592ce852af99e32ae12bba22110d56ce.
The full 29-mode x four-color-space product has 116 policy records. Another
3,329 colors include adjacent f32 values at every 0..254 byte-half threshold
in all channels, the full alpha/u8 lattice and invalid colors. Eight records
reject Color::from_rgba. Source uses actual original policy methods and
Color::premultiply().to_color_u8(), preserving normalized intermediate bits.
Only read-only observers are appended; every original Rust file is guarded.

Original BlendMode source SHA-256
841dcc8eb96fe11fc2316fa0a4ff47c29fd837990ce8c1d8be5fcf08f6c86d19:
29 variants, SourceOver default. Coverage true only for Destination,
DestinationOver, Plus, DestinationOut, SourceAtop, SourceOver and Xor;
to_stage Source=None, Destination=MoveDestinationToSource, all others map
to their original stages. Native RasterBlendMode is independent of the
retained scene's sixteen CSS BlendMode values. Existing PipelineStage suffices.

Original color.rs SHA-256
3cea03557e9e7d8593230149052a11c2062a2439e6ebc2631dac6594e402bd8d:
ColorSpace Linear=None/None; Gamma2=GammaExpandDestination2/GammaCompress2;
SimpleSRGB=GammaExpandDestination22/GammaCompress22;
FullSRGBGamma=GammaExpandDestinationSrgb/GammaCompressSrgb. No new color enum.
to_color_u8 computes each normalized premultiplied component*255+0.5 then
casts to u8. No extra alpha multiplication, unpremultiply, round function,
u16 uniform context transit or reordered f32 operations. Preserve attribution.

Root compiled the actual production package and observed all 3,445 record
comparisons fail via actual stubs; five direct witnesses each independently
called its real missing body and failed assertions. A prior Root witness typo
using .2 instead of Rust 0.2 failed compilation and is excluded from red proof.
Original --check must remain immutable. Root independently requires all 3,445
complete records and five methods green, focused compiling selector/quantization
faults, existing public Shader/producer/run regressions and the registered
Direct2D integration suite with the pinned font and real WARP.

This packet accepts policy dependencies only. Three independent blitter
builders/contexts, borrowed logical surfaces/masks and callback execution
remain separate frontier gates before complete scene/backend/Folia acceptance.
