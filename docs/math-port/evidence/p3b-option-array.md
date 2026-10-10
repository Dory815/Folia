# P3b array adaptation evidence

## Pinned source and source behavior

The source is `.reference/MathJax-src/ts/util/Options.ts`, SHA-256
`b810ffc09864eb8ed9295d829b229194d7e38aaac6c8f01525bc1a4b190a61e3`.
The exact source path and hash are pinned by
`docs/math-port/options-contracts.md` (revision 1 at dispatch, root revision 2 at
acceptance). The relevant method uses are:

| Native member | MathJax source evidence | Port behavior |
| --- | --- | --- |
| `Context::array` | `makeArray`, line 126; array copy in `copy`, lines 157–158; array replacement in `insert`, lines 254–257 | Allocates an Array-kind record with the real array prototype and a writable, non-enumerable, non-configurable own `length` descriptor. A nonzero requested length creates holes. |
| `Context::arrayLength` | Array length is observed by `insert` through array operations at lines 240–246 and by native array iteration | Reads the own data descriptor and enforces the uint32 range already guaranteed by the shared graph's define/set path. |
| `Context::arrayValues` | `insert` spreads `dval` and `sval[APPEND]` at line 246 | A host adapter for ordinary array spread. It performs `Get` at each index, so holes yield inherited values or `undefined`, and re-reads `length` for every iteration so getter-driven changes affect the live walk. It is not a JS iterator object or public `.values()` implementation. |
| `Context::arrayIndexOf` | `insert`, line 240: `sval[REMOVE].indexOf(x)` | Captures the initial length; visits only indices present through own or inherited properties; compares with strict `Value::operator==`, so `NaN` does not match and object/symbol references match by identity. |
| `Context::arrayFilter` | `insert`, line 240: `dval.filter((x) => ...)` | Captures initial length, checks `HasProperty` at each visited index, reads the current value, invokes the predicate, and appends matches densely to a new array. Mutations to later indices are observed; additions at or beyond the captured length are not visited. |
| Array prototype initialization | The source relies on built-in `Array.isArray`, spread, `indexOf`, and `filter` at lines 126, 157, 220–246, and 254 | Seeds the intrinsic array prototype's own `length` and registers non-enumerable writable/configurable identities for `values`, `indexOf`, and `filter`. `indexOf` and `filter` have native callbacks; `values` is opaque. |

`Options.ts` does not explicitly invoke `.values()`. Its `...array` spread
uses the standard iterator protocol implicitly. The frozen native contract maps
that source use to `arrayValues()` as a closed-profile host adapter. It returns
the values consumed by ordinary array iteration, including `undefined` for
holes, and checks the current array length on each iteration. It does not
construct an iterator, return `{value, done}` records, expose
`Symbol.iterator`, or promise support for custom iterables. The registered
`Array.prototype.values` identity is opaque and direct invocation raises the
native TypeError used for other unbound standard-library methods.

## Mapping to the shared native graph

`src/math/util/option_array.cpp` is an implementation unit of
`folia.math.util.option_types`; it includes the root-owned
`option_storage.hpp` after the module declaration and `import std;`. Array
elements, holes, extra string/symbol keys, and descriptor flags remain in the
root-owned `Record::properties`. No parallel vector or alternate array value
store is introduced. `Context::has` and `Context::get` provide inherited
index behavior and accessor invocation; `Context::define`/`set` in the object
unit provide index growth and length truncation. Array prototype methods are
identity-bearing native function records and have ordinary non-enumerable
prototype descriptors.

The `indexOf` prototype callback accepts the source-profile one-argument use
and also handles numeric/boolean/nullish `fromIndex` values. Its general
JavaScript `ToNumber` conversion for string and object `fromIndex` values is
outside this raw-TeX profile and raises TypeError. The public
`arrayIndexOf(array, needle)` adapter corresponds exactly to the Options.ts
call. `filter`'s prototype callback accepts a native function identity and
passes `(value, index, array)` plus `thisArg`; the public C++ `arrayFilter`
adapter accepts the host predicate type frozen in the interface. Arbitrary JS
callbacks, proxies, species overrides, and custom iterator protocols are not
part of the host API.

## Frozen coverage and remaining gaps

The root-owned `tests/math/reference_options.cpp` includes frozen witnesses in
`math_port_options_array_copy_and_holes`,
`math_port_options_append_remove_identity`, and
`math_port_options_malformed_sentinels`. They cover holes, own `undefined`,
extra own string/symbol properties, copied array descriptors, and strict
remove-list behavior for `NaN` and object identity. The Options.ts use sites
exercise the one-argument `indexOf`, filter predicate, and array spread paths.

Array copying is handled by `Options.ts`'s `keys` loop and indexed assignments,
not by `arrayValues()`. That preserves source-present indices and extra own
keys while naturally dropping trailing holes when the destination length is
derived from the last copied index.

The fixture does not establish a general JavaScript Array API. In particular,
it does not cover `.values()` iterator results or iterator mutation between
`next()` calls, custom iterables, proxies, species, arbitrary callback
execution, or all `fromIndex` coercions. The adapter deliberately models only
the ordinary array behavior required by the pinned Options.ts source closure;
adding a source use of any excluded protocol requires new source evidence and
a root-owned contract/fixture update.
