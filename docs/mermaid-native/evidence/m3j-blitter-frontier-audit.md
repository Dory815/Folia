# Root next blitter frontier after public Shader acceptance

Read-only audit and responsibility decision. Sol compared original and native
dependencies; Root inspected the original constructor and policy bodies.
No blitter implementation, declaration packet, Source gold, backend or Folia
acceptance is created by this audit. Public Shader acceptance is ccee6f3.

Original pipeline/blitter.rs constructs anti_h, rect, mask in that order, each
from a fresh builder, each invoking shader.push_stages once and compiling its
own context. Root will preserve three independently owned CompiledPipeline
values. No clone of one builder/context, SVG intermediate or fourth retained
context. Calls temporarily reborrow source/mask/AA/target into existing
PipelineResources. Anti_h current_coverage must mutate only its own context.

Constructor dependencies that must be frozen before assignment:

- A prepared Paint containing the existing borrowed PreparedShader and original
  blend/anti_alias/color-space/force values. Scene's f64 paint descriptions and
  sixteen CSS blend values cannot replace this original raster boundary.
- Complete original BlendMode enum and should_pre_scale_coverage/to_stage
  policy in a narrow source-mapped module; color-space destination expansion
  and compression remain color responsibility. Source color f32-to-u8 conversion
  is needed for memset, separate from existing uniform u16 context quantization.
- Borrowed destination and optional mask descriptors carrying original logical
  width/height, backing bytes and real stride. None and Some remain distinct;
  current executor surfaces lack constructor dimension checks and memset bounds.
- Original sparse AlphaRun/coverage representation and all Blitter entry points.
  No dense-alpha reinterpretation. Keep AA shift from original bounds.left +
  bounds.top*row_bytes; it is not derived from clipped draw rectangles.

new first checks mask logical dimensions/warning, Destination and opaque solid
DestinationIn fast reject, opaque SourceOver strength reduction, solid Source
memset and non-AA Clear memset. Each of the three pipelines then appends its
own original coverage/destination/expand/blend/compress/store sequence. Rect's
unmasked SourceOverRgba branch differs from anti_h and mask. new_mask constructs
three separate white-uniform U8 programs even if rect later uses memset.

Root original oracle must actually call new/new_mask and Blitter methods,
with appended read-only observations. Record rejected/accepted state, warnings,
memset/is_mask, three compiled full/tail instruction identities and complete
contexts, source borrow identity and complete target backing bytes after each
callback. Stimuli span five Shader variants and opacity, every blend policy
class, four color spaces, force/AA, absent/matching/mismatched masks, nonzero
asymmetric Pattern, logical subview/real stride and untouched backing regions.
Execution spans rect, h/v, sparse anti_h zero/full/intermediate coverage with
consecutive calls, anti_h2/v2 and mask, nonzero origins, multiple rows and
8/16-lane full/tail boundaries. Stage-only or one-pipeline fixtures are inadequate.

Root next freezes original policy/borrow dependencies and Source construction
records before any blitter algorithm assignment. All expected values come from
original code; native declarations must compile and real stub calls must fail
before Sol implementation. Production integration still waits for complete
retained-scene, text/paint, SVG/Direct2D comparison and Folia acceptance.

Pinned tiny-skia 0.12.0 SHA-256:

| Original file | SHA-256 |
|---|---|
| pipeline/blitter.rs | a3189191d6ad4e1e03e738d21f5a26f310b7c00c33bae0021f0961054ad08f06 |
| blitter.rs | 3262acaac9f2a78736295e937eee9542667b1de07b5a23db554830b192eb929e |
| mask.rs | 5c6b8d615f219f78ceebc714a56e9916745eeb93fc9213b87c96b3f00f55792b |
| pixmap.rs | 484d49d1f4c3c3c8fb3fab3e1b1535292de828c0f258c3cab9bda02c7d9626fc |
| painter.rs | 40f1d9523ab9875d83496cb7a1b4570aee2d958204dc91424fb6a5bd00c97105 |
| blend_mode.rs | 841dcc8eb96fe11fc2316fa0a4ff47c29fd837990ce8c1d8be5fcf08f6c86d19 |
| color.rs | 3cea03557e9e7d8593230149052a11c2062a2439e6ebc2631dac6594e402bd8d |
