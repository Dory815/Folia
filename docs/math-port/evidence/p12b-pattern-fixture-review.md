# P12b fixture and source-lock review

## Scope and result

Read-only review of the P12b contract, pinned MathJax TypeScript/CommonJS,
source lock, data/oracle generators, and frozen native fixture. I did not run a
build or change expectations. I found no incorrect expected match results or
source-lock mismatch. This review does not accept the implementation or packet.

## Pinned source and generator checks

The lock at `docs/math-port/evidence/p12b-source-lock.json` records MathJax
4.1.3 / commit `fb987178c2d279a99b0db5ea02e751691703955e`, Node
`v24.13.0`, TypeScript path `ts/core/MmlTree/MmlNodes/mo.ts` with SHA256
`2082cd372a4c21fa46c67ab3972b51bcc950f711dd3369cbb2bd7ced62c2f605`, and
compiled CommonJS `cjs/core/MmlTree/MmlNodes/mo.js` with SHA256
`7c68f0570d82f585eb6ca7e8bbca119b96a02666e0d4b268a413bf4035ff0b2c`.
The reference script also locks the compiled package closure at 424 files and
aggregate SHA256
`773bf3ae1c075baa36c02b6958f7e3bed5fb116d0818ecf99bb70c5337d9e537`.
`mo-pattern-reference.cjs --check`, `mo-pattern-data.cjs --check`, and
`mo-pattern-oracle.cjs --check` all passed in this review. The oracle reports
5 patterns, 16 cases, 971 assertions, 2,342,329 actual-source queries, and 30
full UTF-16-domain masks.

The TypeScript declarations at `mo.ts:92,110,121,145,169` correspond to the
five static patterns in contract order: `pseudoScripts`, `primes`,
`opPattern`, `mathaccents`, `mathaccentsWithWidth`. The reference requires the
runtime properties to be RegExp values with no flags and `lastIndex == 0`
before extracting each source. It accepts only the source's anchored class
forms and repetition suffixes, then checks every individual UTF-16 unit with
the actual pinned CommonJS RegExp repeated to its minimum length. The generated
range summary is 13, 3, 2, 18, and 4 inclusive ranges respectively; bounds are
1+, 1+, 2+, 1, and 1. Since none of these regular expressions has `g` or `y`,
the repeated `.test()` used to derive membership does not mutate state.

## Fixture coverage

For each pattern, the fixture compares native matching to `String.match` on
all 65,536 possible single units, all two-unit strings, and all three-unit
strings using packed masks. The additional prefix/interior/suffix masks use
the first actual allowed code unit and exercise a disallowed code unit at each
position. Thus the exhaustive assertions directly cover UTF-16 code-unit
semantics, including lone surrogates and every class endpoint; they do not
assume Unicode scalar iteration. Per-pattern ordered products cover every
pair of allowed units with empty, allowed, LF, CRLF, U+2028, U+2029, and NUL
tails. The adversarial rows include empty and long accepted strings, line
terminators in both positions, embedded invalid units, case variants,
non-ASCII letters, combining accents, primes, symbols, surrogate halves and
pairs, and emoji. Expected booleans are obtained from the locked CJS
`!!text.match(M[name])`, not from the generated membership ranges.

The callback checks exercise all five callbacks on common distinguishing
inputs, invoke both callbacks on each per-pattern adversarial row, clear the
first callback, and continue invoking the second. Invalid enum values -1, 5,
and 99 are checked for both the direct matcher and callback factory, expecting
the host `TypeError`. These are appropriate tests of the typed callback API;
they do not claim mutable JavaScript RegExp or arbitrary user-pattern support.

The contract's generic `$` warning is addressed by explicit final LF, CR,
CRLF, U+2028, and U+2029 rows and product tails. I also queried the pinned CJS
directly with an otherwise-allowed character followed by each of those
terminators; all five patterns rejected all such strings. The fixture does
not contradict the source's observed behavior.

## Concrete fixture gap

The metadata cases assert `spec.source`, empty `spec.flags`, `minimum`, and
`maximum` (`mo-pattern-oracle.cjs:19-21`; generated cases are visible, for
example, at `reference_mo_patterns.cpp:9-14`). They do not inspect
`spec.units` directly. Exhaustive matching tests strongly constrain the
effective native predicate, and `mo-pattern-data.cjs --check` separately
recomputes the generated data file from the pinned reference; however, neither
is an assertion that the runtime-exported `MoPatternSpec::units` span itself
is sorted, inclusive, and equal to the locked ranges. A consumer reading the
metadata could therefore receive a wrong range table while all current
match-result assertions still pass. Add explicit length and element-by-element
range comparisons against the source-derived ranges if the exported metadata
is part of this packet's required contract.

The callback lifetime witness proves clearing one callback does not invalidate
another. The native `Pattern` is a callable value and has no equality contract,
so the fixture does not establish callback object-identity inequality; that is
not required by the stated “independent callback” lifetime semantics. The
native fixture also cannot test arbitrary RegExp flags, mutable `lastIndex`,
general RegExp construction, or JS descriptors, which the contract explicitly
excludes from this service.

## Disposition

The checked lock/generators and actual-CJS expectations are internally
consistent for the reviewed source. The missing direct assertion for exported
range metadata should be resolved or explicitly waived by the root owner. No
claim is made here about native implementation correctness, mutation-test
detection, integration, or overall MathJax parity.
