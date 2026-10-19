# Gate219 SVG observation and input audit

The incremental build completed all 61 requested targets with at most four
compile jobs. Its 2,183 registered input entries matched the pre-build snapshot
when the build ended. The profile probe adds observation of frozen Scene
listener positions to the existing public Renderer target.

The native profile process exited 0 after about 121 seconds, producing 150 SVG
records in continuous and fresh-session-per-case modes, with zero thrown
render errors. The executable SHA-256 remained
`0e18e07b2f420df23c7e0b41022782a153dad523779c97ec82c85a69739df817`.
Against the independently audited passive-host Source baseline, 148/150 SVG
hashes match. Both differences are `action-tooltip`; the native output has an
empty `mjx-tip`, while Source includes its nested SVG. Native `foreignObject`
also has additional placement/size attributes. This remains a rendering
difference, not an accepted host implementation.

A separate diagnostic checks all four Maction mode/case pairs: the six event
registrations have the same ordered types and exact SVG-root-relative
kind/index/`data-mml-node` paths as Source. This metadata agreement does not
make the tooltip SVG bytes equal. The strict combined comparator correctly
fails on the tooltip byte hash.

The final input audit also failed: 25 distinct source/test files changed from
LF to CRLF during the native observation. Every file normalizes exactly to its
frozen SHA-256; no non-newline content difference was found. Their changes are
preserved. The audit is recorded as failed, rather than replacing the frozen
fingerprints or describing the inputs as unchanged. Thus the native output is
a diagnostic observation of the identified executable, not a passed frozen
source acceptance gate.

Artifacts under `build/math-port` include `profile-evaluation-g219.json`,
`profile75-full-svg-native-g219.json` and its provenance/output files,
`profile75-full-svg-binary-audit-g219.json`,
`gate-219-profile-input-changes.json`, `gate-219-eol-only-audit.json`, and
`listener-metadata-only-g219.json`. All native/build processes terminated
before Root released the input freeze. Full Core and lifetime acceptance remain
pending, and Folia continues to use its math QuickJS path.
