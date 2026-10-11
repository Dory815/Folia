# P22 TeX parser boundary source audit

This is a read-only dependency and interface audit for the pinned MathJax
4.1.3 `TexParser`, `ParseOptions`, and `Configuration` sources. It identifies
the methods and state that native declarations must account for; it does not
authorize or implement a parser.

## Complete source member inventories

### `TexParser`

Pinned source: `.reference/MathJax-src/ts/input/tex/TexParser.ts`, SHA-256
`9afdfff070ec7ed7115a1afe97048b3d1f1befae5b75e8ec963e0456685ee167`.

| Member | Lines | State / direct role |
|---|---:|---|
| `macroCount`, `stack`, `i`, `currentCS`, `saveI` | 48, 55, 62, 69, 74 | Mutable recursion count, parse stack, UTF-16 code-unit cursor, current control-sequence string, and saved cursor for token/LaTeX bookkeeping. |
| constructor (`_string`, `env`, `configuration`) | 83–103 | Stores the input and ParseOptions; extracts/deletes `env.isInner`, shallow-copies remaining env keys, pushes itself on the shared parser stack, constructs Stack, parses immediately, pushes `stop`, then assigns stack env. |
| getters `options`, `itemFactory`, `tags` | 108–124 | Forward configuration state and factories. |
| `string` setter/getter | 131–139 | Mutable current parse string. |
| `parse` | 149–165 | Saves/restores cursor bookkeeping; temporarily rewinds for character tokens; dispatches through `configuration.handlers.get(kind).parse`; updates result except for macro kind. |
| `lookup`, `contains`, `toString` | 174–203 | Delegate to SubHandlers; `toString` snapshots handler keys with `Array.from` and emits each handler. |
| `Parse` | 209–216 | Loops while cursor is below UTF-16 string length, reads one Unicode code point, advances by that string's UTF-16 length, and dispatches a character token. |
| `Push`, `PushAll` | 224–246 | `instanceof BaseItem` writes source-span metadata; `instanceof AbstractMmlNode && isInferred` flattens children; otherwise pushes to Stack. `PushAll` preserves iteration order. |
| `mml` | 251–262 | Pops the active parser, returns null unless stack top is `mml`, takes `First`, trims the complete TeX source, and stores nonempty LATEX on the result. |
| `convertDelimiter`, `getCodePoint`, `nextIsSpace`, `GetNext`, `GetCS` | 275–319 | Token lookup, UTF-16-indexed code-point handling, regex whitespace/control-sequence scanning and cursor mutation. |
| `GetArgument` | 329–378 | Reads one character/control sequence or scans nested braces; handles escapes, optional missing argument, unmatched close/open, and source-position updates. |
| `GetBrackets` | 388–436 | Scans bracket argument while tracking braces and optionally nested brackets; escaped characters are skipped; throws on unmatched delimiters. |
| `GetDelimiter` | 445–465 | Reads a delimiter/control sequence, optionally unwraps braces and trims, validates against delimiter handlers, then converts the mapping. |
| `GetDimen` | 473–496 | Reads braced or unbraced dimension using `UnitUtil.matchDimen`; advances by returned input length in unbraced form. |
| `GetUpTo` | 505–545 | Scans to a target token outside braces, preserving escapes and cursor boundaries; throws for unmatched close or absent target. |
| `ParseArg`, `ParseUpTo` | 553–574 | Recursively constructs a new TexParser sharing `stack.env` and the same ParseOptions, then asks it for MML. |
| `GetDelimiterArg`, `GetStar` | 582–607 | Trims and validates delimiter arguments; consumes a following star when present. |
| `create` | 617–625 | Calls configured TeX NodeFactory with variadic arguments; for tokens with explicit hyphen-prefixed mathvariant, writes `ignore-variant`. |
| `trimTex` | 633–635 | Trims source edges but preserves one trailing escaped space according to its regex. |
| `updateResult` | 645–743 | Reads prior stack node and LATEX/LATEXITEM attributes; composes brace and sub/sup source metadata or records the current substring. Runs after each non-macro dispatch. |
| `composeLatex`, `composeBraces`, `composeBracedContent` | 753–797 | Reads indexed child nodes' LATEX, writes composed attribute; recursively joins braced content with a space between adjacent letter fragments. |

### `ParseOptions`

Pinned source: `.reference/MathJax-src/ts/input/tex/ParseOptions.ts`, SHA-256
`ab60a07dbb24040c0fb9bd67b942755ffc5b0b18980abf1938736d8d90b22209`.

| Member | Lines | State / direct role |
|---|---:|---|
| static `getVariant` | 46–64 | Four style callbacks (`TeX`, `ISO`, `French`, `upright`) using codepoint/string tests and MathVariant constants. |
| `handlers`, `options`, `itemFactory`, `nodeFactory`, `tags`, `mathStyle`, `columnParser`, `packageData` | 71–116 | Package-merged handlers/options, factories, tags/style callback, column grammar parser, and mutable package-keyed `Map<string, any>`. |
| `parsers`, `mathItem`, `root`, `nodeLists`, `error` | 126–152 | Ephemeral recursive parser stack, current MathItem/root, saved node lists for filters, and parse error flag. |
| constructor | 160–178 | Captures handler set; constructs/configures TeX NodeFactory and StackItemFactory from ParserConfiguration; merges options, chooses style callback with TeX fallback. |
| `pushParser`, `popParser`, `parser` | 186–201 | LIFO parser stack via `unshift`/`shift`, current parser at index zero. |
| `clear` | 207–213 | Replaces parser/root/node-list state, clears error, and resets tags. |
| `addNode` | 222–239 | Adds node to named list; when list key differs from node kind, stores comma-joined `in-lists` property via NodeUtil. |
| `getList` | 254–264 | Filters saved list through `inTree(node)` against current root and replaces stored list with the survivors. |
| `removeFromList` | 273–280 | Removes each requested node by object identity from a named list. |
| `inTree` | 290–295 | Walks `parent` pointers until root or null. |

### `Configuration` and `ParserConfiguration`

Pinned source: `.reference/MathJax-src/ts/input/tex/Configuration.ts`, SHA-256
`8184716579abf3ffa14c64413d276dab4fc721e3e3f43afb0cdccbe2ab782719`.

| Members | Lines | Role |
|---|---:|---|
| `StackItemConfig`, `TagsConfig`, `Processor<T>`, `ProtoProcessor<T>`, `ProcessorList`, `ConfigMethod`, `InitMethod` | 35–45 | Package registration and callback type aliases; processors carry a function and priority. |
| `Configuration.makeProcessor`, `_create` | 56–114 | Normalizes bare callbacks to `[callback, priority]`, maps package fields, defaults missing values, and creates the package record. |
| `Configuration.create`, `local` | 139–185 | Registers named configs in a module-level map or creates ephemeral unregistered config. |
| constructor fields `name`, `handler`, `fallback`, `items`, `tags`, `options`, `nodes`, `preprocessors`, `postprocessors`, `initMethod`, `configMethod`, `priority`, `parser` | 205–229 | Stores package definition and merges four default handler keys with supplied handlers. |
| getters `init`, `config` | 236–247 | Return the callback slot from an optional priority pair or null. |
| module field `maps`; `ConfigurationHandler.set`, `get`, `keys` | 250–279 | Module-global package registry. `set` overwrites; `get` returns absent as undefined; `keys` returns the Map iterator. |
| `ParserConfiguration.initMethod`, `configMethod`, `configurations`, `parsers` | 293–313 | Ordered callback queues, priority-sorted package records and allowed parser names. |
| `handlers`, `items`, `tags`, `options`, `nodes` | 320–348 | Collated package data consumed by parser and TeX factories. |
| constructor | 356–367 | Copies/reverses requested packages, adds them to prioritized storage, then appends in priority iteration order. |
| `init`, `config` | 372–386 | Executes init/config callback queues; after config callback, attaches each package's pre/post filters. |
| `addPackage`, `getPackage` | 393–402, 437–446 | Validates parser target and adds known package with selected priority; warns and omits missing packages. |
| `add` | 412–429 | Dynamic `\require` path: append package, register priority, initialize, install node/item creators/tags/options/filters, invoke package config callback. |
| `append` | 454–467 | Merges callback queues, handlers/fallbacks, items, tags, options, and node definitions; source uses `priority || config.priority`. |
| `addFilters`, `warn` | 475–491 | Adds processors to TeX pre/post filters and emits MathJax warning. |

## Direct dependency and callback chain

`TexParser.ts` imports `HandlerType`, `UnitUtil`, `Stack`, `StackItemFactory`,
`Tags`, `TexError`, MML node classes, TeX `ParseInput`/`ParseResult`,
`ParseOptions`, `StackItem`/`BaseItem`, `Token`, `OptionList`, and `TexConstant`
(imports at lines 24–37). Main input dispatch is:

1. `Parse` obtains a code point and calls `parse(character,[this,c])`.
2. `parse` stages `saveI`, invokes the selected `SubHandler.parse`, then may
   invoke `updateResult`; the save cursor restore occurs only after successful
   handler/update calls.
3. A mapping callback can access the Parser object, mutate its cursor/current
   control sequence, read its `stack`/environment, call parse argument helpers,
   recursively instantiate parsers, and create MML nodes.
4. Stack-item checks and MML nodes call back into parser methods/factories.
5. `ParseOptions` retains recursive parser identity and node lists until
   `mml()` pops the active parser; filters use the saved root to prune lists.

`ParseOptions.ts` imports `StackItemFactory`, `Tags`, `SubHandlers`, TeX
`NodeFactory`, `NodeUtil`, `MmlNode`, `MathItem`, `TexParser`, constants,
options, `ParserConfiguration`, and `ColumnParser` (lines 24–35).
`Configuration.ts` imports handler maps, stack item/tag classes, option helpers,
FunctionList, TeX, PrioritizedList, and TagsFactory (lines 24–33). These are
observable runtime relationships, not merely type aliases: package callbacks
are run at init/config/parse time and can change registries/factories.

| Source file | Import lines |
|---|---:|
| `TexParser.ts` | 24–37 |
| `ParseOptions.ts` | 24–35 |
| `Configuration.ts` | 24–33 |

Important imported call targets with source locations (their hashes are in the
ledger below) are: `Stack` constructor/Push/Top/Prev at
`Stack.ts:48–55,84–90,123–154`; stack-item factory use and creation at
`StackItemFactory.ts:38` onward and `StackItem.ts:220–236`; dimension and space
helpers at `UnitUtil.ts:125,166`; tag reset/create at `Tags.ts:465,696`;
TeX-node creators at `NodeFactory.ts:78–130,158–177`; parser node properties
at `NodeUtil.ts:149–182`; `MathItem` as ParseOptions state at
`ParseOptions.ts:30,129–132`; and the owning TeX jax's parse-options/filter
pipeline at `tex.ts:153–183,190–229`.

## Existing native seams

The current declarations were compared directly; hashes are in the source
ledger below.

* `src/math/input/tex/types.ixx:7–24` provides `Property` aliases for Args,
  Attributes, Environment; `ParserContext` exposes only virtual current-CS
  get/set; `ParseAtom` carries Property/TokenRef/StackItemRef; `ParseInput`
  carries a ParserContext reference and token Property; ParseResult carries
  Property/StackItemRef/ParseSymbolRef; and callable bridge functions preserve
  a receiver and `std::any` rest arguments. This is a callback seam, not a
  concrete parser.
  The pinned `Types.ts:28–41` instead narrows `Args` to boolean/number/string/null,
  models Attributes/Environment as string-keyed records, uses
  `[TexParser,string]` for `ParseInput`, and uses void/boolean/StackItem/symbol
  for `ParseResult`; the native Property/struct/variant carriers are explicit
  host adaptations whose value-domain and omitted/void conventions need to
  be frozen against actual parser callsites. `ParseSymbolRef` itself preserves
  symbol identity by shared-pointer identity; distinct symbols with the same
  diagnostic description remain distinct.
* `token_map_types.ixx:7–22` declares abstract TokenMap operations and the
  CharacterMap RTTI marker. The concrete map families are not in this
  declaration. `map_handler.ixx:8–64` declares token-handler routing,
  fallback lists, and live handler keys. `handler_types.ixx:6–8` supplies
  PropertyList constants for configuration and dispatch keys.
* `token.ixx:6–16` provides a Property-backed Token. Existing `macro.ixx`
  similarly provides a Macro record. Neither supplies the TextParser cursor,
  TeX stack semantics, or mapping implementations.
* `mml_factory.ixx:6–16` and `tree/node_factory.ixx` provide native MML class
  creation. `mml_node.ixx:11–128` provides MML node types, child/parent and
  property operations, attributes, TexClass, and MML factory hooks. This can
  represent parser output, but the imported TeX `NodeFactory` wrappers and
  StackItem/MML conversion logic are not present as a TeX parser API.

## Blocking behavior versus declaration organization

These source files refer to each other: `TexParser` owns `ParseOptions` and
calls its parser-stack methods; `ParseOptions` stores `TexParser[]`; package
callbacks in `Configuration` accept `ParserConfiguration`; that class stores
`Configuration` records and invokes callbacks. TypeScript permits these module
cycles. The native `types.ixx` currently avoids a direct Parser/ParseOptions
cycle by defining the abstract `ParserContext` protocol. This is a declaration
dependency issue for root to freeze; it is separate from implementing the
scanner, stack, or callbacks.

The existing seam is insufficient to implement the pinned behavior by itself:
most TokenMap callbacks need parser string/cursor, lookup/contains, stack,
argument scanners, node creation, environment and recursive parser access,
whereas `ParserContext` presently exposes only `currentCS`. Source callbacks
also distinguish `StackItem`, `MmlNode`, `Token`, void, booleans, and unique
symbols. Native `StackItemContext` is empty, which leaves the StackItem
protocol unrepresented here; `ParseSymbolRef` is a
`shared_ptr<const ParseSymbol>`, so it preserves symbol identity and its
`description` field is diagnostic text only. Any remaining mapping of these
values is a root-frozen interface question, not a parser-algorithm choice.

The algorithmic implementation still absent from the native input tree is the
concrete TexParser/ParseOptions behavior: UTF-16 cursor and codepoint loop,
control-sequence and argument scanners, braces/brackets/dimension/delimiter
parsing, handler dispatch sequencing, recursive subparsers, Stack and
StackItem operations, `updateResult` LATEX bookkeeping, node-list liveness,
package configuration, TeX NodeFactory/Tags/ColumnParser integration. Current
MML node operations do not supply these input algorithms. This report does not
choose their module boundaries.

There are also external dependencies with no native counterpart found in
`src/math`: TeX `Stack`, `StackItemFactory`, `StackItem` subclasses, `Tags`/
`TagsFactory`, TeX `NodeFactory`, `ColumnParser`, and `UnitUtil`. Their use is
visible in `TexParser.ts`, `ParseOptions.ts`, `Configuration.ts`, and the
imported source files below. Root may choose their implementation scope after
freezing parser types; this audit does not replace them with generic state or
invent a fallback path.

## Source and native hash ledger

Line references above refer to these exact source snapshots:

| File | SHA-256 |
|---|---|
| `.reference/MathJax-src/ts/input/tex/TexParser.ts` | `9afdfff070ec7ed7115a1afe97048b3d1f1befae5b75e8ec963e0456685ee167` |
| `.reference/MathJax-src/ts/input/tex/ParseOptions.ts` | `ab60a07dbb24040c0fb9bd67b942755ffc5b0b18980abf1938736d8d90b22209` |
| `.reference/MathJax-src/ts/input/tex/Configuration.ts` | `8184716579abf3ffa14c64413d276dab4fc721e3e3f43afb0cdccbe2ab782719` |
| `.reference/MathJax-src/ts/input/tex/HandlerTypes.ts` | `5b9d091fc53640603956ab4ae39f146735621def9c36f312f8abd00afb043cfc` |
| `.reference/MathJax-src/ts/input/tex/UnitUtil.ts` | `90f461310aa6d07b494cda98dc5398e9737231f046e20099ea0f5c19fa24c66e` |
| `.reference/MathJax-src/ts/input/tex/Stack.ts` | `36be7583b14111961367e9e3f1dd5911456d235b096a2fcee62f438d37c7e11a` |
| `.reference/MathJax-src/ts/input/tex/StackItemFactory.ts` | `229bf0f6bd58642bff86102bb8fb6fd2b9a61d3f52ad31ce738f3f2cefa2e405` |
| `.reference/MathJax-src/ts/input/tex/Tags.ts` | `5a73a8d16f1d3c87ece167fc06dac0c323ea05a519e5ee5ea9aadbb76cef9a5d` |
| `.reference/MathJax-src/ts/input/tex/TexError.ts` | `30f398ff1910edecf029b1e4dde846bef10990539f60c43a4ef2da8783ba7ac8` |
| `.reference/MathJax-src/ts/core/MmlTree/MmlNode.ts` | `cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656` |
| `.reference/MathJax-src/ts/input/tex/Types.ts` | `924d0b48bf7b5fdb0ccc995171fbdfb001bc3ac51a0af1409bb8c1d953430927` |
| `.reference/MathJax-src/ts/input/tex/StackItem.ts` | `ac138d321e1efbc54afbd961a4b6bfa24f7d0923eddba1c8d891dee9424602b5` |
| `.reference/MathJax-src/ts/input/tex/Token.ts` | `6b4c0e42f277daebe8e676afc3b4d3b6083ebc87939ef8203672a831be1595bf` |
| `.reference/MathJax-src/ts/input/tex/NodeFactory.ts` | `bf20e8d4f7ed43450df42d65556ccbceea1469541b214c37544f180aa205c840` |
| `.reference/MathJax-src/ts/input/tex/ColumnParser.ts` | `4bffbf64ef23395d551d0e190043222bfffe7fd71e2417e3fb8008c5f3377d5c` |
| `.reference/MathJax-src/ts/util/Options.ts` | `b810ffc09864eb8ed9295d829b229194d7e38aaac6c8f01525bc1a4b190a61e3` |
| `.reference/MathJax-src/ts/util/FunctionList.ts` | `d4106c42268e0d7cb3549f8ffdbeb5bab87eae6daffae7314d5e724045a6918f` |
| `.reference/MathJax-src/ts/util/PrioritizedList.ts` | `a700459f984ba1a5f18bf0e7dc183c242961b94c33587a9aa81bbaacf97180c8` |
| `.reference/MathJax-src/ts/input/tex/MapHandler.ts` | `7cacdaef3ed1df90b4f05cc0b3183a2956ae8f848f20ae8833932faa4efcb2a2` |
| `.reference/MathJax-src/ts/input/tex/TexConstants.ts` | `d9380972dd21333d4699ff351f6af0a589f7c1f9096f0918dc1152287aed37b5` |
| `.reference/MathJax-src/ts/input/tex.ts` | `151d3c73697bfbf874001549f6e0c80dbebeabba2b36ab50501a4ffd7753bf06` |
| `.reference/MathJax-src/ts/core/MathItem.ts` | `c1c7308320ae15514fd1114c3f94642a4e8a8268bfe6f68192e8a557829626ce` |
| `.reference/MathJax-src/ts/input/tex/NodeUtil.ts` | `bd7e048c49cf042cb5878c6a73d049ae49e7d6437fb4ba975651dae5a5125cc2` |

| Native file | SHA-256 | Relevant declarations |
|---|---|---|
| `src/math/input/tex/types.ixx` | `1b8e043de12a9e31f80765f46dbd9ab6e4466bf975eedde7a40b965be3f8fb97` | 7–24 |
| `src/math/input/tex/token_map_types.ixx` | `c2d530d5aa024f68c2a089d7bd2fcc6632ec730c78ebb6661920a95ba244fd89` | 7–22 |
| `src/math/input/tex/handler_types.ixx` | `65e5b4d28796fbe05f287a7ea6005e17c1c6da93f6bc8b64800be907bfe23e4c` | 6–8 |
| `src/math/input/tex/map_handler.ixx` | `4a56c19e67623c5c2d85b902a9de60f2e447488553e3b86e8cf9bb7121976f3e` | 8–64 |
| `src/math/input/tex/token.ixx` | `74f101ff57b837bf7ff6f49f9741d0020447d130169de2be686dfa710d3b9a07` | 6–16 |
| `src/math/core/mml_tree/mml_node.ixx` | `36f47c27f389ac337c8f08dd890d066ba362f8ed9e51ea9ad53cad999ceeba3e` | 11–128 |
| `src/math/core/mml_tree/mml_factory.ixx` | `9a7591bcc8a2eaad8c02b511047615957568295e4ee47e1013c09a9a559ecbe8` | 6–16 |
| `src/math/core/tree/node_factory.ixx` | `c39d713686b348b1f73d1aab5f2230c9dd218de463b3d67218d8f322c4539e92` | 6–39 |
| `src/math/util/function_list.ixx` | `6b8b99c2b6002fed6e0f56236e09f10fdb182f830d97a4603de3febf14f46446` | 6–22, `FunctionList` callback queue declaration. |
| `src/math/util/prioritized_list.ixx` | `f76416dac7f5d6028e4438a9fae90703051b961f746d963e8f2d4dcaf2d10bdd` | 21 onward, priority-ordered storage declaration. |
