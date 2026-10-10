# P12c Mo script and accent implementation ledger

Source: pinned MathJax 4.1.3 `ts/core/MmlTree/MmlNodes/mo.ts` (packet lock in
`docs/math-port/evidence/p12b-source-lock.json`). Implementation:
`src/math/core/mml_tree/mml_nodes/mo_scripts.cpp`. This ledger records the
porting decisions only; it does not claim that root's fixtures, build,
mutations, or acceptance gate pass.

| Method | Pinned source | Native mapping |
| --- | --- | --- |
| `setInheritedAttributes` | `mo.ts:413-425` | `mo_scripts.cpp:13-27`; apply source default-parameter values at entry, call the inherited implementation first, snapshot `getText()` once, then dispatch checks in source order: operator table, pseudoscript, prime, accent. |
| `checkPseudoScripts` | `mo.ts:518-530` | `mo_scripts.cpp:29-48`; obtain the captured constructor's current pattern provider at the read site, match UTF-16 text, evaluate `coreParent().Parent` with a typed null failure, retain the `msubsup && !msub` short circuit, set `pseudoscript`, and only for pseudo scripts write inherited left then right spacing. |
| `checkPrimes` | `mo.ts:536-546` | `mo_scripts.cpp:50-71`; read the captured pattern before the remap object; iterate Unicode code points using `string_util::unicodeChars`, dynamically read every remap property into a temporary vector, then coerce all values to numbers and validate/reassemble through `string_util::unicodeString`, then write `primes`. Missing mappings flow as NaN to the source-compatible RangeError path. |
| `checkMathAccent` | `mo.ts:549-570` | `mo_scripts.cpp:73-103`; read `Parent` before the explicit-property check, preserve the parent-kind short circuit, snapshot base/under/over pointers before virtual base calls, retain the base short circuit, under check and source `isOver` expression that checks `under.coreMO()`. Pattern predicates run only after either positional condition succeeds, with the width-aware predicate evaluated only if the accent predicate is false. |
| `isMathAccent` | `mo.ts:576-582` | `mo_scripts.cpp:101-108`; `nullopt` models omitted default argument and calls `getText()`; an engaged empty view remains empty. Read the captured constructor pattern and invoke its shared callback. |
| `isMathAccentWithWidth` | `mo.ts:587-593` | `mo_scripts.cpp:110-117`; same optional text/default semantics, using the width pattern provider. |

The implementation uses `tree::TreeCallGuard<MmlNode>` on every public/protected
entry. `host::moStatic` and `host::moMatch` retain native TypeError behavior for
missing constructor services/pattern callbacks; no base-class fallback is
introduced. `host::moAttributes`-equivalent fresh checks are used around each
spacing write, and null/missing tree receivers are converted to typed errors
rather than dereferenced. These are host-boundary adaptations for the typed
native tree and preserve the operation order witnessed by the root-owned CJS
fixture; they do not implement arbitrary JS descriptors or proxies.

No tests or builds were run for this assignment. Root owns independent source
review, additional fixtures, mutation checks, builds, and acceptance. Any
discrepancy discovered by those checks should return here as a focused source
correction rather than being resolved by changing expected observations.
