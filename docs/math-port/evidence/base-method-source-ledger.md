# BaseMethods source-to-native responsibility ledger

This ledger maps every exported source method to its native callback registration. It records implementation locations, not a claim that every method branch is differentially accepted. Source/CJS pins:

- TypeScript: `.reference/MathJax-src/ts/input/tex/base/BaseMethods.ts` — `7336240c8f9c5037f9fdcd5a84ea079312854f1ef3f406aeb5a8b6a8ff765cda`
- Installed CJS: `tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/base/BaseMethods.js` — `b8d5c74e01f85ae391ec8d16ffcd216a4ca4267bd8ffa091c5bfe1462ad7b491`
- Native callback table: `src/math/input/tex/base/base_methods.cpp`; callbacks are registered in `makeMethods()` and adapted through `BaseMethods::initializeModule`.

| Source method | TypeScript line | Native callback |
|---|---:|---|
| `Open` | 132 | `base_methods.cpp:174` |
| `Close` | 143 | `base_methods.cpp:175` |
| `Bar` | 154 | `base_methods.cpp:176` |
| `Tilde` | 171 | `base_methods.cpp:182` |
| `Space` | 182 | `base_methods.cpp:183` |
| `Superscript` | 190 | `shared script helper at base_methods.cpp:184 (registered at 223)` |
| `Subscript` | 267 | `shared script helper at base_methods.cpp:184 (registered at 223)` |
| `Prime` | 342 | `base_methods.cpp:254` |
| `Comment` | 383 | `base_methods.cpp:246` |
| `Hash` | 398 | `base_methods.cpp:251` |
| `MathFont` | 414 | `base_methods.cpp:292` |
| `SetFont` | 442 | `base_methods.cpp:276` |
| `SetStyle` | 456 | `base_methods.cpp:280` |
| `SetSize` | 479 | `base_methods.cpp:286` |
| `Spacer` | 495 | `base_methods.cpp:310` |
| `DiscretionaryTimes` | 508 | `base_methods.cpp:318` |
| `AllowBreak` | 520 | `base_methods.cpp:321` |
| `Break` | 530 | `base_methods.cpp:322` |
| `Linebreak` | 545 | `base_methods.cpp:323` |
| `LeftRight` | 564 | `base_methods.cpp:582` |
| `NamedFn` | 583 | `base_methods.cpp:587` |
| `NamedOp` | 599 | `base_methods.cpp:592` |
| `Limits` | 626 | `base_methods.cpp:225` |
| `Over` | 677 | `base_methods.cpp:707` |
| `Frac` | 707 | `base_methods.cpp:605` |
| `Sqrt` | 721 | `base_methods.cpp:610` |
| `Root` | 745 | `base_methods.cpp:637` |
| `MoveRoot` | 759 | `base_methods.cpp:533` |
| `Accent` | 801 | `base_methods.cpp:671` |
| `UnderOver` | 839 | `base_methods.cpp:408` |
| `Overset` | 861 | `base_methods.cpp:652` |
| `Underset` | 879 | `base_methods.cpp:658` |
| `Overunderset` | 897 | `base_methods.cpp:664` |
| `TeXAtom` | 922 | `base_methods.cpp:686` |
| `VBox` | 958 | `base_methods.cpp:1018` |
| `Hsize` | 983 | `base_methods.cpp:1014` |
| `ParBox` | 997 | `base_methods.cpp:795` |
| `BreakAlign` | 1017 | `base_methods.cpp:804` |
| `MmlToken` | 1073 | `base_methods.cpp:417` |
| `Strut` | 1142 | `base_methods.cpp:362` |
| `Phantom` | 1161 | `base_methods.cpp:366` |
| `Smash` | 1187 | `base_methods.cpp:372` |
| `Lap` | 1213 | `base_methods.cpp:379` |
| `RaiseLower` | 1232 | `base_methods.cpp:478` |
| `MoveLeftRight` | 1262 | `base_methods.cpp:495` |
| `Hskip` | 1288 | `base_methods.cpp:326` |
| `Nonscript` | 1305 | `base_methods.cpp:330` |
| `Rule` | 1316 | `base_methods.cpp:331` |
| `rule` | 1316 | `base_methods.cpp:331` |
| `MakeBig` | 1365 | `base_methods.cpp:998` |
| `BuildRel` | 1392 | `base_methods.cpp:1008` |
| `HBox` | 1415 | `base_methods.cpp:349` |
| `FBox` | 1428 | `base_methods.cpp:356` |
| `FrameBox` | 1443 | `base_methods.cpp:507` |
| `MakeBox` | 1470 | `base_methods.cpp:520` |
| `Not` | 1497 | `base_methods.cpp:722` |
| `Dots` | 1509 | `base_methods.cpp:723` |
| `Matrix` | 1547 | `base_methods.cpp:852` |
| `Entry` | 1618 | `base_methods.cpp:738` |
| `Cr` | 1727 | `base_methods.cpp:773` |
| `CrLaTeX` | 1744 | `base_methods.cpp:777` |
| `HLine` | 1800 | `base_methods.cpp:827` |
| `HFill` | 1831 | `base_methods.cpp:729` |
| `NewColumnType` | 1852 | `base_methods.cpp:979` |
| `BeginEnd` | 1886 | `base_methods.cpp:869` |
| `Array` | 1926 | `base_methods.cpp:906` |
| `AlignedArray` | 1989 | `base_methods.cpp:923` |
| `IndentAlign` | 2015 | `base_methods.cpp:935` |
| `Equation` | 2082 | `base_methods.cpp:956` |
| `EqnArray` | 2110 | `base_methods.cpp:963` |
| `HandleNoTag` | 2163 | `base_methods.cpp:885` |
| `HandleLabel` | 2173 | `base_methods.cpp:888` |
| `HandleRef` | 2210 | `base_methods.cpp:550` |
| `Macro` | 2247 | `base_methods.cpp:384` |
| `MathChoice` | 2281 | `base_methods.cpp:598` |

The module rebuilds the Source object own-key order in `sourceOrder`, then retains the same per-runtime FunctionRefs in the method table and package maps. Function arities are set on those canonical Context function objects. Superscript and Subscript share an implementation helper while retaining distinct function identities.

Coverage boundary: `base-package-original-coverage.md` describes the independent pinned CJS corpus. It covers representative formulas and captures the method names/arities, but is not exhaustive branch coverage. Equation paths requiring a live MathItem and ParseSymbol result conversion remain explicit host dependencies.
