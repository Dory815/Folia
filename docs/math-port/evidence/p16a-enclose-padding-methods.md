# P16a `menclose` and `mpadded` method evidence

The pinned MathJax 4.1.3 sources are `.reference/MathJax-src/ts/core/MmlTree/MmlNodes/menclose.ts` (SHA-256 `ffa4ab507744cb4f190312ca8dcd692474c333bee6e19c86709554aa9dad4333`) and `mpadded.ts` (SHA-256 `b5aac516c32fb019ac5ace504f86312f3e0849e1faa44006567bac05815c7271`). Their production-resolved CommonJS files are `tools/mathjax-bundle/node_modules/@mathjax/src/cjs/core/MmlTree/MmlNodes/menclose.js` (SHA-256 `2cea1d1819a2eb545727f37940b341ac0ce802fad4d7daa11f813ce7381f23de`) and `mpadded.js` (SHA-256 `675aae98959b257582171d58584f220da4543deb8e0e3d3b8b07a48ab6fc23a2`). Native implementations are [menclose.cpp](../../../src/math/core/mml_tree/mml_nodes/menclose.cpp) and [mpadded.cpp](../../../src/math/core/mml_tree/mml_nodes/mpadded.cpp). Both preserve MathJax's Apache-2.0 attribution; see `src/math/LICENSE-MathJax.txt`.

## Method mappings

| Source method | Source lines | Native behavior |
| --- | ---: | --- |
| `MmlMenclose.setTeXclass(prev)` | `menclose.ts:76–80` | Enters one `TreeCallGuard`; requires current child zero with `arithmeticChild`; calls its virtual `setTeXclass(prev)` and saves the returned pointer. After the child callback it re-reads the current `childNodes` property and slot zero, passes that nullable pointer to virtual `updateTeXclass`, then returns the saved first result. It does not return the update hook's result. |
| `MmlMpadded.setTeXclass(prev)` | `mpadded.ts:62–70` | Enters one `TreeCallGuard`; calls virtual `getProperty("vbox")` once and tests raw JS truthiness. A falsey result invokes qualified `AbstractMmlLayoutNode::setTeXclass(prev)` and returns it. A truthy result calls virtual `getPrevClass(prev)`, virtual `setTexClass(TEXCLASS.ORD)`, then requires the current child zero and calls its virtual `setTeXclass(nullptr)`, ignoring that result and returning `this`. |

All other source members—fresh defaults, class-field initialization, `kind`, `arity`, and `linebreakContainer`—are root-generated and are not reimplemented here.

## Evaluation order and adaptations

For `menclose`, the first child lookup is required: an absent child array, empty array, or null child fails before `updateTeXclass`. The result from that first child call is retained even if the callback replaces the parent's array. The second lookup captures the then-current child-array handle; null array access fails, while an empty vector or null first slot is represented by a null pointer and passed through to `updateTeXclass`. This nullable second read is intentional and differs from the first `arithmeticChild` access.

For `mpadded`, `getProperty("vbox")` precedes every other action. Raw falsey values—including missing, null, false, numeric zero, and the empty string—take the qualified layout-super path; arrays and objects remain truthy under the existing host conversion. The truthy path preserves the ordered virtual calls and their partial effects: if `getPrevClass` or `setTexClass` throws, child classification does not run; if child access fails, earlier previous-class and class writes remain. Source assignment to `this.texClass` is represented by the contract's virtual `setTexClass(TEXCLASS.ORD)` call. The return from the child is discarded.

These are semantic TeX-class methods only. No padding/enclosure geometry, TeX command handling, validation fallback, or output rendering is added. The actual-CJS fixture was frozen independently by root at 7,627 assertions across 98 cases, including 1,605 raw rows. I did not change interfaces, defaults, bindings, fixtures, tests, CMake, or scope, and did not run builds or tests. Root owns independent review, mutation/build checks, and acceptance.
