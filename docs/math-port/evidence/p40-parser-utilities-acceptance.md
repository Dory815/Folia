# P40 parser utilities acceptance

Root accepts the 21 newly implemented NodeUtil methods and both TexError
methods within the typed profile in p40-parser-utilities-contract.md. The three
previous child-array methods remain unchanged. This supplies parser utilities,
not a TeX parser or a working renderer.

Root froze the interfaces, method ownership, host adapters and actual-original
fixtures in f0c4057 before releasing semantic bodies to Luna. Compiled stub
tests failed through Unimplemented (ignored build/p40-red.log and
build/p40-node-red.log). Root reviewed the methods against pinned TS/CJS and
independently checked the restored implementation after fault injection.

Source observations execute pinned MathJax 4.1.3. NodeUtil uses the existing
locked compiled-package loader; TexError verifies both its TS and CJS hashes.
All three oracle --check commands reproduce the saved expectations:

| Original observations | Cases | Verification |
| --- | ---: | --- |
| node-util-oracle.cjs | 153 | all utility methods, actual MML kinds, routing, live child identity, UTF-16, null errors, operator forms and explicit attributes |
| node-util-order-oracle.cjs | 18 | shared Attributes, entry snapshot, callback order, partial failures, global-table capture, false/Null operator attributes and inherited attributes |
| tex-error-oracle.cjs | 1,981 | complete private pattern grammar, malformed tokens, prioritized backtracking, line terminators, surrogate units, numbered/braced/plural handling and deferred Property joining |

The TexError test executes 5,943 assertions. The utility fixtures execute 171
new comparisons; the previously accepted child-array and receiver tests also
pass. Do not interpret case counts as formula or package coverage.

Root's first pattern implementation incorrectly selected the longest plural
capture. Actual original case 1758 shows that ordered alternatives can prefer
an earlier closing brace. Root changed the fixed-pattern host to a suffix
dynamic program preserving source alternative priority and backtracking; all
1,981 original split and constructor observations now match. This is a host
adaptation for the two private patterns, not a general RegExp engine.

Root corrected NodeUtil compilation and two observed error boundaries during
review. In compiled __values, null children fail during o[Symbol.iterator]
before the helper's explicit not-iterable throw. Invalid operator forms require
the exact nullish property-key error, after computing mo.getText. The scoped
propertyAt adapter preserves that order without changing the shared Property
host. Native fixture text setup was corrected to create a TextNode and call
setText, matching the original; no production factory fallback was introduced.

parser-utilities-mutation-test.cjs proves eight independent compiling faults
are detected: lost class assignment, wrong movablelimits kind, live values
instead of entry snapshot, re-read global form table after text callback,
false/Null attribute fallback, first-form-only search, first-digit braced
comparison, and incorrect null join. Every fault compiles and exits through
failed assertions. The runner restores both source units and verifies the
baseline again; hashes and outcomes are saved in the mutations JSON.

The complete normal `cmd /c build_test.bat` builds and passes, exit 0, in ignored
build/p40-full-core.log. The folia_math target now contains 92 module interfaces.
No platform code or editor interaction changed in this packet.

Limits remain explicit: canonical NodeValue/MmlNode* parser arguments, integral
package child indices, Property's single Missing tag for undefined, and attrs
membership without Source Set iteration promises. Public Source reflection
over arbitrary unrelated JavaScript objects is outside this typed service.
NodeFactory, ParseOptions, configuration/stack/TeXParser and enabled packages
remain unimplemented, followed by fonts/common layout/shared scene/SVG and
Folia integration. Mermaid remains paused.
