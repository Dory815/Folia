# Original common-layout and Unicode observations

`tools/math-port/common-layout-original-oracle.cjs` executes the actual pinned
MathJax 4.1.3 profile with all 38 configured packages, NewCM and three extensions.
It validates scope/profile hashes, 59 common/SVG wrapper TypeScript hashes,
seven principal executable hashes and package versions, and records 295 loaded
dependency hashes. The observer records actual wrapper BBoxes, variants, children,
raw place/placeChar calls and primitive emission before SVG serialization.
It does not parse SVG into layout, and native output is never its input.

All 18 initial cases succeed: identifiers, mathbb, fractions/atop, roots, scripts,
limits, stretch fences, accents, matrix, arrows, text, color, bbox, phantom and
width-driven linebreaking. Two complete results agree exactly. Disabling tracing
gives the same SVG, errors and font-load requests. The traced output SHA-256 is
7dd6bc040670b4feab2a2b5b6bafe6be1dad9c4344c9e0e2da47b4363edec0b6.
All selected cases have no dynamic font-load requests; loader coverage is open.
The ignored observations are `build/math-port/common-layout-original-root*.json`.

## Expanded configured-profile corpus

`--profile-corpus` adds 57 cases from
`tools/math-port/common-layout-reference/profile-cases.json`, for 75 total.
Its `profile-source-lock.json` validates all 299 executable dependency hashes
and the scope/corpus hashes, then checks the actual loaded closure exactly.
The external font loader points to the actual NewCM `cjs/svg/dynamic` directory.
AsyncLoad and retry implementations remain the original implementations.

The observer also records actual compiled MML before Common wrapping and Source
warnings. Two complete results and a subsequent locked run agree exactly:
SHA-256 `9f58d09411512b4682b1f0049e40d6a47844ddffb9794eda52e2700998e0d740`.
Disabling layout tracing preserves SVG, errors, loads, MML and warnings. The MML
compile observer remains enabled in that comparison. The original 18-case result
still has exactly its earlier hash.

The expanded corpus observes 31 wrapper kinds and all 38 configured packages are
loaded. Intended feature labels are corpus metadata, not proof that every package
handler or every wrapper method has been exercised. There are 73 SVG outputs and
two actual Lite-host failures: action-toggle and action-tooltip throw
`TypeError: node.addEventListener is not a function`. These failures are retained
as Source observations. Error MML is likewise retained for invalid inputs.

Four actual dynamic font requests are now observed: arrows, symbols, monospace
and sans-serif. The corresponding cases are extpfeil-arrows, textcomp-symbols,
verb-literal and unknown-text-families. Source's mhchem variant warning is retained.
These observations do not establish native dynamic-loader parity.

Ignored results are `build/math-port/common-layout-profile-root*.json`.

`tools/math-port/unicode-regexp-original-oracle.cjs` records 37 actual JavaScript
host observations, including the pinned LiteParser and NodeMixin patterns read
from the actual modules. Node is v24.13.0, Unicode 16.0. Two full results agree;
their SHA-256 is e6c01910ecb588816a0b1c99f7746b7a9b21bd8abeb1cd84f93a3b7163e5d634.
The corpus covers Unicode code points, UTF-16 capture offsets, lastIndex within
surrogate pairs, lone surrogates, empty exec versus split advancement, case folds,
word boundaries/backreferences, syntax errors and actual parser split captures.
Native Unicode RegExp support has not been accepted by these observations.

Root also repeated the corrected Entities original oracle twice, without replacing
AsyncLoad or retry implementations. Only the external mathjax.asyncLoad provider
is injected. The whole-result SHA-256 is
b6e2d916b4b6e8c50bd7768cae15f3e35780cce443a8efe718b70c33cade4f8b.
The 406 base entries, 29 shard additions, numeric conversions, mutations and five
lazy-load paths agree exactly. Failed/missing providers still leave Source's
loaded-before-request state; the second request does not retry the provider.
The native EntityModule candidate is uncompiled and unaccepted.

These are baseline Source observations. Full native common layout, completed
scene, SVG serialization, semantic-fault tests and Folia integration are still
required by `../common-layout-work-plan.md`.
