# FontData parity and native render diagnosis, Gates73–75

Gate73 exits 1 on the remaining Braket and Color compilation errors, with all
1,525 captured input hashes unchanged. Gate74 builds all 22 targets, including
the XML/HTML wrapper closure, with all 1,531 captured hashes unchanged. Its
document and Foundation lifetime probes throw because Braket initialization
reinitializes the shared StackItemFactory module. The repair reuses the actual
cached dependency owner, matching Source module initialization.

Gate75 builds all 22 targets with the same 1,531 inputs unchanged. This confirms
compilation and linking, not full package or renderer acceptance.

## Actual document and Foundation execution

| Gate75 executable | SHA256 | PID | Actual exit |
| --- | --- | ---: | ---: |
| Foundation | `c4bffad973b39ba2eea0970fdcb30404d05975da4a9d3cb60ad8666cd612141f` | 44540 | 0 |
| Document probe | `cf9f1055f50a189b621595709030c754add0a34d2e5e317d17039e4492a4fdc2` | 8324 | 0 |
| Whole75 driver | `483cda8e15bc3235873761561e77486fca392dce5bf0a9cd750bdb93e094edc5` | 76232 | 3221225477 |
| Six-package observer | `1f247c0b1378cceb905931ea149430aa4909d3f004b4a9e83239ed729758b1c4` | 40788 | 2 |

These are actual launches on 2026-10-13 UTC. Foundation runs
17:44:39.190–17:44:41.052 and passes 303 assertions in 50 tests. The document
probe runs 17:44:41.544–17:44:42.954 and completes the actual
NativeRenderSession → HTMLMathDocument → TeX → CommonWrapper → Scene → SVG path.
The Scene remains valid after document, Context, service graph and host release.
Its `x+y` SVG still differs from Source: 2,422 versus 2,502 characters, with
geometry, attributes and ordered paint observations unequal.

The whole75 driver runs 17:44:41.584–17:44:50.444. It requests the unchanged
full38 package list, independently observes actual registrations and warnings,
and completes cases1–27. Case28, `braket-delimiters`, begins conversion and
terminates with `0xC0000005`. This executable buffers its JSON until all cases
finish, so its stdout contains no complete75 result. A subsequent diagnostic
launch of the same executable reproduces a null read. Partial progress and
successful `mathbb` conversion are not exact Source acceptance. Gate70 remains
the latest complete whole75 result, with 0/75 full matches.

The six-package observer runs 17:46:48.208–17:46:48.920. Its outer error label
is `fixture-init`, but the failing operation is token-map FunctionRef reflection
during record serialization: `Unbound FunctionRef in token map`. No whole
six-package native record set is accepted. The next packet registers the actual
package-owned callable identities and adds focused parser-only diagnosis.

## FontData strict parity

The Gate75 FontData dump is a fresh native execution, SHA256
`db1fb8738cdf7c044f9c534648f68f0ef2b3d938f8f39d4a895a7e438ce49400`, PID53472,
17:46:01.295–17:46:09.573 UTC, actual exit0. Its 22,611,961-byte output is
`build/math-port/font-native-75.json`. Root independently runs
`compare-font-data.cjs` and gets exit0 for all four static sources, extension
merges and every synchronous dynamic glyph, option and delimiter.

The earlier empty-array mismatch was in the native observer. It read
`PropertyStorage.arrayValues/arrayPresent` instead of the canonical bound
`PropertyArray.size/has/get` accessors. The repair observes the actual live
owner; it does not modify Source algorithms or substitute expected arrays.

Font tests pass 176 assertions in 21 tests. The actual lifecycle9, methods,
tuple and bundled-module provider candidates pass their strict comparers. Root
also independently reruns `compare-newcm-font-module-provider.cjs` against
`build/math-port/fontmodule-native-75.json` and gets exit0. This bounded parity
does not establish arbitrary raw-field replacement semantics or production
profile assembly of the BBM, BBoldx and Dsfont extensions.

## Remaining architectural work

One per-Context Map service must own font `dynamicExtensions`, preserve live
replacement and old aliases, and serve the output Jax and Color package without
parallel storage. SVG menclose must use actual Source instance method dispatch
and ordered notation callbacks through the same typed Scene sink; direct helper
geometry alone does not accept that Source method surface. XML/HTML and typed
paint ownership compile and their freeze tests pass, but whole output parity
still requires real profile conversion.

All38 handlers, all75 complete observations and Folia integration remain
required. Folia's math QuickJS worker is still active. Mermaid remains paused.
The latest full core execution is still Gate71: 345,635 assertions, 5,322 tests
and 171 suites pass; no Gate75 full-core rerun is claimed.
