# P9 row and math node evidence

Status: implementation submitted for root review. This packet did not build or
execute the root-owned `reference_leaf.cpp` fixture (currently reported as 610
assertions across 13 cases). No native test pass is claimed here.

## Pinned sources

| Source | SHA-256 |
| --- | --- |
| `ts/core/MmlTree/MmlNodes/mrow.ts` | `af58f0c1aa39a71d6c3dc5addf358a8cefeecb3f76cb483ec940f3a7ea72b4b9` |
| `ts/core/MmlTree/MmlNodes/math.ts` | `ca0769f26d00145b2c520ac7bcfb1ac1f3e3eb34c387d0eeb8e4444e7d382ea0` |
| resolved CJS `MmlNodes/mrow.js` | `5846d2b0334223b759a4059dfb0ac5e31aef1f5a96c041da45b3de94c0689896` |
| resolved CJS `MmlNodes/math.js` | `44debed089b254bc09e73e22d4c30336db13ff8dad0af140e915a320bdda8e71` |

Both TypeScript hashes were independently read from the pinned source tree.
The CJS hashes are from the pinned `tools/mathjax-bundle/node_modules/@mathjax/src/cjs`
files. MathJax 4.1.3 source is Apache-2.0; each implementation file carries
the provenance header and points to `src/math/LICENSE-MathJax.txt`.

## Member-to-body ledger

Root owns `defaults()` implementations and class registration/binding. This
packet implements all executable members in the two assigned source classes
and the native post-super field initializer required by the frozen class
binding contract.

| Source member | TS lines | Mapping |
| --- | ---: | --- |
| `MmlMrow.defaults` | 36–39 | Root `mrowDefaults()` live reference binding. |
| `_core` field initialization | 43 | `initializeClassFields()` clears the optional only after `initializeMml`, matching the derived field-initializer phase. |
| `kind` | 48–51 | Returns `mrow`. |
| `isSpacelike` | 57–69 | Captured child-array handle, live-length for-of; null children throw at `isSpacelike`. |
| `isEmbellished` | 72–92 | Captured/live iteration; null children are skipped; unique embellished child records its current index; a later non-spacelike or second embellished child returns false without clearing a previously recorded core. |
| `core` | 95–102 | Calls virtual `isEmbellished` first, then re-reads the current child array and uses the persistent `_core` index. |
| `coreMO` | 105–112 | Same predicate/index order, then calls the current selected child’s virtual `coreMO`. |
| `nonSpaceLength` | 115–125 | Captured/live iteration, skipping null and counting non-spacelike children. |
| `firstNonSpace` | 128–137 | Captured/live iteration, returning first non-null non-spacelike child. |
| `lastNonSpace` | 140–151 | Captures initial length but re-reads `this.childNodes` on every descending index, as the source indexed loop does. |
| `setTeXclass` | 154–188 | Uses nullish `open`/`close` checks. Delimiter mode updates previous state, then traverses with null incoming previous and assigns INNER only if virtual `texClass()` remains null. Normal mode traverses, then re-reads the current array’s first child for `updateTeXclass`. |
| `MmlInferredMrow.defaults` | 195–196 | Root binding aliases `MmlMrow::defaults()`. |
| `kind`, `isInferred`, `notParent` | 200–216 | Returns `inferredMrow`, true, and true. |
| `toString` | 223–226 | Captures child-array handle and length, joins null/out-of-range entries as empty fields, uses `TreeJoinGuard` for Array.join recursion suppression and `TreeCallGuard` for bounded recursion; it does not call `kind()`. |
| `MmlMath.defaults` | 36–67 | Root `mathDefaults()` live reference binding. |
| `kind`, `linebreakContainer`, `linebreakAlign` | 71–92 | Returns `math`, true, and empty string. |
| `setChildInheritedAttributes` | 100–124 | Preserves mode read and strict `"display"` test; adds explicit attributes to inherited records; evaluates displaystyle truthiness then strict display `"block"`, with a fresh virtual attributes read for each source property access; stores computed displaystyle; uses truthy scriptlevel or captured constructor class defaults; forwards incoming prime unchanged. Uses the shared tree call guard for its recursive frame. |
| `verifyTree` | 126–133 | Uses the shared tree call guard, calls base verification first, then for nested math nodes passes the same non-null options record or an explicit null `ErrorOptions` into `mError`, preserving the source error path and ordering. |

## Host representations and ordering

The frozen `mrow.ixx` and `math.ixx` declarations own the public shapes.
`classBinding<T>` adopts the complete object, binds the actual constructor class
metadata, invokes the MML constructor phase, and only then calls
`initializeClassFields()`. Consequently math's scriptlevel fallback calls
`constructorDefaults()` and observes the constructor class captured at
construction, not a later factory registration lookup. Defaults themselves
remain root-owned bindings.

`childNodes` is the accepted shared vector handle. The implementation captures
that handle for source for-of loops but reads its current size on each step.
`lastNonSpace` and post-traversal core lookups intentionally read the current
`childNodes` property again. The source gives null children different
semantics: `isSpacelike` fails, while `isEmbellished` and nonspace scans skip
them. Inferred stringification uses the shared native join guard, preserving
recursive array behavior and the captured `Array.join` length.

`Property` retains null versus Missing, and the frozen `leaf_host` operations
provide nullish/truthiness tests and source primitive rules. Row delimiters use
nullish checks rather than truthiness. Math `mode`/`display` use strict string
equality, while `displaystyle` and `scriptlevel` use JS truthiness. When
`MmlMath::verifyTree` receives null and the node has a parent, it first performs
base verification (which is the source no-op), then delegates with
`ErrorOptions(nullptr)`; the base `mError` path allocates the error and writes
its message before its first null-options read throws.

## Review boundary

Root still needs to source-review every body, run the frozen differential
fixture/build and mutations, and accept this packet. This implementation does
not supply the other concrete node classes, the complete factory registration
table, parsing, operator dictionary behavior, layout, or rendering.
