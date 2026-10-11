# P26 simple/mfenced observer emitter repair report

This is a partial implementation report for the native C++ observer emitters
in `tools/math-port/simple-mml-oracle.cjs` and
`tools/math-port/mfenced-oracle.cjs`. It does not claim independent completion,
Source-equivalence acceptance, or compilation.

I updated the simple emitter's ChildArray construction helper to store tagged
`NodeValue`s, adapted its record/probe virtual overrides to the current tagged
`setTeXclass`, `getPrevClass`, and `updateTeXclass` signatures, and changed its
captured state child projection to use the ChildArray handle, initial length,
`has`, and `get`. I also adapted the simple native setup parent assignments and
the inferred-child observer to the current tagged APIs. Root completed the
remaining simple emitter changes after my writes began failing.

I initially adapted mfenced's native ChildArray helper and selected tagged
virtual signatures. During a broad text replacement, I also changed part of
the original JavaScript prefix (`prev.nodeOrNull()`); that was incorrect because
those JavaScript values are ordinary source objects. Root rejected and restored
that Source-side change. I also suggested a live indexed `for-of` loop for the
mfenced `TraceFactory` observer, but the source expression is `args[1].map(label)`.
Root corrected the native emitter to preserve the map observer's captured
length and `Has`/`Get` behavior. These corrections preserve the frozen Source
stimuli and expected values.

Further writes to the oracle scripts failed with the Windows error that the
file was open through a mapped section. Reads remained available, but both
`apply_patch` and in-place `WriteAllText` failed. I did not move, replace, or
truncate either file. I sent Root the remaining emitter locations and proposed
tagged API adaptations; Root then completed the edits directly. No generated
fixture was written by me.

Root reports that the three original oracle `--check` runs and Source-table /
prefix invariants pass after its repairs. At report time Root's compile-only
gate is still in progress. The frozen simple profile remains 106 cases, 7,667
assertions, 1,605 raw rows, and 296 values; mfenced remains 225 cases, 984
assertions, and 509 values. Those checks and profile counts are Root's
verification, not independent acceptance by this implementation report.
