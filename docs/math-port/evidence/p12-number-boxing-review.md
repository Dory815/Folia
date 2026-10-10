# P12a Number boxing review

Read-only comparison of the frozen P12a number boxing bindings with the
installed Node `v24.13.0` intrinsics. This is a dependency review, not an
implementation or acceptance report. No source, contract, fixture, test, or
build file was edited; no build was run.

Reviewed native snapshots:

| File | SHA-256 |
| --- | --- |
| `src/math/core/tree/property.ixx` | `062620af1c2738306a9b63e6253ce78cfcbf06caf78fcb9e96313f01a8bf755d` |
| `src/math/core/mml_tree/mml_node_host.ixx` | `6f704bf6a82ce2c8cf8d362d18ba3e315dc9fdb172a505846c5c36715ad462a9` |
| `.reference/MathJax-src/ts/core/MmlTree/MmlNode.ts` | `cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656` |

## Observed Node 24.13.0 behavior

The read-only runtime probe confirmed:

| Expression / identity | Node result |
| --- | --- |
| `typeof Number.prototype` | `"object"` |
| `String(Number.prototype)` / `Number.prototype.toString()` / `Number.prototype.valueOf()` / `Number(Number.prototype)` | `"0"` / `"0"` / `0` / `0` |
| `Object.getPrototypeOf(Number.prototype) === Object.prototype` | `true` |
| `Number.prototype.constructor === Number` | `true` |
| `Object.getPrototypeOf(3) === Number.prototype` | `true` |
| `Number.name`, `Number.length` | `"Number"`, `1` |
| `Number.prototype.toString.name`, `.length` | `"toString"`, `1` |
| `Number.prototype.valueOf.name`, `.length` | `"valueOf"`, `0` |
| Number prototype's own string-named methods | `constructor`, `toExponential`, `toFixed`, `toPrecision`, `toString`, `valueOf`, `toLocaleString` |

The probe also inspected own-property descriptors: those seven named properties
are writable and configurable, and none is enumerable. This is a mutable
intrinsic object, not an immutable scalar constant. The class-state task
explicitly excludes arbitrary JavaScript descriptors, evaluated callbacks,
and global intrinsic mutation from the typed host API, so this object-level
mutability is a boundary to record rather than a complete modeled surface.

## Native mapping compared with the runtime

`BuiltinProperty` in `property.ixx:14–16` has separate identities for
`numberPrototype`, `numberConstructor`, `numberToString`, and
`numberValueOf`. `mml_node_host.ixx:72–74` converts the Number prototype
sentinel to numeric zero before ordinary object primitive conversion. This
matches the observed `ToPrimitive` outcomes for the unmodified intrinsic:
string hint yields `"0"`, number hint yields `0`, and it remains truthy as an
object before coercion.

`host::get` in `mml_node_host.ixx:136–151` maps a primitive number's
`__proto__`, `constructor`, `toString`, and `valueOf` to stable sentinels. The
Number prototype sentinel has the Object prototype as its parent and resolves
the corresponding `Number`, `toString`, and `valueOf` sentinels. The Number
constructor's `prototype` resolves to the same `numberPrototype` sentinel
(`mml_node_host.ixx:154–155`). These identity edges agree with the Node probe
for the narrow bindings present. Primitive numeric lookup falls back to
Object.prototype members for other names; it does not expose the full mutable
Number.prototype own-property set.

The sentinel is a deliberate primitive-coercion model: it does not represent
all Number-object brand operations. For example, Node's
`Object.prototype.toString.call(Number.prototype)` is `"[object Number]"`,
while there is no corresponding callable Object-prototype receiver binding in
this host layer. That distinction is outside `texSpacing`'s source operations.

`host::string` in `mml_node_host.ixx:110–113` returns native source-like strings
for the Number constructor and the two Number methods. `host::get` synthesizes
function `name` from that string (`mml_node_host.ixx:169–171`); this yields
`Number`, `toString`, and `valueOf` for the bindings. Number constructor
length is handled as `1` (`mml_node_host.ixx:158–164`), matching Node.

## Concrete mismatches and use in spacing

The initial review found `Number.prototype.toString.length` falling through to
zero. Root added `BuiltinProperty::numberToString` to the length-one switch
before this review was finalized; the current native snapshot now agrees with
Node for that arity. `valueOf.length === 0`, the two method names, and
`Number.length === 1` also match.

The explicit `numberPrototype` branch in `host::get` returns
`PropertyList{}.get(key)` for other keys before generic callable `name` and
`length` handling. Therefore `Number.prototype.name` and `.length` both map to
Missing, matching Node's undefined values; the earlier concern that those
queries returned `"0"` and `0` was incorrect. This also means inherited
Object.prototype keys are handled by the existing property-list binding.

Current unbound Number-specific properties remain: Node exposes
`toExponential`, `toFixed`, and `toPrecision`, and its `toLocaleString` is an
own Number.prototype method. The native Number-prototype branch returns
Missing for the first three and falls through to the generic Object
`toLocaleString` sentinel for the fourth, so it does not preserve that
method's Number-specific identity. Their Node names/arities are respectively
`toExponential/1`, `toFixed/1`, `toPrecision/1`, and `toLocaleString/0`.
These members are not used by `texSpacing`'s primitive conversion; they are a
bounded intrinsic-binding limitation, not an observed spacing divergence.

The pinned `MmlNode.texSpacing()` (`MmlNode.ts:726–740`) does not explicitly
read a Number intrinsic's `name` or `length`. It reads `prevClass`, then
`texClass`, uses truthy/nullish checks and strict class comparisons, indexes
the spacing rows by property-key coercion, evaluates script-level comparisons,
and calls `Math.abs` on the selected spacing. For a raw Number.prototype-like
class value, the relevant operations are truthiness and primitive coercion to
the key `"0"`; the modeled zero conversion is consistent with Node's intrinsic
behavior. Number method arity is therefore not exercised by this spacing
method. This review did not find a class-state spacing path that calls the
Number `toString` function or observes its `.length` metadata.

These conclusions concern the bound, unmodified Number intrinsics only.
They do not validate mutable intrinsic properties, arbitrary Number objects,
`new Number(...)`, custom prototypes, descriptors, or replacement conversion
methods. The remaining Number-specific method bindings are reported for
root's contract review; this report does not decide whether they are in-scope
for the frozen P12a acceptance surface.
