# Live property-copy metadata correction after Gate209

Status: the focused Host tests pass; this does not accept the complete renderer.

Root reviewed and applied candidate D868BE40276171E73E5F5332A1C7DA7EEF132FEAA5BA49D40F1840387DE5E1CD.
An independent Luna review and the pinned Attributes.js CJS witness preceded
application. Root's review also required closing the PropertyList-bound copy
and Symbol enumeration paths, rather than fixing only the two direct loops.

Object.assign-style copies now snapshot keys, query the current descriptor's
enumerability without retrieving its value, then perform a separate live Get
and Set. NativeRecordView and PropertyRecordBinding expose metadata-only
queries. MML/Common creator views read descriptor flags without invoking or
materializing creator getters. Bound record copying and Symbol enumeration use
the same metadata query. Cross-Context string-property projection preserves
the source owner's enumerable flags without reading values.

The first focused build failed. Root corrected NativeArray's qualified
NativeRecord::keyText call, a test fixture's optional Descriptor access, and
the test's options::PrimitiveHint qualification. The first build command also
omitted vcvars initialization; the retry uses the repository VS18 environment.
No old executable's results are attributed to that failed build.

The retry built FoliaMathHostTests in the existing build/core directory,
exit 0. Five positional-filter runs all exited zero and each executed nonzero
assertions, totaling 68 assertions across these groups:

- math_context_property_assign: 40 assertions;
- math_tree_context_copy: 8 assertions;
- math_context_bound_property_list_copy: 8 assertions;
- math_context_reprojection: 6 assertions;
- mml_creator_record_enumerability: 6 assertions.

All 2,161 captured inputs were unchanged through the focused build and runs.
The separately running G209 Core executable was not relinked; its before and
after SHA-256 remained 7d1f106de5901958ad1b465bde888ded9b529366a1c35ab66ebed61c076c51d8.
The authoritative result is build/math-port/metadata-focused-g209-r2-result.json,
with per-filter logs and input hashes alongside it.

The Common creator-record test is not included in this Host-only result and
still needs the Core target rebuild. Full SVG, layout and Renderer lifetime
acceptance have not been rerun after this correction. Foreign Context Symbol
identities remain a separate projection limitation; this result covers the
same-Context Symbol path and cross-Context string metadata only.

The original G209 Core was also run to completion under a non-admin Windows
debug child launcher. It returned 0xFFFFFFFF with ordinary UT failures, without
a second-chance access violation, and no dump was produced. This does not
explain or resolve Gate209's direct-run 0xC0000005. The exact executable was
unchanged. Records are build/math-port/g209-core-crash-debugger/run.json and
debug-events.log, with output in build/math-port/g209-debugger-run.log.

## Subsequent Tree-call focused check and Core capture

Root also applied the independently scoped ordinary Tree-call candidate
99D352380DB67CF3216FCB4199ACB4B1A967E3749C076C40B25A498D61756235.
Ordinary member invocation now calls the selected function directly, retaining
its receiver and original Get point, without reading the function's own call
property. Emitted explicit .call operations remain separate and observable.

The Host build passed. The new focused test initially opened a CRT runtime
dialog and timed out. Root added temporary stage output to establish that the
body reached the copied-constructor case; the callback then indexed its missing
second argument. Root corrected the test to supply Undefined when that argument
is omitted, matching Source call semantics, and removed the temporary output.
The subsequent Host build and exact Tree-call test passed all seven assertions,
with 2,163 unchanged captured inputs and the original G209 Core executable
unchanged. The prior five metadata groups also passed after the production
Tree-call change. The authoritative final Tree record is
build/math-port/tree-ordinary-focused-g209-r3-result.json.

A controlled debugger child run with _NO_DEBUG_HEAP=1 reproduced the G209 Core
access violation, twice at the same exact executable RVA 0x22A6F19, reading
address zero. The first minidump attempt failed because the helper had used
default eight-byte struct packing. Root checked the installed Windows SDK's
minidumpapiset.h: it declares MINIDUMP_EXCEPTION_INFORMATION under pshpack4.
After correcting the helper to size 16 and pointer/BOOL offsets 4/12, the second
run produced a 62,528-byte MiniDumpNormal with an ExceptionStream. The exact
executable SHA stayed unchanged. No registry setting or binary copy was used.

The dump is build/math-port/g209-core-crash-original-heap-packed/dumps/
FoliaTests-43708-20261016-062834-394.dmp. The matching debug-events.log records
the process, thread, instruction pointer, module base, RVA and read target.
The old unwind_dump helper prints stack addresses among its frames and cannot
be treated as a reliable source stack. The source function and actual cause
remain under investigation; the heap-setting experiment is not a diagnosis.
