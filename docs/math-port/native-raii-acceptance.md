# Native RAII math acceptance

These gates replace JS-reflection/identity compatibility acceptance. The pinned
MathJax 4.1.3 commit, 38 packages, NewCM and extensions remain unchanged. Earlier
green corpora establish reference data, not acceptance for rewritten code.

## A. Architecture and build graph

- Every math implementation file has a keep/rewrite/remove/regenerate decision,
  a native responsibility, Source provenance and a named owner.
- Start from the pinned upstream file and family responsibility. Each proposed
  new file/module must explain why an existing owner cannot contain the work,
  identify actual consumers, and identify the implementation it replaces.
  Native numerical family work replaces the canonical Common wrapper files;
  it must not coexist with a competing placement or Scene wrapper hierarchy.
- Audit both the active dependency graph and obsolete code remaining on disk.
  Record missing production callers explicitly. Passing isolated numerical
  tests cannot establish that CommonWrapper or the renderer is connected.
- The active `folia_math` graph contains no Context/ObjectRef/Descriptor,
  prototype services, universal `Value` call/construct machinery, arbitrary
  payloads, identity registries or manual request retirement scopes.
- Check transitive imports and linked source lists, not only spelling searches.
  Inspect implementation for renamed equivalent object-heap machinery.
- Common measurement and placement run once. SVG depends on completed Scene,
  never the TeX parser or mutable MML/wrappers. Scene contains no raw SVG DOM,
  parser pointers or callbacks into a request. The future D2D boundary is the
  same Scene interface; implementing that backend is not this SVG gate.
- Production C++23 modules use `import std;`; math is platform independent.
  Windows console runners link `folia_console_test_runtime`, with stderr-only
  CRT/abort reporting. At most four compiler jobs and one build at a time.

## B. Ownership and mathematical structures

- Test MML create/link/unlink/replace/move/clone, token text, attribute
  own/default/inherited precedence, reparenting, cycle rejection and foreign
  document rejection. Incomplete/error math nodes retain their semantic role.
- Instrument actual request-owned allocator/node/wrapper lifetime in tests.
  Every return, exception, TeX error and cancellation returns transient live
  counts and outstanding bytes to baseline after retained results are accounted
  for. Inspect ownership as well as counters; do not redefine the counters to
  hide retained storage.
- Repeat mixed render and scene-only requests in measured phases (at least
  192 requests, then a longer run). With fixed semantic session state, there
  is no per-request growth in live nodes, wrappers, transient allocations,
  renderer caches or package scratch. RSS is corroborating evidence, since an
  allocator may retain freed capacity. Report time and cleanup cost too.
- Test persistent macros, labels, references and package state separately.
  Their intentional updates match Source. No persistent object borrows a
  request; frozen Scenes and SVG bytes survive later renders and destruction
  of their Renderer.
- Exercise errors/cancellation at real pipeline stages using bounded test
  observation seams, without adding production behavior switches. Verify
  cleanup and the Source-defined state of the next request.

## C. Source fidelity

- Tagformat must compare real TeX/MML output for none/all/ams, explicit and
  automatic tags, references, all five format callbacks, ref fallback, string
  and vector tag results, Unicode IDs/URLs, option updates and error recovery.
  Test custom parent behavior and formatting precedence independently. Repeated
  construction/destruction must release selected Tags and captured factories;
  unrelated instances and shared package registries must not accumulate entries.
  Source's generated class names are internal registration identities, replaced
  by a configuration-local creator; they are not the rendered-output oracle.

- Freshly build and observe the current native revision against independently
  regenerated, hash-locked Source results. Preserve corpus/profile/source pins.
- Compare all 75 common-layout cases, 150 complete SVG records byte-for-byte,
  paired Scene geometry and the four meaningful interaction-position cases.
  Preserve font tables, instance extensions, 40-module/80-request/24,646-glyph
  provider coverage and existing package-specific/malformed-input corpora.
- Add focused native tests at each changed mathematical algorithm boundary:
  TeX expansion/scopes, operator classes/spacing, inherited attributes,
  fractions, scripts/limits, radicals, stretch delimiters, tables, line breaks,
  colors/enclosures/actions, HTML/annotations, tags and errors.
- Keep Source-paired case data separate from a compact table-driven test runner.
  Commit the reproducible hash-locked Source witness, not thousands of repeated
  generated C++ assertion blocks. Preserve case/field diagnostics and exact
  comparisons; splitting a generated test into many source files is not a fix.
- C++ registration and option types need their own typed contracts. JS
  property descriptors, prototype replacement, Function.call mutation, sparse
  JS array holes and arbitrary JS coercion are withdrawn API requirements.
  Retire only those tests, with an explicit reason; retain their mathematical
  behavior coverage through typed tests and Source output comparisons.
- Missing package/node/primitive support is a failing/incomplete gate, never
  silent omission, reduced scope, hardcoded formula output or legacy fallback.

## D. Application integration

- Validate opaque C ABI UTF-16 input, result ownership, thread contract,
  cancellation, deadlines and errors against the accepted native library.
- Exercise the default real Folia worker through queue -> C ABI -> native math
  -> SVG parser. Include mathbb, fractions, malformed TeX, repeated cache keys,
  generation changes and shutdown. Preserve worker scheduling/cache behavior.
- Use the existing SVG path. No extra D2D pixel gate is required. No QuickJS
  math fallback or native runtime-object shim is allowed after switching.
- Restore with `setup.ps1`, build the supported WinUI configuration, and run
  relevant deterministic platform tests. Keep Mermaid's dependencies separate.

## Evidence and failure handling

Root owns builds, executions, target edits, registered source promotion and Git.
Freeze inputs during a build/evaluation. Record revision, source hashes, actual
exit status, commands, counts and limitations. Stop a broader evaluation on an
abnormal exit and diagnose with a bounded focused run; no desktop error dialogs.
Compile success, static review or a Source-only witness is never native runtime
acceptance. Commit small coherent batches, preserving unrelated work.
