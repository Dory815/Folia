# Current native MathJax status

The goal remains native TeX -> shared mathematical layout -> SVG, then Folia
integration replacing the math QuickJS worker. Native conversion runs, but the
renderer has not passed acceptance; Folia still uses its QuickJS MathJax path.

Latest shared evaluation (2026-10-15 local, Gate134): all 51 targets compile
with 1,968 captured inputs unchanged. Complete Core, Host, Parser, Foundation,
Regexp and LinkedList pass. Full rendering now matches 74/75: tooltip's entire
record matches through the shared layout-observation boundary. Bussproofs
remains unregistered. Public API stays 9/9 and the complete texttip SVG stays
exact. The standalone TeX fixtures now match Source construction; fonts 13/16
and next-six 18/20 expose a missing configMethod.items field next. TexHtml and
DOMStrings pass; two independent XML namespace differences remain. Stack,
parser, factory fields, XML creator, lifecycle and cancellation remain required.
See `evidence/shared-gate134.md`. Folia is unchanged; Mermaid remains paused.

Earlier shared evaluation (2026-10-15 local, Gate133): all 51 targets compile
with 1,965 captured inputs unchanged. Complete Core, Host, Parser, Regexp and
LinkedList pass. Foundation's XML font fallback test now passes; its one new
tooltip test has the wrong branch expectation and requires correction.
Rendering remains 73/75: tooltip now reaches the exact Source error, 23 calls
and 21 paints, but lacks its nested layout observer snapshot; Bussproofs remains
unregistered. Public API is 9/9 and the complete texttip SVG remains exact.
Standalone XML captures again complete, revealing two extra native-check
envelope fields as well as the two existing namespace differences. These are
not accepted. Fonts/next-six still expose their standalone fixture and pending
Stack/parser fields. Required owner, XML creator, lifetime and cancellation
packets remain under review. See `evidence/shared-gate133.md`. Folia is
unchanged and Mermaid remains paused.

Earlier shared evaluation (2026-10-15 local, Gate131): all 51 targets compile
with 1,963 captured inputs unchanged. Complete rendering now matches 73/75;
both TexHtml examples and valid Empheq match. Public API remains 9/9 and the
complete texttip SVG stays exact. Remaining renderer records are tooltip
(missing Context processMath method) and unregistered Bussproofs. This is not
an accepted checkpoint: Core has four failing new test fixtures/assertions,
Foundation has one failing new font-family test, and the XML capture exits
with an eager Lite-node argument check. Tags metadata matches its pinned CJS
witness; the fonts/next-six comparisons now expose a standalone Jax adaptor
setup difference and the pending parser/Stack alias. Stack/parser, XML creator,
factory field, lifetime and cancellation packets remain under review. See
`evidence/shared-gate131.md`. Folia is unchanged and Mermaid remains paused.

Earlier complete shared checkpoint (2026-10-15 local, Gate129): all 51 targets
compile with all 1,957 captured inputs unchanged. Complete Core, Host, Parser,
Foundation, Regexp and LinkedList pass. Public SVG remains 9/9 and texttip's
complete SVG remains byte-exact. Complete rendering matches 70/75, with 37/38
registered packages and one warning. MML fields/factory and Empheq now compile
and participate in the real session; the invalid Empheq option record matches.
Remaining renderer records are math tooltip, Bussproofs, valid Empheq and both
TexHtml examples. Next-six remains 18/20 and fonts 13/16, now exposing Tags
method metadata and a factory alias difference beyond the MML fields.
The independent TexHtml/XML comparison has two namespace failures; earlier
Gate126 reporting incorrectly said all twelve records passed. Its raw report
already contained those failures. Lifetime and cancellation implementation
packets are drafted, not accepted; Folia integration remains pending. See
`evidence/shared-gate129.md`. Mermaid remains paused.

Earlier complete shared checkpoint (2026-10-15 local, Gate126): all 51 targets
compile with all 1,948 captured inputs unchanged. Core, Host, Parser,
Foundation, Regexp and LinkedList pass; public SVG remains 9/9. Complete
rendering matches 69/75, with 36/38 registered packages and two warnings.
Cases, Centernot, Textcomp, HTML and TexHtml now join the actual session with
their canonical module owners. The independent XML comparison has two
namespace regressions (corrected reporting after Gate129). Gate127 changes
only the texttip witness options to match the pinned
profile: its complete 1,315-character SVG now matches Source exactly; that gate
does not claim another complete 75-record run. Remaining full-renderer cases
are math tooltip, Bussproofs, both Empheq examples and both TexHtml examples.
MML field/factory, Empheq and Stack packets are saved but remain unregistered
and uncompiled. Repeated-render lifetime acceptance still blocks Folia
integration. See `evidence/shared-gate126.md`. Mermaid remains paused.

Earlier shared checkpoint (2026-10-15 local, Gate123): all 51 selected targets
compile with all 1,944 captured inputs unchanged. Core, Host, Parser,
Foundation, Regexp and LinkedList pass; public SVG remains 9/9. Complete
rendering matches 63/75, with 31/38 registered packages and seven warnings.
Units and AMSCD now use canonical per-runtime method exports and participate
in the actual SVG session. All prior passing package comparisons remain
passing. Next-six and fonts still expose incomplete MML node field projections
(18/20 and 13/16). Texttip's collision is precisely the factory's `jax` field:
repeated canonical factory projection attempts to bind it twice. Its narrow
owner-reuse fix and regression await the next gate. See
`evidence/shared-gate123.md`. Folia and Mermaid remain unchanged.

Earlier shared checkpoint (2026-10-15 local, Gate121): all 51 selected targets
compile with all 1,941 captured inputs unchanged. Core, Host, Parser,
Foundation, Regexp and LinkedList pass; public SVG remains 9/9. Complete
rendering matches 60/75, with 29/38 registered packages and nine warnings.
Mhchem again passes all 220 cases and every alias/replacement/map/mutation
record; its native observation is byte-identical to Source. Existing passing
package comparisons remain passing. The handler-owner probe now passes its
prototype metadata assertions. Next-six remains 18/20 and fonts 13/16,
exposing MathItem fields and observer alias traversal respectively. Texttip
still fails with a native-field binding collision; the exact owner/key needs
diagnosis. Gate120 ended with a UTF-16 diagnostic streaming compile error;
Gate121 fixes it without changing assertions. No application or Mermaid code
changed; complete renderer and lifecycle acceptance are still required.

Earlier shared checkpoint (2026-10-15 local, Gate119): all 51 selected targets
compile with all 1,937 captured inputs unchanged. Core, Host, Parser,
Foundation, Regexp and LinkedList pass; public SVG remains 9/9. The Maction
base-initialization fix removes the native process exit. Complete rendering
now matches 59/75, with 29/38 registered packages and nine warnings.
Mathtools, Physics, Colortbl, original/expanded Bussproofs, Tag/SetOptions and
its defaults retain parity. The new exact Bussproofs module-scope comparison
passes, including its cleared-document error. Mhchem matches all 220 cases
but its BaseMethods alias check regresses; font aliases also regress after
Mathtools loading reveals BaseMethods table reinitialization. Canonical table
guards and regressions are saved for Gate120. Next-six remains 18/20, now
exposing missing StackItemFactory fields. Texttip and action records expose
missing canonical wrapper/MML fields and methods; their owners are correcting
those shared boundaries. A complete Source comparison and the original scope
remain required before application integration. The independent Stack binder
and lifetime/integration reviews are unregistered; Folia and Mermaid are unchanged.

Earlier shared checkpoint (2026-10-15 local, Gate118): all 51 selected targets
compile with all 1,936 captured inputs unchanged. Complete Core passes
345,722 assertions / 5,337 tests / 173 suites; Host, Parser, Foundation,
Regexp and LinkedList all pass. The public SVG/error API matches 9/9.
Mathtools, Physics, Colortbl and original Bussproofs retain full package/map
parity; expanded Bussproofs now matches 21/21, Tag/SetOptions 23/23 plus its
default-method witness, and Mhchem 220/220 with every replacement/map/mutation
record. TexHtml/DOMStrings/XML, Attributes, parser projections and host/layout
witnesses pass. Next-six remains 18/20 and fonts 13/16, now exposing a repeated
handler identity difference after their missing callable identities were fixed.
The full renderer and texttip terminate during maction execution; there is no
complete Gate118 renderer record or accepted count. Gate117 below remains the
last complete 75-record comparison. A new handler-prototype probe also fails
on emitted method metadata; its correction awaits the next build. Physics is
registered in the renderer; Mathtools registration awaits its canonical module
export/counter owner. Folia and Mermaid remain unchanged.

Earlier shared checkpoint (2026-10-15 local, Gate117): all 51 selected targets
compile with all 1,935 captured inputs unchanged. Complete Core, Host, Regexp
and LinkedList pass. The public SVG/error API matches 9/9 complete Source
records. Full rendering matches 56/75, with 27/38 registered packages and 11
configuration warnings; it is not accepted. Mathtools matches 42/42 cases and
5/5 maps, Physics 63/63 and 11/11 maps, Colortbl 29/29 and its map, original
Bussproofs 31/31 and both maps. TexHtml/DOMStrings/XML matches all 12 records;
Attributes and both parser projection records match their Source witnesses.
Expanded Bussproofs exposes four empty-content differences (17/21 matched).
Parser exceptions, missing callable identities in two package probes, texttip's
missing shared String.trim intrinsic, module import visibility, and three
Foundation fixture corrections remain. See `evidence/shared-gate114-next.md`.
Folia still uses its existing math worker; Mermaid remains paused.

Earlier, Gate114 selected 50 targets and terminated with compilation errors; all 1,932
captured inputs remained unchanged. The new canonical binding interfaces and
Runtime wiring are now present. Remaining build repairs concern Map access and
iteration, string/Property overloads, binding temporaries, and observer/test
types. No Gate114 runtime result is claimed. See
`evidence/shared-gate114-next.md`.

Earlier shared checkpoint (2026-10-15 local, Gate113): all 46 selected targets
compile with 1,904 captured inputs unchanged. Core passes 345,659 assertions /
5,325 tests; Host, Regexp and LinkedList pass. Parser and the new TexHtml tests
still fail. Former Scene layering failures pass. Full renderer is 53/75,
public SVG 8/9, actual registration 23/38. Colortbl is 29/29, Physics 61/63,
next-six 16/20, fonts 13/16. Mhchem matches all 220 package cases and 7,960
dependency records; its intermittent Unicode difference is proven to come
from chunked stdout decoding, now corrected with a byte-boundary regression.
Root enrolls Colortbl/Mhchem for the next build; Folia is unchanged. Complete
handler graph bindings are assigned as explicit Luna packets. See
`evidence/shared-gate111-next.md` and `canonical-handler-graph-plan.md`.

Earlier shared checkpoint (Gate110): all 38 selected targets
compile/link with 1,836 captured inputs unchanged. Complete Core passes
345,659 assertions / 5,325 tests / 171 suites. Host, Regexp and LinkedList pass;
Parser and three typed Scene tests still fail. The full renderer matches
47/75 records (64 layouts, 11 errors); public SVG/error records match 8/9.
Registration remains 23/38. Mhchem dependency comparison passes 7,960/7,960;
its recorded package capture is 219/220, with one Unicode difference absent
in diagnostic reruns. Physics is 49/63, next-six 15/20, fonts 13/16. No new
package is accepted. See `evidence/shared-gate107-110.md`.

Gates111–112 enroll 46 targets, including the complete new package packets,
but have not passed compilation. Inputs remained unchanged in both builds;
remaining observer/test API corrections are assigned to their Luna owners.
The shared TextParser list-identity cause is identified and corrected in a
candidate awaiting the next build. See `evidence/shared-gate111-next.md`.

Current implementation work is primarily Luna. Root owns architecture and
acceptance. Per-session MapHandler and Bussproofs state, arbitrary Context
callable identity, HTMLDomStrings/document wiring, and the remaining package
and Scene differences are in progress. One focused Sol review covers only
the callable boundary. Folia integration and Mermaid remain unchanged.

Earlier shared checkpoint (2026-10-15 local, Gates102–106): Gate106 builds all
31 targets with 1,804 captured inputs unchanged. Runtime acceptance fails:
the live parser Array needs the missing generic unshift operation, all75
renderer records fail, and Core reports two SourceStringMap errors. Host,
Regexp and LinkedList pass. The seven Scene Context tests pass 56 assertions;
four typed layout-data tests remain blocked during parsing. Registration stays
23/38. Gate107 failed one ParseOptions compilation unit with all 1,804 captured
inputs unchanged; that owner correction and the runtime fixes await Gate108.
Neither earlier isolated passes nor the successful compile certify these edits. See
`evidence/shared-gate102-106.md`.

Current source candidates (2026-10-15 local):

- The public `folia.math` facade compiles against the completed Gate101
  library. Adding the current table separator correction gives 9/9 exact
  public SVG/error records against the pinned Source. All nine returned
  Scenes remain traversable after Renderer destruction (213 nodes).
- The generic Lite node/create/setAttributes dispatch correction passes an
  isolated Foundation run: 390 assertions in 59 tests, including the new
  live-method replacement and exception propagation regression.
- These are isolated diagnostics over the Gate101 dependency library; they
  do not certify current shared-module edits or complete package support.
  The latest fully passing Core checkpoint remains Gate101. Physics's initial isolated compile
  identifies import and type errors; its owner is correcting the complete
  package. Canonical parser projections and the shared SVG Scene Context
  projection remain in progress. Folia has not switched its math path.
  See `evidence/public-api-isolated-g101.md`.

Active complete shared-build checkpoint (2026-10-15 local, Gates99–101):

- Gate99 fails because an automatically collected draft test imports an
  unregistered module. After moving the draft, Gates100 and 101 build all
  28 targets with 1,761 captured inputs unchanged. Input capture now includes
  automatically discovered math tests and local quoted includes.
- Gate101 complete Core passes 345,659 assertions in 5,325 tests across 171
  suites. Host, Parser, Regexp, LinkedList and Foundation also pass. Gate100
  Font tests and the complete raw DynamicFile Source comparison pass.
- Complete75 improves to 53/75 exact records; registration remains 23/38
  with 15 warnings. The cancel-lines record now matches completely. Actual
  `x+y` SVG remains byte-exact and detached Scene checks pass.
- Table comparison remains 12/14. Its remaining aligned-label SVG separator
  correction awaits a new compile. Next-six remains 0/20 with all records
  emitted; Font3 remains 10/12. No partial package is newly registered.
- Luna owners continue package semantics, Physics, and Maction/TimerHost.
  Folia integration remains pending; Mermaid remains paused.
  See `evidence/shared-gate99-101.md`.

Earlier native execution checkpoint (2026-10-14 local, Gate98):

- All 28 targets build with 1,631 captured input hashes unchanged. Complete75
  remains 52/75 exact, with 23/38 packages registered and 15 warnings.
- Complete Core passes 345,656 assertions in 5,324 tests across 171 suites.
  Foundation, Host, Parser, Regexp, LinkedList and Font tests also pass.
  Three instance-extension records match exactly; actual `x+y` SVG and
  detached Scene checks remain green.
- Table comparison remains 12/14; label paint now agrees and the remaining
  aligned-label difference is full-width SVG behavior. Menclose Common boxes
  and placement calls for cancel-lines now agree, but paint/SVG and its
  separate 30-case method traces still require work.
- Next-six emits all 20 records but matches 0/20; Font3 matches 10/12. Raw
  DynamicFile observation stops at the unimplemented data-to-accessor
  descriptor change. These are explicit remaining acceptance failures.
- Follow-up fixes remain uncompiled. Folia integration remains pending;
  Mermaid remains paused. See `evidence/shared-gate98.md`.

Earlier native execution checkpoint (2026-10-14 local, Gates95–97):

- Gate95 is cancelled before its build command starts; Gate96 fails compilation.
  Gate97 builds all 27 targets with 1,628 captured input hashes unchanged.
  Complete75 improves to 52/75 exact records; registration remains 23/38 with
  15 warnings. The renderer has not passed acceptance.
- Table/spacing improves to 12/14 exact records. Label-node creation order and
  colortbl parsing remain. Actual `x+y` SVG stays byte-exact and its detached
  Scene checks pass. Foundation passes 383 assertions in 58 tests.
- The remaining Core assertion still fails in Gate97; a corrected expected
  file key awaits a fresh build. Gate82 remains the last fully passing full
  Core run. Next-six's canonical token-map callbacks still need completion
  before its full 20-record comparison and production registration.
- Folia integration remains pending; Mermaid remains paused.
  See `evidence/shared-gate95-97.md`.

Earlier native execution checkpoint (2026-10-14 local, Gates93–94):

- Gate94 builds and links all 27 targets with all 1,628 captured hashes
  unchanged. Complete75 finishes with 50/75 records exactly equal to Source.
  Actual registration remains 23/38 packages with 15 warnings; full renderer
  acceptance still fails.
- The table/spacing corpus passes 11/14 complete exact records. Remaining
  differences are column-rule arithmetic order, label placement and colortbl
  parsing. Actual `x+y` SVG remains byte-exact; detached Scene checks pass.
- Foundation passes 383 assertions/58 tests; Host passes 465 assertions/197
  tests. Parser, Regexp and LinkedList pass. Full Core retains one assertion
  failure in a dynamic-provider retry test; the earlier six provider failures
  have reduced to one. Gate82 remains the latest fully passing full Core run.
- Old-six and next-seven parser corpora pass 24/24 and 23/23. Font3 observation
  metadata and next-six's first-input exit still need fixes and fresh execution;
  their packages are not newly registered in production.
- Folia integration remains pending; Mermaid remains paused.
  See `evidence/shared-gate93-94.md`.

Earlier native execution checkpoint (2026-10-14 local, Gates89–92):

- Gate92 builds and links all 27 targets with all 1,628 captured hashes
  unchanged. Actual `x+y` SVG remains byte-exact and its detached Scene
  lifetime checks pass.
- The 14-case table/spacing corpus completes: four mpadded records match
  exactly. Table construction order, labels, the box model and colortbl
  parsing still have differences under review.
- Old-six and next-seven parser corpora remain exact at 24/24 and 23/23.
  Font3 map callables and next-six initialization still require a fresh run
  after their current fixes; neither corpus is accepted.
- Foundation and Host each fail a new font/Context test. Full Core also has
  six dynamic-font-provider failures. Parser, Regexp and LinkedList pass.
  The iterator test remains unresolved after using the intrinsic Symbol.
- Complete75 exits during the 59th input, `verb-literal`; its partial output
  is retained only for diagnosis. Gate88 remains the latest completed75
  result (16/75), with 23/38 packages registered. Full acceptance and Folia
  integration remain open. See `evidence/shared-gate89-92.md`.

Earlier native execution checkpoint (2026-10-14 local, Gates86–88):

- Gate88 builds all 26 targets with all 1,581 captured hashes unchanged. The
  whole driver emits 75 complete actual records: 16/75 match every Source field.
  It registers 23/38 packages and reports 15 missing-package warnings; complete
  renderer acceptance still fails.
- Actual `x+y` SVG is byte-exact (2,502 characters). Its shared Scene survives
  Context/graph/host release. Foundation passes 383 assertions/58 tests.
- The six-package and next-seven parser corpora pass 24/24 and 23/23. Font3
  remains 0/12 due to a canonical function length mismatch, corrected but not
  rerun. Instance extensions complete three actual rows but their comparer
  still rejects an empty-record schema difference.
- Gate88 Core/Host fail one newly added Symbol iterator test because its test
  input used a fresh Symbol instead of the intrinsic Symbol.iterator. The test
  source is corrected and awaits execution; Parser/Regexp/LinkedList pass.
  Gate82 remains the latest fully passing complete Core run.
- Remaining package and Scene placements, complete method/field fidelity,
  all75 parity and Folia integration remain open. Mermaid remains paused.
  See `evidence/shared-gate86-88.md`.

Earlier native execution checkpoint (2026-10-14 local, Gates76–85):

- Gate85 builds all 26 targets with all 1,581 captured hashes unchanged.
  Actual `x+y` conversion now retains two SVG lines and their flow break in the
  completed Scene; Context/graph/host release and detached Scene reads pass.
  Its SVG still differs from Source, so output parity is not accepted.
- The six-package observer passes 24/24 complete Source records. The next-seven
  corpus remains 12/23; the font-package corpus remains 0/12 due to an observer
  reflection error. Other enabled packages still require implementation.
- Whole75 currently stops during actual instance font-extension initialization:
  the Jax font Context field is absent. No complete75 result is emitted.
  Gate70 remains the latest complete whole-profile run, with 0/75 full matches.
- Gate82 fresh full core passes 345,646 assertions/5,323 tests/171 suites; Host,
  Parser, Regexp and LinkedList also pass. Gate85 Foundation has one raw font
  array-field test failure. Common extension delegation and wrapper length
  conversion tests pass.
- Gate81 complete FontData tables and synchronous dynamic glyph/option/delimiter
  data pass strict comparison; arbitrary raw fields and actual Jax extension
  execution remain open. Menclose's actual method trace still differs.
  Folia integration remains pending and Mermaid remains paused.
  See `evidence/shared-gate76-85.md`.

Earlier native execution checkpoint (2026-10-14 local, Gates73–75):

- Gate75 builds all 22 targets with all 1,531 captured hashes unchanged.
  Foundation passes 303 assertions/50 tests. The actual document probe completes
  an immutable Scene and SVG and verifies release of Context, graph and host;
  its `x+y` SVG remains unequal to Source.
- The full38 package request now reaches actual case conversion. Thirteen
  package handlers are implemented. The whole driver completes cases 1–27, then
  exits with a native access violation in case28, `braket-delimiters`. It emits
  no complete75 JSON result, so Gate70 remains the last complete whole-profile
  run (0/75 exact matches).
- Full FontData tables, extension merges and every synchronous dynamic glyph,
  option and delimiter now pass the strict Source comparer. The apparent empty
  merged arrays were an observer bug: it bypassed the live bound array accessors.
  Font tests pass 176 assertions/21 tests; lifecycle, methods, tuple and bundled
  provider observations also pass their exact comparers.
- The six-package parser observer aborts during token-map reflection with an
  unbound FunctionRef. Canonical function bindings and an isolated Braket parse
  mode are being prepared for the next build; they are not yet accepted.
- Live font Map replacement, SVG menclose instance dispatch, the remaining
  package handlers, complete75 output parity and Folia integration remain open.
  Mermaid remains paused. See `evidence/shared-gate73-75.md`.

Earlier native execution checkpoint (2026-10-14 local, Gates70–72):

- Gate71 builds all 21 targets with all 1,147 captured hashes unchanged.
  Actual `x+y` conversion now completes a native SVG and immutable Scene.
  The completed Scene survives release of the document, Context, graph and
  host. Its SVG still differs from Source and is not renderer acceptance.
- Gate70's whole run emits 75 actual records: 58 SVGs and 17 errors, with
  0/75 complete Source matches. Its base-only TeX selection means many SVGs
  are parser error output. Gate71 requests all 38 packages and records actual
  registrations/warnings separately; a missing live TeX.options binding
  prevents that run from reaching the first case.
- Fresh Gate71 core passes 345,635 assertions/5,322 tests/171 suites; Host,
  Parser, Regexp and LinkedList also pass. Foundation passes 295 assertions/
  49 tests. The rejected-provider fixture now distinguishes the actual NewCM
  placeholder's first fulfilled load from the subsequent failed-file rejection.
- Gate72 enrolls six Source-split TeX packages and five script placement pairs.
  Its 22-target build exits 1 with all 1,525 captured hashes unchanged. Script
  placements and the shared typed HTML/transform Scene interface compile;
  package module/import and syntax errors prevent linking and fresh execution.
- Complete FontData merges still fail the exact comparer. All 38 enabled
  packages, all 75 complete observations and Folia integration remain required.
  Mermaid remains paused. See `evidence/shared-gate70-72.md`.

Earlier native execution checkpoint (2026-10-13, Gate69):

- Gates68/69 build all 21 targets with all 1,147 captured hashes unchanged.
  Real document conversion now compiles actual TeX into MML and reaches Scene
  dispatch, where inferredMrow registration is missing. No SVG result is yet
  accepted. Both whole-profile runs emit 75 actual errors and match 0/75 full
  Source records. See `evidence/shared-gate67-69.md`.
- The actual NewCM module-provider trace matches 40 modules, 80 ordered events
  and 24,646 glyphs exactly. Full FontData option/extension behavior remains
  open. The Gate69 document probe verifies Context/graph/host release after an
  actual failed conversion; completed Scene lifetime remains unverified.
- Gate68's fresh full core exits 1 with one failed dynamic-font placeholder
  assertion/test; its other 170 suites pass. Host, Parser, Regexp and LinkedList
  pass. Foundation has two FontData option failures; its eight other packets
  pass, including the new shared Context numeric conversion witnesses.
- All 38 enabled profile packages, all 75 whole observations and Folia
  integration remain required. Mermaid remains paused.

Earlier document/font enrollment checkpoint (Gate66):

- Gates65/66 exit 1 with all 1,127/1,147 captured input hashes unchanged.
  Gate66 enrolls the real Core/HTML document and item owners, SVG assembly,
  dynamic NewCM module provider, actual conversion probe and whole-layout
  driver. Common/Scene observation compiles; document/tool boundary errors
  still prevent a fresh conversion run. See `evidence/shared-gate65-66.md`.
- The native session still configures only base. All 38 profile packages and
  all 75 complete Source output records remain required. No native TeX-to-SVG
  conversion or full layout parity has been accepted. Mermaid remains paused.

Latest complete passing output build and substrate checkpoint (Gate64):

- Gate64 builds all 13 targets with all 1,120 captured input hashes unchanged.
  Fresh Foundation passes 215 assertions/36 tests. The real SVG Jax now
  constructs through NativeRenderSession's actual Context-owned providers;
  lifecycle tests verify retained public handles and eventual service-graph
  destruction. Shared primitive/binary-plus coercion, RenderActions and Common
  menclose enter the compiled closure. Fresh parser corpora match 150/150 whole
  records and Unicode matches 37/37. See `evidence/shared-gate59-64.md`.
- This establishes actual construction and substrate behavior. The real
  MathDocument/HTMLMathItem conversion entry and whole-layout observer are
  still draft work; no complete native TeX-to-SVG conversion has been accepted.
  Complete Source own-fields/prototypes/methods, font extension/dynamic loading,
  XML/HTML and remaining SVG painting, the full 75-case comparison and Folia
  integration remain required. The enabled Folia profile remains unchanged.

Latest complete passing core/diagnostic execution (Gate63):

- The freshly linked Gate63 full core exits 0 with 345,620 assertions/5,320
  tests/171 suites. Host, Parser, Regexp and LinkedList also exit 0. Root runs
  byte-identical copied executables to avoid Windows linker locks during the
  next build. Gate63's overall build exits 1 because Foundation cannot yet link;
  Gate64 repairs that closure and executes the real Jax lifecycle tests. The
  full core has not been re-executed from Gate64, so its latest execution is
  explicitly Gate63. See `evidence/shared-gate59-64.md`.

Earlier complete passing build and core/diagnostic checkpoint (Gate49):

- Gate49 builds all 13 targets and keeps all 1,029 captured input hashes
  unchanged. Fresh full core passes 345,607 assertions/5,314 tests/170 suites;
  Host, Parser, Regexp and LinkedList tests also pass. All five parser corpora
  match 150/150 whole records, Unicode matches 37/37, and LinkedList matches
  13/13. Gate63 subsequently supplies the latest full core execution.
- Gate49's then-new Foundation runner exits 1: three Lite projection tests pass, but
  both Scene ownership tests throw during SVG serialization through an unbound
  LiteAdaptor. Subsequent fixes pass the focused Scene tests in Gates53 and 55.
  See `evidence/shared-gate48-49.md`. Complete native
  TeX-to-SVG and Folia integration are still unaccepted.

- Gates45 through 47 progress into actual Common per-kind bodies, output
  bindings and Scene/SVG painting. Gate47 exits 1 with all 1,029 input hashes
  unchanged. Its new actual Scene ownership tests compile but have not linked
  or executed; remaining errors are in six production files and the Lite
  projection test. Gate46 has one changed input during compilation and is not
  a clean frozen snapshot. See `evidence/shared-gate45-47.md`.
- Source constructor initialization required a separate semantic repair:
  C++ base construction prevented MathJax's derived virtual dispatch.
  Actual derived allocation, canonical identity and ordered initialization
  phases now compile and still need real layout verification. Full
  constructor/prototype/method coverage, FontData, remaining wrapper kinds and
  the production session lifetime remain open.

- The first current Common/Lite/Scene/SVG closure is explicitly enrolled in
  CMake. Gate42 stops on a missing Entities generation rule and has two changed
  mtr input hashes from Root's enrollment/snapshot communication race; it is not
  a valid frozen build. Gate43 fixes generation but stops on duplicate C++
  partition providers. Gate44 fixes those providers and reaches C++ compilation;
  it exits 1 on actual Lite declarations and CommonWrapper inheritance/imports.
  Gates43/44 each retain all 1,010 frozen hashes unchanged. Owners have repairs
  and are continuing the complete module packets. See
  `evidence/shared-gate42-44.md`; no native SVG path is accepted.

- Gate41 builds all 12 selected targets successfully with all 826 frozen
  input hashes unchanged. It fixes the Source-order BaseItems static error-table
  initialization that made Gate40's Core/Host/Parser abort before main. Fresh
  Host, Parser, Regexp and LinkedList runners all exit 0. All five parser corpora
  match 150/150 complete original records; LinkedList matches 13/13.
- Gate41's full core exits 1 with one failed Unicode assertion in
  `unicode_regexp_u_scans_code_points_and_keeps_utf16_indices`. The other 169
  suites pass 345,572 assertions/5,301 tests. The actual Unicode observer matches
  36/37 strict original records; the remaining tag-split record has identical
  parts and different RegExp `.source` slash spelling. The latest fully passing
  core remains Gate36. See `evidence/shared-gate39-41.md`.
- Root has enrolled the current Common/Lite/Scene/SVG module closure for its
  first full compile. Source-aligned draft bodies and module-closure checks
  establish neither native rendering parity nor Folia integration.

Previous checkpoints:

- Gate38 is the newest executed combined build and exits 1 with all 826
  input hashes unchanged. The fresh library and diagnostic executables link,
  but the core runner's Unicode test printer and LinkedList dump's cross-module
  MathLocation type ownership fail. The latest passing full core remains Gate36;
  repair edits require another build. See `evidence/shared-gate37-38.md`.
- Gate38's five fresh parser corpora match all 150 original records. HostTests
  passes 430 assertions/192 tests/10 suites. ParserTests now passes the new
  readonly/setter constructor-order cases, but fails four assertions in two
  tests preserving supplied object identities. RegexpTests fails two obsolete
  expectations that reject the newly implemented Unicode flag; they do not
  establish complete Unicode behavior. The actual production observer matches
  27/37 whole original Unicode records, including exact error text and patterns.
  An actual parser tag pattern fails to compile and remains a production gap.

- OutputJax and its FunctionList Context binding are registered and compiled.
  Gate27's original lifecycle comparison passes all 14 records and its tests
  pass 98 assertions in 21 tests. Complete common layout and SVG remain open.
- Gate34's combined build and ordinary core runner both exit 0. Core passes
  345,517 assertions/5,291 tests in 168 suites. Fresh Base, Frontend,
  Bold/ConfigMacros, AMS and AMS Boundary whole-record comparisons pass all
  150 observations. Equation's actual Source default display=true is restored.
  BoldTests passes 69 assertions/seven tests. ParserTests still fails two
  constructor/reflection tests; complete class acceptance remains open.
- Gate36 fixes Gate35's two compile errors; the complete target build exits 0.
  Registered RuntimeModules/LinkedList/MathList/MathLocation compile, the 13
  LinkedList original records match exactly, and lifetime tests pass 27/7.
  Native indexed-array provider/descriptor tests pass 17 assertions/four tests.
  All five parser corpora again match all 150 records. The full Gate36 core
  run passes 345,534 assertions/5,295 tests/169 suites, exit 0. Broader MathList
  coercion/getter acceptance is still open. See
  `evidence/shared-gate34-35.md` for exact scope and executable provenance.
- Three persistent Luna owners now implement Core DOM/Lite/Entities/font
  identities; CommonWrapper/factory/per-kind/linebreak algorithms; and
  CommonOutputJax and the completed scene/Source SVG placement/serialization.
  The two parser owners also resolve their
  remaining diagnostic regressions. New output packets are unregistered and
  unaccepted until their actual production graph compiles and compares.
- Root froze `common-layout-work-plan.md`: common measurement feeds one completed
  backend-independent scene; SVG serialization and future Direct2D consume that
  scene. The expanded original corpus has 75 cases, with 299 executable hashes
  locked, 31 observed wrapper kinds and four actual dynamic font requests.
  Whole results repeat exactly; tracing does not change the SVG/MML projection.
  Two actual Lite-host action exceptions remain in the Source records.
  A 37-row actual Unicode host corpus and corrected
  Entities source oracle also repeat exactly. See
  `evidence/common-layout-root-observations.md`; these are Source baselines,
  not native renderer acceptance.

Completed current-state evidence:

- MathJax-src 4.1.3 is cloned in .reference/MathJax-src, pinned to
  fb987178c2d279a99b0db5ea02e751691703955e. scope.json inventories 246 reachable
  upstream files for the enabled Folia profile.
- The C++23 folia_math target uses explicit module/source registration.
  Individual accepted packets have actual Source observations, Native checks
  and independent acceptance evidence. The last completed full core baseline
  was commit 2e58d8a. Gate23 worktree full core passes 345,517 assertions/
  5,291 tests; later uncompiled edits and failed parser diagnostics do not
  inherit that snapshot's acceptance.
- Utility, attribute, factory, tagged child-array/tree and parts of MML/TeX
  token/handler-map foundations exist. The architecture keeps semantic nodes,
  parsing, common layout and output responsibilities separate.
- P33/P34/P35 restores thirteen bounded MML consumer methods: error construction,
  spacing, row/identifier classes, style inheritance and subclass verification.
  The joint baseline passes 5,815 assertions in 238 executed tests and detects
  all 38 compiling semantic faults. The normal FoliaTests target builds and
  passes its new error filter plus constructor/base-verification regressions.
  See evidence/p33-p35-consumer-acceptance.md for precise scope and limitations.
- P36 restores fifteen table/multiscript/fraction/radical methods. Its independent
  baseline passes 19,609 assertions in 373 executed tests and detects all 39
  compiling semantic faults; the normal target's affected tests also pass.
  See evidence/p36-table-multiscript-acceptance.md.
- P37 restores thirteen operator/TeXAtom/under-over semantic methods, sharing the
  effective operator adjustment body. The independent baseline passes 32,828
  assertions in 162 executed tests and detects all 33 compiling semantic faults.
  The normal build and affected constructor/MML/script/operator filters pass.
  See evidence/p37-operator-script-acceptance.md for bounded scope and limitations.
- P38 restores nine action/MathChoice/menclose/mpadded methods. Its independent
  baseline passes 9,031 assertions in 1,452 executed tests and detects all 25
  compiling semantic faults. The normal build and affected filters also pass.
  See evidence/p38-action-choice-acceptance.md.
- P39 restores four mfenced methods. Its baseline passes 1,012 assertions in
  229 executed tests and detects all 21 compiling semantic faults. The complete
  normal core build/test now passes (build_test.bat, exit 0). No explicit MML
  Unimplemented stubs remain. See evidence/p39-mfenced-acceptance.md.
- P40 completes the 21 remaining NodeUtil methods and TexError's two methods.
  171 new utility observations and 1,981 TexError observations match the pinned
  original; all eight compiling semantic faults are detected. The complete core
  build/test passes, exit 0. See evidence/p40-parser-utilities-acceptance.md for
  the typed host boundaries and exact scope.
- P41 supplies all nine TeX NodeFactory methods, using the existing callable
  identity and MML graph with a narrow borrowed ParseOptions registration edge.
  All 78 original observations pass and all nine compiling semantic faults are
  detected. The full core suite passes, exit 0. See
  evidence/p41-node-factory-acceptance.md.
- P42A supplies MmlStack's eleven semantic methods over the same existing MML
  graph and node-array identity. All 76 original records pass; all twelve
  compiling semantic faults are detected. The full core suite passes, exit 0.
  This is one responsibility of StackItem.ts, not full stack/parser acceptance.
  See evidence/p42-mml-stack-acceptance.md.
- P42B completes BaseItem's seventeen methods and the StackItemFactory
  specialization over one actual shared StackItem identity, captured class
  metadata and Source constructor phases. All 85 original records pass; all
  seventeen compiling semantic faults are detected. Full core tests pass,
  exit 0. See evidence/p42b-base-item-acceptance.md. Stack.ts and concrete reducers
  are still required before actual TeX parsing.
- P43 supplies the actual parser Stack's nine responsibilities over existing
  BaseItem owners. All 72 original records pass; all sixteen compiling semantic
  faults are detected. Full core tests pass, exit 0. See
  evidence/p43-parser-stack-acceptance.md. Complete configured reducers,
  ParseOptions and TeXParser remain; the active next ownership boundary is P45.
- P44 moves the invocation context to its sole module above Stack and completes
  UnitMap/UnitUtil's nine responsibilities plus muReplace. All 245 original
  records pass; all sixteen compiling semantic faults are detected; full core
  tests pass, exit 0. See evidence/p44-unit-util-acceptance.md. P45 selects one
  parser-family module with Source-aligned partitions after a passing MSVC/CMake
  toolchain pilot. The actual parser-family classes are not implemented yet.
- P45 supplies core MathItem's actual fields, constructor, state table/registration
  and complete AbstractMathItem lifecycle through typed borrowed engine edges.
  All 92 original records pass; all twenty compiling semantic faults are detected;
  full core tests pass, exit 0. See evidence/p45-math-item-acceptance.md. Parser
  declarations use P46's single-runtime Context/Value option-graph decision in
  evidence/p46-parser-options-ownership.md. The actual declaration graph and
  native option callback/regexp bindings are the next implementation boundary.
  The real core InputJax base must also precede the actual TeX declaration.
- P47 supplies live Context option-array input to the same FunctionList algorithm
  and a converter-only binding to the canonical native function body. All 19
  original records pass, all seven compiling faults are detected, and full core
  tests pass. A Source subclass-default witness first failed and now confirms
  omitted priorities reach the actual virtual callee. See
  evidence/p47-option-filter-acceptance.md. Actual core InputJax is next.
- P48 supplies the real generic core InputJax base above the same MathItem
  compile service, including actual class/options/filters and the shared mutable
  filter record. All 30 original records pass, all fifteen compiling faults are
  detected, and full core tests pass. See evidence/p48-input-jax-acceptance.md.
  The next gate is the real complete parser-family declaration graph and its
  native class/array/regexp/Unicode bindings; no TeX parser is claimed yet.

- P49 supplies canonical typed native identities in the same Context option
  graph, with retained actual MML factory lifetimes, and the actual core
  AbstractFindMath superclass. Nine original identity/Options/MML observations,
  26 native ownership assertions, and eleven original FindMath observations
  pass. All eight host faults and five FindMath faults are detected; the full
  core suite passes. See evidence/p49-prerequisites-acceptance.md. Actual FindTeX
  and its UTF-16 non-Unicode regexp binding are the next implementation boundary.

- P50 supplies the native UTF-16 non-Unicode RegExp host needed by actual
  FindTeX. It uses SRELL code-unit traits and preserves configured punctuation
  delimiters. All 34 original observations, four explicit host-scope assertions
  and seven compiling faults pass their gates; full core tests pass. See
  evidence/p50-regexp-host-acceptance.md. The complete actual FindTeX boundary
  and remaining array/regexp bindings are recorded in
  evidence/p51-find-tex-declaration-frontier.md; its class is not implemented yet.

- P51 supplies actual option-array forEach and canonical RegExp bindings in the
  same Context graph. All 22 array and eight identity/cursor original observations,
  ten lifetime assertions and thirteen compiling faults pass their gates. Full
  core tests pass. See evidence/p51-bindings-acceptance.md. The actual complete
  FindTeX class is next; its prerequisite host bindings are now available.

- P52 completes the actual FindTeX scanner class and all seven method/constructor
  responsibilities over its existing superclass and runtime bindings. All 77
  original observations and ten compiling semantic faults pass their gates;
  the complete core build/test passes, exit 0. Root review corrected constructor
  arity metadata and live tuple iterator reads before acceptance. See
  evidence/p52-find-tex-acceptance.md. This is formula discovery; actual
  parser/configuration/package declarations and bodies are the next main route.

- P53 establishes the actual parser-family declaration graph, aligned with the
  Source class/file responsibilities and one named-module owner for genuine
  cycles. Compile-time contracts check canonical identities, all 26 BaseItems
  and selected field/signature types. Independent Source audits corrected tuple,
  nullable-return and initialization details before acceptance. These are
  declarations; parser-family algorithms remain unimplemented. See
  evidence/p53-parser-declarations.md.
- P54 supplies the portable Unicode 16 NFD host needed by ParseOptions and
  ParseUtil. All 99825 official corpus outputs, 85 exact UTF-16 sequences and
  17 complete-plane output fingerprints pass. All nine compiling NFD faults
  and four string-key Map faults are detected; correct baselines pass again.
  Independent Luna review and complete core build/tests pass, exit 0. See
  evidence/p54-nfd-acceptance.md. The next main route is actual parser-family
  method bodies and their module-lifetime registrations.

- P55 supplies canonical Context configuration-record inputs to the actual
  SubHandlers and NodeFactory registries, sharing their existing reducers, plus
  complete Label/TagInfo constructors. All 27 pinned original observations and
  eleven compiling faults pass their gates. Independent Luna reviews and the
  full core build/tests pass, exit 0. Root repaired three Source-semantic errors
  before acceptance and removed a test's shared-registry pre-state dependency.
  See evidence/p55-parser-record-acceptance.md. P56 freezes Array.map and runtime
  initialization requirements for the next actual Configuration methods.

- P56 supplies the actual ordinary Context Array.map host needed by
  Configuration, with retained prototype callable, holes, live getters and
  callback/return identities. All 41 original observations and two native-scope
  assertions pass; all eighteen compiling faults are detected; independent
  Luna review and full core tests pass, exit 0. Custom species remain explicitly
  unsupported. See evidence/p56-array-map-acceptance.md. The sustained complete
  Configuration/runtime, Tags/ColumnParser and independent original verification
  assignments are recorded in parser-family-work-plan.md.

Incomplete requirements and current executed checkpoints:

- Current verified worktree checkpoint is Gate27: all targets build with 814
  unchanged input hashes, and ordinary core passes 345,517 assertions in 5,291
  tests/168 suites. OutputJax is registered and freshly linked: strict 14/14
  original observations and 98 assertions/21 focused tests pass. BaseMethods
  now matches all 37 tree/parser observations. This supersedes older
  unregistered/pending-build descriptions below; it is bounded parity, not
  complete host or renderer acceptance. See evidence/shared-gate27.md.
- TextMacros' actual configuration count is zero. The parser owner found and
  is correcting package registration into the canonical configuration list;
  the correction is after Gate27 and awaits fresh compilation. Begingroup's
  failing wrapper-text UT expectation is invalid in pinned original CJS and
  is being corrected without changing production NodeUtil semantics. Empty
  arrow node-list/retained-tree filtering and tag display differences remain.
- The OutputJax owner is proceeding with the separate complete LiteAdaptor
  packet, including Source Entities utility and canonical Unicode-RegExp
  dependencies. One module/runtime graph store supports shared node identity
  across adaptors. No native LiteAdaptor/scene/SVG result is accepted yet.

- Gate 21 completes all eighteen focused build targets, exit 0, with unchanged
  registered/generated input hashes. Gates 19 and 20 did not link FontTests;
  their other freshly linked observer programs remain valid evidence for their
  actual source snapshots. Gate 22 builds the official core runner but its execution fails 17 assertions
  in two regex policy tests. Root isolated the SRELL namespaces in 937f91b;
  the freshly linked gate23 ordinary runner passes all 345,517 assertions/
  5,291 tests. Its ParserTests target alone fails on one overload ambiguity;
  that diagnostic correction and later production edits remain uncompiled.
  Two diagnostic candidate files were edited during its combined snapshot and
  their new assertions require a later build. They are not FoliaTests inputs.
- Fresh gate-19 HostTests passes 420 assertions in 190 tests; RegexpTests passes
  18,518 assertions in 41 tests. Structured Array.prototype.concat comparisons
  match 19 of 25 original rows, with zero supported mismatches and six explicit
  boxed-primitive/species/conversion boundaries. The comparator returns partial
  status, not complete host acceptance.
- NewCM/common/SVG FontData and three extensions now use one canonical sparse
  PropertyArray tuple model. Root's fresh gate-20 full-table comparison passes
  all four static sources and every synchronous dynamic glyph/options record
  and delimiter, resolver samples and class merge state. Tuple presence and
  identity comparisons pass; nine lifecycle traces and supplemental FontMethods
  observations also pass. Fresh gate-21 FontTests passes 97 assertions/11 tests.
  Eight independent corrupted output candidates are all rejected; those are
  comparator sensitivity checks, not compiled production fault tests. Full
  FontData member/profile acceptance and wrapper consumption remain separate.
  See evidence/font-styles-gate21.md for provenance and limits.
- Styles/StyleJson native modules compile. Fresh gate-21 focused tests pass
  19 assertions in six tests and the strict whole-record comparison passes all
  37 pinned original observations. The capturing String.split caller hang is
  fixed without changing shared RegExp.exec. The modules expose the bounded
  typed interface; dynamic Context constructors and browser CSS are not accepted.
- BaseMethods' actual gate-19 comparison matches 33/37 original MML/parser cases.
  Array row controls, column types and two AMS table cases remain mismatched.
  Some separate BaseItems/ParseUtil tests had guessed flattened shapes; Root
  directly called the same-profile original and found the actual TeXAtom/mstyle/
  mrow wrappers agree with the native results. Their corrected diagnostic tests
  await fresh compilation; required Source wrappers must remain intact.
- TextMacros now uses the real configuration list, inner nodeFactory and shared
  ParseOptions filter payload; its Gate32 37-row frontend comparison passes.
  ConfigMacros and Bold rewriting also pass their full original corpus. AMS
  display/tag behavior and supplemental arrow verification remain open.
- Commit c7dc052 accepts the bounded canonical Symbol-valued Property marker
  bridge: 11 assertions in two executed tests plus a pinned original oracle.
  It does not accept Symbol keys, whole TokenMap reflection or Begingroup.
  Complete Source constructor/prototype/reflection coverage is still missing.
- Root froze the OutputJax/DOM-host responsibility and interface contract plus
  fourteen repeated original lifecycle/filter observations in commit 7f36040.
  A Luna now owns the complete OutputJax implementation, observer, tests and
  member ledger. The bounded OutputJax packet is registered/compiled and its
  original observations pass. Native LiteAdaptor, CommonOutputJax and all common
  wrappers remain required; no new glyph or math-item state is permitted.
- Three persistent Luna owners cover the DOM/common-layout responsibilities above
  while finishing their parser/font packets.
  Root owns architecture, registration, shared builds and independent acceptance.
  Configuration now allows twelve threads, but this active runtime exposes four total
  slots including Root and no tool to raise that injected session limit.
- The remaining enabled packages, common wrappers, completed shared scene,
  SVG renderer and public TeX -> SVG entry remain outstanding. Folia still links
  QuickJS and MathJax assets; native Folia integration waits for actual parity.
  Mermaid remains paused at its documented checkpoint.
The historical scope ledger marks 54/246 files implemented and 192 not started.
That is not a reliable 22% completion claim: several historical implementations
have tagged-tree consumers awaiting restoration, and source-file counts do not
measure coverage or usability. Module counts likewise measure compilation
structure, not completed upstream files. No complete-clean-port claim follows
from the existing build or local test counts.
