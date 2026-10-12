# Current native MathJax status

The goal remains native TeX -> shared mathematical layout -> SVG, then Folia
integration replacing the math QuickJS worker. It is not currently usable as a
renderer, and Folia still uses the existing QuickJS MathJax path.

Completed current-state evidence:

- MathJax-src 4.1.3 is cloned in .reference/MathJax-src, pinned to
  fb987178c2d279a99b0db5ea02e751691703955e. scope.json inventories 246 reachable
  upstream files for the enabled Folia profile.
- The C++23 folia_math target contains 109 module interfaces. The normal
  FoliaTests target builds and links; individual accepted packets have actual
  Source observations, Native checks and independent acceptance evidence.
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

Incomplete requirements, verified from the current source tree:

- ParseOptions, BaseItems and the configuration pipeline
  are the next parser prerequisites. Accepted input utilities do not establish
  complete TeX/package parsing or whole-source reflection parity.
- TeXParser and the complete base/AMS/enabled-package command pipeline
  have not been implemented. Accepted Stack and input foundations do not provide
  a functioning TeX parser.
- There is no src/math/output common-wrapper/scene/SVG implementation, nor a
  native mathematical font service or public complete TeX -> SVG entry.
- Formula SVG parity and Folia native integration cannot be accepted without
  those stages. The WinUI project still links QuickJS and retains MathJax assets.

The historical scope ledger marks 54/246 files implemented and 192 not started.
That is not a reliable 22% completion claim: several historical implementations
have tagged-tree consumers awaiting restoration, and source-file counts do not
measure coverage or usability. The 109 interfaces likewise measure compilation
structure, not completed upstream files. No complete-clean-port claim follows
from the existing build or local test counts.
