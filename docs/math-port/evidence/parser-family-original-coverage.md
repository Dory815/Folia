# Parser family original-source coverage

The executable observer is `tools/math-port/parser-family-original-oracle.cjs`.
Its generated candidate gold is `parser-family-original-source.json`. It reads
the repository's pinned MathJax source scope, requires the installed CJS modules,
checks the 19 original and installed TypeScript files byte-equivalent after
newline normalization, and asserts all 19 pinned CJS SHA-256 values before it
constructs any observations. No native implementation is loaded by the oracle.

The observers instantiate the actual CJS classes. The FunctionList receiver
case uses a normal (non-arrow) callback so the call receiver is observable.
Tag and column cases use a
real `TeX`, `MmlFactory`, `TexParser`, `ParseOptions`, `NodeFactory`, and
`ArrayItem`; no fake parser or fake MathJax object is used for algorithm
acceptance. The real TeX is configured with the base package to keep this
independent observer deterministic.

## Configuration and ParserConfiguration

| Source responsibility | Original observations |
|---|---|
| `Configuration.makeProcessor`, `_create`, `create`, `local`, constructor | `configuration-defaults-local-versus-create`; `configuration-config-property-get-order`; `configuration-truthy-property-rereads-and-priority-coalescing`; `configuration-falsy-init-config-and-array-processor-alias`; `configuration-array-map-captured-length-holes-and-getter-effects`; `configuration-preprocessor-map-override-get-and-receiver`; `configuration-init-config-falsy-short-circuit-read-count`; `configuration-init-config-truthy-getter-rereads-current-values`; `configuration-runtime-descriptors-direct-constructor-and-mutable-method-metadata` |
| Own handler initialization / `Object.assign` | `configuration-handler-object-assign-enumerability-symbol-and-proto-setter` |
| `init` / `config` getters and raw field mutability | `configuration-config-property-get-order`; `configuration-runtime-descriptors-direct-constructor-and-mutable-method-metadata` |
| Constructor/direct-call behavior, method arities, names, private static entry points and property descriptors | `configuration-callable-metadata-and-direct-call-versus-new`; `configuration-runtime-descriptors-direct-constructor-and-mutable-method-metadata` |
| `ConfigurationHandler.set/get/keys` | `configuration-defaults-local-versus-create`; `configuration-create-registers-only-after-success`; `configuration-handler-live-map-keys-and-overwrite-order` |
| `ParserConfiguration` constructor fields, method descriptors/arities, package reverse-copy, priorities | `parser-configuration-own-fields-and-array-alias`; `parser-addpackage-snapshot-reverse-and-tuple-index-get-order`; `parser-configuration-init-and-config-method-callback-order` |
| `init` / `config` and FunctionList callback receiver/arguments | `parser-configuration-init-and-config-method-callback-order` |
| `addPackage` / `getPackage` / warning / parser-target rejection | `parser-addpackage-snapshot-reverse-and-tuple-index-get-order`; `parser-getpackage-parser-target-error-and-missing-warning` |
| `append`, tuple direct-index `Get`, field rereads and merge order | `parser-append-processor-direct-index-and-live-object-assign-order`; `parser-append-reads-live-configuration-fields-and-method-tuples`; `parser-append-processor-index-get-order-on-inherited-tuple` |
| Runtime replacement of the parser's public `items`/`parsers` properties | `parser-configuration-public-fields-are-live-authoritative-properties` |
| `add`, live TeX/package setup, filters, option updates, config callback receiver | `parser-add-package-real-tex-order-and-configuration-callback-receiver` |
| `addFilters`, pre/postprocessor `.map` receiver and callback result, tuple destructuring iterator/close order | `parser-constructor-config-getters-and-filter-array-map-override`; `parser-addfilters-destructures-through-tuple-iterator-and-closes-it`; `parser-add-package-real-tex-order-and-configuration-callback-receiver` |
| FunctionList `execute` callback receiver | `function-list-execute-binds-priority-entry-as-receiver` verifies `this` is the actual prioritized `{item, priority}` entry, not the parser configuration or undefined |

## ParseOptions

| Source responsibility | Original observations |
|---|---|
| Constructor field order, shared configuration handlers, per-instance defaults, static math-style Map and descriptors | `parse-options-field-order-sharing-and-math-style-variants` |
| `pushParser`, `popParser`, live parser getter and array identity | `parse-options-parser-stack-live-array-and-identity` |
| `clear` rebinds only parser/list state, resets root/error and dispatches `tags.resetTag` | `parse-options-clear-rebinds-ephemeral-fields-and-resets-tags` |
| `addNode` in-lists bookkeeping, `getList` tree reachability and list replacement | `parse-options-addnode-inlists-and-getlist-live-tree-replacement` |
| `removeFromList` first strict match and list identity | `parse-options-removelist-first-strict-match-preserves-array-identity` |

The CJS constructor has runtime arity 13, and `Configuration.prototype` exposes
non-enumerable `init` and `config` getters. The emitted class also exposes
`makeProcessor` and `_create` as enumerable own static functions even though
TypeScript marks them private. Direct `new Configuration('runtime-direct')`
works in the emitted CJS; its own fields are enumerable writable data
properties. The readonly annotations do not prevent runtime mutation. These
runtime observations are recorded rather than inferred from TypeScript types.

## Tags

| Source responsibility | Original observations |
|---|---|
| `Label` / `TagInfo` defaults and explicit undefined/null | `tag-record-constructor-default-and-explicit-values` |
| Constructor arities, prototype own keys/inheritance, instance fields and own function fields | `tags-class-runtime-arity-own-enumerability-and-prototype-inheritance` |
| `start`, `env`, `end`, `tag`, `notag`, `noTag`, `label` | `abstract-tags-start-tag-notag-end-history-label`; `abstract-tags-format-tag-ref-label-and-cleartag` |
| `formatUrl`, `formatTag`, `formatRef`, `formatId`, `formatNumber` | `abstract-tags-format-url-and-encode-uri-errors`; `abstract-tags-format-tag-ref-label-and-cleartag`; `abstract-tags-gettag-gates-auto-and-force-real-mml` |
| `autoTag`, `clearTag`, `getTag`, `resetTag`, `reset` | `abstract-tags-gettag-gates-auto-and-force-real-mml`; `abstract-tags-format-tag-ref-label-and-cleartag`; `abstract-tags-reset-and-equation-lifetime-real-math-item` |
| `startEquation`, `finishEquation`, recompile/reference state | `abstract-tags-reset-and-equation-lifetime-real-math-item` (uses a real `AbstractMathItem`) |
| `finalize`, `makeTag`, `makeId`, owned `enTag` callback and actual MML | `abstract-tags-finalize-enTag-real-tex-parser-and-mml`; `abstract-tags-finalize-display-env-tag-short-circuits`; `abstract-tags-finalize-live-guard-order-and-owned-entag-dispatch` |
| `NoTags.autoTag/getTag`; `AllTags.finalize` | `no-tags-and-all-tags-overrides` |
| `TagsFactory.add/addTags/create/setDefault/getDefault` | `tags-factory-default-fallback-addtags-key-selection` |
| Static `TagsFactory.OPTIONS` object identity versus `defaultTags` state | `tags-factory-options-record-and-default-selection-are-independent` |

## ColumnParser

| Source responsibility | Original observations |
|---|---|
| `process`, template scanning and full array results | `column-process-*` rows, covering basic and dimension columns, explicit alignments, rules/spacing, prefix/suffix, at/bang, repeats, braced expansion, macros, whitespace, astral and unknown tokens, missing arguments, invalid dimensions and braces |
| Constructor defaults, handler table, function arities and prototype descriptors | `column-constructor-default-handler-keys-and-maxcolumns` |
| Own-handler gate, handler receiver, live handler replacement | `column-processColumn-own-handler-only-and-function-receiver`; `column-processColumn-rereads-replaced-handler-object` |
| Finalizer dispatch order and current handler values | `column-process-handler-and-finalizer-order-live-lookups` |
| `getColumn`, `getDimen`, `getAlign`, `getBraces` | `column-process-column-dimensions`; `column-public-brace-align-dimen-direct-via-real-state`; `column-macro-getbraces-error-boundaries` |
| `macroColumn`, `addRule`, `addAt`, `addBang`, `repeat`, `brace` | `column-process-macro-columns`; `column-process-rules-and-spacing`; `column-process-at-and-bang`; `column-process-repeat`; `column-process-braced-expansion`; `column-repeat-number-boundaries-and-partial-state` |
| `MAXCOLUMNS`, partial state on handler error | `column-process-captured-maxcolumns-boundary`; `column-process-partial-array-writes-before-invalid-column` |

## Native fixture seam and limits

`parser-family-native-candidate.md` records a candidate fixture shape and the
real dependencies it must receive. It is deliberately outside test discovery.
The actual C++ peers are still being integrated, so the oracle does not claim a
native comparison or complete TeX/SVG acceptance. In particular, root-level
module metadata, parser callback bindings, and the complete runtime Context
graph need to be linked before a native fixture can call these same operations
without substituting fake peers.
