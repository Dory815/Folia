# Shared native math checkpoints 102–106

These checkpoints compile the current native parser, Common layout, completed
Scene and SVG path. They do not accept the full Folia package profile.

Gates102–105 failed compilation or linking. Their input audits reported no
changed files during their respective builds. Repairs addressed actual API
types, explicit module imports and test declarations. Gate105's link error
identified a wrongly owned NodeFactory forward declaration in the parser
module: the runtime now imports the actual node_factory module instead.

Gate106 compiles and links all 31 targets with 1,804 captured inputs unchanged.
Its log is `build/math-font-package-gate-106.log`; source hashes are in
`build/math-port/gate-106-inputs-before.json`. Runtime executables were copied
to `build/math-port/gate106-runners` and each invocation records hashes, PID,
UTC start/end and the real exit code.

The completed compile contains the lower Scene foundation module and injected
Menclose service, avoiding a Scene import of concrete SVG services. Text size,
numeric transforms, stroke dashes and viewport policies are retained as typed
Scene data. These structural changes still require runtime acceptance.

Observed runtime results:

- Host, Regexp and LinkedList pass. Regexp includes 18,518 assertions over
  41 tests. The generic legacy brace correction therefore retains that suite.
- Core exits 1 with two SourceStringMap observation failures: a present
  undefined value is treated as absent, and an exhausted key iterator resumes
  after insertion. Its process runs from 06:10:24 to 06:13:23 UTC on October15.
- Parser exits 1. Foundation exits 1; its seven Scene Context tests pass
  56 assertions, while four new layout-data tests are blocked during TeX parse.
- All 75 full renderer records fail, with zero successful layouts. The first
  common runtime error is `this.parsers.unshift is not a function` after the
  parser changed to the actual live Array operations. Registration remains
  23 of 38 required packages with 15 real warnings.
- Physics exits 2 during initialization because it looks for BaseItem in the
  concrete BaseItems export table rather than the actual StackItem module.
  Next-six exits 2 because its borrowed fixture MML factory lacks a canonical
  Context identity. Font-package observation also exits 2.
- Colortbl emits all 29 records and its command map matches, but no complete
  case matches: its old observer supplies a non-canonical MathItem. Menclose emits its 30 error records
  and exits 3 because no completed Scene exists; its Context does release.
- The public API emits nine error records and retains no successful Scene;
  its earlier isolated nine-case pass is not a pass for this checkpoint.

Detailed records remain under `build/math-port/*106*`, including
`diagnostic-exits-106-root.json` and `common-layout-comparison-g106.json`.
Subsequent candidates address these failures at their actual owner boundaries.
No new package is accepted or registered from these observations. Folia still
uses its existing math worker; Mermaid remains paused.

Gate107 subsequently failed one ParseOptions implementation unit while all
1,804 captured inputs remained unchanged. Its setter passed a Context object
identity to a lookup that accepts a shared native ParseOptions owner. The
owner corrected that boundary before Gate108; this failed build supplies no
new runtime acceptance result.
