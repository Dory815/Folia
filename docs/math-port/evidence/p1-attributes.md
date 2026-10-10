# P1 Attributes port evidence

Implementation target: `src/math/core/mml_tree/attributes.ixx`.
Reference: MathJax 4.1.3, `ts/core/MmlTree/Attributes.ts`.
Reference SHA256: `c1e363944e0c059c7ed554b4b6aeb8ea56d43c844f84934c267ccc79b090468a`

## Method ledger

| Upstream method | Native method | Behavior carried over |
| --- | --- | --- |
| constructor(defaults, global) | `Attributes(defaults, global)` | Keeps the supplied global handle live; creates defaults over globals, inherited over defaults, and explicit over inherited; copies only defaults' own enumerable entries into the new defaults layer. |
| `set(name, value)` | `set` | Assigns to the explicit layer using `PropertyList::set`, including scalar `__proto__` assignment semantics. |
| `setList(list)` | `setList` | `Object.assign`-style own enumerable copy into explicit attributes. |
| `unset(name)` | `unset` | Deletes only an own explicit entry, revealing inherited/default/global values again. |
| `get(name)` | `get` | Reads the full prototype chain. If the resolved value is exactly the string `_inherit_`, resolves the same key through globals once. |
| `getExplicit(name)` | `getExplicit` | Returns the own explicit value, including an own `Missing`; otherwise returns `Missing`. Does not resolve `_inherit_`. |
| `hasExplicit(name)` | `hasExplicit` | Own-property query on explicit layer. |
| `hasOneOf(names)` | `hasOneOf` | True if any listed key is an own explicit property. |
| `getList(...names)` | `getList(vector<string>)` | Produces a fresh list containing each requested name assigned its `get` result, including `Missing` values. |
| `setInherited(name, value)` | `setInherited` | Assigns to the inherited layer while preserving its defaults prototype. |
| `getInherited(name)` | `getInherited` | Reads the inherited object as JavaScript property access does, including lookup through defaults/globals; returns `_inherit_` literally if that is the resolved value. |
| `getDefault(name)` | `getDefault` | Reads the defaults object, including its global prototype; does not resolve `_inherit_`. |
| `isSet(name)` | `isSet` | True only for own explicit or own inherited entries. Defaults and globals do not count. |
| `hasDefault(name)` | `hasDefault` | Uses prototype-aware presence (`name in defaults`), so global and Object.prototype presence is preserved. |
| `getExplicitNames()` | `getExplicitNames` | Own explicit keys in `PropertyList::keys()` / JS `Object.keys` ordering. |
| `getInheritedNames()` | `getInheritedNames` | Own inherited keys in JS key ordering. |
| `getDefaultNames()` | `getDefaultNames` | Own defaults keys in JS key ordering. |
| `getGlobalNames()` | `getGlobalNames` | Own global keys in JS key ordering. |
| `getAllAttributes()` | `getAllAttributes` | Returns a shared handle to explicit layer. |
| `getAllInherited()` | `getAllInherited` | Returns a shared handle to inherited layer. |
| `getAllDefaults()` | `getAllDefaults` | Returns a shared handle to defaults layer. |
| `getAllGlobals()` | `getAllGlobals` | Returns the shared global handle. |

## Adaptations and limits

- The exported sentinel is `std::string_view INHERIT = "_inherit_"`; the
  `Property` storage contract has no separate `INHERIT` variant, matching the
  upstream string sentinel.
- C++ `getList` takes `const std::vector<std::string>&` rather than JavaScript
  rest arguments, as fixed by primitive-contracts revision 1.
- `PropertyList` handles encode JavaScript object identity and prototype layers;
  the `Attributes` accessors return handles by value, which remains live because
  handle copies alias the same storage.
- There is no upstream `clone` method; none was added. Native copy construction
  keeps the four shared handles, per the frozen contract.
- No behavior was intentionally omitted. Validation and acceptance remain with
  root; this packet did not run builds or alter fixtures/oracle files.

## Uncertainties

The shared `PropertyList` contract now preserves dictionary identity in
`Property`, so object-valued `__proto__` assignment, getter identity, and cycle
rejection are delegated to `PropertyList::set` as required. Scalar values keep
the JavaScript setter's ignore behavior. No additional uncertainty was
identified in the Attributes source methods themselves.
