# Typed MML pinned Source witness coverage

This manifest describes eleven independent Source witnesses mirrored to their
formal target names under `tools/math-port/`. Each script preserves its own
cases, assertions, version checks, and source locks. They are not a shared
test framework and do not establish C++ behavior, native ownership, native
exception safety, or full MathJax parity.

Run a script from any working directory; it discovers the repository by
walking upward from its own location and checking for
`docs/math-port/typed-mml-implementation.md` plus the pinned Source package. Each
also accepts `--repo-root <repository>` for explicit invocation. JSON evidence
is written under the corresponding `build/math-port/typed-mml-*/source-witness.json`.
When capturing console output, place logs under `build/math-port/` as well.
The scripts write no generated evidence into the tool or repository root.

| Formal script | Source and exercised scope | Specific limits |
| --- | --- | --- |
| `tools/math-port/mml-native-string-source-witness.cjs` | Pins the 4.1.3 string utility CJS hash and version; 19 comparisons cover ordering, pattern quoting, UTF-16 code points, whitespace/percent/split, Unicode escape conversion and error messages, and entity conversion. | Prints JSON to stdout for capture under build/math-port; does not establish every numeric input or native exception/lifetime behavior. |
| `tools/math-port/mml-native-base-source-witness.cjs` | Pinned `@mathjax/src` CJS constructors and methods from `MmlNode`, Tree `Factory`/`Node` and `MML`; 19 strict result comparisons for attribute inheritance/exclusion and origin, script-level propagation, TeX classes and spacing, verification/error branches, inferred rows, `copy(keepIds)`, and finite/infinite arity construction. Checks package version and exact hashes for each loaded source file. | Covers the enumerated base-node cases only. It does not exercise every MML family method, native factory/clone implementation, rendering, or parser integration. |
| `tools/math-port/mml-native-branches-source-witness.cjs` | Pinned CJS MML base, factory/registry, token/layout/base/empty and HTML classes; deep-compares representative token walking/text/defaults, layout and base delegation, TextNode UTF-16 text/copy, XML adaptor cloning, HTML wrapping/text/debug output and parent verification. Checks package version and exact CJS file hashes. | Representative branch behavior only; does not exhaust empty-node errors, every child arrangement, native XML lifetime, or all HTML adaptor failures. |
| `tools/math-port/mml-native-row-tokens-source-witness.cjs` | Pinned CJS MML base plus `mrow`, `mi`, `mn`, `mtext`, `ms`, `mspace`, and `mglyph`. Checks row core/embellishment/space scans, inferred-row string form, token whitespace/newline/break predicates, `mi` operator/single-character rules, inherited attributes and auto-operator cases, and glyph diagnostics. Validates source files against the shared profile source lock and writes the case result JSON. | Focuses on listed row/token inputs and predicates; it does not cover all Unicode classifications, all token normalization paths, native family dispatch, or full TeX tokenization. |
| `tools/math-port/mml-native-arithmetic-source-witness.cjs` | Fixed CJS source hashes and direct `MmlNode`-prototype/family calls for `mfrac`, `msqrt`, `mroot`, `mstyle`, `merror`, `mpadded`, `mphantom`, and `menclose`. Captures defaults, metadata and typed-call traces for arithmetic child/inheritance/class dispatch; checks selected style-level numeric edge cases. Checks package version and exact hashes for selected files. | Prototype fixtures do not run full source tree construction or layout; no claim about classes outside these eight or native output. |
| `tools/math-port/mml-native-scripts-source-witness.cjs` | Calls installed `@mathjax/src` CJS prototypes for script, under/over and multiscript nodes; 11 asserted scenarios cover slot/index mapping, child-inherited attributes and accent/scriptlevel behavior, marker/trailing insertion, malformed verification/repair diagnostics, and `none` parent validation. Checks 4.1.3 plus fixed hashes for three implementation files and `MmlNode`. | Focused prototype probes and child spies; not full Source tree construction or every malformed script arrangement. |
| `tools/math-port/mml-native-table-source-witness.cjs` | Executes eight Source behavior cases against pinned CJS `MmlFactory`, `mtable`, `mtr`, `mtd`, `maligngroup`, and `malignmark`: alignment/inheritance origins, indent transfer, labeled-row column behavior, malformed-tree repair modes, alignment markers, defaults and class hierarchy. Checks 4.1.3 and fixed hashes for each loaded implementation file. | The eight selected inputs do not exhaust table row/column attributes, malformed trees, or native equivalence. |
| `tools/math-port/mml-native-containers-source-witness.cjs` | Pinned `@mathjax/src` ESM factory and family classes; 45 assertions cover Math linebreak behavior, MathChoice branch replacement, Maction selection/toggle (including numeric-string edge cases), annotations, TeXAtom, and mfenced generated fences/separators/attributes/TeX classes. Checks package version and fixed hashes for all directly imported ESM modules. | Covers listed scenarios only; not invalid fence attributes, all action ordering, native ownership, or every class method. |
| `tools/math-port/mml-native-mo-source-witness.cjs` | Uses the separate locked `mo-pattern-reference.cjs` source/runtime check, then executes focused pinned `MmlMo` cases for form ordering, dictionary/range fallback, spacing/class updates, prime remapping and pseudo-script handling. Records 23 witness assertions. | The finite cases and shared source lock do not cover every operator dictionary input or all MML tree contexts. This is not a full operator renderer or TeX integration test. |
| `tools/math-port/mml-native-consumers-source-witness.cjs` | Instantiates all 43 entries from pinned `MmlFactory.defaultNodes`; records constructor name and kind, then invokes Source `copy()` and checks a distinct result with matching constructor/kind. XML and HTML use a tiny test adaptor that returns a fresh payload from `clone`. Checks package version and hashes for the registry, node, factory, HtmlNode, and TeX NodeFactory TypeScript. | Confirms constructor/copy classification only. It does not test family methods, copied attributes/tree data for all 43 kinds, native RTTI/factory registration, ownership, or rendering. The XML adaptor is a witness fixture, not a production DOM implementation. |
| `tools/math-port/mml-native-copy-source-witness.cjs` | Pins CJS `MmlNode`, `MmlFactory`, `mi`, attributes and Tree `NodeFactory`; exercises Source `copy()` on an `mi` with text, TeX classes, attributes and property data, then mutates the copy to verify separation from the source. | A focused source-copy case; it does not establish exact-clone semantics or cover every family. |

The upstream pin is MathJax `@mathjax/src` 4.1.3 at commit
`fb987178c2d279a99b0db5ea02e751691703955`, except where a script's locked
profile explicitly supplies its source/runtime digest. Each script retains
its own hash/version checks; this document does not replace those locks.

Successful Source execution is evidence about those specific pinned source
cases. Native family acceptance is in the separate Boost.UT cohort and remains
subject to the root-owned integrated build. Neither set alone establishes
end-to-end TeX-to-SVG behavior.
