# M3i8 public Shader implementation handoff

This is an implementation ledger, not an acceptance result. Root owns the
unchanged original Source oracle, native comparisons, mutations, build and
integration review. The implementation changes only the six algorithm files
assigned by `m3i8-public-shader-contract.md`; this report is the seventh file.
No build, test, gold generation or commit was run by the implementing agent.

## Source body mapping

| Original tiny-skia 0.12.0 body | Native body and preserved operation order |
|---|---|
| `shaders/linear_gradient.rs:35–87` `LinearGradient::new` | `linear_program.rs::new_linear_gradient`: empty and singleton before length; finite length before near-degenerate 1/32768 branch; Pad chooses last color, Reflect/Repeat call the original SIMD average; inversion before points-to-unit matrix and `PreparedGradient::new`. |
| `shaders/linear_gradient.rs:99–109` `points_to_unit_ts`, `:112–162` `average_gradient_color`, `:164–178` matrix helpers | Local helpers in `linear_program.rs` retain the vector-length/inverse/scale and post-transform sequence, the accepted original `sampling::wide::f32x4` color accumulation and implicit endpoint intervals, and `sdot` f32 operation order. |
| `shaders/radial_gradient.rs:120–201,215–231,316–390` `RadialGradient::new`, `new_radial_unchecked`, `create` | `radial_program.rs::new_radial_gradient`: negative-radius and stop-count checks precede the accepted `prepare_radial_geometry` projection of inversion, degenerate, concentric, Strip and Focal mapping. `thin_ring` reuses the original stop allocation, clears it, inserts the three 0/1 hard stops, then constructs one `PreparedGradient`. The already accepted `radial_geometry.rs` supplies `kind` and `points_to_unit`. |
| `shaders/sweep_gradient.rs:29–82` `SweepGradient::new` | `sweep_program.rs::new_sweep_gradient`: angle validity precedes stops/inversion; near-degenerate Pad reconstructs three hard stops and recursively calls the same factory; whole-circle changes spread to Pad; computes `t0/t1` and translated points-to-unit transform. |
| `shaders/sweep_gradient.rs:88–104` `SweepGradient::push_stages` | `sweep_program.rs::push_sweep_gradient_stages`: computes scale then bias, writes `radial_p` before generic production, and supplies the exact `XYToUnitAngle` plus conditional scale-bias pre-color callback to the existing sole-builder generic seam. |
| `shaders/mod.rs:65–74,95–112,127–145` Shader methods | `shader_dispatch.rs`: exhaustive five-variant `is_opaque`, post-concat `transform` and normalized `apply_opacity`; Radial and Pattern always report nonopaque. |
| `shaders/mod.rs:81–93` `Shader::push_stages` | `shader_dispatch.rs::push_stages`: Solid expands, premultiplies and calls the uniform builder; the other four variants call their existing/native producer on the same builder. |
| `color.rs:337–354` `Color::premultiply` | `gradient_stops.rs::StopColor::premultiply`: opaque branch copies normalized components; nonopaque branch multiplies RGB by alpha independently through `NormalizedF32::new_clamped`, retaining alpha. |
| `pipeline/mod.rs:377–393` `RasterPipelineBuilder::push_uniform_color` | `pipeline_builder.rs::push_uniform_color`: reads four f32 components, computes `(component * 255.0 + 0.5) as u16` per channel, builds uniform context, pushes `UniformColor` first and assigns context afterward, retaining capacity-panic partial effects. |

## Boundaries for Root verification

The public constructor/dispatch algorithms have not been compared by the
implementing agent. The existing controlled producer/run, prepared context,
numeric stage and Direct2D gates are not a substitute for the 7,905-case public
factory oracle. Public factory choice and Sweep execution remain open until
Root's independent bit-identical run and mutation review. Three blitter-owned
pipelines, complete shared scene paint/text, SVG/Direct2D equality and Folia
integration remain separate unfinished gates.
