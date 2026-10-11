# Root bounded acceptance: actual shader producer through complete run

Root independently reviewed Sol's native observer and original-source append
wrappers, compiled and executed both, and completed mutation/regression gates.
No production algorithm change was needed. PreparedGradient/PreparedPattern,
the live image borrow and original builder remain alive through production
producer, compile and run. All compiled state is owned by CompiledPipeline;
invocation resources contain only borrowed source/mask/AA and exclusive target.

The main immutable original fixture has 10,016 complete runs, 281 gradient
orders, 80 pattern orders and 1,352 force/spread/color-space/kind/sentinel bands.
Root's separate sampling supplement adds 3,564 runs over all 594 original
dimension/spread/quality/final-transform/positive-opacity/force bands in linear
color space. Both execute unchanged original push_stages exactly once, append
suffixes on that same builder and call unchanged original compile/run. Full
original producer records and warnings reproduce the earlier original gold.
Pattern bytes are asymmetric nonzero premultiplied RGBA. All .rs source bytes
are guarded against anything except the declared appended observers.

`node tools/mermaid-native/shader-run-oracle.cjs --check` and its
`--sampling-boundary --check` mode independently reproduce both fixtures:

- Main SHA-256:
  `a43512800ca4c68bb0ac0c542dd7def3487d3661ee10d4bd43f50e73d07b4e28`.
- Supplement SHA-256:
  `3ca6607eb6d682357ce719f2846b320355629a2677d354e7b803ce3969b0bc0f`.
- Original total: 13,580 runs, zero panics and 2,313 compiled-context mutations.

Native observation compares the entire producer record and warnings, then
precision, width, every full/tail operation identity, panic, all target bytes
and all fifteen compiled-context groups before and after. The separate force
flag remains builder metadata. No expected context or stage is reconstructed
for execution. Final comparison has zero mismatches, stubs and run errors.

`node tools/mermaid-native/shader-run-mutations.cjs` detects all nine compiling
faults: context cloned per run/per tile, borrowed image zeroed, highp/lowp
shader origin dropped, highp/lowp Gradient dispatched as two-stop, Gather
dispatched as Bilinear, and gradient allocations cloned during compile.
The final mutation runner establishes a freshly rebuilt package baseline and
requires an explicit package rebuild for each mutant, assertion failure rather
than crash, and unchanged working files. Its four tests also retain the earlier
13,932 Pattern producer records, 12,872 compiler records and three-allocation
identity witness. Final baseline log SHA-256:
`5f8023251ab0f617f1d25b2670fedc3e357b4392624eb4e56db9ff1567602e6c`.
Final mutation result SHA-256:
`c7e3ef05257f2c6af70ce17741a5bf8f44043c4fb05069854ee2d695903bf3f6`.

The configured complete integration matrix was rerun after adding the sampling
supplement: 25 targets / 68 tests pass, including 11 actual Direct2D WARP tests.
Command uses `--locked --offline --no-default-features --features direct2d
--test '*'`, the ignored build target directory and the actual Arial test font.
Final log SHA-256:
`fcc51f30c5431c253f3838f4d35cea67966c82f5fb81a8d027e857be0e5e3008`.
This does not claim excluded upstream library tests requiring absent documents.

## Rejected intermediate evidence

Gather-to-Bilinear initially compiled and survived the main selection matrix.
Root did not count that as a detected fault or weaken comparison. The original
main fixture remains unchanged; the separate positive-opacity/dimension/
transform supplement detects that dispatch error.

After copying restored source over a previous mutation build, a cached isolated
package produced 688 pixel mismatches despite byte-identical production source.
The working checkout passed all 13,580 rows; a clean isolated package rebuild
also passed. Those intermediate cached results are excluded. The runner now
cleans only its ignored package products before baseline and requires mutant
rebuild evidence. A parallel existing producer test temporarily suppresses the
global panic hook, hiding the allocation assertion text; the final runner uses
one test thread and validates the actual assertion report. The intermediate
failure is not counted as complete acceptance. Neither issue changed gold.

## Remaining boundaries

This closes the controlled shader-to-executor composition gate, not public
Linear/Radial/Sweep constructors or Shader variant dispatch, three independent
blitter contexts, complete retained scene/text/paint, final SVG/Direct2D parity
or Folia replacement. The historical rendering tally does not advance here.
See m3i8-shader-dispatch-audit.md for the next source-mapped responsibilities.
Folia is not connected before the complete acceptance gates pass.
