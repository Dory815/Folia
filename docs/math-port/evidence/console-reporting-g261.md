# Windows console failure reporting: gate 261

The popup reported by the user named `FoliaMathTexNextSixDump.exe` and
`abort()`. Console tests and probes use the common CRT configuration in
`tools/test-support/windows_console_runtime.cpp`, linked directly as an
object library into the CMake executables.

Gate 261 rebuilt 16 targets with at most four compile jobs. The build exited
0 and all 2,210 registered inputs remained unchanged. The rebuilt
`FoliaMathTexNextSixDump.exe` then exited normally with code 0 in about five
seconds. Its SHA-256 was
`ff74739c9006c42534b7fa98198351980951841fee30e66d95b5e2a73abfc76a`.
The execution record is in ignored
`build/math-port/g261-next-six-popup-check.json.provenance.json`.

A separate small executable compiled against the formal console runtime
exercised normal exit, `abort()`, an assertion failure, and an uncaught C++
exception. All four finished within the ten-second per-process limit.
Normal exit returned 0. Each failure returned 3 and wrote its diagnostic to
stderr. No process remained waiting for a dialog. Records are in ignored
`build/math-port/g261-console-failure-results.json`.

The runtime additionally supports an optional absolute-path
`FOLIA_TEST_CRASH_DUMP` environment variable. Unhandled native exceptions
then report their exception code and module-relative address and attempt
to write a normal minidump. A five-case standalone check covered normal
exit with and without the variable, a native exception, an unwritable
destination, and rejection of a relative path; results are in ignored
`build/math-port/g260-console-probes/results.json`. Dump writing is disabled
when the variable is absent. No system-wide error-reporting settings change.

These checks establish the finite console-reporting behavior and normal exit
of the reported probe. They do not establish full core-suite acceptance or
completion of the MathJax port.
