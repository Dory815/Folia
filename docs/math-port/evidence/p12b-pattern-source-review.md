# P12b read-only source and binding review

This is a source/data/interface review only. It does not accept MmlMo or the
five-pattern service. No code, declaration, generated data, test, lock, or
contract file was changed.

## Provenance and reproducibility

The pinned upstream source is MathJax 4.1.3
`ts/core/MmlTree/MmlNodes/mo.ts`, SHA-256
`2082cd372a4c21fa46c67ab3972b51bcc950f711dd3369cbb2bd7ced62c2f605`.
The locked resolved CJS is
`cjs/core/MmlTree/MmlNodes/mo.js`, SHA-256
`7c68f0570d82f585eb6ca7e8bbca119b96a02666e0d4b268a413bf4035ff0b2c`.
The lock pins MathJax 4.1.3, Node `v24.13.0`, and the 424-file compiled
package tree hash `773bf3ae1c075baa36c02b6958f7e3bed5fb116d0818ecf99bb70c5337d9e537`.

Read-only checks succeeded:

- `node tools/math-port/mo-pattern-reference.cjs` verified the source/CJS
  identity and emitted the five actual CJS regex metadata records.
- `node tools/math-port/mo-pattern-data.cjs --check` verified generated
  immutable data against the pinned source-derived records.
- `node tools/math-port/mo-pattern-oracle.cjs --check` verified the frozen
  reference fixture: 5 patterns, 16 cases, 971 assertions, and 2,342,329
  actual-source queries. No native build was run.

The generated source arrays and intervals in
`src/math/core/mml_tree/mo_pattern_data.cpp` agree with the generator output.
The data retains the source order and uses UTF-16 code-unit intervals. The
`MoPatternSpec` ranges point into static arrays, so the `span` and
`u16string_view` lifetimes are stable. All five `flags` values are empty.

## Exact source patterns and generated metadata

The upstream static members appear in this order and match the native enum and
spec order:

| Member | Source line | Actual `RegExp.source` | Min/max UTF-16 units | Merged allowed intervals |
| --- | ---: | --- | --- | ---: |
| `pseudoScripts` | 92–105 | ``^["'*`ª°²-´¹º‘-‟′-‷⁗⁰ⁱ⁴-ⁿ₀-₎]+$`` | min 1; unbounded | 13 |
| `primes` | 110–116 | `^["'‘-‟]+$` | min 1; unbounded | 3 |
| `opPattern` | 121 | `^[a-zA-Z]{2,}$` | min 2; unbounded | 2 |
| `mathaccents` | 145–162 | ``^[´́ˊ`̀ˋ¨̈~̃˜¯̄ˉ˘̆ˇ̌^̂ˆ⃐⃑⃖⃗⃡˙̇˚̊⃛⃜]$`` | exactly 1 | 18 |
| `mathaccentsWithWidth` | 169–175 | `^[←→↔⏜⏝⏞⏟]$` | exactly 1 | 4 |

All five expressions are anchored at both ends and were constructed without
flags. The first two classes use `]+` (one or more), `opPattern` uses `{2,}`,
and both accent classes require exactly one UTF-16 code unit. No class contains
astral code points: surrogate code units therefore do not match, and the
native code-unit ranges preserve that behavior. The expressions have no
global or sticky flag and do not depend on mutable `lastIndex` state.

The generated intervals are sorted, inclusive, and merge only adjacent
allowed code units. In particular, `pseudoScripts` includes `[0x2074,0x208E]`
and the separate `[0x2070,0x2071]` interval; `mathaccents` includes the
combining ranges `[0x0300,0x0304]`, `[0x0306,0x0308]`, and the listed
combining-symbol marks; `mathaccentsWithWidth` includes U+23DC–U+23DF. The
reference generator derives intervals by testing every BMP code unit against
the actual CJS expression at the pattern's minimum length, and the fixture
checks complete 65,536-unit masks at several sequence positions and lengths,
plus mixed products and adversarial suffixes.

## Full `MmlMo` source member inventory

The pinned TypeScript AST ledger contains 32 implemented class members. Only
the five regex statics above are in the P12b binding packet. The remaining
members are not implemented or accepted by this packet.

| Source lines | Member(s) | Status for this review |
| --- | --- | --- |
| 52–76 | `defaults` | Outside five-pattern scope. |
| 81 | `MMLSPACING` | Outside five-pattern scope. |
| 86 | `OPTABLE` | Outside five-pattern scope. |
| 92–105 | `pseudoScripts` | Included. |
| 110–116 | `primes` | Included. |
| 121 | `opPattern` | Included. |
| 126–137 | `remapPrimes` | Outside five-pattern scope. |
| 145–162 | `mathaccents` | Included. |
| 169–175 | `mathaccentsWithWidth` | Included. |
| 180 | `_texClass` | Outside five-pattern scope. |
| 189–194, 199–201 | `texClass` getter/setter | Outside five-pattern scope. |
| 207, 213 | `lspace`, `rspace` | Outside five-pattern scope. |
| 218–220 | `kind` | Outside five-pattern scope. |
| 227–229 | `isEmbellished` | Outside five-pattern scope. |
| 235–249 | `coreParent` | Outside five-pattern scope. |
| 255–274 | `coreText` | Outside five-pattern scope. |
| 279–281 | `hasSpacingAttributes` | Outside five-pattern scope. |
| 286–313 | `isAccent` | Outside five-pattern scope. |
| 320–340 | `setTeXclass` | Outside five-pattern scope. |
| 347–405 | `adjustTeXclass` | Outside five-pattern scope. |
| 413–425 | `setInheritedAttributes` | Outside five-pattern scope. |
| 433–451 | `getOperatorDef` | Outside five-pattern scope. |
| 458–468 | `checkOperatorTable` | Outside five-pattern scope. |
| 475–498 | `getForms` | Outside five-pattern scope. |
| 504–510 | `handleExplicitForm` | Outside five-pattern scope. |
| 518–529 | `checkPseudoScripts` | Outside five-pattern scope. |
| 536–542 | `checkPrimes` | Outside five-pattern scope. |
| 549–568 | `checkMathAccent` | Outside five-pattern scope. |
| 576–579 | `isMathAccent` | Outside five-pattern scope. |
| 587–590 | `isMathAccentWithWidth` | Outside five-pattern scope. |

The source contains no custom constructor among these class members. The five
static RegExp members are captured in `mo_patterns.ixx` as five enum values
and immutable specs, not as general mutable JavaScript RegExp objects.

## Native callback boundary review

`mo_patterns.ixx` exposes a `MoPattern` enum, `MoPatternSpec` metadata,
`matchMoPattern(MoPattern, std::u16string_view)`, and
`makeMoPattern(MoPattern) -> Pattern`. `Pattern` is the existing typed
`std::function<bool(std::u16string_view)>` native callback. The current
`mo_patterns.cpp` implementation reads generated metadata, checks the
minimum/optional maximum, and tests every UTF-16 unit against the inclusive
ranges. It validates the enum before returning a lambda which captures the
enum by value; the callback does not borrow a temporary spec or object.
Invalid enum values are rejected by `moPatternSpec` with the shared typed
`TypeError` path.

That binding matches the five closed, no-flags expressions and does not claim
to construct arbitrary RegExp values, expose `lastIndex`, invoke user
callbacks, or emulate JavaScript regex behavior generally. The data and
fixture checks are consistent with the current implementation by inspection;
root compilation, runtime comparison, mutation checks, and acceptance remain
pending and root-owned.
