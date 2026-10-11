# M3i3a Root resource stage freeze

User's complete Mermaid goal remains shared scene preparation with SVG and
Direct2D parallel consumers, followed by Folia only after whole-path acceptance.
This packet fills resource execution in that architecture; it does not reduce
the goal to isolated gradient arithmetic or certify final pixel rendering.

## File responsibilities and authorized implementation

Root owns sampling/resources/types.rs, mod.rs, public signatures, numeric
provider, manifests, Source observers, expected values, tests, builds,
mutations and acceptance. Sol may implement only resources/highp.rs and
resources/lowp.rs plus evidence/m3i3-resource-stage-implementation.md.
Private helpers in those two files are permitted. No delegation or commits.

Existing sampling/highp.rs and lowp.rs keep gradient/register execution;
pipeline_control keeps whole-sequence precision and full/tail compilation.
The new resources files execute external-buffer stages against the same
HighRegisters/LowRegisters. They do not select precision, assemble shaders,
own a new color/gradient cache, or call SVG/Windows/Direct2D APIs.

ResourceContext is a typed projection of original Context's uniform_color and
current_coverage plus the original Pipeline's borrowed destination/mask and
value AA mask. Existing GradientProgram holds the other original Context
fields. No resource field is duplicated between these projections.
DestinationSurface owns no memory; its mutable borrow and MaskSurface's
immutable borrow span the call. real_width is pixels for RGBA stages and bytes
for A8 stages, exactly as Source SubPixmapMut. The producer must provide valid
stride, storage and cursor; this packet does not invent image bounds or a
generic surface allocator. ResourceProgram Full/Tail is the actual compiler
function-family choice, not precision or a coverage heuristic.

Highp owns ordinals 5,7,8,13,14,15,16,17,45. Lowp additionally owns 9,10,12.
Full/tail variants of 7,8,9,10,45 bind separately using cursor.program;
all other stages use the same body in both program lists. cursor.tail remains
the active lane count for masks/AA. Each function executes one stage and
returns Continue in place of next_stage, except MaskU8 returns Stop when its
entire coverage vector is zero, before scaling any register. Stop terminates
only this invocation, allowing later chunks/rows to execute.

Keep original highp f32x8 and lowp u16x16 arithmetic through the accepted
sampling/wide provider; do not replace SSE min/max or round_int with scalar
clamping/rounding, do not use FMA/tolerances, and do not normalize lowp bytes
through a float highp substitute. Preserve NaN/signed-zero bits and exact
div255 `(v+255)>>8`, lerp order, quantization, and u16 operations. UniformColor's
float and byte forms are independent precomputed Source fields. AAMask is the
original two-byte coverage run, not a general scanline mask: its three legal
offset/tail results and zero result elsewhere must remain exact. SourceOverRgba
loads from the destination, changes destination registers, blends source RGBA
and stores back to that same mutable destination in order. Tail loads pad with
zero and tail stores leave all inactive bytes unchanged.

All other ordinals return Unsupported with the exact precision/stage before
modifying anything. Source highp 9,10,12 are marked unreachable and are not
called by the normal A8 blitter; this bounded packet excludes force-high A8
experiments. This is an explicit incomplete consumer boundary, not a claim
that Source arbitrary builder sequences error in the same way. Blend 18–44,
Gather/filter/pattern resources, full blitter assembly and final consumers are
still required. Remove Unimplemented results from delivered owned bodies.

## Root red and independent original evidence

resource-stage-oracle.cjs verifies five pinned Source files, copies the original
tiny-skia 0.12.0 crate and appends read-only observers. Actual original compiler
chooses its original precision/function/tail pointers; setup/capture are
interleaved after compiling in a separate Vec. Original start and pointers
execute with initialized borrowed pixel/mask data. Captures contain raw
registers, cursor and complete destination bytes, including invocation setup.
No native Mermaid equation supplies any expected value. Dummy 1x1 source pixmap
is unused by this packet and does not claim pattern sampling.

There are 2,959 cases (1,276 highp / 1,683 lowp), 13,281 captures: single stages,
ordered uniform/load/coverage/store/fused paths, RGBA and A8 strides, one/two
rows, 1/2/7/8/9/15/16/17/33 lengths, sparse/all-zero clip masks, AA offsets,
byte and float boundary values, nonfinite Store inputs. All destination bytes
are checked, including outside the active region. Fixture SHA256:
281b3ecf96096f0a8f4bf810feedf6dcce359c8dd0cacc309aec3e537999a122.

The native fixture compiled successfully against the actual frozen APIs.
Its three tests then exited 101 through the actual Unimplemented bodies,
showing both High and Low UniformColor entrypoints and the unsupported gate.
The initial missing Cargo test registration was rejected as red evidence and
fixed before this successful compile/execution. Raw red build SHA256:
1226b8b17d4b6f2155b325eee1d5897f6c1cf8d315ed4e5999d8b88d6277c78c;
raw red test SHA256:
18c0f07d326f375c2173cb8754ced85877b847e4ff8d230364d024431321ba55.
Files are build/m3i3-resource-red-{build,test}.log.

After delivery Root must review bodies against Source, reproduce oracle/golds,
run all 2,959 native comparisons, compile semantic mutations and require
assertion failures (no crashes/build failures), then run the existing native
matrix. No 34/70 tally advancement, complete-paint/WARP/Folia gate or arbitrary
resource/platform acceptance follows merely from this packet.
