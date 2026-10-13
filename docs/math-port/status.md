# Current native MathJax status

The goal remains native TeX -> shared mathematical layout -> SVG, then Folia
integration replacing the math QuickJS worker. It is not currently usable as a
renderer, and Folia still uses the existing QuickJS MathJax path.

Active output checkpoint (2026-10-13, Gate64):

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
