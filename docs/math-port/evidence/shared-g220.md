# Gate220 focused evaluation

All 61 requested targets built successfully with at most four compile jobs.
The 2,185 registered input entries remained unchanged through the focused
evaluation. All 13 test/capture invocations exited 0, and each executable's
before/after SHA-256 matched. No full Core, layout, SVG or lifetime evaluation
was performed in this gate.

The ParseMethod receiver regression passes 11 assertions: ordinary Context
objects and primitive Values retain Source `this`, alongside undefined and
native owner receivers. This removes the textcomp-symbols receiver exception;
it does not resolve the package state comparison.

The new TextMacros live-state checks pass for a single TeX and for a second
TeX sharing a Context with an uncompiled warmup TeX. They verify the actual
jax identity, canonical ParseOptions, shared packageData, compiled mathItem,
root and outer texParser identity. The package capture still differs, so these
minimal graph tests do not establish the complete NativeRenderSession path.

The next-six comparison remains rejected at 18/20. Both textcomp cases now
first differ in the recorded jax ParseOptions keys: native lacks mathItem.
Font packages remain rejected at 13/16 with the same first state difference.
Capture exit 0 is not package acceptance; the font comparator also returns
process exit 0 despite reporting `accepted: false`.

Artifacts are under `build/math-port`: `focused-evaluation-g220.json`,
`gate-220-focused-input-changes.json`, per-step stdout/stderr and capture
provenance, `next-six-focused-comparison-g220.json`, and
`tex-bboldx-font-packages-comparison.json`. The latter is the comparator's
default output name. Folia still uses QuickJS for math; native integration
remains pending actual renderer and lifetime acceptance.
