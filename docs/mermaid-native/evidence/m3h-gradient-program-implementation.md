# M3h shared gradient program — implementation report

Status: **awaiting root's independent Source oracle and mutation acceptance**. This packet changes only `third_party/mermaid-rs-renderer/src/scene/prepare/gradient_color_space.rs`, `gradient_program.rs`, and this report. The root-frozen types, prepared M3g stops, M3f radial stage plans and module declarations are unchanged. Sol did not build, run tests/oracles, edit fixtures, change a backend, or commit.

Pinned original Source files: `tiny-skia 0.12.0/src/color.rs` SHA-256 `3cea03557e9e7d8593230149052a11c2062a2439e6ebc2631dac6594e402bd8d`; `src/math.rs` `9bdc3fc49895b286a082f457ce65ace24e1fdb9f5d648c5c078ba646c517e341`; `src/shaders/gradient.rs` `c8a87c87acb85764d5e13c95fd5f66531d24e2c145513e84adfa372d9a878847`; `src/shaders/radial_gradient.rs` `3ae883526cead4d5361f8b6974a91dece8f62fe3740a1712be8ce6219efdd74c`; `src/pipeline/mod.rs` `fbbe3f276b7e95fdf501fe0420f60d18ca313395cdf79aa1a943e9ef996243a4`. BSD Source notices are retained in both implementation files.

## Color expansion and context helper mapping

| Source operation | Native implementation |
| --- | --- |
| `math::approx_powf`, `math.rs:27–56` | `approximate_power` copies both early returns (`x==0 || x==1`), bit-derived `e`/`m`, the ordered f32 log approximation, `floor`, sequential `a` updates, infinity threshold, positive/zero branches, `round` and `from_bits`. It does not call a host `powf` or use f64 intermediates. |
| `ColorSpace::expand_channel`, `color.rs:479–495` | Private `ColorSpace::expand_channel` includes all four branches: identity Linear; `NormalizedF32` multiplication Gamma2; Source approximate power + normalized clamp SimpleSRGB; inclusive `x<=0.04045` piecewise FullSRGBGamma with original add/divide/power order and normalized clamp. |
| `ColorSpace::expand_color`, `color.rs:497–502` | `ColorSpace::expand_color` writes r, then g, then b through the channel helper and returns the original alpha component untouched. It consumes/returns the frozen `StopColor` and does not normalize alpha again. |
| `GradientColor::new`, `pipeline/mod.rs:291–295` | Private `GradientColor::new` copies four raw f32 values into the frozen array; no `[0,1]` constraint is added. |
| `From<Color> for GradientColor`, `pipeline/mod.rs:297–306` | Private `GradientColor::from_color` copies the accepted `StopColor` components in r/g/b/a order. This is a value projection of the Source getters, without adding a new public conversion API. |
| `GradientCtx::push_const_color`, `pipeline/mod.rs:326–331` | Private `GradientContext::push_const_color` appends a zero factor and the supplied color bias, exactly once at the beginning and once at the end of the general path. |
| `RasterPipelineBuilder::push_transform`, `pipeline/mod.rs:370–375` | The wrapper appends `Transform` and stores the composed matrix only when `ts.is_finite() && !ts.is_identity()`; otherwise the frozen program's default identity transform remains. |

`NormalizedF32` and `Transform` come from the pinned `tiny-skia-path` dependency. The accepted M3g `PreparedGradient` already supplies the Source-normalized stops and flags; no stop normalization or opacity update was recomputed here.

## `Gradient::push_stages` complete wrapper mapping

| Original branch or field assignment | Native correspondence |
| --- | --- |
| `radial_gradient.rs:253–259`, radial context before wrapper | `radial_p=[p0,p1]` is copied from the accepted M3f plan **before** the generic wrapper. Eight mask lanes initialize to zero. With no radial plan, both p values and mask remain Source defaults. |
| `gradient.rs:113–121`, SeedShader and inversion | Append `SeedShader` first. Failed `gradient.transform.invert()` returns `returned=false` immediately with the partial stage list, identity effective transform, and default inactive two-stop/general contexts. The earlier radial p assignment survives this return. |
| `gradient.rs:122–125`, transform and pre callback | Compose `inverse.post_concat(points_to_unit)` with the pinned `Transform`; apply the finite/nonidentity push condition; then append M3f `before_color` in its existing order. `None` is the Source empty callback. |
| `gradient.rs:127–143`, spread | Reflect→`ReflectX1`; Repeat→`RepeatX1`; Pad→`PadX1` only when M3g `has_uniform_stops` is true. An irregular Pad gradient has no spread stage, preserving hard-stop general lookup. |
| `gradient.rs:145–162`, exactly two stops | Assert Source's uniform precondition; expand first then second colors with selected ColorSpace; write active `TwoStopContext.factor = c1-c0` by four f32 channels and `bias=c0`; append `EvenlySpaced2StopGradient`. The general context remains default. |
| `gradient.rs:163–177`, general context setup | Create default context; reserve `(stops.len()+1).max(16)` factor/bias capacity and `stops.len()+1` t-value capacity. This branch is selected by stop count other than two, regardless of uniformity. |
| `gradient.rs:179–197`, endpoint exclusion | Compare **unexpanded original** `StopColor`s at the first and last pairs to select `first_stop`/`last_stop`; preserve the Source's fallback pair for the other count branch. Equal colors suppress dummy endpoint contributions; offsets are not used for this choice. |
| `gradient.rs:199–227`, initial and interval context | Expand initial color, append zero factor/color bias and `NormalizedF32::ZERO` t; for each right stop, expand color, assert nondecreasing t and only when `t_l<t_r` append four `(c_r-c_l)/(t_r-t_l)` factors, four `c_l-f*t_l` biases and normalized `t_l`. The arithmetic remains f32 in Source operation order, without clamping raw factors/biases. |
| `gradient.rs:229–250`, hard stops/final/padding | Update `t_l` and `c_l` even for a skipped equal-position interval; append final constant color/t; set `len` **before** padding; assert equal active vector lengths; pad only factor/bias vectors to at least 16 with default zeros; append `Gradient`, then assign the active general context. The two-stop context remains default. |
| `gradient.rs:252–258`, opacity/post/return | Append `Premultiply` only when accepted M3g `colors_are_opaque` is false; then append M3f `after_color` stages, including any `ApplyVectorMask`; finally set `returned=true`. |

The frozen `GradientProgram` therefore records Source stage order, effective transform, both active/inactive context values, radial p/mask initialization, and the original bool return. It is a preparation artifact; there is no sampling, whole-pipeline highp/lowp decision, D2D brush approximation, SVG transit, or Folia integration here.

Unresolved dependency outside this packet: actual highp/lowp stage execution and pixel/color-space blending after this prepared wrapper still require separate shared-consumer work and independent Source/pixel comparison. No issue requiring a change to the frozen M3h types was identified during implementation. Root's frozen 13,556 program, 1,296 expansion and 8,010 power Source observations remain the sole intended parity authority; this report makes no acceptance claim.
