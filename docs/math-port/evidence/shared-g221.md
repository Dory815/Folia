# Gate221 SVG and package diagnostics

All 61 targets built at a maximum of four compile jobs. The 2,187 frozen
input entries remained unchanged through focused tests and the SVG profile.
Executable hashes remained unchanged. All processes terminated before Root
released the source freeze.

The tooltip candidate is not accepted. Its new Scene nesting check fails;
the attribute replacement fixture fails because it queries the adaptor before
constructing its output host. Maction's existing public Renderer test throws
`Invalid SVG geometry number`. Other focused checks pass, including 2,821 Host
assertions and 96 TextMacros assertions.

The SVG profile process exits 0 after about 247 seconds with all 150 records.
148 SVG byte hashes match Source. Both tooltip records now contain the geometry
TypeError instead of SVG, so strict SVG/listener acceptance fails. These results
identify a remaining nested output boundary; they do not accept the candidate.
In particular, Renderer.renderScene replaces the Jax output consumer globally,
so a nested processMath captures a Scene without appending it to the outer
parent. The ordinary SVG consumer materializes the nested Scene, whose DOM is
then reimported through geometry parsing. Nested completed layout must be
composed into the canonical parent without that round trip.

The real next-six session diagnostics show correct state after compile in
both fresh single-case and sequential modes: recorded jax equals the compiled
jax, outer ParseOptions identities match, mathItem is present, root is non-null,
and texParser resolves to the outer parser. Yet the packageData-entry trace
condition is never reached for the current jax and captured JSON is still old.
Package comparators remain 18/20 and 13/16. Observation ordering is therefore
being isolated; no synthetic fields or comparator exceptions were added.

Evidence under `build/math-port`: `focused-evaluation-g221.json`, per-step
stdout/stderr, `profile-evaluation-g221.json`, profile capture/provenance,
`gate-221-profile-input-changes.json`, and the two per-gate package comparison
reports. Full Core/lifetime acceptance remains pending, and Folia still uses
QuickJS for math.
