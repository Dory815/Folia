# P12b: five MmlMo regex bindings — root contract revision 1

Owner: root. This accepts only the exact five pristine source patterns as a
native dependency service. MmlMo has no accepted implementation yet, and no
partial MML factory or production registration is permitted. Full G0–G7 stay
closed. The complete TeX/scene/SVG/Folia objective and package profile remain.

Source: MathJax 4.1.3, commit fb987178c2d279a99b0db5ea02e751691703955e,
ts/core/MmlTree/MmlNodes/mo.ts. Root locks TS/CJS and Node version before
generating source data or reference fixtures. Other MmlMo members remain
unimplemented; neither five patterns nor their tests establish Mo acceptance.

## Exact responsibilities

| Owner | Sole files | Responsibility |
| --- | --- | --- |
| Root | mo_patterns.ixx, mo_pattern_data.cpp, mo-pattern-data.cjs, mo-pattern-oracle.cjs, reference_mo_patterns.cpp, mutation tool, CMake, source lock, scope/tasks/acceptance | Interface, exact generated source data, immutable expectations, integration and acceptance |
| tex_parser | src/math/core/mml_tree/mo_patterns.cpp; evidence/p12b-pattern-implementation.md | Implement only matchMoPattern and makeMoPattern from the frozen spec; map to source regex operations |
| math_layout | evidence/p12b-pattern-source-review.md only | Read-only source/data/interface review; report gaps without repairing shared files |
| math_fonts | evidence/p12b-pattern-fixture-review.md only | Read-only oracle/coverage/lock review; report gaps without changing expectations |

Agents may read dependencies but cannot change declarations, generated data,
fixtures, shared hosts, CMake, source lock, scope, contracts, other files or
production. No builds or commits. Root freezes interfaces/data/fixtures in a
commit before dispatch. An interface gap returns to root before dependent work.

## Binding and semantics

MoPattern has exactly five enumerators, in source-static order: pseudoScripts,
primes, opPattern, mathaccents, mathaccentsWithWidth. MoPatternSpec preserves
RegExp.source, absent flags, sorted inclusive UTF-16 code-unit ranges, minimum
length and optional maximum. Root derives ranges from the actual pinned CJS
regex, first checking the source's anchored-class/quantifier structure. These
are generated source data, never native-rendered expectations.

matchMoPattern consumes UTF-16 units exactly, with strict start/end and the
source class repetition bounds. It must retain mixed allowed strings, reject
one extra disallowed unit anywhere, and preserve isolated surrogate behavior.
No normalization, Unicode scalar iteration, locale folding, whitespace trim,
final-newline relaxation, heuristic symbol categories or regex approximations.
Five default patterns have no global/sticky state. makeMoPattern returns a
nonempty independent native callback with the same predicate and safe lifetime.
Invalid native enum values fail with FactoryError TypeError both on matching
and callback creation, before any callback is returned. This is a typed binding
error, not a claim about JS RegExp construction outside the source patterns.

These callbacks use the existing Pattern native service. Later MmlMo statics
will expose replaceable callbacks through actual-constructor metadata, as
existing per-kind static services do. A general user-provided JS regex, mutable
RegExp descriptors/lastIndex, flags or JS evaluation are not offered by this
compiled callback API. Required future callers need their own reviewed native
bindings; no source feature is dropped because this packet has narrower duties.

## Acceptance required before use

Actual-CJS masks cover the complete 65,536 UTF-16 unit domain for singleton,
double and triple repetitions, plus allowed/disallowed prefixes and suffixes.
Ordered allowed-pair products and adversarial multicharacter strings cover
mixed classes, repetition bounds, NUL, whitespace, all line terminators,
surrogate pairs/lone surrogates and non-ASCII alphabet characters. Source
metadata, invalid enum errors, independent callback creation, repeated use and
callback lifetime have separate assertions. Expected results come from actual
String.match(RegExp) truthiness, not a rewritten membership formula.

Root checks reproducible generators, source correspondence, relevant tests
and a complete core build/regression. Isolated mutants must compile and fail
unchanged assertions for bounds, skipped units, anchoring, class endpoints,
newline behavior, scalar/locale assumptions and callback dispatch/lifetime.
Source/fixture copies remain byte-equal. A compiler error or crash is not
detection. Agent reports alone cannot accept this packet.
