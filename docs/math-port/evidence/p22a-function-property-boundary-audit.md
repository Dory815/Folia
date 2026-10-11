# P22a: callable values in the Property graph — dependency and callsite audit

This is a read-only inventory of the existing native boundary. It does not
freeze the new type layout or authorize implementation. The specific proposal
under review is: move `Missing` from `folia.math.core.tree.property` to
`folia.math.util.object_types`; make `callable_types` depend only on
`object_types` and `std`; then have `property` export-import `callable_types`
and add the existing `FunctionRef` as a `Property::Value` alternative.

## Dependency direction

Current imports form a direct cycle if `property` begins importing
`callable_types`: `callable_types.ixx:2–4` currently imports both
`object_types` and `core.tree.property`, while `property.ixx:1–3` would then
need `callable_types` to name `FunctionRef`. `callable_types` uses `Missing`
in `FunctionReceiver` (`callable_types.ixx:7`) and defines `FunctionRef` at
line 26. `object_types.ixx:1–7` currently owns only `NativeObject` and
`NativeObjectRef`; `Missing` is presently declared at `property.ixx:8`.

Under the proposed dependency change, the observed type dependencies can be
ordered `object_types -> callable_types -> property`: `Missing` and
`NativeObjectRef` are declared by `object_types`; callable receiver/body/ref
types use only those and standard-library types; `Property::Value` can then
name both `FunctionRef` and the existing Property arms. This removes the
module cycle without moving `Function` bodies or creating a second callable
identity. `function_types.ixx` can continue to export-import callable and
property APIs. This is a dependency observation, not an approval of an exact
export/import declaration.

Direct callable-module dependents found by repository search:

| File | Relationship |
| --- | --- |
| `src/math/util/function_types.ixx:3–6` | Re-exports `callable_types`; also imports `property`, prioritized-list, and promise APIs. |
| `src/math/input/tex/parse_callable.cpp:6` | Imports it directly for `FunctionRef`, receiver validation, and callback invocation. |
| `src/math/input/tex/types.ixx:4` | Imports `function_types`; `ParseCallableRef` aliases `FunctionRef`, and `ParseMethod` aliases `FunctionValue` (`:25–26`). |
| `src/math/util/method_config.ixx:3` | Imports `function_types`; stores `FunctionValue` slots. |
| `src/math/util/function_list.ixx:3` and `function_list.cpp` | Uses `FunctionValue` and `FunctionRef` through function types. |

The declarations in `token_map_types.ixx` and `map_handler.ixx` reach callable
types through the TeX type/function-list imports; they do not introduce a
second `Function` class.

## Property variant and assumptions

`Property::Value` is declared at `src/math/core/tree/property.ixx:23–25` as
`Missing`, `PropertyNull`, `bool`, `double`, `std::string`, `BuiltinProperty`,
and `shared_ptr<PropertyStorage>`. Its defaulted equality (`:40`) delegates
to variant equality. A `FunctionRef` alternative would therefore compare by
the existing `shared_ptr<Function>` identity, consistent with the callable
identity already used in `FunctionValue`; it must remain a distinct arm from
`shared_ptr<PropertyStorage>`.

The scan found **no `std::visit` over `Property::Value` and no exhaustive
switch over its alternatives** in `src/math` or `tests/math`. The visits in
`map_handler.cpp:35,50,68` and `parse_callable.cpp:26` are over the separate
TeX `ParseResult` variant. The option-object visits/value tests use
`options::Value`, not `Property::Value`. Thus there is no in-scope visitor
that needs a new callable branch solely to remain exhaustive.

Production Property readers are mostly partial `get_if`/`holds_alternative`
chains. The relevant observable behavior if a function-valued `Property`
reaches them is:

| Consumer | Existing behavior relevant to a new arm |
| --- | --- |
| `property.ixx:111–119, 151–160, 217–226, 235–255` | Object/record detection recognizes only `PropertyStorage`; arrays and their spread/get copy Property values. A callable value should remain non-record while copying/equality retain its pointer. `keys()` enumerates stored keys, not value kinds. |
| `property_host.ixx:11–16` | `truthy` falls through to `true` for unrecognized non-nullish values, so a non-null FunctionRef is already naturally truthy. An empty `FunctionRef` stored as an arm would also fall through as true unless its null representation is explicitly accounted for; the current type boundary does not define that mapping. |
| `property_host.ixx:64–89, 91–108` | `primitive`/`string` know PropertyStorage and `BuiltinProperty`, not actual FunctionRef. A direct function-valued Property reaches the “cannot convert” or “unbound native string conversion” TypeError path; function `toString`/`name` behavior is not implied by adding identity storage. |
| `property_host.ixx:123–190` | `get` handles records, arrays, strings, numbers, and builtin marker identities. A FunctionRef currently falls through to ordinary builtin lookup; it does not expose actual function own metadata or Function.prototype properties. This audit does not propose synthetic methods or metadata. |
| `property_host.ixx:194–207` | `ownKeys` returns record keys and string indices; `objectAssign` copies values with `get`. A stored callable is preserved as a value in an ordinary record, while key enumeration is independent of that value. |
| `property_host.ixx:209–224` | Assignment to record slots keeps a Property value; numeric conversion calls primitive/string handling, so function numeric coercion is not provided by the new arm alone. |
| `argument_expansion.cpp:52–92` | Source-style array/array-like expansion gets each element and copies it into a vector. A callable element can retain identity through PropertyArray reads/spread; expansion does not itself invoke or coerce it. |
| `mml_bindings.cpp:20` | Checks specifically for string-valued MML binding input; a callable arm is not mistaken for a string. |
| `action_host.ixx:8`, `leaf_host.ixx:17–20, 26–42`, `table_host.ixx:24–30`, `mo_host.ixx:16–25`, `mfenced.cpp` | These are type-specific string/array/node readers. Callable values follow their existing unsupported-type branches; no exhaustive Property dispatch exists in these helpers. |

The `std::get` occurrences located in math code/tests are generally applied
to a *different outer variant* (`FunctionDefinition`, `FunctionValue`,
`ParseResult`, `ParseAtom`, `TokenLookup`, an option descriptor), or to known
fixture values such as a numeric/string tree property. The generated
`reference_*` `deep(Property, Property)` helpers use `PropertyList::record`,
`PropertyArray::array`, then `Property::operator==`; for a callable arm the
existing final equality path will observe shared-pointer identity. No test
helper found treats the Property variant as a closed visitor.

## Configuration and tuple data paths

Pinned MathJax `ts/util/FunctionList.ts:26–28,42–60` defines each item as
either a function or `[function, priority]`; `addList` reads tuple slot 0 as
the callback and slot 1 as the optional priority. Native
`function_types.ixx:10–18` currently models this with
`FunctionValue = variant<Property, FunctionRef>` and a distinct
`FunctionTuple::item` field; `function_list.cpp:75–91` branches on that typed
definition and reads its item before priority. Therefore callbacks are
representable in the *typed FunctionList API already*, but a raw
PropertyArray/PropertyList representation of source tuples cannot currently
store a FunctionRef in slot 0. If configuration tuples cross this shared
Property graph (including processor tuple data), the new arm is the affected
storage path; this report makes no recommendation about replacing the typed
FunctionDefinition interface.

For TeX configuration, pinned `Configuration.ts:207–227` stores `handler`
and `fallback` separately, and `:462` passes both to handlers. Native
`map_handler.ixx:8–9` maps these to `HandlerConfig = PropertyList` and
`FallbackConfig = MethodConfig`. `SubHandlers::add` in `map_handler.cpp:246–257`
enumerates handler keys and reads map arrays from the PropertyList, while
retrieving fallback methods from MethodConfig. `method_config.ixx:9–29`
stores `FunctionValue` per key with prototype lookup, and `map_handler.cpp:62–65`
checks ParseMethod truthiness across its Property/FunctionRef alternatives.
Thus the existing fallback route already stores callback identity in the
shared FunctionRef representation, but it bypasses `Property::Value`.
The current `src/math/input/tex` tree contains the handler/type constants and
the native routing API, but no package configuration tables that populate
these values. Consequently, callable Property assignment in future concrete
configuration records is an identified boundary requirement, not an existing
native config-data callsite.

The concrete Property-graph operations relevant to configuration are:

* **Assignment and get:** `PropertyList::setOwn/set/get/getOwn/assign` and
  `PropertyArray::set/push/get/spread` copy Property values. The new function
  arm must preserve the same FunctionRef; no body copy is needed to preserve
  identity.
* **Identity:** default Property equality will compare shared pointers, as
  `FunctionValue` and current callable tests already do. Do not compare
  callback bodies or textual labels in place of pointer identity.
* **Truthiness:** source function objects are truthy. Current fallback
  `truthy` handles non-null FunctionRef arms as truthy, but an empty native
  shared pointer requires an explicit representational policy before claiming
  null parity.
* **Key enumeration:** PropertyList keys and object spread enumerate own keys
  independently of stored value type. Adding a callback value must not cause
  it to appear as a property key.
* **Array copy/slice:** `PropertyArray::spread` and `arguments::expand` copy
  indexed values; callback slots should survive by identity. They are not
  called by slicing/spreading.
* **FunctionList tuple slot 0:** source expects an actual callable there. The
  existing typed tuple can carry FunctionRef; if the source tuple is held in
  the generic Property graph, slot 0 specifically needs the new Property arm
  for storage and subsequent retrieval without degrading identity.

The scope of these statements is storage and callsite impact only. A
FunctionRef Property arm alone does not make PropertyHost's generic function
property lookup, coercion, serialization, or invocation semantics equivalent
to JavaScript functions.

## Evidence files and hashes

Hashes below identify the native snapshot inspected for this report.

| File | SHA-256 |
| --- | --- |
| `src/math/core/tree/property.ixx` | `3c49619f8600b15f7738a8ea2f253a35594c75785aef97163f998bc592c07701` |
| `src/math/util/object_types.ixx` | `96239a2030b405cf0a71f74065dd3554b5ca1519e75f0ecd7d5463fde66e037f` |
| `src/math/util/callable_types.ixx` | `fbda254cbb10b669eca371733f41d2533742020adbea213be09da3b36eb5deee` |
| `src/math/util/property_host.ixx` | `26ea3728df7d9fdb39daa4570a4d23c7e76334bedf7c21b4250a9b0d596924ad` |
| `src/math/util/function_types.ixx` | `c0da936a52e128d4a0619acbee156eb6ae93b86d7a601c514f499435e17375bc` |
| `src/math/util/function_list.ixx` | `6b8b99c2b6002fed6e0f56236e09f10fdb182f830d97a4603de3febf14f46446` |
| `src/math/util/function_list.cpp` | `408925fe5861f1d9ccbd761db267c89133150a5de3b23ffc96d2fe90430da7cd` |
| `src/math/util/method_config.ixx` | `8ebbc2911cfabd512ce29bb2b5f9e47af7ae06a81b90bce91863419ec361cf6e` |
| `src/math/input/tex/types.ixx` | `1b8e043de12a9e31f80765f46dbd9ab6e4466bf975eedde7a40b965be3f8fb97` |
| `src/math/input/tex/token_map_types.ixx` | `c2d530d5aa024f68c2a089d7bd2fcc6632ec730c78ebb6661920a95ba244fd89` |
| `src/math/input/tex/map_handler.ixx` | `4a56c19e67623c5c2d85b902a9de60f2e447488553e3b86e8cf9bb7121976f3e` |
| `src/math/input/tex/map_handler.cpp` | `484dcb1af95609c3f80fcba857dcc5857b2c5fefb241a3d731f50f01a711cece` |
| `src/math/input/tex/parse_callable.cpp` | `36bcf222c3bbcd03685817025d2e962eb2015c7724e8dbd8a214f233fd1bdca1` |
| `src/math/input/tex/macro.ixx` | `fcd900b1311b5768911df178818630927be301a0fc8d826c94ef0284559113ea` |
| `src/math/input/tex/macro.cpp` | `80615de3df6f4edd396ea598dbeea199babb25e73ab0610ebb6042ee91201c9a` |
| `src/math/util/argument_expansion.cpp` | `cf6592cf00e7508c3bfbd6a82ee15fb97e0348d1a8c157f9b5ffe492122e749a` |

Pinned upstream callsite references inspected:

| File | SHA-256 |
| --- | --- |
| `.reference/MathJax-src/ts/util/FunctionList.ts` | `d4106c42268e0d7cb3549f8ffdbeb5bab87eae6daffae7314d5e724045a6918f` |
| `.reference/MathJax-src/ts/input/tex/Configuration.ts` | `8184716579abf3ffa14c64413d276dab4fc721e3e3f43afb0cdccbe2ab782719` |
| `.reference/MathJax-src/ts/input/tex/MapHandler.ts` | `7cacdaef3ed1df90b4f05cc0b3183a2956ae8f848f20ae8833932faa4efcb2a2` |
