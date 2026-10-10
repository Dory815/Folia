# E2 Options prerequisite evidence

This is a read-only evidence ledger for `util/Options.ts`; it does not freeze
the native value model or approve exclusions. The pinned source is MathJax-src
4.1.3 at commit `fb987178c2d279a99b0db5ea02e751691703955e`.

## Provenance and profile reachability

| Source | SHA256 | Profile evidence |
| --- | --- | --- |
| `.reference/MathJax-src/ts/util/Options.ts` | `b810ffc09864eb8ed9295d829b229194d7e38aaac6c8f01525bc1a4b190a61e3` | `docs/math-port/scope.json` lists it as reachable from the selected TeX, common output, and SVG roots; disposition remains unreviewed. |
| `tools/mathjax-bundle/entry.cjs` | `a5a03eed8a10872b9b435b9e602523a08a9f574e21cc5636e8377f0222811ac9` | Active package list and render call: display, `em`, `ex`, and container width are passed to `MathDocument.convert`. |
| `docs/math-port/scope.json` | `6c04066ff1ef7be89f24d14b16b3c1ca7e37c01afdc83ad343dd7f8136685865` | Names the reachable source closure, enabled 38 TeX packages, and pinned profile. |

The direct runtime imports among the scoped profile are in `ts/input/tex.ts`,
`ts/input/tex/Configuration.ts`, `ts/input/tex/ParseOptions.ts`,
`ts/core/MathDocument.ts`, `ts/core/OutputJax.ts`, `ts/output/common.ts`,
`ts/output/common/FontData.ts`, and the enabled package configurations for
`configmacros`, `mathtools`, and `setoptions`. `FindTeX.ts` and several core
classes also carry the `OptionList` type. Scoped callers of `lookup` include
`input/tex/base/BaseMethods.ts`, `input/tex/ColumnParser.ts`, and
`input/tex/mathtools/MathtoolsUtil.ts`. The source closure also lists files
whose only dependency is the `OptionList` type; that type has no runtime
behavior of its own.

## Export and method ledger

References below are line numbers in the pinned `Options.ts`.

| Source member | Behavior in the source | Evidence from the selected profile |
| --- | --- | --- |
| `OBJECT` (25) | Captures the built-in `Object` constructor identity for `isObject`. | Internal implementation detail. |
| `isObject` (33–41) | True only for non-null values whose `.constructor` is exactly `Object` or `Expandable`; arrays, functions, class instances, null-prototype objects, and objects with a replaced constructor are false. | Used internally by `insert`; `setoptions` imports it to accept/reject option values. No structural “map-like” test is equivalent. |
| `OptionList` (45) | TypeScript `{[name:string]: any}`; `any` is not constrained to scalars. | Actual option maps contain nested records, arrays, functions, constructors, instances, nulls and primitives. |
| `APPEND` (59), `REMOVE` (73) | String sentinels `'[+]'` and `'[-]'` for array edits in `insert`. | `[+]` is used by each enabled font extension’s `variants`, `sizeVariants` and `stretchVariants` data. Example: locked `@mathjax/mathjax-bboldx-font-extension` `cjs/svg.js:6-9,420-421`; bbm and dsfont likewise append variant records and `'normal'`. No active-profile `[-]` example was found, but its source branch remains in scope. `FontData.mergeOptions` (source `ts/output/common/FontData.ts:362-369, 971-984`) reaches `defaultOptions` and therefore this merge behavior. |
| `OPTIONS.invalidOption`, `OPTIONS.optionError` (78–94) | Mutable mode is `'warn'` by default. Invalid options call `console.warn('MathJax: ' + message)` and continue; changing the global mode to `'fatal'` throws `Error(message)`. The `_key` callback argument is unused. | The profile does not override the global mode. User/package merge paths pass `warn=true`; invalid configuration can warn and skip the bad key. Fatal remains a language branch an exact utility port must preserve unless root explicitly narrows the contract. |
| `Expandable` (97) | Marker class used to make arbitrary own option names valid. | Active defaults: document `renderActions` in `core/MathDocument.ts:691+`; `configmacros` `active`, `macros`, `environments` (`ConfigMacrosConfiguration.ts:156-160`); `mathtools` `pairedDelimiters`, `tagforms` (`MathtoolsConfiguration.ts:127-129`); `setoptions.allowOptions` (`SetOptionsConfiguration.ts:204+`). |
| `expandable` (114–116) | Returns `Object.assign(Object.create(Expandable.prototype), def)`: a distinct Expandable object with copied enumerable own string/symbol values, ordinary writable/configurable assignments, and the marker prototype. It does not preserve descriptors from `def`. | Used by all active examples in the preceding row. Arbitrary macro names and render action names can be added without unknown-option warnings. |
| `makeArray` (125–127) | Returns an existing array by identity; otherwise wraps the value in a new one-element array. | No call in the scoped runtime profile was found. Exported source behavior is recorded; call-site absence is not an exclusion decision. |
| `keys` (136–142) | Falsey input gives `[]`; otherwise returns `Object.keys(def)` followed by every own symbol from `Object.getOwnPropertySymbols(def)`. String keys therefore use JS enumerable-key ordering; symbols retain symbol identity and come after strings. | Used by all recursive merge/copy operations. Active default option declarations inspected are ordinary enumerable string-keyed object literals; no symbol-keyed profile option was found. Generic symbol behavior is still in the method contract. |
| `copy` (152–178) | Deep-copies arrays and values accepted by `isObject`; other values retain identity/value. It enumerates string keys plus symbols, reads each own descriptor, replaces the descriptor’s value with a recursive copy when applicable, keeps only enumerable descriptors, and defines them on `{}` or a new Expandable object when the input constructor is `Expandable`. Accessor descriptors are retained as descriptors; nested values may be copied. | Reached when `insert` must assign an object literal where no mergeable destination exists. Active defaults primarily merge into known nested objects/Expandable maps. No profile code constructing custom property descriptors or null-prototype option maps was found; the generic copy semantics remain relevant to the exported helper. |
| `insert` (182–278) | Mutates `dst`, iterating source keys in `keys(src)` order. If `warn`, a key is invalid when `dst[key] === undefined` and `dst.constructor !== Expandable`; it reports and skips that key. If source value is an eligible object and destination is non-null object/function, it recursively merges; for destination arrays, the exact `[+]`/`[-]` shapes are recognized. `[-]` filters using `indexOf` (strict equality), then `[+]` appends via a new array; remove happens before append. Otherwise object values recurse. Source arrays replace the destination with a fresh array and are inserted recursively with warnings off. Eligible object values assigned to a non-object destination are deep-copied. All other values, including functions, class constructors/instances, symbols, and primitives, are assigned directly. | This is the active merge engine for TeX/package defaults and user options, output/document options, and font-extension data. It must retain nested recursion, array replacement, append/remove, warning behavior, and callback/instance identity. |
| `defaultOptions` (285–298) | Applies each definition to the same destination in argument order via `insert(..., false)`; returns that destination. | `ParseOptions` (172–174) combines TeX options, tag defaults and package configuration; `Configuration` (423, 465) accumulates package defaults; `OutputJax` (168) and `MathDocument` (776) seed instance options; `FontData` uses it for extension merge. Defaults do not warn for new keys. |
| `userOptions` (302–311) | Applies each definition in order via `insert(..., true)`; returns destination. | Used by TeX configuration and input construction (`tex.ts:143-157`, `Configuration.ts:423-424`), `OutputJax`, `MathDocument` and font data. Invalid new keys warn/skip under default mode. |
| `selectOptions` (318–330) | Creates a plain object; copies a requested key only when it is an own property of source, preserving the value reference rather than deep-copying it. Uses ordinary assignment on the output object. | No call in the scoped profile was found. Its `Object.hasOwn` rule and shallow value identity are source semantics, not inferred from the name. |
| `selectOptionsFromKeys` (340–346) | Uses enumerable string keys from the second object (`Object.keys`) and delegates to `selectOptions`; symbol keys in the key-source are not selected. | No call in the scoped profile was found. |
| `separateOptions` (362–383) | Sequentially partitions the current string-keyed enumerable entries of `options`: for each selector, `object[key] === undefined` goes to the continuing `missing` set; otherwise it goes to that selector’s set. It returns the final remainder first, followed by selector sets in argument order, because it `unshift`s the remainder after collecting them. It reads selector properties including inherited ones, treats explicit `undefined` as missing, and does not mutate the input. | `TeX` destructures `[rest, tex, find]` from selectors `[TeX.OPTIONS, FindTeX.OPTIONS]` (`tex.ts:143-148`); `CommonOutputJax` destructures `[jaxOptions, fontOptions]` for its font selector (`output/common.ts:301-305`). This confirms remainder-first order at both active call sites. `HTMLDocument` and other scoped host pieces also use it. |
| `lookup` (390–391) | Returns a value only if the key is own; otherwise returns the supplied default (default `null`). An own property whose value is `undefined` returns `undefined`; inherited properties are ignored. | Used by active `BaseMethods`, `ColumnParser`, and `MathtoolsUtil` for compact string-to-value tables. This is not the same as ordinary prototype lookup. |

## Concrete option values and merge examples

The active `entry.cjs` calls conversion with `display` (boolean), `em`/`ex`
(numbers), and `containerWidth` (number). `MathDocument.convert` supplies
defaults for `format`, `display`, `end`, `em`, `ex`, `containerWidth`, `scale`,
and `family`, then calls `userOptions` (`core/MathDocument.ts:886-907`).

Across the profile’s constructors and enabled packages, concrete values include:

* **Nested objects:** SVG/common `linebreaks` (`inline`, `width`,
  `lineleading`, `LinebreakVisitor`); `skipAttributes`; `mathtools` settings;
  and setoptions policy maps. Known object destinations recursively merge, so
  an omitted sibling default remains present.
* **Arrays and arrays of mixed values:** TeX `packages`; input/output
  `preFilters` and `postFilters`; document `renderActions` entries such as
  `[STATE.FINDMATH, 'findMath', '', false]`; and extension data with
  `{ '[+]': [...] }`. Ordinary option arrays replace and recursively copy the
  source array. The sentinel form edits an existing destination array.
* **Constructors and object instances:** document `MathList` and `MathItem`
  defaults are constructors; `InputJax`, `OutputJax`, and `MmlFactory` slots
  represent instances; SVG/common options can receive wrapper/font factories,
  a `FontData` instance, or a font-data constructor. Functions and class
  instances are not `isObject` values, so assignment preserves their identity.
* **Callbacks:** `formatError`, `compileError`, and `typesetError` are function
  values. `preFilters`/`postFilters` arrays and setoptions filter fields also
  carry callbacks. `Options` assigns or copies callback references; it does not
  invoke or compare callback bodies.
* **Expandable maps:** the active configmacros, mathtools, setoptions, and
  document render-action defaults listed above allow caller-defined keys.
  This differs from an ordinary nested object, where unknown entries can warn.
* **Symbols and descriptors:** `keys` and `copy` explicitly account for symbol
  keys and property descriptors, but the active profile defaults and call
  arguments seen here use ordinary enumerable string-keyed data properties.
  The exported JS helper behavior still distinguishes symbols, enumerable vs
  non-enumerable properties, and accessor descriptors. Profile observation
  alone does not settle whether the native API may narrow those cases.

## Merge and diagnostic edge cases to retain in the prerequisite contract

`insert` checks `dst[key] === undefined`, not own-property presence. An inherited
known value is accepted; a present own property whose value is `undefined` is
treated as unknown. `Expandable` is detected by the destination’s constructor
identity. `isObject` can consequently change when an object shadows its
`constructor` field. A source function is assigned as a value, while a
mergeable object source can recursively add properties to a function
destination. The APPEND/REMOVE form is recognized only for one valid sentinel
whose value is an array, or exactly both sentinel keys with array values;
removal uses strict `indexOf` membership, then addition appends in source order.

Warnings can occur recursively for unknown nested keys. In warn mode each bad
key is skipped and later keys continue; in fatal mode the callback throws at
the first encountered invalid key. Key iteration order therefore affects which
warning/error appears first. `copy` preserves enumerable descriptors for
ordinary string/symbol keys but does not copy non-enumerables. `selectOptions`
and the selector side of `separateOptions` use different own/inherited rules;
`lookup` is own-only. These functions must not be normalized to one shared
“map lookup” policy.

This evidence records what the pinned source and selected profile demonstrate.
It does not decide whether a future C++ Options API must preserve unobserved
symbol/descriptor cases, what native representation to choose for recursive
values or callbacks, or which dependencies root may formally adapt. Those are
root-owned contract and scope decisions.

## Remaining evidence limits

The current source hashes above were rechecked against the workspace, and the
package count was rechecked from `scope.json` (38) and the matching profile in
`entry.cjs`. `separateOptions` return order was rechecked against its final
`results.unshift(options)` and the active `[rest, tex, find]` destructuring.
Static defaults and scoped direct call sites establish representative value
kinds, but they do not enumerate every runtime option object a future caller
could supply. The checked-in profile does not set `OPTIONS.invalidOption` to
`fatal`, use `REMOVE` in its locked font-extension data, or present symbol-keyed
and custom-descriptor option maps. The source still implements those branches;
root must decide the native contract based on the reviewed scope rather than
this absence alone. `makeArray`, `selectOptions`, and `selectOptionsFromKeys`
have no call sites in the currently scoped production closure found during
this audit; this is call-site evidence, not an exclusion recommendation.
