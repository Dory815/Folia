# Current native MathJax status

The goal remains native TeX -> shared mathematical layout -> SVG, then Folia
integration replacing the math QuickJS worker. It is not currently usable as a
renderer, and Folia still uses the existing QuickJS MathJax path.

Active output/parser checkpoint (2026-10-13, Gate24):

- OutputJax and its FunctionList Context binding are being implemented; they
  are not registered or compiled yet. The native indexed-view protocol edits
  compiled in Gate24 and passed existing core/focused tests, but their new
  specific tests are unregistered. Later parser/test edits are uncompiled.
  Gate24 core passes 345,517 assertions/5,291 tests; see
  `evidence/shared-gate24.md` for snapshot scope and incomplete diagnostics.
- Root froze the LiteAdaptor file responsibilities and ownership/measurement
  contract in `lite-adaptor-work-plan.md`. Its 35 original CJS observations
  repeat exactly, with nine TS and nine CJS source locks. Native LiteAdaptor,
  CommonOutputJax, wrappers and SVG scene construction remain outstanding.
- Root independently repeated the Boldsymbol/ConfigMacros original oracle.
  Its output equals the existing original corpus exactly. Comparing all native
  emitted fields from Gate24 gives 10/16 matching cases (all ConfigMacros);
  all six Boldsymbol cases still retain `fixBold` and miss the Source rewrite.
  The Source oracle uses full `TeX.compile`, not direct `TexParser.mml`.
- AMS default array spacing was traced to UTF-16 string literals selecting the
  bool Property constructor. The two caller defaults have been corrected to
  string Properties and now match. Base37 is 35/37, AMS24 is 23/24 and the
  supplemental36 is 33/36. Both remaining Base parser states match; an extra
  `align:'undefined'` attribute has a subsequent uncompiled caller fix. The
  alignedat supplemental row now matches. AMS tag/factory/filter paths,
  Begingroup and TextMacros registration remain assigned to the parser owners.

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
- Real TextMacros preprocessing still fails at the actual inner nodeFactory
  probe before parsing. AMS table attributes, one-sided-arrow postfilter flow,
  package map registration and Begingroup scope behavior still require fixes.
  ConfigMacros' extra count digits were traced to an early string conversion in
  BaseMethods.Macro; its raw argument propagation and ParseUtil boundary are now
  assigned to the actual owners. Fresh integration failures are not accepted
  successful Source parses or substitute error fixtures.
- Commit c7dc052 accepts the bounded canonical Symbol-valued Property marker
  bridge: 11 assertions in two executed tests plus a pinned original oracle.
  It does not accept Symbol keys, whole TokenMap reflection or Begingroup.
  Complete Source constructor/prototype/reflection coverage is still missing.
- Root froze the OutputJax/DOM-host responsibility and interface contract plus
  fourteen repeated original lifecycle/filter observations in commit 7f36040.
  A Luna now owns the complete OutputJax implementation, observer, tests and
  member ledger. These new files are unregistered/uncompiled and do not inherit
  earlier build acceptance. Native LiteAdaptor, CommonOutputJax and all common
  wrappers remain required; no new glyph or math-item state is permitted.
- Three persistent Luna owners cover OutputJax/font services, BaseItems/
  Begingroup/ConfigMacros, and TeX/TextMacros/AMS/Newcommand/shared filters.
  Root owns architecture, registration, shared builds and independent acceptance.
  Configuration allows eight threads, but this active runtime exposes four total
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
