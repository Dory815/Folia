# P11 operator dictionary host implementation

The upstream source is MathJax 4.1.3
`ts/core/MmlTree/OperatorDictionary.ts` (SHA-256
`9afac6e3a8f3b778247a5a678c529b9edddb8172850df7384511bf61dfbffd37`).
The source-derived implementation is
[operator_dictionary.cpp](../../../src/math/core/mml_tree/operator_dictionary.cpp).
It retains the Apache-2.0 provenance notice and references
`src/math/LICENSE-MathJax.txt`.

## Member mapping

| Upstream member | Source lines | Native ownership |
| --- | ---: | --- |
| `OPDEF` | 41–48 | `OPDEF` allocates a new dense four-slot `PropertyArray`; defaults are selected only for `Missing`. |
| `MO` | 53–97 | Root-generated pristine identity graph in `operator_dictionary_data.cpp`. |
| `RANGES` | 102–153 | Root-generated pristine identity graph in `operator_dictionary_data.cpp`. |
| `getRange` | 161–177 | `getRange` performs dictionary lookup, UTF-16 code-point selection, live range traversal, and fallback allocation. |
| `MMLSPACING` | 182–191 | Root-generated pristine identity graph in `operator_dictionary_data.cpp`. |
| `OPTABLE` | 197–1456 | Root-generated pristine identity graph in `operator_dictionary_data.cpp`. |

The generated globals remain root-owned and unchanged by this implementation.
They retain the pinned object/array alias graph and do not include the separate
`LinebreakVisitor` startup mutation.

## `OPDEF`

The function receives `left`, `right`, `texClass`, and `properties` as raw
`Property` values. A `Missing` class becomes `TEXCLASS.BIN`; a `Missing`
properties value becomes `null`. Explicit `null`, false, zero, strings,
objects, and arrays remain untouched. Each invocation creates a fresh dense
array while preserving object/array identity in the raw slots.

## `getRange`

The function converts the full UTF-16 input text to the host's lossless
property-key encoding and reads `OPTABLE.infix[text]`, then `prefix[text]`
only if the first value is falsey, then `postfix[text]` only if needed. These
are direct prototype-aware property reads. A truthy hit reads slot `2` once
and creates a fresh `[0, 0, texClass, "mo"]` array; inherited built-in
function values count as hits and their missing numeric slot stays
`Missing`.

On a dictionary miss, `utf16::codePointAt(text, 0)` is called once. Empty text
becomes `Missing` and therefore compares as `NaN`; a supplementary scalar is
decoded from its UTF-16 pair, and an unpaired surrogate remains its code-unit
value. The function captures the current `RANGES` array handle, then follows
the normal array iterator's live length and indexed slots. For each row it
reads and compares `[1]` first. Only when that comparison succeeds does it
read `[0]`; it returns the original row `Property` handle when the lower
comparison also succeeds, and otherwise stops immediately. If no row matches,
it allocates a fresh `[0, 0, TEXCLASS.REL, "mo"]` array.

Nullish table receivers and malformed range rows raise the shared typed host
`TypeError` during the corresponding property access. Numeric comparison
uses the frozen host conversion service; unsupported evaluated callable
coercions remain explicit host errors rather than invoking a JS runtime.

## Scope boundary

The root-generated `MO`, `RANGES`, `MMLSPACING`, and `OPTABLE` accessors are
the source of the pristine graph. This file does not apply output-specific
startup mutations, port `MmlMo`, or replace those shared records with a
separate dictionary. No build, oracle, mutation run, or acceptance claim was
made by this packet; the root owns those checks.
