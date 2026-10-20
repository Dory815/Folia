# Native RAII math acceptance

These gates replace JS-reflection/identity compatibility acceptance. The pinned
MathJax 4.1.3 commit, 38 packages, NewCM and extensions remain unchanged. Earlier
green corpora establish reference data, not acceptance for rewritten code.

## A. Architecture and build graph

- Every math implementation file has a keep/rewrite/remove/regenerate decision,
  a native responsibility, Source provenance and a named owner.
- Name files and modules by their responsibility, retaining the upstream owner
  where it still applies. Do not use native/new/typed suffixes or prefixes to
  keep competing implementations beside the canonical owner. Retire obsolete
  files after accounting for their remaining consumers; renaming is not removal.
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
- Both backends share one completed mathematical layout and placement. Source-required
  package subtree measurements may repeat during compilation using the same
  Common implementation. SVG depends on completed Scene,
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

- Mathtools must preserve all six upstream file responsibilities and every
  implemented mapping. Compare matrices/cases/multlined/spreadlines, alignment
  commands, paired delimiters and expansion limits, relations/options, struts,
  laps/brackets/fractions and prescripts with actual Source MML. Check null/void
  dispatch results, pre/postscript holes and postfilter priority -6. Exercise
  named forms, renewal of a selected name, empty selection, errors/recovery,
  none/all/ams/cases/custom parents and both orders of Tagformat composition.
  Repeated input-jax construction must release forms and local creators without
  growing a shared registry. Closed typed option shapes replace arbitrary JS
  object/array mutation; report that boundary separately from rendered fidelity.
  Preserve GetArgCount's absent/undefined default separately from explicit empty
  brackets, the truthy string "0", and an explicitly supplied null argument.
  Retain all 42 historical formulas and verify actual extpfeil/Mathtools override
  order; obsolete reflection-dump comparisons are not mathematical acceptance.

- Colortbl must compare actual Source trees for cell/row/column precedence,
  clearing, sparse columns, preambles, overlap arguments and each array wrapper.
  Preserve unset versus defined-empty column values, color-argument errors before
  placement errors, and row-color writes before RowColorNotFirst. Exercise virtual
  createMml through ArrayItem, not only direct derived calls. Check that the real
  registered color config callback creates a missing model without enabling its
  commands, preserves an existing model, and keeps independent sessions isolated.

- Cases must compare actual Source MML, text-column scanning, ordinary-array
  fallback, tags/labels/references, tag modes/sides and configured formatters.
  The first matching end must pop one activation through an empty replay;
  Entry's explicit null must continue character dispatch to the base map.
  Verify explicit zero and UTF-16 subnumber wrapping, and preserve each error's
  consumed equation numbering in the next request rather than resetting it.

- Empheq must compare actual Source trees for all allowed environments and
  starred forms, side material and labels, all fixed/delimiter commands, and
  direct table transformations. Check creator dispatch, copied-node isolation,
  parent links and complete inferred-child transfer. Preserve false/empty side
  disabling and distinguish Source type failures from successful fixtures;
  neither arbitrary object mutation nor a successful SVG pipeline is implied.

- Setoptions must verify every Source option name against the finite native
  schema, distinguish absent fields from false/empty values, preserve the exact
  default permissions and error order, and exercise actual following formulas.
  Cover regex conversion, captured construction-time patterns, numeric limits,
  skipped/custom filters, partial writes, groups, persistence, macro priority,
  immutable aliases and independent input-jax state. A field-access check for a
  package not yet migrated does not establish that package's rendering fidelity.
  Report native type-domain rejections separately from Source-compatible cases;
  accepting arbitrary JS object/array/function mutation is not this native API.

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

### Bussproofs component boundary (2026-10-20)

The proof-frame component compares pinned Source trees, proof markers, replay
order and TeX errors for empty/direct/multiple contents, ignored toMml flags,
ordinary/grouped prefixes, matching/mismatched end, stop and unfinished groups.
Verify the actual same frame transfers into the inner Stack, remains alive
after the outer Drop, and is destroyed after close or owning proof destruction;
test completion through the real outer begin/proof/end/stop replay. Repeat mixed
successful and failing frame scopes and confirm all tracked frames are released.
No full Bussproofs or SVG claim follows from these component tests: real Common
subtree widths, sequent/rule adjustments, semantic attributes and package filter
ordering still require end-to-end Source comparison after integration.

For Common subtree measurement, compare real Source getBBox outputs for
fixed-space, padded/styled, nested fraction/radical/table and token layouts
using the same profile fonts. Verify root/physical parents/attributes/layout
flags are unchanged apart from Source's intended setTeXclass state. A later
MML change must affect the next measurement, with no cached wrapper reuse.
Track actual custom wrappers/visitors through repeated successes and throws;
each call destroys its transient arena, and no BBox result borrows it. Exercise
supplied class registrations, metrics and MeasurementTarget callbacks as well
as a foreign-document rejection. Compile tests must observe the same output
borrow in both filter phases, null on a later unbound compile, correct nested
borrows, and recovery after pre/postfilter exceptions without retained service
or document ownership.
The per-call arena claim concerns Common wrappers/visitors. Source-generated
auxiliary MML stays document-owned: report its growth under repeated measures
on one document explicitly. At least 192 independent document requests with
radicals/bevelled fractions must release every tracked auxiliary MML node on
request destruction; a persistent session may not retain those documents.

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
