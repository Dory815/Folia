# P3b native option object graph

Implementation unit: `src/math/util/option_object.cpp`, attached to the
root-owned `folia.math.util.option_types` module. The pinned source being
adapted is MathJax 4.1.3 `ts/util/Options.ts`, SHA256
`b810ffc09864eb8ed9295d829b229194d7e38aaac6c8f01525bc1a4b190a61e3`.
The implementation supplies the JavaScript object/property semantics used by
Options.ts without evaluating JavaScript source or bytecode. `option_types.ixx`
and `option_storage.hpp` remain root-owned and unchanged.

## Behavior ledger

| Native member or helper | Ported behavior / adaptation |
| --- | --- |
| `truthy` | Implements ToBoolean for undefined, null, booleans, binary64 (including NaN and signed zero), UTF-16 strings, symbols, and object references. |
| `sameValue` | Implements Object.is-style comparison: all NaNs compare same; positive and negative zero differ; object and symbol references compare by identity. |
| `Context::Context` | Creates one context identity and stable records for Object, Function, Array, Expandable, Number, Boolean, String, and Symbol prototypes/constructors; constructor own-property insertion order is `length`, `name`, `prototype`; connects each prototype's `constructor`; installs Object.prototype's identity-bearing opaque methods and the live `__proto__` accessor; delegates Array.prototype initialization to the array implementation unit. |
| `Context::~Context` | Releases all records through the Context-owned `unique_ptr<Record>` arena. Inter-record prototype/property/function references remain non-owning IDs, allowing cycles without shared ownership cycles. |
| `record`, `allocate` | Private checked arena operations. Object references must match the current context and an allocated record; allocated prototypes are also context-validated. |
| `object`, `nullObject`, `objectWithPrototype` | Allocate ordinary objects with Object.prototype, null prototype, or a validated explicit prototype. |
| `function`, `constructor`, `instance`, `payload`, `kind` | Preserve function/constructor/instance identity, callable service and `std::any` payload; ordinary constructors receive an own prototype object with a constructor link; instances inherit from that prototype. Calls to identity-only functions without native bindings raise TypeError. |
| `symbol`, `symbolString`, `keyString` | Create context-qualified symbol identities, preserve optional UTF-16 descriptions, and stringify symbol keys as `Symbol(description)` while retaining the key's symbol identity. |
| `prototype`, `setPrototype` | Read/change validated prototype references and reject cross-context links and cycles. |
| `ownKeys`, `enumerableStringKeys`, `ownSymbols` | Enumerate own keys with canonical uint32 index strings in ascending numeric order, then other strings in insertion order, then symbols in insertion order. `Context::ownKeys` validates Symbol primitive context identity and exposes own keys in insertion order, including constructor metadata (`length`, `name`, `prototype`). `Options.keys` uses enumerable string keys plus all own symbols; enumerable string keys exclude non-enumerable properties. Primitive strings expose UTF-16 index properties and a non-enumerable `length`. |
| `descriptor`, `hasOwn`, `has` | Read descriptors without invoking getters; distinguish absent from own undefined; validate Symbol primitive context identity in descriptor lookup; expose immutable UTF-16 string index/length descriptors; follow prototypes for ordinary `has`. Primitive string/number/boolean/symbol lookup uses the corresponding intrinsic prototype. |
| `get` | Performs own/prototype lookup and invokes accessors with the original receiver, including when the accessor was found on a prototype or primitive wrapper. Null and undefined reads throw TypeError. |
| `set` | Invokes own or inherited setters with the original receiver, enforces non-writable properties, creates ordinary own data descriptors, and honors inherited `Object.prototype.__proto__` assignment. Array length/index writes use the array special cases described below. |
| `define` | Defines/replaces data and accessor descriptors while preserving insertion position; checks getter/setter function identity, context ownership, and non-configurable/non-writable invariants. Array index definitions grow length; array length definition validates and truncates indices in descending order, respecting non-configurable entries. |
| `erase` | Removes configurable own properties and rejects deletion of non-configurable properties, including Array length. |
| `call` | Invokes only a registered native callable after context-validating receiver and arguments. Opaque intrinsic function identities do not receive plausible stand-in behavior and raise TypeError when invoked. |
| `descriptorObject`, `descriptorFromObject` | Convert between descriptors and ordinary native descriptor objects, preserving field order and accessor/data shape. Descriptor conversion reads fields through ordinary inherited property access, invokes getters, applies ToBoolean, and validates accessors as functions or undefined. |
| private `primitive_number` | Adapts array-length ToNumber for undefined/null/booleans/numbers/UTF-16 numeric strings, including ECMAScript whitespace, decimal, Infinity, NaN, and unsigned radix prefixes. Object-to-primitive conversion has no JS engine binding and reports TypeError. |
| array length handling in `set` and `define` | Validates a finite integral uint32 range, normalizes negative zero to zero, preserves a writable non-enumerable/non-configurable length descriptor, truncates configurable indices from high to low, and on a non-configurable index failure leaves length at that index plus one (and retains a requested non-writable state). Index `4294967294` grows length to `4294967295`; string key `4294967295` is not an array index. |

## Source mapping

`Options.ts` lines 25–41 define `isObject` using constructor identity, so this
module supplies actual intrinsic prototype and constructor records rather than
structural object classification. Lines 114–116 implement `expandable` via
`Object.create(Expandable.prototype)` and `Object.assign`, which reaches the
module's property lookup, accessor invocation, and assignment rules. Lines
136–142 and 152–178 require ordered string/symbol keys and descriptor reads
that do not invoke accessors during copy. Lines 182–278 exercise ordinary
property reads/writes, recursive merges, inherited setters, and array
sentinels. Lines 318–346 distinguish own-key selection and ordinary assignment
(including `__proto__`); lines 362–383 use inherited selector lookup and
assignment into newly created option objects; lines 390–391 use own property
lookup. Root's frozen `docs/math-port/options-contracts.md`, E2 ledger, E7
identity probes, and `tests/math/reference_options.cpp` define the native
acceptance contract. No frozen tests, oracle, contract, shared declaration, or
build file was changed.

## Limits and verification status

Array allocation, `arrayLength`, `arrayValues`, `arrayIndexOf`, `arrayFilter`,
and `initializeArrayIntrinsics` are implemented by the separately assigned
array unit. The frozen API returns `std::vector<Value>` from `arrayValues` and
has no iterator-result object, so invoking JavaScript `Array.prototype.values`
cannot produce the full iterator protocol; the array unit owns this explicit
adaptation note. Likewise, opaque built-in prototype methods carry identities
but their JavaScript bodies are not executed. Array-length coercion of an
object would require the omitted JS ToPrimitive protocol and currently raises
TypeError; primitive coercions are handled. Native VM exception messages are
documented adaptations, while semantic categories and explicit MathJax
diagnostics remain distinct.

The initial pre-dispatch native reference contained 414 assertions. Root extended
and independently accepted it with 593 assertions in 21 tests; authoritative
results and host adaptations are in `options-acceptance.md`. Root owns its
build, oracle comparison, graph-lifetime check, mutation review, and acceptance;
this packet does not claim that the object unit has passed those checks.
