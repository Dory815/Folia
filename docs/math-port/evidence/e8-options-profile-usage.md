# E8 Options usage in the pinned production profile

## Scope and evidence boundary

This audit uses MathJax-src 4.1.3 at commit
`fb987178c2d279a99b0db5ea02e751691703955e`, the entry point
`tools/mathjax-bundle/entry.cjs` (SHA-256
`a5a03eed8a10872b9b435b9e602523a08a9f574e21cc5636e8377f0222811ac9`), and
the `scope.json` source closure. The pinned `ts/util/Options.ts` hash is
`b810ffc09864eb8ed9295d829b229194d7e38aaac6c8f01525bc1a4b190a61e3`.
The entry selects 38 TeX packages and constructs one TeX input jax and one SVG
output jax; it does not load the MathJax startup component. These are source
observations, not a claim that every future host or configuration is equivalent.

## Reachable imports and exported API

The scope closure directly imports runtime exports from `Options.ts` at these
call sites:

| Reachable importer | Imported exports and source calls |
| --- | --- |
| `ts/core/FindMath.ts:24,78` | `defaultOptions`, `userOptions`; constructor merges class defaults with passed options. |
| `ts/core/InputJax.ts:28,168` | `defaultOptions`, `userOptions`; constructor merges defaults and then constructs filter lists from the resulting arrays. |
| `ts/core/OutputJax.ts:24,168` | `defaultOptions`, `userOptions`; same default-plus-user merge pattern. |
| `ts/core/MathDocument.ts:24-29,691,776,888,1195` | `expandable` builds the `renderActions` default; `defaultOptions`/`userOptions` merge document defaults, conversion options, and reset options. |
| `ts/input/tex.ts:25,144,157` | `separateOptions` partitions constructor input against `TeX.OPTIONS` and `FindTeX.OPTIONS`; `userOptions` applies the remaining TeX options. |
| `ts/input/tex/Configuration.ts:28,423-424,465` | `defaultOptions` merges package defaults; `userOptions` applies package options during dynamic package addition. |
| `ts/input/tex/ParseOptions.ts:33,173-174` | `defaultOptions` merges the per-parse inputs and selected package configuration. |
| `ts/input/tex/ColumnParser.ts:27,307` | `lookup` selects column alignment from a small object map with a default. |
| `ts/input/tex/base/BaseMethods.ts:43,1451,1481` | `lookup` selects alignment names for base TeX methods. |
| `ts/input/tex/configmacros/ConfigMacrosConfiguration.ts:26,157-159` | `expandable` creates open `active`, `macros`, and `environments` option objects. |
| `ts/input/tex/mathtools/MathtoolsConfiguration.ts:29,127,129` | `expandable` creates open `pairedDelimiters` and `tagforms` option objects. |
| `ts/input/tex/mathtools/MathtoolsMethods.ts:36,346` and `MathtoolsUtil.ts:27,46` | `lookup` selects alignment and display/script settings from object maps. |
| `ts/input/tex/setoptions/SetOptionsConfiguration.ts:37,80,204` | `expandable` constructs the allow-list; `isObject` classifies values while filtering options. The configuration also carries `filterPackage`, `filterOption`, and `filterValue` function values. |
| `ts/handlers/html/HTMLDocument.ts:25-31,77,106,190` | `expandable` extends document render actions, `separateOptions` partitions HTML string options from document options, and `userOptions` merges conversion options. |
| `ts/handlers/html/HTMLDomStrings.ts:24-31,62,133,153-155` | `expandable` constructs `includeHtmlTags`; `defaultOptions`/`userOptions` build active options; `makeArray` normalizes `skipHtmlTags`, `ignoreHtmlClass`, and `processHtmlClass`. |
| `ts/output/common.ts:39,301` | `separateOptions` splits output-jax options from font-data options. |
| `ts/output/common/FontData.ts:26,367,921,945,971-972` | `defaultOptions` and `userOptions` merge font class defaults, instance options, extension options, and parameter maps. |
| `ts/output/common/Wrapper.ts:40,1372` | `lookup` maps a rule-thickness keyword through a small object map. |

`Options.ts` uses its own `keys`, `copy`, `insert`, `APPEND`, `REMOVE`, and
`OPTIONS.optionError` internally. `copy` calls `insert` for array values and
recurses for object-literal values (`Options.ts:152-168`); `insert` calls
`keys`, recognizes the append/remove sentinels, uses `filter`/`indexOf`, and
uses spread (`Options.ts:188-257`). `selectOptions` and
`selectOptionsFromKeys` are exported but have no direct importer in the scoped
source closure. `makeArray` is used by `HTMLDomStrings` as listed above.

The following scoped imports are `OptionList` type annotations and have no
runtime reference to an `Options.ts` value: `ts/core/DOMAdaptor.ts:24`,
`Handler.ts:29`, `HandlerList.ts:25`, `MathItem.ts:26`,
`ts/input/tex/FindTeX.ts:25`, `TexParser.ts:36`, `ts/mathjax.ts:27`,
`ts/output/svg.ts:25`, `ts/output/svg/Wrapper.ts:24`,
`ts/output/svg/Wrappers/menclose.ts:43`, `mglyph.ts:42`, and
`mtable.ts:43`. TypeScript erases these type-only uses.
This is the complete direct-import set found by scanning every `scope.json`
source entry for `util/Options.js`; imports from excluded components and other
output/input jax implementations are not treated as production evidence.

## Concrete values entering the option graph

`entry.cjs:54-67` passes a 38-name ordinary array as `TeX`'s `packages`
option. `entry.cjs:68-74` constructs `SVG` with `fontData` (a font object),
`fontCache: 'none'`, `exFactor: 0.5`, `displayOverflow: 'linebreak'`, and a
nested `linebreaks` object containing `inline: true`. `entry.cjs:83-90` calls
the document conversion with `display` plus numeric `em`, `ex`, and
`containerWidth` fields. The entry does not accept an arbitrary option object
from the TeX input string.

Reachable defaults are more varied than those entry arguments. `TeX.OPTIONS`
contains the `packages` array and a `formatError` function
(`ts/input/tex.ts:67-81`); `AbstractInputJax` and `AbstractOutputJax` default
`preFilters` and `postFilters` to arrays (`ts/core/InputJax.ts:132-135`,
`ts/core/OutputJax.ts:138-141`). `MathDocument.OPTIONS.renderActions` is an
Expandable containing nested arrays (`ts/core/MathDocument.ts:691-697`).
`MathtoolsConfiguration` nests open objects under `pairedDelimiters` and
`tagforms` (`ts/input/tex/mathtools/MathtoolsConfiguration.ts:127-130`), and
`ConfigMacrosConfiguration` has expandable maps
(`ts/input/tex/configmacros/ConfigMacrosConfiguration.ts:156-160`).
`SetOptionsConfiguration` stores callable filter functions alongside a nested
Expandable allow-list (`ts/input/tex/setoptions/SetOptionsConfiguration.ts:197-220`).
Thus the real profile requires identity-preserving function/constructor values,
arrays, nested plain objects, and Expandable objects. The Options helpers
preserve callable values as values; package and parser code invokes its
callbacks outside the Options helper itself.

The font-data merge path accepts `data.options` and `data.parameters` objects
and merges `sizeVariants`, `stretchVariants`, variants, and delimiter data
(`ts/output/common/FontData.ts:935-1000`). The entry attaches the three font
extensions explicitly (`entry.cjs:76-79`). Those font objects contain structured
font tables; this audit does not treat their glyph arrays as arbitrary
JavaScript option overrides.

## Array behavior, length, and intrinsic calls

Observed source calls:

- The profile's package list is an ordinary array literal. The pinned entry
  does not replace `Array.prototype`, define `Symbol.iterator`, provide an
  Array subclass, or pass a custom iterator.
- `Options.ts:240-246` calls `dval.filter(...)`, the removal list's
  `indexOf(...)`, and spreads ordinary arrays. There are no scoped assignments
  to an option array's own `filter`, `indexOf`, `values`, or iterator property.
  No source use of `Symbol.species` was found in the scoped closure.
- `Options.ts:152-168` copies an array by enumerating its keys and assigning
  present indices into a new array. `Object.keys` omits the built-in
  non-enumerable `length`; this code does not assign an object to an array's
  `length` property. The profile entry supplies `packages` as an array, not as
  an object with a `length` member.
- `Options.ts` itself calls standard built-ins directly: `Object.assign` and
  `Object.create` in `expandable` (line 118); `Array.isArray` in `makeArray`,
  `copy`, and `insert` (126, 157, 220-234, 254); `Object.keys`,
  `Object.getOwnPropertySymbols`, and `Object.getOwnPropertyDescriptor` in
  `keys`/`copy` (139-168); `Object.defineProperties` in `copy` (169-173);
  `Object.hasOwn` in selection and lookup (319-355); array `filter`,
  `indexOf`, and spread in `insert` (240-246). No module in the scoped entry
  supplies replacement intrinsic bodies. The native contract therefore needs
  faithful results for these source operations, not a general user-visible
  JavaScript intrinsic registry.

No enabled default in the inspected source assigns an object value to an
array's `length`. This is a bounded source observation, not proof against an
arbitrary host object or future configuration. A plain object containing a
`length` key could be merged into an array destination by the generic
`insert` implementation; the entry/profile provides no concrete witness for
that path. The source also does not alter `filter`/`indexOf` or use custom
species/iterators, but generic JavaScript permits such overrides; their absence
from this entry is not a universal JavaScript guarantee.

## Recursion, cycles, and option-error replacement

`copy` recursively copies arrays and object-literal values at
`Options.ts:152-168`; `insert` recursively merges nested object values at
`Options.ts:211-253`. The active entry's explicit option objects and enabled
package defaults are statically constructed; no self-referential option object
or cyclic Expandable map is constructed in the reviewed profile. The TeX
`\setOptions` path parses key/value text and writes scalar string/boolean or
RegExp values after checking an existing option key
(`ts/input/tex/setoptions/SetOptionsConfiguration.ts:65-164`), so TeX source
does not supply object cycles through that path. The audited entry does not
expose a host API for passing an arbitrary cyclic option graph. Whether a
separate host integration might pass one is outside this evidence.

The default `OPTIONS.invalidOption` is `'warn'`, and the default
`OPTIONS.optionError` logs a `MathJax: ` warning or throws when mode is
`fatal` (`Options.ts:77-89`). It is invoked only for an unknown source key in
`insert` (`Options.ts:188-196`). The scoped entry does not import `OPTIONS`
and does not load `ts/components/startup.ts`; the startup code that can replace
`OPTIONS.invalidOption`/`OPTIONS.optionError` is at `startup.ts:630-634` and is
not reachable from `entry.cjs`. The entry's initial TeX/SVG option keys match
their declared defaults. `\setOptions` checks package and option names before
assignment (`SetOptionsConfiguration.ts:48-113`). Therefore, no replacement
`optionError` callback is observed in this production profile. The exported
`OPTIONS` object remains mutable in upstream; its replacement behavior is
unknown for any host that adds MathJax startup configuration.

## P3b contract check and unresolved behavior

The upstream calls require the native mapping to preserve these ordinary-array
semantics: spread observes the live iterator length; holes without an inherited
index are read as `undefined` by iteration; `indexOf` skips absent indices and
uses strict equality; `filter` captures the starting length, checks property
presence, observes later mutations, and creates a dense result. The source's
one-argument `indexOf` call does not exercise `fromIndex` coercion. Direct
`.values()` is not called; `arrayValues()` is justified only as the native
adapter for spread consumption, not as the result of the public JS
`Array.prototype.values()` method. The profile contains no evidence for custom
iterables, species, proxies, overridden array methods, or direct method calls
from arbitrary TeX input.

No P3b contract contradiction was found for those observed ordinary-array
operations. The profile does not prove the full JavaScript behavior for
arbitrary callables, arbitrary option graphs, nonnumeric `fromIndex`, custom
iterators/species, or a replacement of the global option-error handler; those
remain unobserved rather than implicitly accepted.
