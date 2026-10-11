# Root acceptance: original Linear/Radial producer wrapper bindings

Root reviewed the two delivered bodies. Linear passes the caller's sole builder
to generic Gradient with no radial plan. Radial uses the already accepted stage
provider, resets mask and raw p0/p1 before any generic call, then passes that same
builder and local plan. No extra context/stage owner or SVG path was introduced.
Original constructors, SolidColor fallback and Shader variant dispatch are not
part of this controlled-state wrapper acceptance.

All **53,862** original complete-state observations pass with **zero mismatches
and zero stubs**, including 3,266 radial-kind stimuli, 21 original radial stage
orders, sentinel context, force flags, inverse failures and capacity partial
effects. The unchanged original fixture independently reproduces SHA
`add70dc18ff8dd0bc67e266e45cd6dbc5a7d92ab72e68d657d50d55b8d9853a9`.
The pre-delegation actual red separately executed 15,572 Linear and 38,210 Radial
stubs and had 53,654 mismatches. Rejected inputs/coincident Linear capacity panic
snapshots were never claimed individually red.

All **16** Root semantic mutants compile and fail assertions, covering both
delegations, returned result, color space, context/force preservation, unwanted
radial callbacks, p0/p1/kind/plan corruption, whole mask reset and writes moved
after generic calls. Crashes/build failures do not count. Working source and
fixtures remain byte-identical throughout the isolated run.
Result: build/mermaid-gradient-shader-mutations/results.json, SHA
`c759aeda113a44c07a3861d7683378da9ccb57f2494d7b1759ac6f0c06f3108b`.

Final locked/offline native matrix passes **23 targets / 66 tests**, including
**11 actual Direct2D WARP tests**, with the fixed Arial font input. Prior generic,
Pattern, geometry, stage, sampling, scene and compiler comparisons remain green.
Log: build/m3i6d-final-native-matrix.log, SHA
`b8ac8dce70e22fd5093c54b3f38c077d8b0dcd4c140219e9a084db5f1a52480f`.

This packet completes the selected wrapper wiring, not whole rendering. The
historical whole-rendering tally stays **34/70**; the full shared executor, all
three blitter paths, Shader constructors/variant dispatch, complete scene
paint/text and final SVG/Direct2D/Folia acceptance remain open. Both backends must
consume the same native scene, and Folia is not connected before full acceptance.
