# Root acceptance of the first primitive packets

This is local acceptance of P1 Attributes, P2 BBox and P3a PrioritizedList only.
It does not pass full G0, G1, G2 or G4, and is not permission to integrate a
native renderer. Options, MML nodes, TeX, fonts, common wrappers, shared scenes
and SVG are not implemented by this batch.

Root reviewed all three pinned source files and every method against each
submitted native module. The separate packet ledgers enumerate the mapping.
Shared PropertyList storage and numeric constants were designed by root before
dispatch; agents could edit only their module and evidence report. Root alone
owns build integration, source inventory, fixtures and acceptance tools.

## Evidence

- Pinned source and installed executable-source equality verified by
  `node tools/math-port/primitive-oracle.cjs --check`.
- Root-generated upstream fixtures exercise each primitive method, attribute
  views and live maps, JS integer-key ordering, built-in/prototype identity,
  missing/false/zero/empty distinctions, BBox full-field state and seeded
  operations, stable priority ties, backward duplicate removal, NaN/Infinity,
  shared defaults across native types, and live iteration with removed records.
- `cmd /c build_test.bat "*math_port*" 2>&1` compiles C++23 modules and executes
  all 1,223 assertions across three tests without compiler warnings.
  `cmd /c build_test.bat 2>&1` passes the complete core suite.
- `node tools/math-port/mutation-test.cjs` compiles an isolated baseline and
  four wrong native implementations. All wrong implementations compile but
  fail frozen assertions: inherited lookup flattened to own lookup; BBox left
  spacing removed; priority ties reversed; default priority split by type.
  Logs/results are ignored under `build/math-port-mutations`. Working source
  files are never mutated by this tool.

## Review corrections and explicit adaptations

Root rejected the initial priority implementation's per-type static default
and dangling yielded records on removal. The corrected port has one default
backing value, shared ownership and a cursor-local record cache. A second review
corrected end-sentinel reflexivity and default iterator construction for native
callers. C++ iterator adaptation checks are included in addition to JS snapshots.

Static BBox.empty becomes makeEmpty because C++ cannot overload that signature
with instance empty. Metadata fullWidth/boxSides are read-only native constants;
the pinned production profile only reads them. Rest arguments become vectors.
Prototype storage uses native shared handles; built-in function identities are
opaque, not callable. Those are language/host adaptations, not replacement
algorithms. The mathematical bodies of BBox and priority insertion/removal
retain upstream expression and comparison order.

## Remaining scope and limits

Only three complete upstream files are locally accepted. Node.ts Property
declarations and lengths.ts BIGDIMEN are extracted prerequisites; their remaining
methods are not accepted. The scope manifest deliberately leaves those and the
other dependency files unreviewed. Native iterator references are retained for
the current loop body; persistent users must own identity handles rather than
borrow a reference past increment. Scalar/property dictionary contracts do not
claim to implement arbitrary JavaScript functions or object descriptors.

The full renderer oracle still needs semantic MML, full-precision wrapper
geometry, all fragments/paint/clip/viewport and raster mutation checks. Primitive
tests cannot establish those gates. Folia's production backend is still the
existing MathJax service; this library is linked into tests only.
