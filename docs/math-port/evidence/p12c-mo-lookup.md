# P12c MmlMo lookup implementation map

This evidence maps the six assigned methods to the pinned MathJax 4.1.3
`MmlNodes/mo.ts` implementation. Only `src/math/core/mml_tree/mml_nodes/mo_lookup.cpp`
and this file were edited. No build, test, fixture, interface, oracle, or
acceptance change was made; root owns review and acceptance.

| Native method | Upstream source | Mapping and source-order behavior |
| --- | --- | --- |
| `texClass()` | getter, `mo.ts:189–194` | Uses strict null-only `_texClass` test. On null it reads text, dispatches virtual `getOperatorDef`, reads slot 2, and returns that raw property without caching. Missing, undefined, zero, and other values return as stored. |
| `hasSpacingAttributes()` | `mo.ts:279–282` | Preserves short-circuit order: `lspace` check first; obtains the attributes receiver again and checks `rspace` only on false. |
| `getOperatorDef(mo)` | `mo.ts:433–455` | Gets forms and applies explicit-form handling, destructures three iterator values, writes inherited `form`, then reads captured-constructor OPTABLE once. It performs the three `form`/operator reads in source truthy-OR order and returns a truthy definition handle unchanged. On miss it writes `noDictDef`, reads `movablelimits`, fetches/calls the captured `opPattern`, conditionally creates a fresh OPDEF, or calls module-level `operator_dict::getRange`, reads constructor MMLSPACING, destructures the selected spacing iterator, re-reads the range class slot, and returns a fresh OPDEF. |
| `checkOperatorTable(mo)` | `mo.ts:458–472` | Dispatches `getOperatorDef`; if node property `texClass` is missing, reads definition slot 2 and calls the virtual setter. It evaluates `def[3] || {}` for `moKeys`, then for each key reads the `this.attributes` receiver first, re-reads `def[3]`, gets that value, and invokes `setInherited`. It finally reads/converts slot 0 and assigns `lspace`, then reads/converts slot 1 and assigns `rspace`. |
| `getForms()` | `mo.ts:475–499` | Retains distinct `parent` and virtual `Parent()` reads while walking embellished ancestors. It calls `isKind('mrow')`, then `nonSpaceLength`, `firstNonSpace`, and only if needed `lastNonSpace`, returning a fresh array in each branch. |
| `handleExplicitForm(forms)` | `mo.ts:504–512` | Returns the original array handle when form is not set. Otherwise reads form again, snapshots the array length, tests each index for own-or-inherited presence, filters by raw strict `name !== form`, and prepends the raw form property to a fresh result. |

All six methods use `TreeCallGuard<MmlNode>`. Captured constructor services
are resolved through `constructorOperators()` and each class static getter is
called at its source operation point with `host::moStatic`; the current
registry is not consulted. Table lookups retain the raw `Property`, so
inherited prototype functions and other non-array values remain observable.
Fallback spacing uses the separate module-level `operator_dict::getRange`,
then the captured class spacing getter. The range's class slot is read once
for spacing-table indexing and again when passed to OPDEF.

`host::moHead` binds the source array/string iterator semantics used by
destructuring; it preserves missing slots and inherited array values and
iterates primitive strings by code point. `host::moKeys` binds source
`Object.keys` behavior for ordinary property objects, arrays, and strings.
These are typed host adapters, not a general JavaScript iterator, proxy,
descriptor, or accessor engine. The six native methods do not add such an
engine or special-case formula text. Their exact profile-specific behavior
remains subject to the frozen CJS oracle and root's source/fixture review.
