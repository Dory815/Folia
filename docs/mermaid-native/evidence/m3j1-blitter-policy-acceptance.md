# Root blitter policy dependency acceptance

The five dependency bodies are independently accepted within the frozen M3j1
contract. This does not accept blitter construction, callbacks, complete scene
rendering, or Folia integration. The historical rendering-file tally is unchanged.

Root replayed original tiny-skia 0.12.0 after implementation. The unchanged
fixture contains 3,445 records: all 29 blend modes crossed with four colour
spaces (116 policies), plus 3,329 colour conversion cases including eight
constructor rejections. Its SHA256 remains
`d4b941e3840104a0a7a11cc31cf2e302592ce852af99e32ae12bba22110d56ce`.
Native comparison passes every record and all five direct dependency witnesses,
with zero mismatches, stub calls or unexpected errors.

Root's isolated mutation gate detects all twelve compiling assertion failures:
missing Plus/Destination coverage, incorrect Source coverage, lost Source no-op
and Destination move, Hue/Luminosity substitution, source/destination gamma
confusion, Gamma2/Gamma22 substitution, extra Linear stage, and three byte
quantization errors. The byte cases detect replacing the original float add/cast
with round(), omitting its half-unit addition, and premultiplying alpha again.
The baseline also passes the existing public Shader, producer and complete-run
observations. Original working files remain byte-identical throughout the run.
The initial gate attempt stopped before mutation because Root's anchor matched
twice; Root corrected the anchor and the copied status regex, then reran the
whole gate. That failed harness attempt is not acceptance evidence.

All 27 registered Direct2D integration targets pass: 72 tests, including 11
actual WARP tests, with the previously pinned Arial font. This is the published
crate integration suite, not unavailable upstream library fixtures.

Reproduce with `node tools/mermaid-native/blitter-policy-oracle.cjs --check`,
`node tools/mermaid-native/blitter-policy-mutations.cjs`, and the registered
Cargo integration suite using `--locked --offline --no-default-features
--features direct2d --test '*' -- --nocapture --test-threads=1`.

Ignored evidence SHA256:

| Evidence | SHA256 |
|---|---|
| build/mermaid-blitter-policy-baseline.log | e92d2ca9bca4834726e98205829e73ef9aea50f481844eaeff1ef7bc81eb12db |
| build/mermaid-blitter-policy-mutations/results.json | 3ad3f52bc5da1f765e11d1325d818ae7078f038fbdfe346dc623aed8d40ebd24 |
| build/mermaid-blitter-policy-native-suite.log | 3820f352e0a41aae8e66e04927a34386c47998185adbb49e174c0d5f5edf48f2 |

The next boundary remains three independent original blitter pipelines borrowing
the same target/mask resources, logical dimensions and stride, sparse alpha
runs, original constructors and callback observations. Root must freeze those
interfaces and original records before assigning implementation.
