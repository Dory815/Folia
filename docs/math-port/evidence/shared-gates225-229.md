# Gates 225–229 — integration failures and console reporting

These gates do not accept the native MathJax port or Folia integration.
Mermaid remains paused. Folia still uses QuickJS for math.

## Build evidence

- Gate225: actual build exit 1; all 2,192 inputs unchanged. New MML ownership
  tests incorrectly treated reference-returning APIs as pointers and the MML
  class vector as an object with `get()`. Root corrected these test API calls.
- Gate226: actual build exit 1; all 2,195 inputs unchanged. Colortbl's observer
  still dereferenced its new ConfigurationRef parameter twice. Root corrected
  that one expression.
- Gate227: all 61 requested targets built, exit 0, maximum four build workers.
  All 2,195 inputs remained unchanged through focused native evaluation.
- Gates228 and 229: all 61 targets built, exit 0, maximum four workers.
  Each gate audited 2,196 unchanged inputs through its single native diagnostic.
  These builds add process-local console failure reporting, not math fixes.

## Gate227 native results

`build/math-port/focused-evaluation-g227.json` records each executable hash,
arguments, actual process result, stdout/stderr paths and timestamps.

- MML ownership: 13/17 tests passed. Owner tests 1, 2, 7 and 14 failed.
  The newly added constructor-call test preserving an existing native product's
  allocation owner passed. Owner7's service expiry expectation remains open.
- Nested Scene: 1/7 tests passed. Cases 0, 1 and 3 exited 3; cases 2, 4 and 6
  exited 0xC0000005. The same-Builder patch is an unaccepted development checkpoint.
- Host exited 0xC0000005; FunctionList exited 3; Textmacros exited 0x80000003.
  ParseOptions.clear also failed. No full Core or 150-record SVG acceptance ran.
- Strict package comparisons were rejected: six-packages 0/24, next-seven 0/23,
  font packages 13/16. The first two report
  `ParseOptions.mathItem must be a canonical MathItem in this Context`;
  the font difference includes `config.parseOptions.clear is not a function`.
  Earlier Gate222/223 results do not validate this changed ownership graph.

The six new pinned Source tooltip/break captures were independently regenerated
at `build/math-port/nested-scene-source-g226.json`; SHA256 is
`7e33b437f03e4a9240d86fbd25d4957d6bc7d9587208aa7ba544f48302de568e`.
No native equivalence is claimed for those captures.

## Desktop dialog correction

The user's repeated Debug Error dialog came from the Debug CRT handling
`abort()` in `FoliaMathTexNextSixDump.exe`. CMake console executables now link
one common startup object. It routes CRT reports to stderr, keeps the abort
message, disables ReportFault UI, and sets process-local Windows error modes.
It leaves the process failure visible to the runner. WinUI uses its separate
MSBuild project.

The exact failing executable was rerun with short timeouts, without a debugger:

- Gate228: PID70560 exited 3 naturally after 3.710 seconds, before its five-second
  limit. stderr contains `abort() has been called`; no timeout or interaction.
- Gate229: PID45048 exited 3 naturally after 3.369 seconds, before its eight-second
  limit. stderr contains the same abort message. Binary SHA256:
  `2f990f6665339aeb0d73cf19ace9eddc658c16e061170ce8ddaa5e36762a6d8b`.

Provenance is in `build/math-port/next-six-console-report-g228.json.provenance.json`
and `build/math-port/next-six-console-report-g229.json.provenance.json`.
Both audit unchanged inputs and executable bytes. Root confirmed no remaining
Folia, compiler, linker, CMake or Ninja processes before unlocking inputs.
The abort's underlying cause remains unresolved; removing its dialog is not
a successful math test.

## Next boundaries

Diagnose the current canonical MathItem/ParseOptions/service-owner regressions
and the nested Scene failures before rerunning broad acceptance. An independent
review also confirms that Source products accumulate in the Context ambient
allocation factory while Renderer currently retires a distinct session factory.
A private Renderer allocation-domain design remains draft-only and unvalidated.
