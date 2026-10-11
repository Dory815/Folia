# Root frozen shader producer to executor acceptance boundary

This packet adds a native observer only. Sol may edit exactly
`third_party/mermaid-rs-renderer/tests/shader_run_parity.rs` and append its
explicit `[[test]]` entry in that crate's Cargo.toml. No production source,
original-source observers, fixture, lock/pin, old tests, Folia, build execution,
commit or delegation. Report the binding map and leave independent execution
and acceptance to Root. Missing production behavior must remain visible.

Root's append-only original tiny-skia 0.12.0 observer reconstructs each actual
Gradient or Pattern from previous fixture inputs, calls original push_stages
exactly once and compares its complete precompile record and warnings to the
unchanged old Source gold. It appends terminal stages on that same live builder,
compiles it once, and calls unchanged RasterPipeline::run. Pattern borrows the
input's nonzero asymmetric premultiplied RGBA bytes. No stage substitution,
capture operation, register seeding or deserialized context reconstruction.
All original .rs bytes are checked unchanged except exact appended observers.

Frozen `fixtures/shader-run.json` SHA-256:
`a43512800ca4c68bb0ac0c542dd7def3487d3661ee10d4bd43f50e73d07b4e28`.
10,016 cases, 281 original gradient stage orders, 80 original pattern orders;
872 gradient and 480 pattern force/spread/color-space/kind/sentinel bands.
Every selected original order is run at widths 7/8/9/15/16/17, nonzero origin
and two rows, with Store, SourceOverRgba, LoadDestination+SourceOver+Store,
and MaskU8+SourceOverRgba suffixes. Original run reports zero panics and 2,313
persistent-context mutations. This uses the main 13,284-case pattern fixture;
the separate 648-case pattern boundary is not silently merged into its input.

Native binding must keep prepared shader and borrowed image alive, invoke the
existing push_linear_gradient_stages/push_radial_gradient_stages or
PreparedPattern APIs once, project the original producer record, append the
frozen suffix on the same PipelineBuilder, then compile and call run_pipeline
using the same compiled context as sole owner. Compile consumes the builder.
Do not copy a context into invocation resources, reconstruct it from expected
groups, call individual stage APIs or maintain an alternative executor/cache.
Use current tagged pipeline operations and existing SourcePixmap/resource
borrows. Only destination bytes are copied for mutable per-case execution.

Compare entire producer record and warning vector, then compiled precision,
width, complete full/tail operation identities, panic status, every destination
byte and every compiled-context group before/after. Compiled context groups
are the first fifteen of the existing producer group projection; the sixteenth
builder force flag is not Context. Detect and report Unimplemented, other
PipelineRunError and unexpected panic separately. Count all cases and make
zero mismatches mandatory. Diagnostic output should identify first mismatches
by case/source index and differing categories without dumping full fixtures.

Source transport's result is `[producer-record, run-record]` for both families.
Inputs retain `family`, `source_index`, exact old `producer`, nonzero `image`,
terminal `ids` and complete resource/rect data. Gradient construction retains
the current controlled private Gradient/GradientType profile; public gradient
constructors, SolidColor/Sweep dispatch and three original blitter contexts
remain excluded. Passing this gate is not overall Mermaid/Folia acceptance.

Root requires unchanged original `--check`, full native comparison, effective
compiling context/image/dispatch faults and existing relevant regressions.
Any runtime mismatch goes into a separate Root-defined implementation packet
with precise file/body ownership before Sol changes production algorithms.
