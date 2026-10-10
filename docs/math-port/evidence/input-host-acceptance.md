# P8a root acceptance: input primitive host services

Status: locally accepted prerequisite host services under
`../input-host-contracts.md` revision 1. MmlNode.ts, concrete MML nodes,
TeX input, Common layout, scene/SVG and Folia replacement remain unaccepted.
Ten of 246 upstream inventory entries retain local acceptance; this host
helper packet does not increase that upstream source count. Full G0–G7 remain
closed. The existing single production math backend remains QuickJS.

## Root responsibility and file boundaries

Root committed interfaces/contracts and actual-Node expectations before
implementation dispatch (602a3c2, with independently added rounding boundaries
in 08bae83). math_layout implemented only js_numeric_input.ixx and its ledger;
tex_parser implemented only utf16.ixx and its ledger. math_fonts independently
audited MML state and the host contract. Root controlled expected results,
types, CMake, mutation tooling, builds, review, disposition and commits.
No agent changed shared contracts or expectations or built shared output.

The numeric module supplies string primitives, not JS evaluation or object
coercion. UTF-16 text is retained internally. UTF-8 input validation and byte
offsets are one explicit diagnostic boundary; external encode replaces lone
surrogates as TextEncoder does. No editor coordinate, custom math tree,
parser/router, font fallback, placement equation or runtime backend was added.

## Rejections and corrections from independent review

- Root rejected E11's claim that empty MML children were uninitialized. Actual
  pinned CJS AbstractNode assigns [] first. Root independently constructed Text
  and XML via MmlFactory and verified the empty array, null attributes/parent.
- Direct Array.from attribution to TextParser was incorrect. Source checks
  identify util/string.unicodeChars and Common scriptbase; TexParser and mo
  wrappers use codePointAt. Contract and evidence now distinguish the consumers.
- V8's arbitrary-radix parseInt does not always equal correctly rounded exact
  integer conversion (the base-36 witness differs). Root checked source callers
  and froze only automatic/8/10/16 forms, with explicit invalid-enum rejection.
- The initial generated Number callback exceeded the Windows stack capacity.
  Root split Number/parseInt fixtures into bounded callbacks without changing
  any expected values. This was a fixture repair, not relaxed native semantics.
- Root rejected the numeric implementation's arbitrary 1e9 exponent/offset
  saturation. The replacement keeps exact mantissa offsets and an overflow-safe
  input-length-derived exponent cap whose value cannot be cancelled by the
  mantissa offset. Full original input still reaches from_chars after validation.
- Independent review found missing parseInt long-leading-zero witnesses. Root
  added them, plus long mantissa/exponent cancellation and large exponent cases,
  deriving expectations solely from the pinned runtime before native execution.

## Independent verification

`input-host-oracle.cjs --check` pins Node v24.13.0 and proves the frozen fixture
is unchanged. The final suite passes **3,811 assertions in 87 tests**: 1,013
Number inputs, 944 parseInt inputs, four invalid-enum assertions, 112 UTF-16
strings, 68 valid UTF-8 decodes and 60 invalid sequences with known prefixes.
Numeric comparison is bit-exact including signed zero; all NaNs are equivalent.
Unicode units, codepoints, bytes and offset tables compare exactly.

Root ran `cmd /c build_test.bat 2>&1` after the final implementation/fixture
changes; the complete core suite passes. Logs stay in ignored build directories.
No platform or WinUI changes require a platform build for this packet.

The isolated mutation runner first compiles and passes an unchanged baseline,
then requires each mutation to compile and exit with assertion failure rather
than crash. Thirteen mutations were detected before the exponent-counter
refinement; numeric mutations were rerun after it, with an additional reversed
decimal range-classification mutation. Coverage includes signed zero, trim,
signed radix, complete consumption, guard/sticky rounding, integer sign,
surrogate pairing/iteration, UTF-8 surrogate rejection, pair byte offsets,
replacement encoding and BOM preservation. Working modules/fixture byte
snapshots must remain unchanged across each run; overlapping runs are rejected.

Normal checks:

```powershell
node tools/math-port/input-host-oracle.cjs --check
cmd /c build_test.bat "*math_port_input_host*" 2>&1
node tools/math-port/input-host-mutation-test.cjs
```

## Explicit unaccepted caller obligations

The enabled maction/toggle path passes a numeric selection value to parseInt.
Root independently verified default number 1 becomes 2 after nextToggleSelection.
A TS string cast does not perform conversion. The future MML contract must
preserve this through a reviewed value-to-string adapter, using the accepted
Number formatter and this parser; no feature exclusion is authorized.
Raw scriptlevel relational conversion and mstyle toString likewise remain
caller obligations. This packet accepts string parsing only, not these adapters.

Codepoint indices are native size_t rather than arbitrary JS values. UTF-8
DecodeError offsets are a root-defined diagnostic convention; fatal TextDecoder
only supplies rejection, not offsets. Decimal conversion relies on the tested
MSVC from_chars implementation; deterministic samples and mutations are evidence,
not an exhaustive proof over all strings or another compiler. MML contracts must
resolve remaining attribute value/identity and initialization semantics before
any mathematical-node implementation is dispatched.
