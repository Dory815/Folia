# Font and common-style checkpoint, gates 20–21

This is bounded executed evidence, not full renderer acceptance. The original
is MathJax 4.1.3 at `fb987178c2d279a99b0db5ea02e751691703955e`.
Root independently ran the native observers and strict comparisons below.

## Build provenance

Gate 19 links the production library and 16 of 17 diagnostic executables;
FontTests fails compilation. Gate 20 links 17 of 18 executables, including the
new Styles observer, but FontTests still fails. Gate 21 exits zero and links all
18. All three builds have unchanged snapshots of their registered/generated
inputs. Do not attribute the later FontTests run to gate 20.

Gate 22 additionally builds and runs the ordinary core tests with
`cmd /c build_test.bat 2>&1`. Compilation succeeds; execution exits one:
345,511 assertions in 5,290 tests across 168 suites, with 17 assertion failures
in two non-Unicode ignore-case tests. Math's legacy case policy and search's
Unicode policy shared SRELL inline symbol names, violating the C++ ODR.
Root isolates their namespaces and adds a joint search/math regression.
Gate 23 links the corrected ordinary runner; its full execution exits zero:
345,517 assertions in 5,291 tests across 168 suites. This is the actual gate-23
worktree snapshot, not acceptance of later uncompiled edits or the renderer.
The regex policy packet and joint regression are committed as `937f91b`.
The focused ParserTests target alone fails compilation on the new substituteArgs
overload's literal ambiguity; its source correction still needs a fresh link.

Two diagnostic candidate files changed during gate 22's combined snapshot:
BaseItems and ParseUtil test wrapper expectations. They were not ordinary
FoliaTests inputs and their new assertions were not executed by that gate.
New unregistered OutputJax files are excluded from these input snapshots and
do not inherit their compilation evidence.

## Font observations

Root's actual gate-20 `FoliaMathFontDump` output passes
`compare-font-data.cjs`: four complete static font sources, every synchronous
dynamic glyph/options record and delimiter, instance resolver samples, and
the class-extension merge observations. The comparator reads full native
tables and checks independent original hashes/counts and full class state.

Canonical sparse PropertyArray storage preserves tuple length, holes versus
undefined, raw null/undefined option slots and live shared record identity.
The real tuple-presence observer's full `results` deep comparison passes.
Nine sync/async dynamic lifecycle traces pass separately; the supplemental
FontMethods observations also have zero differences.

The class-merge observer must construct a real NewCM instance before class
snapshots, matching the original observer's order. Source `defineCssFonts`
mutates aliased default tuple records. Earlier snapshots taken before that
construction caused seven CSS differences; copying expected CSS data into
the candidate would not fix the ordering/identity obligation.

Fresh gate-21 `FoliaMathFontTests` passes 97 assertions in 11 tests. Root's
independent comparator-sensitivity check detects all eight deliberately
corrupted output candidates: width, SVG path, removed glyph, dynamic hole,
SMP remap, dynamic delimiter, class CSS font tuple and class variant tuple.
These are output-candidate faults, not eight independently compiled production
mutations. They establish comparator coverage only.

The evidence does not accept arbitrary dynamic JS constructor/species behavior,
unusual unbound host values, every FontData member or future wrapper consumers.
The exact Source method/profile ledger remains necessary for full subsystem
acceptance.

## Styles and StyleJson observations

Fresh gate-21 `FoliaMathStylesTests` passes 19 assertions in six tests. Root's
`FoliaMathStylesDump` and strict comparator pass all 37 frozen original rows,
including intermediate mutations, ordered keys, constructor-specific patterns,
base connection lookup, nested StyleJson storage identity and lifecycle.

Gate 19 exposed a real infinite loop in the capturing String.split adaptation.
The corrected caller follows sticky separator search and advances its own
UTF-16 cursor on empty matches. Shared RegExp.exec empty-match behavior is
unchanged. Fresh gate-20 and gate-21 executions terminate normally.

Two gate-20 observer metadata defects were corrected: operation JSON metadata
serializes negative zero as ordinary JSON zero, and displayed RegExp source
escapes slash characters as the original does. The actual primitive-value
style trace already matched. A focused stylesheet test also guessed nested
indentation incorrectly; the exact pinned CJS output established its correction.
No production formatting algorithm was changed to satisfy that guessed text.

These modules expose the frozen typed behavior. Complete dynamic Context
constructor/prototype bindings and general browser CSS parsing are not accepted.

## Remaining pipeline failures

Real TextMacros factory ownership, shared TeX postfilter execution, package map
registration, ConfigMacros argument expansion and Begingroup scope behavior
still need repair. Some BaseItems/ParseUtil UTs guessed flattened root shapes:
Root's same-profile original calls show a TeXAtom for a braced fraction, mstyle
for arrays and one mrow from internalMath. Those guesses must be corrected from
original evidence; required Source wrappers must not be removed to pass them.

CommonOutputJax, DOM/LiteAdaptor, wrappers, completed scene, SVG generation and
Folia integration remain outstanding. Folia still uses its existing math
QuickJS path. Mermaid remains paused.
