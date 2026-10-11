# P22 TokenMap native-boundary audit

Read-only prerequisite report. No implementation, test, build, replay, or
contract change is authorized by this audit. The complete pinned source is
`ts/input/tex/TokenMap.ts`, SHA-256
`0209895b13569008b2129b61c88050e7c70e70ce9477d234b9a6e303a86a738e`.
Its installed CJS counterpart is `tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/TokenMap.js`, SHA-256
`9e3f086080a6ab052b68b22a5d9ee3dbc5e17d7b0c03a31e789664b01df36cf7`.
The CJS bundle is derived from the pinned TS and confirms the effective bare
calls and spread lowering; it is not a replacement source authority. The root
member-ledger AST reports 23 inventoried rows: 22 implemented methods,
accessors, constructors, or module functions, plus the initialized
`AbstractParseMap.map` field. Abstract declarations and the `TokenMap`
interface are declarations rather than executable ledger rows.
`Types.ts` is SHA-256
`924d0b48bf7b5fdb0ccc995171fbdfb001bc3ac51a0af1409bb8c1d953430927`;
the related pinned `MapHandler.ts` is SHA-256
`7cacdaef3ed1df90b4f05cc0b3183a2956ae8f848f20ae8833932faa4efcb2a2`.

## Source member inventory and behavior-bearing order

| Source member | Lines | Behavior and observable order |
|---|---:|---|
| `parseResult` | 78–80 | Converts only `undefined` to `true`; passes false, null, objects, and symbols through unchanged. |
| `AbstractTokenMap.constructor` | 94–100 | Assigns `_name`, then `_parser`, then immediately calls `MapHandler.register(this)` from `super()`. |
| `name` getter | 104–106 | Returns `_name`; this is the property read by registration. |
| abstract `contains`, abstract `lookup` | 111, 141 | Subclass obligations; not concrete ledger methods. |
| `parserFor` | 116–118 | Calls virtual `contains(token)`; only on truthy result reads `this.parser`, otherwise returns null. |
| `parse` | 123–127 | Destructures input, calls `parserFor` first, then calls virtual `lookup` regardless of parser result; JS `parser && mapped` short-circuits the call; invokes the saved parser as a bare function and applies `parseResult`. |
| `parser` setter/getter | 129–135 | Setter replaces `_parser`; getter returns the current `_parser`. `EnvironmentMap` assigns this property after `MacroMap` construction. |
| `RegExpMap.constructor` | 155–163 | Calls `super(name, parser)` first (registering before `_regExp` is assigned), then stores the regex object. |
| `RegExpMap.contains` | 166–168 | Calls the live `_regExp.test(token)` method. Its receiver is the regex object and state such as `lastIndex` is observable. |
| `RegExpMap.lookup` | 173–175 | Calls `contains` again and returns the original token or null. Thus the inherited `parse` may call `.test` once through `parserFor` and again through `lookup`. |
| `AbstractParseMap.map` initialized field | 186 | A distinct JS `Map`, initialized after the base `super()` returns. It is not an ordinary object or `PropertyList`. |
| `AbstractParseMap.lookup` | 191–193 | Returns `Map.get(token)`, distinguishing missing/undefined from null at the JS level. |
| `contains` | 198–200 | Returns `Map.has(token)`, independent of the stored value. |
| `add` | 208–210 | `Map.set(token, object)` replaces an existing key's value without changing its insertion position. |
| `remove` | 217–219 | Deletes the key from the map. |
| `CharacterMap.constructor` | 235–250 | Calls `super` first; snapshots `Object.keys(json)`, then for each key reads `json[key]`, chooses `[value, null]` only when the value is a string or destructures the provided value, creates `Token(key, char, attrs)`, and calls virtual `this.add`. |
| `DelimiterMap.parse` | 260–262 | Calls `super.parse([env, "\\" + token])`; augmentation occurs before inherited parser/lookup dispatch. |
| `MacroMap.constructor` | 281–305 | Calls `super(name, null)` first, then resolves each `Object.entries(json)` value. Missing `functionMap` defaults to a fresh `{}`. String function names index `functionMap`; array values use index 0 as method and `slice(1)` as args; scalar values use an empty args array. Stores each `new Macro(key, func, args)` with `this.add`. |
| `MacroMap.parserFor` | 307–310 | Calls `lookup(token)` and, if the Macro object is truthy, reads its live `func`; otherwise returns null. |
| `MacroMap.parse` | 315–325 | Calls `lookup` into `macro`, then calls `parserFor` (which performs a second lookup); if both are truthy, reads `macro.token`, spreads current `macro.args`, invokes the parser as a bare function, and maps only undefined to true. |
| `CommandMap.parse` | 335–348 | Repeats the two lookup path. Reads/saves `env.currentCS`, writes `"\\" + token`, reads `macro.token`, spreads `macro.args`, and bare-calls parser. It restores currentCS only after normal return; exceptions leave the modified value. |
| `EnvironmentMap.constructor` | 366–376 | Calls the MacroMap constructor (including its registration and map population), then sets `this.parser = parser`. |
| `EnvironmentMap.parse` | 379–389 | Calls `lookup`, calls `parserFor` (second lookup), tests both, then invokes `this.parser(env, macro.token, envParser, macro.args)`. This is a member call with the exact EnvironmentMap receiver; `macro.args` is passed as one array argument, not spread. |

The source imports `Token`, `Macro`, and `MapHandler`; `Types.ts` (SHA-256
`924d0b48bf7b5fdb0ccc995171fbdfb001bc3ac51a0af1409bb8c1d953430927`) defines
`Args = boolean | number | string | null`, `Attributes` as a record, `ParseInput`
as `[TexParser, string]`, `ParseResult` as `void | boolean | StackItem | symbol`,
and `ParseMethod(parser, c, ...rest)` (lines 20–35). The native P17 types
deliberately widen values to the accepted Property graph and typed object
references, but still need to preserve the source result and receiver rules
above.

## Existing native boundary and concrete gaps

| Native file / identity | Current shape | Gap relevant to a faithful TokenMap port |
|---|---|---|
| `src/math/input/tex/token_map_types.ixx` (SHA-256 `c2d530d5aa024f68c2a089d7bd2fcc6632ec730c78ebb6661920a95ba244fd89`) | Abstract `TokenMap : NativeObject`; shared `TokenMapRef`; virtual name/parser getter+setter/contains/parserFor/parse/lookup. | `TokenLookup = variant<Property, TokenRef>` cannot return the actual `Macro` that `MacroMap.lookup` returns. A typed `MacroRef` alternative is required at this bridge, with the `SubHandler::lookup`/consumers preserving that shared identity. `TokenMap` is only a protocol: no concrete `AbstractTokenMap`, `RegExpMap`, parse-map, character, delimiter, macro, command, or environment class is present. |
| `src/math/input/tex/token.ixx` / `token.cpp` (header SHA-256 `74f101ff57b837bff7ff6f49f9741d0020447d130169de2be686dfa710d3b9a07`; implementation carries the source hash in its banner) | Token owns three `Property` handles and getters. | Sufficient for raw Token identity/attrs once a CharacterMap stores a `TokenRef`; does not supply map storage or the `MacroRef` lookup bridge. |
| `src/math/input/tex/macro.ixx` / `macro.cpp` (header SHA-256 `fcd900b1311b5768911df178818630927be301a0fc8d826c94ef0284559113ea`) | Macro owns token, `ParseMethod`, and shared raw args `Property`; only Missing becomes a fresh array. | Sufficient for Macro values and shared args after `TokenLookup` can carry `MacroRef`; does not implement map construction, spread invocation, or parseResult. |
| `src/math/input/tex/types.ixx` (SHA-256 `1b8e043de12a9e31f80765f46dbd9ab6e4466bf975eedde7a40b965be3f8fb97`) | `ParseMethod = FunctionValue`, `ParseResult = variant<Property, StackItemRef, ParseSymbolRef>`, `ParseAtom = variant<Property, TokenRef, StackItemRef>`, `ParseRest = span<const any>`. | There is no exported native `parseResult` adapter. Source `undefined` must become true; the current typed callable bridge maps an empty native any to `Property(Missing{})`. The map call sites must perform the source conversion without collapsing explicit false/null. |
| `src/math/input/tex/parse_callable.cpp` and `docs/math-port/callable-receiver-contract.md` | One Function/FunctionRef callback path. The receiver may be Missing or the same NativeObjectRef. | The receiver value vocabulary is sufficient, but `EnvironmentMap::parse` has no accepted way to recover its own `NativeObjectRef`: `TokenMap` is only a `NativeObject` marker, the virtual `parse` receives no self handle, `ParseInput` has no receiver field, and `NativeObject` does not inherit `enable_shared_from_this`. The source requires the exact EnvironmentMap receiver. A source-faithful call therefore needs an explicit identity access seam; a null/Missing receiver or a copied lookalike would be wrong. |
| `src/math/input/tex/map_handler.ixx/.cpp` (header SHA-256 `4a56c19e67623c5c2d85b902a9de60f2e447488553e3b86e8cf9bb7121976f3e`; implementation SHA-256 `484dcb1af95609c3f80fcba857dcc5857b2c5fefb241a3d731f50f01a711cece`) | `registerMap(TokenMapRef)` stores shared map identity by name; `TokenLookup` flows through `lookup`; `CharacterMapIdentity` gates `.char` handling in `contains`. | Immediate construction-time registration is not representable with the present `shared_ptr`-after-construction path; details below. `characterOf` currently has no MacroRef branch, though its current CharacterMap-only call path should continue to expect Token/raw-char results. |
| `src/math/core/tree/property.ixx` (SHA-256 `3c49619f8600b15f7738a8ea2f253a35594c75785aef97163f998bc592c07701`), `property_host.ixx` (SHA-256 `26ea3728df7d9fdb39daa4570a4d23c7e76334bedf7c21b4250a9b0d596924ad`) | Shared object/array graph with identity, holes, prototypes, ownKeys, property get/set and scalar coercions. | It models supported own data properties/arrays, not arbitrary getters, proxies, Maps, RegExp objects, or Symbol.iterator overrides. CharacterMap/MacroMap enumeration and indexed reads must not claim getter/proxy fidelity beyond this accepted typed domain. A JS Map needs actual Map semantics (has/get/set/delete and insertion order), not a plain PropertyList. |
| `src/math/util/argument_expansion.ixx/.cpp` (SHA-256 `eb6208a24292394615b495492e6e574b58e9e98188fc34a9af94255d82b78d70` / `cf6592cf00e7508c3bfbd6a82ee15fb97e0348d1a8c157f9b5ffe492122e749a`) | `arguments::expand(Property)` models the pinned CJS `__read`/`__spreadArray` path over supported arrays, strings, and array-like properties, preserving nested handles and inherited numeric reads. | It is a usable dependency for the configured ordinary Macro argument arrays, but it does not bind arbitrary custom iterators/accessors. Native Map parse must pass its expanded results as actual `std::any` rest slots and preserve Macro.args itself by alias. |
| `src/math/core/tree/factory_types.ixx` / `factory.ixx` (SHA-256 `25542be82bc00641c5f5880362aaae8f8c022b635df0914011bff33f10ba4033`; `587c552f55387e0a8b6b5a4cd35c3ac6fdb71197d4783f7aa01a2b0ff595286e`) | Generic Tree factory has constructor records and creator objects, `create`, and explicit `adopt(unique_ptr)` ownership. | This is a separate Tree product factory, not a TokenMap constructor host. Its construct callback returns a raw product before adoption; it does not hand an already-live `TokenMapRef`/shared control block into the base constructor or register a map during `super()`. It should not be assumed to solve TokenMap self-registration. |

The `map_handler.cpp` SHA in the table is exactly
`484dcb1af95609c3f80fcba857dcc5857b2c5fefb241a3d731f50f01a711cece`.

## Constructor-time identity and virtual observation

The TypeScript base constructor assigns its two parameter properties and calls
`MapHandler.register(this)` synchronously before returning to the subclass
constructor. Registration reads `map.name` once and stores the same object
identity in the live registry; duplicate names replace the value in place. In
the pinned class family, `RegExpMap` then assigns `_regExp`; `AbstractParseMap`
initializes its `Map`; `CharacterMap` and `MacroMap` then populate it; and
`EnvironmentMap` sets its parser after the MacroMap constructor returns.
The constructors therefore have observable order even where later calls are
only possible after construction.

Native `MapHandler::registerMap` correctly stores a `TokenMapRef` and calls
`name()` before searching/updating the registry. But C++ cannot obtain the
object's owning `shared_ptr` from an object while its base constructor is
running: the `enable_shared_from_this` weak owner is not initialized until
construction completes. Also, C++ virtual dispatch from a base constructor is
restricted to the base class, unlike a JavaScript property lookup on the
allocated derived object's prototype. Delaying `registerMap` until a factory
receives the completed shared object changes the required synchronous
`super()` point. Reusing the existing Tree factory does not fix either fact;
its `adopt` happens after its class constructor callback returns.

For the pinned built-in maps, `name` is the base getter and the static package
constructors do not subclass these map classes, so no observed package relies
on an overridden `name` getter. That does not make post-construction
registration source-equivalent: it still moves the registration point past
the base constructor and any source-visible subclass initialization. A future
contract must specify a construction/identity mechanism that preserves the
same map object and registration timing, and must separately decide whether
arbitrary native/user subclasses are in scope. No such mechanism exists in the
current TokenMap protocol or generic factory. This is an unresolved dependency,
not a proposal to silently defer registration.

## Calls, rest arguments, and engine boundary

The callable receiver contract already records the relevant routes:
`AbstractTokenMap`, `MacroMap`, and `CommandMap` invoke a saved local parser
with Missing receiver; `EnvironmentMap` invokes the parser member with the
same EnvironmentMap receiver. Native maps must use that shared FunctionRef,
not wrap it in another callback. The callback types can express the required
receiver, but the current TokenMap method boundary does not supply the
EnvironmentMap shared identity at that call site (see the table above).
Source's `parseResult(undefined) === true` is a separate conversion still
absent from the native input interface.

MacroMap and CommandMap spread the current Macro.args at invocation time; do
not pre-expand or replace the aliased args field during construction. The
accepted argument-expansion service can form the current argument vector for
supported Property arrays/array-likes. EnvironmentMap differs: it passes the
whole args array as one positional argument after the environment parser.
CommandMap's currentCS restoration is normal-return-only, matching the source;
scope-exit cleanup on exception would change behavior.

`RegExpMap` remains blocked by the independent regexp-engine gate. Its
`RegExp.test` receiver, global/sticky `lastIndex` mutation, Unicode behavior,
and double-test order are source requirements, not permission to substitute
`std::regex`, a pattern-specific classifier, or the experimental ignored
regexp candidate. The native TokenMap interface has no RegExp identity or
test service today. No engine or regexp contract is accepted by this audit.

## Exact native source hashes and status

The native `map_handler.cpp` SHA-256 is
`484dcb1af95609c3f80fcba857dcc5857b2c5fefb241a3d731f50f01a711cece`.
`callable_types.ixx` is
`fbda254cbb10b669eca371733f41d2533742020adbea213be09da3b36eb5deee`;
the generic Tree factory declarations are as listed above. No source or
native files were modified, and no tests/builds or
engine experiments were run. Root must resolve MacroRef, immediate
construction-time identity/registration, parseResult conversion and the
independent RegExpMap engine dependency before freezing an implementation
packet.
