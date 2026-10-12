# Root borrowed surface dependency acceptance

Accepted the eight original surface methods and three original storage helpers
under the pre-implementation M3j2 contract. Sol changed only the eleven assigned
bodies and its report. Root independently reviewed original algorithms and the
diff, replayed all original observers, ran complete Native comparisons, injected
faults, and verified the full registered integration suite. Root retained the
original copyright/license attribution on the derived types/implementation.

All 3,240 original surface records and 567 exact-integer size records pass with
zero mismatches, actual stubs or unexpected errors; all eleven direct actual
body witnesses pass. Their original SHA256 values remain those frozen in the
contract. Root identified an ownership observation missing from these records:
they witness subview borrowing but do not compare the incoming Vec allocation
with the returned Mask. An independent append-only original observer therefore
adds 80 ownership records, including twelve accepted masks over varied Vec
capacities. Allocation identity and retained capacity now match actual original
Mask::from_vec. This supplement changes neither of the original frozen fixtures.
Its SHA256 is
0e67088f5d6cbf1a88494022b1943a912fd3fb0c25c1c0db94320592ed02b846.

Nineteen compiling faults are effective: RGBA extra-buffer rejection, whole and
subview stride errors, RGBA/A8 offset confusion, truncated retained tails,
incorrect Mask exact-length acceptance, cloned owned Mask buffer, incorrect
logical size, absent intersection, omitted i32 row limit, wrong height/width
arithmetic, wrapping multiply/add, and bypassed helper row validation. Each
fault is detected by actual original records and fails assertions with Cargo
exit 101. All working vendor files remain byte-identical during the isolated
run. Root's later changes to these implementation files are license comments
only; the accepted algorithm bodies are unchanged.

All 29 registered integration targets pass: 77 tests, including eleven actual
Direct2D WARP tests with the previously pinned Arial font. This is the published
crate integration suite; no unavailable upstream library fixtures are claimed.

Reproduce the three original --check oracles (borrowed-surfaces, borrowed-size,
borrowed-ownership), borrowed-surface-mutations.cjs, and the registered Cargo
integration suite with --locked --offline --no-default-features --features
direct2d --test '*' -- --nocapture --test-threads=1.

Ignored evidence SHA256:

| Evidence | SHA256 |
|---|---|
| build/mermaid-borrowed-dependencies-red.log | 30363b4fb9a98ab19aedd960f3a4e35de8a3ab02e24bb3c3eec4a452819f04f0 |
| build/mermaid-borrowed-surface-baseline.log | 6c57616fed519ef643c10bce68e5de7314c66b6fd190ea150942e18cdded7507 |
| build/mermaid-borrowed-surface-mutations/results.json | 7a597f15703ee519e8ec7d6304d297b74cd65e44a70d127df45feddbd29e11e9 |
| build/mermaid-borrowed-surface-native-suite.log | 442ea99a4ef7d67ff36a1f48ce9cf1d22d326f24dc9b4b30bcad9f6216681fd7 |

These are resource dependencies only. Three independently built/compiled blitter
contexts, constructor rejection/optional-mask policy, sparse alpha callbacks,
complete shared scene/SVG/Direct2D and Folia remain required. The historical
whole-rendering tally is unchanged and no Folia path is enabled by this packet.
