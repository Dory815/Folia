# P5 `lengths.ts` source port evidence

The implementation in `src/math/util/lengths.ixx` follows pinned MathJax
4.1.3 `ts/util/lengths.ts` at SHA256
`213f65e9a8fa6d30bda4df4a6c0cc3a1e8d50567267539d85a1f19c1c20cea73`.
The export surface is `folia.math.util.lengths` and namespace
`folia::math::lengths`.

| Source member | Source evidence | Native mapping |
| --- | --- | --- |
| `BIGDIMEN` | `lengths.ts:27` | Imported from `folia.math.util.constants`; no duplicate. |
| `UNITS` | `lengths.ts:33-39` | Native adaptation exposes a read-only 4-entry array with `px`, `in`, `cm`, `mm` factors in source order. The source exports a mutable object and uses own-property membership (`113-114`); the active profile has no writers and the native raw-TeX API has no module-export mutation surface. |
| `RELUNITS` | `lengths.ts:44-50` | Native adaptation exposes a read-only 5-entry array with `em`, `ex`, `pt`, `pc`, `mu` factors in source order. The source exports a mutable object and uses own-property membership (`116-117`); the active profile has no writers and the native raw-TeX API has no module-export mutation surface. |
| `MATHSPACE` | `lengths.ts:56-85` | Native adaptation exposes a read-only 21-entry array preserving source values. The source exports a mutable plain object; lookup runs before the length regexp (`102-103`). The native implementation additionally asks `PropertyList` for inherited `Object.prototype` names and returns their `BuiltinProperty` identity, as required by the numeric contract. |
| `length2em(string, size, scale, em)` | `lengths.ts:90-123` | Empty strings return `size`; truthy named spaces return first; then an anchored prefix scanner applies optional number/unit parsing and parseFloat-like decimal conversion. Absolute units keep the source expression order `(m * factor) / em / scale`; relative units multiply by their factor; percent uses `(m / 100) * size`; missing or unrecognized units use `m * size`. The return type is `Property` to preserve inherited object identities. |
| `length2em(number, size, scale, em)` | `lengths.ts:90-100`, numeric coercion at `98-100` | Calls P4 `js_number::toString` and routes the result through the same string scanner, including exponential text and prefix behavior. |
| `percent(m)` | `lengths.ts:124-131` | Calls P4 `toFixed(100 * m, 1)`, applies the source `\.?0+$` suffix removal, then appends `%`. |
| `em(m)` | `lengths.ts:132-140` | Uses the strict `abs(m) < 0.001` zero threshold; otherwise P4 `toFixed(m, 3)`, the source `\.?0+$` suffix removal, and `em`. |
| `px(m, M, em)` | `lengths.ts:141-153` | Multiplies by em, clamps only when `M` is JS-truthy and the product is less than `M`, applies strict `abs(m) < 0.1`, then P4 `toFixed(m, 1)` and removes only a final `.0` before appending `px`. |

The prefix scanner implements the source's anchored regexp at `lengths.ts:105-108`,
not a full CSS length parser. It consumes ECMAScript whitespace before the
optional decimal prefix, recognizes no exponent, requires the numeric and unit
groups to be adjacent, and ignores suffix text after the matched prefix. It
keeps a whitespace-only match distinct from an empty match. Unicode whitespace
uses the ECMAScript set from the contract; U+180E and U+200B remain ordinary
characters. Decimal conversion uses locale-independent `std::from_chars`,
handling its out-of-range result as signed overflow or underflow based on the
decimal order. The numeric overload uses the frozen P4 conversion rather than
direct arithmetic.

The table arrays are a read-only host adaptation, not a claim about the source
objects: MathJax exports mutable objects. Root's upstream read/write search
found the names are read by the length utility and enabled base, AMS, braket,
and textmacros mappings, with no writes in the active profile. The native
raw-TeX API has no module-export mutation surface. Native `PropertyList` models
the plain object's inherited identities for `constructor`, `toString`,
`__proto__`, and the other built-ins without adding numeric fallback values.

Coverage is the root-generated `tests/math/reference_numeric.cpp`; it exercises
the tables, string and numeric overloads, source prefix behavior, whitespace,
IEEE edge values, suffix formatting, and inherited `MATHSPACE` properties. Root
reports that the native build passed all 15,084 numeric/length assertions.
Remaining unverified behavior includes external mutation of exported source
tables (the active profile has no writers and native tables are read-only),
arbitrary ill-formed UTF-8 input (the boundary contract says UTF-8), and any
broader rendering integration outside the frozen numeric fixtures.

The declared TypeScript input is `string | number`, but its runtime body also
coerces other JavaScript values: typeof non-string first applies String(value)
(`lengths.ts:98-100`), then the empty/null check runs. Thus null, undefined,
and booleans become the strings "null", "undefined", and their boolean
spellings before scanning; these produce an empty regexp match and return
`size`. The later length == null branch is unreachable for those values after
conversion. The frozen native overloads cover only `string_view` and `double`;
if a future Common boundary passes `Property` values, it must apply the
corresponding JavaScript string coercion for booleans and `Missing`
(`undefined`) at that consuming boundary.
