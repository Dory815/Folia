# Root M2b local source-symbol acceptance

Status: two extracted source helpers locally accepted after root review.
Together with M2a this is 6 of the inventoried 70 render functions, not completed
shared diagram compilation, SVG extraction or native/Folia acceptance.

Root independently read the full edge_decoration_svg/arrowhead_svg source and
the sequence/general callsites before freezing m2b-edge-symbols-contract.md.
Root reviewed every line of the resulting edge_symbols.rs implementation;
source geometry/contours, hierarchy, paint roles/defaults, decimal precision and
arithmetic match the frozen boundary. No algorithm correction was needed after
agent submission. Production render.rs callers still use their original source
functions; no caller switching or alternate native rendering path was added.

## Actual-source observations

symbols-oracle.cjs verifies pristine render.rs SHA256
026efceab06b86752789ea2b5c37ecac569b8cd020f97c776f444bec0a2166b4,
extracts exactly the two unchanged private source functions, and executes them
in a separate root Rust harness with the actual pristine EdgeDecoration enum.
It never imports the new vendor implementation. Reference-only roxmltree
0.21.1/svgtypes 0.16.1 parse actual original SVG into expected typed geometry,
hierarchy, paint/default-style and affine observations. These packages remain
oracle-only; the new scene compiler has no SVG/XML parser/emitter dependency.
Source locks preserve existing Folia dependency resolutions.

286 source observations comprise 208 decorations (8 kinds, both endpoints,
13 width/input combinations) plus 78 arrows. Widths cover both clamp boundaries
with adjacent f32 values, zero/signed zero, small/default-display decimal values
and f32::MAX, including valid intermediate overflow followed by size clamp.
Coordinates/angles cover two-decimal boundaries, signed zero, noncommuting
translation/rotation, end-diamond reversal and huge finite positive/negative
angles/coordinates. Tests compare every contour, vertex, nested group and
circle-before-path order; fill/stroke roles; source default/round caps/joins;
solid dash, miter4 and opacity; quantized translation/width/geometry exact f64
bits and independent source-parsed affine coefficients within 1e-12.

SVG matrix multiplication may normalize signed zero. The oracle separately
records the source translate token to check its exact emitted decimal value;
this does not use implementation-produced expectations. Every accepted helper
result also passes SceneBuilder.finish invariants. Invalid raw coordinates,
angles/widths and negative finite widths fail with the frozen structured kinds.

The first oracle build had two unsupported serde_json macro expression forms.
Root corrected only harness expression parentheses; that compile failure was
not accepted evidence. Final generation and --check reproduce the same fixture.
build/mermaid-symbol-oracle2.log and build/mermaid-m2b-oracle-check.log exit0.
The complete source-generated fixture retains both original SVG and observations.

## Compiling omission checks

All 16 final isolated mutations compile and fail Rust assertions with exit101,
without crashes. Original source/test/fixture bytes remain unchanged. Mutations
cover arrow half/min/max, diamond-end rotation, both circle positions/radius,
missing cross contour, wrong crows-foot branch, filled-diamond paint, caps,
translation ordering, f32 trig substitution, binary width widening, wrong
decimal precision, negative width and complete nonfinite validation omission.

The preliminary run was rejected when removing just the raw finite check
survived: the later parsed-decimal finite guard still rejects those inputs.
This equivalent mutation is not counted as detected. The final nonfinite mutant
omits both guards and is caught. See build/mermaid-m2b-mutations.log for the
rejected run; build/mermaid-m2b-mutations-final.log and
build/mermaid-symbol-mutations/results.json record the successful final suite.

Final relevant checks:

- cargo test --locked --offline --no-default-features --manifest-path
  third_party/mermaid-rs-renderer/Cargo.toml --target-dir build/mermaid-native
  --test edge_symbols_parity: both tests pass, exit0;
  build/mermaid-m2b-focused.log.
- node tools/mermaid-native/symbols-oracle.cjs --check: 286 observations reproduce.
- node tools/mermaid-native/symbol-mutations.cjs: all16 compiling mutants caught.
- With fixed FOLIA_SCENE_TEST_FONT, cargo test --locked --offline
  --no-default-features --features direct2d --manifest-path
  third_party/mermaid-rs-renderer/Cargo.toml --target-dir build/mermaid-native
  --test direct2d_warp --test scene_contracts --test path_parity
  --test edge_symbols_parity: all27 tests pass, exit0;
  build/mermaid-m2b-final.log.

SVG marker definitions, shared colors/text/fallback, all remaining per-family
presentation, source caller extraction and scene-consuming SVG output remain
required. Local geometry/source checks cannot substitute for full source SVG
pixel/metadata/options comparison or Folia device/worker/cache/export acceptance.
