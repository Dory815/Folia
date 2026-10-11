# Root decision: one context per compiled drawing pipeline

The accepted standalone stage projections are insufficient to assemble a complete
Pattern producer. The next architecture packet must consolidate their persistent
fields before implementing that producer. This document fixes ownership and file
duties; it does not authorize an agent implementation or certify new behavior.

## Source ownership to preserve

Original tiny-skia Context contains coverage, sampler, uniform color, two-stop
and general gradient data, two-point-conical data, x/y tile limits and transform.
Pixmap source, destination, mask and AA run resources are separate borrows or
arguments. The compiler consumes its builder and moves that builder's Context
into the compiled pipeline. The blitter constructs three independent builders
and therefore three contexts; they borrow the same source image. A single global
context shared by those three programs would also diverge from Source.

Native must have one `PipelineContext` owner per builder/compiled program.
Gradient, pattern, common sampling and resource stages must read/write that same
store. No independently mutable transform or coverage copies, old/new mode,
producer-specific authoritative contexts or fallback implementations may remain.

## Fixed file responsibilities for the consolidation packet

- `prepare/pipeline_context_types.rs` owns persistent Context value types and
  fields: transform, two-stop/general gradient data, conical parameters/mask,
  tile limits, sampler, uniform color and current coverage. Defaults must match
  the pinned Source Context. Storage is portable and contains no platform API,
  destination bytes, source image copy or run cursor.
- `gradient_program_types.rs` keeps producer result/status and stage metadata;
  its result owns a single `context: PipelineContext`. Former flat context fields
  disappear. `gradient_program.rs` writes that owner directly. A context is moved
  into the eventual compiler/builder consumer, not copied into another owner.
- `sampling/highp.rs` and `lowp.rs` consume `&mut PipelineContext` directly. They
  no longer need a GradientProgram to execute a common Transform or gamma stage.
  All original numeric bodies and register order remain unchanged.
- `sampling/pattern/types.rs` owns the already validated borrowed SourcePixmap
  and an ephemeral PatternContext view containing source plus
  `&PipelineContext`. It holds no independent tile/sampler fields. Pattern highp
  borrows the actual context fields through this view.
- `sampling/resources/types.rs` keeps exclusive destination, immutable mask and
  per-run AA inputs plus `&mut PipelineContext`. Uniform color and coverage move
  to the persistent owner. Resource stages use that owner directly; they do not
  copy those fields into run resources.
- Existing register types, wide providers, precision control and stage ordinals
  retain their current responsibilities. No shared layout/scene or backend is
  reconstructed by a stage adapter. Temporary vector registers are permitted.

Public type re-exports may refer to the same moved type identity, but old storage
fields, duplicated mutable state and old-signature execution overloads must be
removed. Tests must instantiate the new owner and borrow it into each view;
updating an observer binding must not change any original expected value.

## Subsequent producer and compiler boundaries

After consolidation passes all original comparisons, Root will freeze a common
builder whose sole stage list has the Source 32-stage capacity and whose context
is consumed by compile. Pattern producer writes into that builder in Source order;
inverse failure leaves it unchanged, capacity panic may leave a prefix. These are
separate outcomes requiring actual original observations. PreparedPattern owns
normalized opacity and transform/quality/spread while retaining a source image
borrow. Source pixels belong to the scene/resource owner, never an SVG document.

Original Pattern::new, Shader::transform/apply_opacity, Pattern::push_stages and
the actual three blitter construction/run paths need append-only Source observers
before algorithm implementation. Their native counterparts must have actual
callable red tests against frozen original results. No producer port is authorized
by the read-only M3i6 audit or this storage decision alone.

## Acceptance rules

For context consolidation, Root must compare unchanged original gradient program,
sampling, radial, pattern, resource, terminal and blend golds through the new
owner, require tests that expose one view's writes to subsequent consumers, and
pass the full explicit native/WARP matrix. Byte-identical gold remains mandatory.
No duplicate-state adapter may make these checks pass. The containing producer,
executor, blitter, SVG/D2D output parity and Folia gates stay open; function tallies
do not advance for this mechanical ownership correction.
