# P49 native identity binding and actual FindMath acceptance

Mermaid remains paused at its recorded checkpoint. This packet advances only
the native MathJax goal; it does not accept a working TeX parser or SVG output.

P49A supplies the typed bridge from the canonical Context option graph to actual
native identities. Dictionaries and option arrays remain Context objects. Node
handles retain the actual shared MML factory that owns their raw pointers; class
handles retain actual constructor descriptors. Rebinding returns the canonical
handle, foreign Contexts and noncanonical payloads fail, and runtime teardown
clears the index and releases retained graph owners. Registry destruction alone
does not invalidate payload owners still retained by Context. This is native
host plumbing, not a substitute parser, node graph, or class registry.

The original Options/MML observations pass (9 assertions), alongside 26 focused
native ownership assertions in six tests. A readonly Luna review exposed a raw
pointer lifetime gap and inconsistent repeated-bind behavior; Root fixed both
and added witnesses. All eight compiling faults fail the corresponding tests;
the restored baseline passes. The mutation manifest locks the final host hash.

P49B supplies the actual core AbstractFindMath superclass required by FindTeX.
ProtoMathItem and ProtoMathItems now belong to their original core MathItem
owner and are shared by InputJax and FindMath. Root froze declarations, allocation
and class-registration host, and original observations in 986974f before Luna
implemented the sole constructor body. It captures this.constructor once,
allocates a fresh destination, reads captured CLASS.OPTIONS, performs the actual
default and user merges, then publishes options. The supplied parameter is
unchanged; exceptions propagate before publication. findMath remains abstract.

All eleven original constructor observations pass. The initial unimplemented
body compiled and failed all eleven assertions. Readonly Luna review found no
concrete mismatch. Five compiling faults are detected: rereading constructor,
sharing defaults as instance options, skipping either merge, and publishing
before a throwing user getter. Restored baseline passes.

The complete core build/test passes, exit 0 (build/p49-full-core.log), including
both packets and existing core/math regressions. Source oracle reproduction uses
locked original TS/CJS, not a reimplementation. Detailed scope and lifetime rules
remain in p49a-native-identities-contract.md and p49b-find-math-contract.md.

Next: actual FindTeX requires a global non-Unicode RegExp host with UTF-16 code
unit indices. SRELL's default char16 traits merge surrogate pairs and reject
some valid non-Unicode identity escapes emitted by quotePattern; both differences
must be adapted and compared before actual finder bodies. Configuration,
ParseOptions, the complete parser/package graph, fonts/common layout/SVG and
Folia integration still remain required by the active goal.
