# M3j1 blitter policy implementation handoff

This packet fills the five Root-frozen stub bodies only. It is awaiting Root's independent original-Source comparison, fault tests and integration checks; it does not accept the three blitter pipelines or the complete Mermaid backend.

## Source-to-native mapping

| Original tiny-skia 0.12.0 | Native implementation | Exact behavior |
| --- | --- | --- |
| `src/blend_mode.rs:67–92`, `BlendMode::should_pre_scale_coverage` | `scene/prepare/blend_policy.rs`, `RasterBlendMode::should_pre_scale_coverage` | `true` only for Destination, DestinationOver, Plus, DestinationOut, SourceAtop, SourceOver and Xor; all other 22 modes are `false`. Destination is retained despite the later blitter fast path. |
| `src/blend_mode.rs:94–126`, `BlendMode::to_stage` | `scene/prepare/blend_policy.rs`, `RasterBlendMode::to_stage` | Source returns `None`; Destination returns `MoveDestinationToSource`; the remaining 27 modes return their corresponding original pipeline stage, exhaustively matched. |
| `src/color.rs:530–537`, `ColorSpace::expand_dest_stage` | `scene/prepare/gradient_color_space.rs`, `ColorSpace::expand_dest_stage` | Linear returns `None`; Gamma2, SimpleSRGB and FullSRGBGamma return `GammaExpandDestination2`, `GammaExpandDestination22` and `GammaExpandDestinationSrgb`, respectively. |
| `src/color.rs:538–545`, `ColorSpace::compress_stage` | `scene/prepare/gradient_color_space.rs`, `ColorSpace::compress_stage` | Linear returns `None`; Gamma2, SimpleSRGB and FullSRGBGamma return `GammaCompress2`, `GammaCompress22` and `GammaCompressSrgb`, respectively. |
| `src/color.rs:424–428` plus `color_f32_to_u8` at 437–449, `PremultipliedColor::to_color_u8` | `scene/prepare/gradient_stops.rs`, `PremultipliedStopColor::to_color_u8` | In RGBA order, each already-premultiplied `NormalizedF32` component is evaluated as `(component.get() * 255.0 + 0.5) as u8`. No new alpha multiplication, unpremultiply, rounding helper or u16 intermediate is introduced. |

The pinned original `blend_mode.rs` SHA-256 is `841dcc8eb96fe11fc2316fa0a4ff47c29fd837990ce8c1d8be5fcf08f6c86d19`; `color.rs` is `3cea03557e9e7d8593230149052a11c2062a2439e6ebc2631dac6594e402bd8d`. The existing Root-declared 29-value `RasterBlendMode`, four-value `ColorSpace`, `PipelineStage` and `NormalizedF32` identities are unchanged. This policy does not use the scene's separate 16-value CSS blend mode.

No declarations, tests, fixtures, gold, source observers, build files, manifests, backends or Folia integration were changed. No build or test was run in this packet. There is no known API gap for these five bodies; blitter builder/context ownership and borrowed destination/mask execution remain outside this packet.
