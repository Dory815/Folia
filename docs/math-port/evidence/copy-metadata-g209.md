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
