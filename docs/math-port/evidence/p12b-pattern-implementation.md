# P12b MmlMo pattern implementation map

This report maps the two native service methods to the pinned MathJax 4.1.3
source patterns. Only `src/math/core/mml_tree/mo_patterns.cpp` and this evidence
file were written. No build, test, fixture, contract, interface, or acceptance
change was made; this is not packet acceptance.

## Source-to-native mapping

| Native operation | Source operation / lines | Preserved behavior |
| --- | --- | --- |
| `matchMoPattern(kind, text)` | `MmlMo.pseudoScripts`, `mo.ts:92–108` | Checks the full input against one or more UTF-16 code units from the generated inclusive ranges. |
| `matchMoPattern(kind, text)` | `MmlMo.primes`, `mo.ts:110–118` | Checks the full input against one or more apostrophe/quote class members. |
| `matchMoPattern(kind, text)` | `MmlMo.opPattern`, `mo.ts:121` | Checks the full input against at least two ASCII letters. |
| `matchMoPattern(kind, text)` | `MmlMo.mathaccents`, `mo.ts:145–165` | Checks exactly one UTF-16 code unit in the single-character accent class. |
| `matchMoPattern(kind, text)` | `MmlMo.mathaccentsWithWidth`, `mo.ts:169–177` | Checks exactly one UTF-16 code unit in the width-preserving accent class. |
| `makeMoPattern(kind)` | Those same static fields | Returns a nonempty independent `Pattern` closure capturing the validated enum value and applying the same match operation on every call. |

The source constructs all five patterns from strings with no flags. Their
anchors require the entire input to match; their classes and repetition bounds
are represented by root-generated `MoPatternSpec` data in
`mo_pattern_data.cpp`. The implementation iterates `char16_t` units directly
and checks each against the spec's inclusive ranges. It does not decode scalar
values, normalize, trim, fold case, relax the end anchor, or interpret source
text. The optional maximum is tested by presence, so a hypothetical maximum
of zero would remain meaningful; the frozen source maxima are one for the two
single-character patterns, while the other three are unbounded above.

An empty string fails all five frozen specs through their minimum lengths.
Minimum and optional maximum checks precede membership scans. For an input
within bounds, every code unit must belong to a range; this enforces strict
start/end behavior and rejects a disallowed unit anywhere, including a prefix
or suffix. Lone surrogates and surrogate pairs are examined as the same UTF-16
units that JavaScript's non-Unicode regular expressions consume.

`moPatternSpec(kind)` validates each enum before matching. It raises the
existing `FactoryError`/`options::Exception` with name `TypeError` for an
invalid enum. `makeMoPattern` performs that validation before constructing or
returning a callback; each callback captures only its own `kind` by value, so
callbacks remain independent of the caller's enum variable and have no shared
match state. The callback routes through `matchMoPattern`, preserving the
same validation and predicate.

These methods bind only the five frozen source patterns. They do not provide
general regular-expression construction, mutable flags or `lastIndex`, custom
pattern replacement, a JavaScript evaluator, or MmlMo registration. Root owns
the actual-CJS oracle, full-domain expectations, mutations, build, and
acceptance review.
