# Private Renderer Source allocation domain: gate 247

Renderer requests now activate a Context-owned lexical allocation domain for
Source MML constructors. This changes the native allocation owner, while each
node preserves its observable Source `factory` value. Existing receivers retain
their original allocation owner when initialization resumes inside another
active domain. Direct Source construction outside Renderer continues to use
the shared Context allocator.

Each private allocator has one canonical factory identity. Cleanup first
retires completed layout/item consumers and captured MML/child-array identities,
then retires the exact private factory and its cached creator-record/function
records. Activation unwinds lexically; the domain remains available when
completion-aware binding or retirement needs a retry. Renderer also rejects
reentrant requests on one session.

The first strict combined test failed at gate 241. Seven isolated probes at
gate 243 established that reading `nodeMap` alone retained an extra factory
owner; factory binding, `node`, creator materialization and text copy alone
released it. The map conversion callback captured a factory it did not use.
Removing that capture preserves the independent class-map owner.

Factory binding also reinstalled the `MML` getter on its shared prototype for
every instance. It now installs the Source class getter once, recording success
only after definition completes. Later bindings preserve a caller's replacement
or deletion. The pinned CJS witness checks the getter, identity and both changes.

Gate 247's complete Host suite passed all 455 tests / 3,185 assertions,
including fourteen domain/getter tests. These check direct native creation,
missing-child repair, text copy, exact canonical retirement, immediate weak
factory expiry, nested activation, prototype behavior, repeated rendering,
cancellation/recovery and detached Scene use after Renderer destruction.
The separately pinned formatter witness also passes. The build used at most
four compile jobs; all 2,204 registered inputs remained unchanged through Host.

This accepts the tested allocation-domain behavior, not overall memory
boundedness. The separate Context inventory still demonstrates growing live
records; see [context-record-inventory-g246.md](context-record-inventory-g246.md).
Broader layout/font/SVG execution, full core/platform acceptance and Folia
integration remain pending. Folia still uses QuickJS for math.

Evidence: `build/math-port/domain-evaluation-g243.json`,
`build/math-port/host-g247.txt.*` and
`build/math-port/mml-prototype-getter-source-g247.json`.
