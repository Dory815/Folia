# P51 actual FindTeX language binding prerequisites accepted

P51A adds ordinary array forEach to the existing Context owner. Root froze the
declaration, actual prototype method registration, unimplemented body and 22
original observations in 182266b. The graph compiled with 20 failing semantic
cases and two passing registration/override cases. Luna implemented only the
authorized body (e95e9c3); Root owns the oracle, review and acceptance.

The final 22 original observations pass, including the actual looked-up method,
arity/nonenumerability, three callback arguments, original receiver identity,
thisArg values, holes and inherited slots, captured length, getter effects,
future-slot edits/deletes, replacement of the caller's array variable, ignored
callback returns and propagated exceptions. All eight compiling faults are
detected and the restored baseline passes. It is the existing ordinary-array
host profile; generic forEach on arbitrary array-like objects is not claimed.

P51B adds only native reference conversion between that same Context and the
accepted RegExp host. The runtime owns one RegExpBindings registration/index per
Context; every actual caller borrows it. It creates one real shared RegExp per
Source new expression, puts its canonical handle in ordinary Source dictionaries
or tuples, and resolves cursor/exec operations to that very native instance.
Options.copy retains the actual native constructor instance alias. No second
dictionary, compiled expression, cursor or matcher is synchronized with it.

All eight original Options/RegExp identity and cursor observations pass, with
ten focused native lifetime/rejection assertions in two tests. Five compiling
faults are detected: ordinary-object constructor metadata, shared instances for
new expressions, discarded cursor writes, recompilation instead of shared exec,
and teardown retaining owners. Restored baseline passes. Hashes are locked in
the mutation manifests, and oracle --check reproduces all gold.

One initial C++ fixture read lastIndex in the same concatenation expression as
exec; C++ operand evaluation order allowed the read to happen first. Root split
execution and observation into consecutive statements. Original expected output
and production binding code stayed unchanged.

Readonly Luna review found no blocking mismatch within either stated profile.
Its earlier P51A CJS-lock concern mixed in another oracle: P51A directly executes
Node's Array.forEach and uses FindTeX TS only for caller provenance. Root checked
the actual file and the reviewer corrected that report. Its RegExp runtime
ownership reminder remains a required integration invariant: do not construct a
new registration/index per finder instance or call.

The complete core build/test passes, exit 0 (build/p51-bindings-full-core.log),
including the extended Context primitive and all previous math/core regressions.
These tests accept host prerequisites, not the actual FindTeX algorithms. The
complete finder is next; its Source superclass, typed option graph, ordinary
array iteration, canonical RegExp references, UTF-16 scanner and lossless
WTF-8 protoItem boundary are now available. Full parser/package behavior,
fonts/common layout/scene/SVG and Folia native integration remain required.
Mermaid is still paused with its existing files and checkpoint preserved.
