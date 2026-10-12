# Current native MathJax status

The goal remains native TeX -> shared mathematical layout -> SVG, then Folia
integration replacing the math QuickJS worker. It is not currently usable as a
renderer, and Folia still uses the existing QuickJS MathJax path.

Completed current-state evidence:

- MathJax-src 4.1.3 is cloned in .reference/MathJax-src, pinned to
  fb987178c2d279a99b0db5ea02e751691703955e. scope.json inventories 246 reachable
  upstream files for the enabled Folia profile.
- The C++23 folia_math target contains 90 module interfaces. The normal
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

Incomplete requirements, verified from the current source tree:

- Remaining tagged-tree MML consumers include explicit Unimplemented stubs.
  The next packet covers action, MathChoice, menclose and mpadded under Root's
  frozen file ownership and acceptance contract.
- TeXParser, Stack and the complete base/AMS/enabled-package command pipeline
  have not been implemented. Current input/tex contains foundations only.
- There is no src/math/output common-wrapper/scene/SVG implementation, nor a
  native mathematical font service or public complete TeX -> SVG entry.
- Formula SVG parity and Folia native integration cannot be accepted without
  those stages. The WinUI project still links QuickJS and retains MathJax assets.

The historical scope ledger marks 54/246 files implemented and 192 not started.
That is not a reliable 22% completion claim: several historical implementations
have tagged-tree consumers awaiting restoration, and source-file counts do not
measure coverage or usability. The 90 interfaces likewise measure compilation
structure, not 90 completed upstream files. No complete-clean-port claim follows
from the existing build or local test counts.
