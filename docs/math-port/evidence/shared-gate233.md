# Shared gate 233: Context teardown and console failures

The 61-target build completed with exit 0 using at most four compile jobs.
All 2,196 registered inputs remained unchanged through native evaluation.

The previously failing termination stack passed through `Context::record`,
`NativeIdentityBindings<MmlFactory*>::clear`, `MmlContextBindings`, and the
`RuntimeModules`/`ParserRuntime` teardown chain. Default vector destruction
released earlier Context record slots before later retained owners finished
their registry cleanup. Context now drains record contents while all slots
remain readable. A focused regression covers both relative allocation orders.

Actual gate 233 results:

- Context teardown regression: exit 0, six assertions.
- All seven nested Scene projection tests: exit 0.
- Nested Source SVG corpus: 12/12 exact SVG matches across continuous and fresh
  sessions, pinned MathJax 4.1.3 commit
  `fb987178c2d279a99b0db5ea02e751691703955e`.
- `FoliaMathTexNextSixDump`: normal exit 0; 20/20 strict Source comparisons.
- Font packages: normal exit 0; 16/16 strict Source comparisons.
- Colortbl: normal exit 0; 29/29 cases and 1/1 maps match Source.
- ParseOptions clear regression: exit 0.
- Six-package and next-seven observers: normal exit 0, but strict comparisons
  still fail (0/24 and 1/23), predominantly missing canonical MML identities.

The console runtime also records uncaught exception details and module-relative
stack addresses to stderr. The existing noninteractive CRT configuration applies
to all CMake Windows console tests/probes. The temporary linker map used for the
diagnosis is not enabled in committed CMake configuration.

Ignored execution artifacts and executable/input hashes are under
`build/math-port/*g233*` and `build/math-port/g233-*`. These focused results do
not establish full core-test acceptance, general lifetime acceptance, or Folia
integration. Those remain open.
