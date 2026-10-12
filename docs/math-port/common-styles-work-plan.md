# Common output style services

This is an implementation contract for two dependencies of CommonWrapper and
CommonOutputJax. It does not accept the common layout or SVG output. Root owns
this contract, original observations, source disposition, CMake and independent
acceptance. The assigned Luna owns the complete implementations, native
observations, focused tests and source/member ledger.

| Pinned MathJax 4.1.3 source | SHA-256 | Native owner |
| --- | --- | --- |
| ts/util/Styles.ts | 74d365d8c12c7cfbe20595ebf8d85d230e8897be1d5cbfd8dc1a6b772d40f3c4 | src/math/util/styles.ixx and styles.cpp; folia.math.util.styles |
| ts/util/StyleJson.ts | 8c1fd2cb2222aeaae70ac11ed35267abdcf126b44dbb3c0d5ac9cd2fadaf48b6 | src/math/util/style_json.ixx and style_json.cpp; folia.math.util.style_json |

These modules depend only on existing math utilities and record services. They
must not import parser, common wrapper, scene, SVG or platform modules. They
parse and combine the Source's deliberately limited CSS grammar; they do not
measure fonts, assign mathematical styles, place geometry or provide a browser
CSS engine. Source helper functions and connection rules stay with Styles;
stylesheet merging and nested rule emission stay with StyleJsonSheet.

The public classes retain the Source member responsibilities and spellings:
Styles constructor, cssText, styleList, set/get, combineParent, and its protected
parsing/name/sanitizing/combining helpers; StyleJsonSheet constructor, cssText,
addStyles, removeStyles, clear, getStyleString, getStyleRules and
getStyleDefString. Static patterns and connections retain their Source data and
callable relationships. The owner supplies exact declarations before CMake
registration; Root reviews ownership and argument/result semantics, rather than
writing the implementation algorithm.

Use UTF-16 for JS string operations and existing explicit conversions at native
Property boundaries. StyleList and stylesheet records use the established
PropertyList graph, or a live Context-backed view of that graph, as their sole
state. Preserve own-key order, missing/empty values, delete/reinsert behavior,
and Source property lookup semantics. styleList creates a distinct shallow
copy. StyleJsonSheet's addStyles shallow-copies each selector's properties into
its actual target record while retaining nested object aliases. It must not
deep-copy nested rules, keep a shadow Context map, or silently snapshot dynamic
inputs through an unbound bridge.

Styles.set uses the Source string/number/boolean input contract and actual
Number formatting. Do not treat semicolon sanitizing, unclosed quotes, regex
captures, comments, invalid parse prefixes, border propagation, shorthand
removal or font splitting as CSS-normalization conveniences. Their current
Source behavior is the obligation. Static constructor patterns are used at
the same lookup points as Source; replacing every lookup with a hard-coded
pattern would change subclass behavior.

Root's common-styles-original-oracle.cjs calls the real installed 4.1.3 CJS
classes and records hashes for both CJS and the pinned TS source. Its initial
37 records cover CSS parsing, TRBL/WSC/font connections, repeated changes,
malformed text, Unicode, protected helpers, static tables, copy/alias behavior
and nested stylesheet emission, including subclass pattern and base-class
connection lookup. They are a starting corpus, not a full-file
acceptance claim. The native observer must call the real classes and produce
the same schema, including intermediate states and key order. Expected-result
strings, skipped rows and relaxed comparisons are forbidden. Additional
source cases may be requested before the frozen corpus is extended by Root.

Acceptance requires a source/member ledger, exact original/native records,
meaningful independent faults for branch-sensitive sanitizing and connection
behavior, focused Boost.UT tests using the repository module runner, a clean
native build, and the complete core tests when integrating the accepted
packet. Raw-TeX profile adaptations and unsupported dynamic class/record
boundaries must be reported explicitly. Green style tests alone do not prove
CommonWrapper measurement, scene construction, SVG or Folia integration.
