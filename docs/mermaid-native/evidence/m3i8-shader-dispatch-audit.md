# Root next dependency frontier after controlled shader runs

This is a read-only audit and responsibility decision, not constructor,
variant, blitter, backend or Folia acceptance. Sol supplied the upstream/local
comparison; Root retains contracts, original fixtures and independent gates.

The next implementation wave should close public Shader construction and
dispatch before RasterPipelineBlitter. Existing scene Rgba/LinearGradient are
f64 retained descriptions with ids; they cannot substitute for an original
f32 prepared shader with normalized stops and borrowed Pattern pixels.
The controlled producer/run fixtures do not observe public constructors'
None/SolidColor/actual-gradient selection.

Preserve original source responsibilities when freezing the next interfaces:

- One prepared Shader enum/dispatch boundary for five variants, borrowing
  Pattern pixels. is_opaque, transform, apply_opacity and push_stages stay at
  that boundary; no backend-specific shader variants or duplicated state.
- Linear construction belongs with the existing linear producer, radial
  construction with the radial producer and its accepted geometry helper.
  Do not accumulate all three constructors in one generic implementation file.
- Sweep construction and its stage producer need their own source-mapped
  module. It writes conical p0/p1 and selects XYToUnitAngle and optional
  ApplyConcentricScaleBias through the same builder.
- PipelineBuilder owns the narrow missing push_uniform_color operation,
  including source f32 components and four original u16 quantized values.
  SolidColor expansion/premultiplication is Shader dispatch responsibility.

Root must freeze complete original factory observations before delegation:
empty/single stops, invalid or degenerate geometry, inverse-transform failures,
linear Pad/end-color vs Reflect/Repeat weighted fallback; radial negative
radius, concentric/Strip/Focal/swap/mapping and Pad thin-ring hard-stop rewrite;
sweep invalid angles, degenerate recursive stop rewrite and whole-circle Pad.
Observe rejection, selected variant, all raw color/stop/matrix/kind values,
warnings, subsequent dispatch and stage/context output. Existing controlled
producer gold cannot establish these decisions. None of these bodies is
assigned by this audit.

Original RasterPipelineBlitter::new creates anti_h, rect and mask builders
separately and calls shader.push_stages once for each. Each compiled pipeline
owns an independent context. Anti_h's coverage updates must affect its owner
only. Pattern retains its original image borrow and destination is exclusive;
non-pattern variants use the original dummy image. Mask dimensions, opacity
fast rejects, SourceOver strength reduction, Clear/solid memset, destination
color expansion and compression precede the three run-entry acceptance gates.
new_mask separately builds three U8 pipelines from white uniform color.
Reusing one compiled shader-run pipeline or cloning one context for all three
is not the original ownership architecture.

Pinned tiny-skia 0.12.0 source SHA-256:

| Source | SHA-256 |
|---|---|
| shaders/mod.rs | 4f80ecb40d5a82985d97324748779cffd98252fca798c01c3f26969a1c27cbd3 |
| shaders/sweep_gradient.rs | 7e9c4223a5be5b78eafa20b7869d269732bd6c950e72abf70bfd475c4ed58816 |
| pipeline/blitter.rs | a3189191d6ad4e1e03e738d21f5a26f310b7c00c33bae0021f0961054ad08f06 |
| blend_mode.rs | 841dcc8eb96fe11fc2316fa0a4ff47c29fd837990ce8c1d8be5fcf08f6c86d19 |

Linear/radial/gradient/pipeline pins remain those of the M3i7b contract.
Complete retained scene, text/paint, shared SVG/Direct2D comparison and final
Folia replacement still follow these dependencies. Rendering tally stays
historical; passing a controlled producer/run gate does not advance it.
