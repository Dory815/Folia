# P3b native Options contract, root revision 2

Scope: all twelve functions and globals in pinned util/Options.ts, SHA256
`b810ffc09864eb8ed9295d829b229194d7e38aaac6c8f01525bc1a4b190a61e3`.
This is the option graph needed by native TeX configuration, factories, font
extensions and Common/SVG defaults. It is not a JavaScript interpreter.

## Fixed graph and ownership

Root owns `src/math/util/option_types.ixx` and `option_storage.hpp`; exact exports
and private record representation are frozen there. Agents may not modify them.
Each engine/session owns one Context. Object/function/instance/array values are
context-qualified identity refs; symbols have separate identity refs. All records
are Context-owned, with non-owning inter-record refs, so ordinary data cycles
do not leak shared_ptr graphs. Context destruction releases the whole graph.
Operations reject handles from another Context with TypeError. Native callbacks
bind C++ services and capture non-owning refs; no JS source or bytecode exists.
Opaque constructor/instance payloads are native std::any services, not strings.

Values distinguish undefined, null, bool, binary64, UTF-16 strings, symbol and
object refs. Strings preserve code units; no normalization or UTF-8 character
counting substitutes for JS key/string-index semantics. Value == is strict
equality (NaN unequal, signed zeros equal, objects/symbols by identity).
sameValue differs for NaN and signed zero, for non-writable descriptor checks.
Own undefined is a real descriptor; absence is optional<Descriptor>.

Property records preserve insertion order, data/accessor kind, flags and
native getter/setter function identities. Getters receive the original receiver,
including through prototype lookup. `descriptor` never invokes a getter; `get`
does. Assignment invokes setters or creates an own data property, respects
read-only fields and inherited setters, and implements inherited __proto__
assignment. Define preserves descriptor flags and rejects incompatible changes
to non-configurables. Prototype cycles reject; ordinary data cycles are allowed.
String keys enumerate uint32 indices except 4294967295 first in ascending order,
then insertion order; symbols follow separately by insertion order. Object.keys
excludes non-enumerable strings, but Options.keys includes *all* own symbols.

Intrinsics are real prototype/constructor identity records. isObject checks
non-null typeof-object and observed constructor equality to Object/Expandable;
it cannot use structural map shape or a kind-only test. Native constructors and
instances retain identity and properties; source objects can override constructor.
Primitive string properties expose UTF-16 indices and non-enumerable length;
other primitive boxing has no own properties. Null/undefined ToObject/get
operations throw TypeError. Option functions themselves retain source falsey
guards and therefore sometimes return unchanged null/undefined without boxing.

Arrays retain holes, non-enumerable length, descriptor flags, own extra keys
and prototype lookup. Index writes/defines grow length, truncation respects
non-configurables; length must be integral uint32. arrayValues follows native
ordinary array iteration (holes become undefined), re-reading live length after
each getter-driven element read; arrayIndexOf checks present
indices/prototypes and strict identity, not SameValueZero; filter captures the
initial length, observes mutations and skips absent indices, returning dense data.

## Explicit host adaptations

This native raw-TeX API does not accept arbitrary JavaScript objects, proxies,
cross-realm objects, custom Array species/iterator protocols or executable JS
callbacks. Source arrays in the enabled profile are ordinary option/data arrays;
their intrinsic iteration/filter/indexOf are native operations. Symbol keys and
native descriptor callbacks are supported; they are not omitted merely because
the current entry uses string data. Native callers may not replace those array
protocols and expect generic JS execution. Future source evidence of a required
protocol invalidates this packet's adaptation and must be resolved by root.
Array.prototype.values has an opaque identity; ordinary spread is consumed by
arrayValues, not a fabricated iterator/result object. Object.prototype callable
methods other than __proto__ are opaque identity
bindings: Options never invokes their JS bodies. Attempted invocation without
a native binding raises TypeError rather than a plausible stub result.
Host exception categories are preserved through `Exception.name`; MathJax's
explicit warning/error strings are exact UTF-16. VM-generated property-error
messages may use documented native wording (they are not MathJax diagnostics).
Copy/insert descent uses explicit heap-owned traversal frames with a depth cap
512 per utility invocation, raising RangeError with
`Maximum call stack size exceeded`; this adapts the VM-dependent stack limit,
not valid recursive option composition. C++ callbacks authored outside the port
are not a JavaScript execution API; arbitrary recursive callback re-entry is
not covered by this traversal budget. Any reachable ported callback requiring
re-entry must receive its own session/depth contract before acceptance.
No JS garbage collector is introduced. OPTIONS has one module object definition
rather than an inline definition because MSVC 19.50 failed to link the exported
self-referencing dynamic inline initializer; its mutable global identity and
default callback behavior are unchanged.

## Sole file ownership and dependencies

- `math_layout`: `src/math/util/option_object.cpp` and
  `evidence/p3b-option-object.md`; implements Context except the array methods
  listed next, plus truthy/sameValue and real intrinsic object/constructor graph.
- `math_fonts`: `src/math/util/option_array.cpp` and
  `evidence/p3b-option-array.md`; implements initializeArrayIntrinsics, array,
  arrayLength/Values/IndexOf/Filter. Array-special property define/set/length
  behavior belongs to option_object.cpp, which owns all descriptors.
- `tex_parser`: `src/math/util/options.ixx` and `evidence/p3b-options.md`;
  ports Options.ts only against the frozen Context API. No replacement value
  tree, dictionaries, registries, parser or layout logic.

Both cpp files are implementation units of folia.math.util.option_types and
include the root private header after module/import std. No standard/platform
headers, Windows APIs, new dependencies, other files, shared builds or commits.
Root owns oracle/tests/contracts/scope/build/production and independently accepts.
If a declaration or prerequisite is insufficient, agents report the exact source
case; they cannot change shared contracts or invent fallbacks.

## Utility exports (namespace folia::math::options)

options.ixx exports module folia.math.util.options, export-imports option_types
and imports std. APPEND/REMOVE are const u16string_view `[+]`/`[-]`.

```cpp
bool isObject(Context&, const Value&);
Value expandable(Context&, Value);
Value makeArray(Context&, Value);
std::vector<Key> keys(Context&, Value);
Value copy(Context&, Value);
Value insert(Context&, Value dst, Value src, bool warn=true);
Value defaultOptions(Context&, Value dst, std::span<const Value> defs);
Value userOptions(Context&, Value dst, std::span<const Value> defs);
Value selectOptions(Context&, Value, std::span<const std::u16string> names);
Value selectOptionsFromKeys(Context&, Value, Value keySource);
std::vector<Value> separateOptions(Context&, Value, std::span<const Value> selectors);
Value lookup(Context&, std::u16string_view, Value, Value defaultValue=Null{});
enum class InvalidOption { warn, fatal };
struct Diagnostics {
    InvalidOption invalidOption = InvalidOption::warn;
    std::function<void(std::u16string,std::u16string)> optionError;
    std::function<void(std::u16string)> warningSink;
};
Diagnostics OPTIONS; // one module definition, actual default callback and stderr sink
```

Default optionError reads global mode at call time: fatal throws Error(message),
warn sends `MathJax: ` + message to warningSink, then insert skips the invalid
key and continues. It must remain replaceable, not a hardcoded merge check.
Warning key order/symbol formatting, own-undefined validation, Expandable bypass,
all source branch order and destination alias identity remain exact.

copy must copy *descriptors*, not read values through getters, recursively copy
eligible values/arrays, omit nonenumerables and preserve eligible root constructor.
Use a native descriptor-object basket with ordinary assignment semantics before
defining the result: source's `props['__proto__']=descriptor` changes the basket's
prototype rather than creating an own descriptor. A custom skip-key rule in copy
is not authorized. expandable uses value assignment and invokes getters.
Array APPEND/REMOVE matches exactly one valid key or the exact two-key shape;
two-key sort with symbols raises TypeError. Remove first, then append; neither
modifies original arrays. Extra keys invoke recursive insert, not sentinel edits.
Source array copy uses insert into [], losing trailing holes as upstream does.
select/lookup are own-only and shallow; separateOptions reads inherited selector
values, consumes each name once and returns remainder **first**.

## Frozen acceptance

Root generates tests/math/reference_options.cpp from pinned JS operation logs
using tools/math-port/options-oracle.cjs. Expected descriptors, identities,
snapshots/warnings/errors and aliases never come from native implementation.
Before dispatch the oracle fixture must exist. Agents cannot edit it or its
generator. Root reviews every function against source, compiles the shared
graph and utility together, runs core filters, independently checks graph release
and mutation detection, and records gaps. Passing examples do not pass full G0
or authorize TeX/layout production wiring.
