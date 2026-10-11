# P19b1 fixed-profile Macro.args data-domain audit

Audit basis: pinned MathJax source at `fb987178c2d279a99b0db5ea02e751691703955e`; runtime entry and package list in `tools/mathjax-bundle/entry.cjs`; scope profile SHA-256 `a5a03eed8a10872b9b435b9e602523a08a9f574e21cc5636e8377f0222811ac9`. This is a source reachability audit, not an API or acceptance decision. The package profile has 38 explicitly registered packages. `autoload` is not in the list; `entry.cjs` says dynamic `autoload` and `require` loaders are intentionally unnecessary in the offline runtime.

## Finding

For values produced by the 38-package profile from TeX source, the contract's raw argument domain is sufficient: Macro args are arrays of primitive `boolean | number | string | null` values (including empty arrays), and user-written definitions can only synthesize those same primitive values. The runtime rest-argument operation still needs the full P19b1 host behavior for a raw Property that represents those arrays and their slots. The package code does not create a custom iterator, boxed string, Proxy, accessor, or descriptor-bearing args object.

That finding does **not** establish coverage for JavaScript configuration or extension injection. The fixed `entry.cjs` constructs TeX with only `packages`, so `configmacros` macros/active/environments and mathtools paired delimiter Macro entries begin empty. However, after initialization the enabled `\setOptions` command can mutate selected option objects. `\setOptions{macros=...}` and `\setOptions{environments=...}` are explicitly rejected; `active` is not a top-level TeX option and is rejected as invalid; `\setOptions[configmacros]{...}` is rejected because that package is denied. `\setOptions[mathtools]{pairedDelimiters=...}` is allowed and mutates that option property, but it runs after the constructor's package CONFIG callbacks and does not rerun `MathtoolsConfiguration` or rebuild Macro entries. Therefore it does not create a new Macro.args value in this fixed runtime. A caller changing TeX constructor options, mutating `Macro._args`/the returned `args` array, installing extensions, or altering prototypes can still supply values outside the demonstrated TeX-source domain. The TypeScript `Args` type does not constrain such JavaScript at runtime.

## Reachable construction and mutation paths

| Path | Source and exact behavior | Data shape reachable from TeX/profile |
|---|---|---|
| Static package maps | `TokenMap.ts:273-300`: for each literal package `MacroMap` entry, an array value is copied with `value.slice(1)` into the new Macro; a scalar function entry gets a fresh `[]`. `MacroMap.parse` spreads that array at `TokenMap.ts:312-321`; `CommandMap.parse` does so at `337-343`. | Static mappings across the selected packages use declarative literal entries; their argument elements are strings, numbers, booleans, or null. The spread receives the copied native array. No source-side reassignment of `Macro.args` or `_args` was found. |
| `\newcommand` / `\renewcommand` | `NewcommandMethods.ts:44-51` reads a name, digit-string count, optional bracket default, and body, then stores `[def, n, opt]`. `NewcommandUtil.ts:103-124` confirms count comes from TeX text and remains a string; optional `GetBrackets` result is string/null. `NewcommandUtil.ts:350-365` constructs and inserts the Macro. | Three-element native array of strings/null; no object-valued argument can be authored through this syntax. |
| `\newenvironment` | `NewcommandMethods.ts:60-73` stores `[true, bdef, edef, n, opt]`; the fields come from TeX argument parsing and `GetArgCount`. | Native array of boolean/string/null primitives. `EnvironmentMap.parse` passes this array as one function argument (`TokenMap.ts:385`), without rest-spreading it. |
| `\def` / `\gdef` | `NewcommandMethods.ts:83-101` stores `[def, params]` when the template has numeric parameter count, or `[def].concat(params)` for delimited templates. `NewcommandUtil.ts:130-194` returns a number or string array derived from the parser's TeX source; insertion uses the same `addMacro` path. | Native array containing body/template strings and a number (or only strings). |
| `\let` of a MacroMap macro | `NewcommandMethods.ts:120-150` gets the existing Macro and passes `macro.args` to `addMacro`; `NewcommandUtil.ts:350-365` stores that same array in a new Macro. This aliases the array, it does not copy or transform it. | In-profile values remain the primitive static/TeX-defined array. No built-in path mutates that array after construction; built-in source uses the getter but contains no `_args` assignment or `macro.args` write. |
| `\let` of a character macro or delimiter | `NewcommandMethods.ts:151-190` synthesizes `[cs, macro.char]` or `[c]`, with strings. | Native arrays of strings. |
| Undefinition blocker | `NewcommandUtil.ts:396-415` removes then may add `new Macro(cs, () => FALLBACK, [])`. | Empty native array. |
| Config macros / active mappings | `ConfigMacrosConfiguration.ts:91-104` accepts host options, normalizes string definitions to `[string]`, handles optional template arrays using `slice`/`concat`, then stores a Macro. Defaults are `{}` at lines 150-154; `entry.cjs:54-66` passes only packages. The `setoptions` filter rejects TeX `macros` (explicit false), rejects `active` as an invalid TeX option, and rejects package `configmacros` (`SetOptionsConfiguration.ts:207-219`, `filterOption` lines 75-111). | These stay empty through fixed-profile TeX source. They can be populated through external constructor options, which are not in-profile. |
| Config environments | `ConfigMacrosConfiguration.ts:130-146` creates `[true].concat(environments[env])`; defaults are `{}`. `setoptions` rejects TeX `environments` (explicit false; same filter lines above). | Remains empty through fixed-profile TeX source; external constructor options are outside this audit. |
| Mathtools configured paired delimiters | `MathtoolsConfiguration.ts:55-61` iterates `parser.options.mathtools.pairedDelimiters` and uses each option value array directly as Macro.args; the default is `expandable({})` at line 127. `\setOptions[mathtools]{pairedDelimiters=...}` passes package and option filters, then assigns the parsed scalar to this option property. | The option can change after construction, but the CONFIG callback already ran and is not re-invoked by `SetOptions`; the already-created Macro map is not rebuilt. Thus this TeX command does not create or replace Macro.args in the fixed profile. External constructor options can populate it before CONFIG and are outside this audit. |
| Mutable TeX regex options | `SetOptionsConfiguration.ts:128-140, 143-158` permits ordinary TeX options not in its deny list; `initialLetter` and `initialDigit` are valid TeX options. `filterValue` recognizes `/pattern/flags` and returns `new RegExp(match[1], match[2])`. `ParseUtil.ts:104-177, 888-919` makes `readKeyval` return `true`/`false` as booleans and all other key values as strings before the optional filter; `\setOptions{initialLetter=/.../flags,initialDigit=/.../flags}` can therefore install non-default RegExp objects in `parser.options`. | These fields are reachable and mutable from TeX. Base CONFIG constructs `RegExpMap('digit'/'letter', ..., options.initialDigit/initialLetter)` once at `BaseConfiguration.ts:162-169`; `RegExpMap` stores the given `RegExp` object and `contains` tests it at `TokenMap.ts:147-172`. A later `\setOptions` changes the option property but does not replace the already-created map. This remains a required non-ASCII-safe source/runtime case for the Regex host contract; do not narrow accepted initial regex behavior to ASCII based on defaults. |
| Mathtools `\Aboxed` | `MathtoolsMethods.ts:476` creates `[box, star]`. | Native array of two strings from parsed TeX input. |
| SetOptions `\Require` / `\require` | `SetOptionsConfiguration.ts:181-187`: `Require` gets the constructor's default empty array; `require` uses a fixed literal args array with template string, numeric `2`, and empty string. | Empty native array or fixed primitive array. Runtime async loading does not itself broaden these Macro args. |

The source-level Args alias is explicitly `boolean | number | string | null` (`Types.ts:28`), and `Macro` stores its constructor argument without cloning and exposes it through a getter (`Token.ts:55-78`). The `_args` field is TypeScript-private only; emitted CJS uses a normal writable property. Consequently, identity-preserving native representation matters for the `\let` path, while arbitrary JavaScript mutation is an extension boundary rather than an in-profile operation. `\setOptions` is an in-profile runtime mutation of option records, but its code only assigns option values; it does not call configuration callbacks or Macro constructors.

## Native host domain versus arbitrary JavaScript

The P19b1 Property domain can faithfully encode the in-profile arrays, their primitive values, ordering, and the raw argument handoff. It also deliberately models JS-like arrays/records and indexed fallback for caller-provided raw properties. Those native facilities cover the ordinary data produced above; they do not imply that a Property array is equivalent to every JavaScript object.

The following are not created by the fixed package code or TeX definition syntax and remain unproven/outside this profile audit: an own or inherited `Symbol.iterator`; a custom iterator returning values or throwing; boxed strings with an overridden iterator; descriptor/accessor side effects; Proxy traps; non-ordinary prototypes or getters with side effects; and arbitrary objects assigned into `_args` or supplied in external options. A future extension surface that admits such JS objects needs a separately specified callback/evaluation protocol. This audit does not recommend one.

## Source locations and hashes

SHA-256 values are for the checked-in pinned TS files and installed CJS files as read in this workspace.

| File | Relevant lines | SHA-256 |
|---|---:|---|
| `.reference/MathJax-src/ts/input/tex/Token.ts` | 55-78 | `6b4c0e42f277daebe8e676afc3b4d3b6083ebc87939ef8203672a831be1595bf` |
| `.reference/MathJax-src/ts/input/tex/Types.ts` | 28 | `924d0b48bf7b5fdb0ccc995171fbdfb001bc3ac51a0af1409bb8c1d953430927` |
| `.reference/MathJax-src/ts/input/tex/TokenMap.ts` | 147-172, 273-343, 375-385 | `0209895b13569008b2129b61c88050e7c70e70ce9477d234b9a6e303a86a738e` |
| `.reference/MathJax-src/ts/input/tex/newcommand/NewcommandMethods.ts` | 44-101, 120-190 | `2d3a5422123d9afb36e9277f506d5a24a7a1d3295163a34371c8fb7400b41cfa` |
| `.reference/MathJax-src/ts/input/tex/newcommand/NewcommandUtil.ts` | 103-194, 350-415 | `9dd994444bd3c0b49c83ab070dfb82f022a69364109e775b535d7281d9b33002` |
| `.reference/MathJax-src/ts/input/tex/configmacros/ConfigMacrosConfiguration.ts` | 56-73, 91-154 | `1578e2cec78cd03e83dff7439c0b2eedbb4564cef2002dbd2dba528c25197c7a` |
| `.reference/MathJax-src/ts/input/tex/mathtools/MathtoolsConfiguration.ts` | 51-61, 127 | `7b14b1d6e4324397ab315fa012ceada7999df6cbf3b4c9f09fc3742f8b5a30ae` |
| `.reference/MathJax-src/ts/input/tex/mathtools/MathtoolsMethods.ts` | 476 | `ad6562e23ba65cafdb480afc33387982c7f7b14a2886ae7844dfbe64ec44e918` |
| `.reference/MathJax-src/ts/input/tex/setoptions/SetOptionsConfiguration.ts` | 75-158, 174-219 | `ccaa243137c68208e7f442013aa5dc6cc04b735ac3e0ca099ea8f30be557790d` |
| `.reference/MathJax-src/ts/input/tex/ParseUtil.ts` | 104-177, 888-919 | `b43646071997f88f27df51610c7972a8cb5791ea76194e7318d17d1a2f81f76a` |
| `.reference/MathJax-src/ts/input/tex/Configuration.ts` | 372-427 | `8184716579abf3ffa14c64413d276dab4fc721e3e3f43afb0cdccbe2ab782719` |
| `.reference/MathJax-src/ts/input/tex.ts` | 143-160 | `151d3c73697bfbf874001549f6e0c80dbebeabba2b36ab50501a4ffd7753bf06` |
| `.reference/MathJax-src/ts/input/tex/base/BaseConfiguration.ts` | 162-169, 226-228 | `578d2c6b03f2f3e902476a0b73a5323b12262215fcbdc831a1135c1d9c1737ad` |
| `tools/mathjax-bundle/entry.cjs` | 54-66 | `a5a03eed8a10872b9b435b9e602523a08a9f574e21cc5636e8377f0222811ac9` |
| `docs/math-port/scope.json` | profile package set | `14c3698834f494953dc2fc09affdb2458e25882e9d4ba45b5489a44c87ac1ba9` |
| `docs/math-port/argument-expansion-contract.md` | full contract | `ee69c1aa96bcb1565d38669d06ae3c60c1c3052456c2ff20d41bb9ebed648e41` |
| `tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/Token.js` | 35-57 | `d317b026fc1ae0ff93d6939a15041d225385a826f8c33115284464bc2f527e35` |
| `tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/TokenMap.js` | 98-104, 194, 218, 237, 259 | `9e3f086080a6ab052b68b22a5d9ee3dbc5e17d7b0c03a31e789664b01df36cf7` |
| `tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/newcommand/NewcommandMethods.js` | let/newcommand lowering | `7aa2e76b2263b2f12961958c86e2325acba83083334ca54f6949f86a25bd80cb` |
| `tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/newcommand/NewcommandUtil.js` | add/undefine lowering | `496a1c7154521e6870c5469b47070323766182150740f2e846cd005460b7e4bf` |
| `tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/configmacros/ConfigMacrosConfiguration.js` | option macros/environment lowering | `050cfb81fdc2914459e6c4128ab2ba8a3162207f72db921351cc1fb3320914a0` |
| `tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/mathtools/MathtoolsConfiguration.js` | paired delimiters lowering | `863415dfbc384852e5b3b39724d50cc453e1bb6e54f8f6a1073b9e5e43b8734d` |
| `tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/setoptions/SetOptionsConfiguration.js` | 39-90, 115-133 | `dac7cec87b390b199257df54f4942a14f7c5ef0cbef54e75ff4f471551a41852` |
| `tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/ParseUtil.js` | 63-139, keyval parser | `adafa18cefad39e8408bcc0e4eb7103dceb6d16c9e7ea2eb013f23602c3f8941` |
| `tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/Configuration.js` | 171-225 | `86f62a6f6e44fec20156546b7e8d424cac493c7465c2c8ec621c604332a276f3` |
| `tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex.js` | 63-77 | `ef2852b197173f373e62326016e980aef01ffc838f5f766ab6076262f944eada` |
| `tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/base/BaseConfiguration.js` | 159-160, 215-217 | `0feedc60ac600b647cf2a1437c7244196a5aec04715f4a301fe6e9ec09722c57` |

The installed CJS confirms the TS behavior: `Token.js` assigns `_args` directly and returns it; `TokenMap.js` emits `parser.apply(... __spreadArray(... __read(macro.args) ...))`; the newcommand/configmacros CJS has the corresponding same constructor arguments and array `slice`/`concat` flow. No separate CJS-only args mutation was found in the package search.

## Unproven boundary

Root follow-up: the directly active mutable-regex path is `numberPattern`.
`BaseConfiguration.ts:164–166` only copies a truthy legacy `digits` option to
`numberPattern` during CONFIG. `ParseMethods.ts:81–82` reads current
`parser.configuration.options['numberPattern']` and calls its exec on the
remaining TeX string. Unlike later reassignment of captured initialLetter/
initialDigit, `\\setOptions{numberPattern=/.../flags}` can therefore change an
actual subsequent matching operation. General regexp compilation/execution and
lastIndex/capture/error behavior cannot be replaced by default ASCII tests.
This follow-up does not broaden Macro.args or accept a regex implementation.

This evidence establishes the Macro.args data produced by the pinned fixed profile and TeX syntax, and distinguishes it from mutable options. It does not establish that the Property representation covers arbitrary MathJax users who instantiate TeX with non-default options, call package maps directly, mutate `Macro._args`/the array returned by `args`, patch built-ins, load third-party packages, or interpose JS objects with iteration/coercion hooks. It also does not establish the complete observable behavior of non-ASCII regexes, stateful regex flags such as `g`/`y`, invalid regex construction errors, or the runtime implications of changing regex option properties after Base CONFIG has captured the original RegExp objects. Those are source-reachable regex-host questions, not grounds for narrowing defaults to ASCII. This audit does not independently validate the native expansion implementation or make an acceptance claim; root owns those checks.
