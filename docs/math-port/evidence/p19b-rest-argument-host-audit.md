# P19b raw rest-argument host audit

Read-only audit of MathJax 4.1.3 `MacroMap.parse` and `CommandMap.parse` against
the installed npm CJS output, plus the existing native Property/host helpers.
No implementation, interface, or manifest changes were made.

Root follow-up: the native hashes below record the pre-refactor audit revision.
Root subsequently moved the generic Property operations and their TLS unit,
without changing bodies, to util/property_host.ixx/cpp. MML keeps only its
inheritance defaults and shared-operation import. See property-host-boundary.md
for verified full-build/core-test evidence and the new responsibility boundary.

## Sources inspected

| Source | SHA-256 |
| --- | --- |
| `.reference/MathJax-src/ts/input/tex/TokenMap.ts` | `0209895b13569008b2129b61c88050e7c70e70ce9477d234b9a6e303a86a738e` |
| `.reference/MathJax-src/ts/input/tex/Types.ts` | `924d0b48bf7b5fdb0ccc995171fbdfb001bc3ac51a0af1409bb8c1d953430927` |
| `.reference/MathJax-src/ts/input/tex/Token.ts` | `6b4c0e42f277daebe8e676afc3b4d3b6083ebc87939ef8203672a831be1595bf` |
| `tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/TokenMap.js` | `9e3f086080a6ab052b68b22a5d9ee3dbc5e17d7b0c03a31e789664b01df36cf7` |
| `tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/Token.js` | `d317b026fc1ae0ff93d6939a15041d225385a826f8c33115284464bc2f527e35` |
| `src/math/input/tex/types.ixx` | `f90f4a9264155af739dbe135efd4ccf4507d9194386ceec2486908f52d7400da` |
| `src/math/input/tex/macro.ixx` | `fcd900b1311b5768911df178818630927be301a0fc8d826c94ef0284559113ea` |
| `src/math/input/tex/macro.cpp` | `80615de3df6f4edd396ea598dbee199babb25e73ab0610ebb6042ee91201c9a` |
| `src/math/core/mml_tree/mml_node_host.ixx` | `9e143f6812d490dea157774b2ecc5a6ae670c922b998c75de1f297329c9dc308` |
| `src/math/core/tree/property.ixx` | `3c49619f8600b15f7738a8ea2f253a35594c75785aef97163f998bc592c07701` |
| `src/math/util/utf16.ixx` | `aea8e9b8f2b42fc3aad5b47c7a2f9be59b5e8aeacfaac81c3049054ecc55d54e` |
| `src/math/util/js_numeric_input.ixx` | `f27bc4eed4a0b5120fb17d7bbc937c62ed7bb7a124d98e0da0aac8b38878dc5f` |
| `src/math/util/function_types.ixx` | `98ba8b43e2a9ac778cff22319e7405b76abef3a0b843642e24cf2523597ffcfe` |
| `src/math/util/function_list.cpp` | `79d07da282d2673f86afe2ce0550490bb75b74ad199a4595296bc7cc9a6a2965` |

The native `Macro` constructor retains its `Property args` handle. Only
`Missing{}` is replaced by a fresh empty `PropertyArray`; explicit null, scalar,
record, and array arguments remain as supplied. Therefore normal construction
does not leave an undefined args value, but explicit null reaches CJS parsing
unchanged.

## CJS evaluation order and expansion

`MacroMap.parse([env, token])` first applies the emitted `__read(input, 2)` to
destructure the input. With an iterable input it reads at most two iterator
values and closes the iterator if it stopped before `done`; without a truthy
iterator it returns the input unchanged and destructuring then gets properties
`0` and `1`. Nullish input fails while reading `Symbol.iterator`. It then calls `lookup(token)`, followed by
`parserFor(token)` (which looks up the macro again). A missing/falsy macro or
falsy parser returns `null`. Otherwise CJS evaluates the parser call arguments
as `__spreadArray([env, macro.token], __read(macro.args), false)` and then calls
`parser.apply(void 0, arguments)`. The complete argument expansion finishes
before the parser runs. A truthy but non-callable parser fails at `.apply`.
Parser exceptions propagate. `parseResult` maps only an `undefined` callback
result to `true`; all other values pass through unchanged.

The emitted `__read(o, n)` first reads `o[Symbol.iterator]`. Null and undefined
throw at that property read. If the iterator property is falsy, it returns `o`
unchanged. Otherwise it calls the iterator method with `o` as receiver and
collects yielded `.value` entries until done; the args call omits `n`, so it
reads the entire iterator. Iterator/property errors propagate, with the
emitted iterator-close path running on abrupt completion. An iterator method
that is truthy but non-callable throws at `.call`.

`__spreadArray(to, from, false)` is called with three arguments, so its copying
branch is skipped and it returns `to.concat(Array.prototype.slice.call(from))`.
Thus the no-iterator fallback is array-like: `slice` reads/coerces `length`
first, then checks each index for presence and reads present values in ascending
order. Its length uses ECMAScript `ToLength`; holes stay holes through slicing,
then become `undefined` arguments when `apply` constructs the call. Inherited
numeric properties count as present and are read. The result of iterator mode
is already a dense array, then is copied through the same `slice` path. A
generic record with no `Symbol.iterator` and a `length` field is expanded by
that length; a record without length contributes no rest arguments. This
lowering does not use `concat` directly on the original object.

The source-visible order for array-like objects is observable: read
`Symbol.iterator`; if falsy, get `length`; coerce the length; for each index in
order perform the slice presence check and then get that index. The actual CJS
Proxy observation was `get Symbol.iterator`, `get length`, `length` getter,
`length.valueOf`, `has:0`, `get:0`, `has:1`, `get:1`. No JavaScript getter or
Proxy behavior is implemented by the current native Property model.

## Observed raw values

These small probes called the actual installed CJS `MacroMap.parse` with a
callback that captured its rest arguments:

| Raw `Macro.args` value | CJS callback rest arguments / outcome |
| --- | --- |
| sparse two-slot array, hole at 0 and own `tail` at 1, prototype value at 0 | `prototype-value`, `tail` (the array iterator performs inherited indexed Gets) |
| string `A𝄞e` + combining acute | `A`, `𝄞`, `e`, combining acute (Unicode code points; the supplementary character remains one argument) |
| boxed string `𝄞` with `Symbol.iterator = null` | high-surrogate string, low-surrogate string (fallback slice reads UTF-16 indexed units) |
| record `{0: "zero", length: 2}` | `"zero"`, `undefined` |
| record `{0: "hidden"}` without length | no rest arguments |
| number `42` | no rest arguments (`length` is undefined after boxing, so ToLength is zero) |
| boolean `false` | no rest arguments for the same reason |
| explicit `null` | TypeError while reading `Symbol.iterator`; callback is not called |
| args forcibly changed to `undefined` after `Macro` construction | TypeError at `__read`; ordinary `Macro` construction would instead default undefined to `[]` |

`Missing{}` is the native representation of source `undefined`, but P17's
`Macro` constructor consumes Missing as the source default and stores an empty
array. Explicit `PropertyNull{}` remains null and exercises the observed
TypeError path. A later raw-rest seam must distinguish these construction-time
behaviors from values inside arrays, where a hole or Missing slot is passed as
an undefined argument.

## CommandMap currentCS partial failure

`CommandMap.parse` performs the same `__read(input, 2)`, macro lookup, and parser
truthiness check. It then reads `env.currentCS`, assigns `'\\' + token`, calls
the parser with `'\\' + macro.token` and the expanded raw args, restores the
saved currentCS only after the parser returns, and finally applies
`parseResult`. There is no `try/finally`. If args expansion throws or the
callback throws, currentCS remains the newly assigned command. The direct CJS
probe with `Macro.args = null` failed before callback and left currentCS as
`\\x`; a callback throwing `Error("boom")` also left it as `\\x`. Getter,
setter, token coercion, and parser exceptions follow their source evaluation
order and propagate. Native `ParserContext` has explicit currentCS get/set
methods, so host code can preserve those partial updates without snapshotting
or restoring on exceptional exit.

## Existing native host surface and limits

`mml_node_host.ixx` is named under the MML tree, but its generic adapters import
only `tree::property`, `tree::factory_types` (for the native TypeError carrier),
and `js_number`, `js_numeric_input`, and `utf16`; the helper implementations
do not use `MmlNode` or another MML type. Its `get`, `truthy`, `nullish`,
`primitive`, `string`, `number`, `text`, and `debugText` functions are existing
shared candidates for the lower generic host layer. `get` already handles
Property records/prototypes, sparse array numeric slots and length, string
UTF-16 indexing/length, and scalar property reads. `primitive`/`string`/`number`
reuse the accepted lossless Property graph and ECMAScript numeric/string
conversion modules. `utf16::codePoints` already combines valid surrogate pairs
and leaves lone surrogates as individual code points, matching the default
String iterator's yielded values.

These helpers do not by themselves implement CJS `__read`/`__spreadArray`.
`Property` keys are strings; the graph has no `Symbol.iterator` or
`Symbol.isConcatSpreadable` key, arbitrary getter, Proxy, or callable iterator
representation. `host::get` string-indexing returns one UTF-16 unit, whereas
the ordinary string iterator yields code points. The existing
`PropertyArray::spread()` simply copies array slots; it does not select an
iterator, preserve custom iteration, apply array-like `ToLength`, or model
custom iterator closing. `PropertyList::has` and `PropertyArray::has/get` can
help represent own/inherited indexed presence for the native data model, while
`host::number` can supply numeric conversion for a supported `length` value;
neither supplies observable accessors or symbol-keyed behavior.

`types.ixx` already provides `ParseMethod`, `ParseCallableRef`, `ParseAtom`,
`ParseRest = span<const any>`, and
`ParseCallable(ParserContext&, const ParseAtom&, ParseRest)`. `Macro.args()`
returns its retained raw Property graph. The exact host bridge from that graph
to `ParseRest` must retain each raw Property value and the CJS expansion cases
above. The general `FunctionList` and `Property` helpers are typed native
services; they are not JavaScript evaluators.
