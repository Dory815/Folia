# Native math development checkpoint — 2026-10-19

This checkpoint saves the accumulated native MathJax development sources,
tests, Source witnesses and integration proposals. Commits are grouped by
module responsibility, with at most 20 files per source batch. It is a
development checkpoint; its intermediate commits are not independently
validated releases. The final tree still requires the acceptance below.

The starting HEAD is `f09f450705e83b57d0ca38561e041ef8706451c0`.
The inventory contains 1,086 math/CMake paths and eight paused Mermaid
observation/record paths. Build products, downloaded dependencies, caches and
ignored drafts are excluded. Source fixtures are retained with their probes.

Verified package evidence: next-six matches 20/20 at Gate222; font packages
match 16/16 at Gate223. Gate221 matches 148/150 SVGs, with both tooltip records
failing geometry handling. Nested Scene composition remains unfinished, and
Folia still uses QuickJS for math.

Gate224 is a failed, interrupted build, not a native test result. Root stopped
its verified process tree after compile errors, confirmed all build processes
ended and audited all 2,192 captured input entries unchanged. Root subsequently
restored the existing Context attached-owner API used by Lite event
registration, fixed a pointer conversion in MML children binding, and corrected
the new lifecycle fixtures. These post-Gate224 changes await compilation.

The MML ownership refactor in this checkpoint replaces per-parser registries
and owner selection with Context-canonical composition and a narrow Source
allocator. Request captures have explicit registry-owned tokens; binding
completion metadata lives on the canonical Context payload. Its new lifetime
tests remain unexecuted. Deep initializer/field-bind failure stages and
Renderer owner-level failure/retry are not fully covered.

Mermaid work remains paused. Its separate checkpoint saves existing
constructor observation sources and notes only; no additional implementation
or testing is performed as part of saving them.

## Subsequent validation

The historical checkpoint above describes its original tree. Gates 233–236
subsequently fixed Context teardown, passed all 17 MML allocator-owner tests,
and reached 150/150 exact SVG matches plus event-position parity. Standalone
package comparisons now pass 24/24, 23/23, 16/16, and 20/20. See
`evidence/shared-gate233.md`, `evidence/shared-gate234.md`, and
`evidence/shared-gates235-236.md` for the exact scope and remaining failures.
The full host suite still has three assertion failures in two tests. Private
Renderer allocation lifetime and Folia integration remain unfinished.
