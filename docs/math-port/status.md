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
- Latest accepted semantic change is ec87cc5: base verifyTree/verifyChildren.
  It matches 3,752 original records, passes four Native budget/recovery checks,
  and detects 24 compiling semantic faults. The normal verification filter
  also passes the 476 raw virtual-options checks: 4,232 assertions in three
  executed tests. See evidence/p32-verification-acceptance.md for scope.

Incomplete requirements, verified from the current source tree:

- Remaining tagged-tree MML consumers include explicit Unimplemented stubs.
  Error-node construction, row/identifier class assignment and subclass
  verification are the next bounded implementation packets P33/P34/P35.
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
