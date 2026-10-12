# P44 UnitUtil acceptance and parser-context ownership

Root accepts input/tex/UnitUtil.ts's nine responsibilities plus its private
muReplace, within the declared Source-generated pattern profile. Native math
still has no complete configured TeX parser, mathematical layout or SVG entry.

## Evidence

- 62e0755 moves ParserContext/ParseInput and parser-dependent callable functions
  into their sole tex_parser_context owner above Stack. Pure types retain value
  and StackItem identities; token-map declarations consume the moved context.
  Existing callable tests pass (build/p44-context-callable.log), and the original
  44-case/128-assertion token oracle reproduces unchanged values with --check.
  No state, parallel context, compatibility import or duplicate bridge is added.
- 3cd3fc1 freezes actual UnitMap/UnitUtil declarations, native pattern/value host,
  default unit table/singleton and original observations. Compiled stubs fail
  all 182 original assertions, with zero passed
  (build/p44-unit-util-stub-red.log).
- Luna implements only unit_util.cpp. The first body matches 182/182 original
  records. Root adds every ECMAScript whitespace code point, two exclusions,
  rest truthiness and large-exponent trailing-zero behavior, reaching 245/245.
  Singleton/map owners disallow native copying; shared handles retain identity.
- Source comparison covers ordered map construction/updates, duplicate keys,
  same-instance set, failed-delete identity, live factor/pt fallback, current
  regex handles, exact metadata, empty map/prefix backtracking, comma conversion,
  valid/invalid dimensions, signed zero/extreme numbers, mu rounding and retained
  UTF-16 match length, one-space rest consumption, non-BMP custom units,
  non-string TypeErrors and trim pass-through/escaped space behavior.
- All sixteen independently injected compiling semantic faults are detected;
  baseline and restored bodies pass. See p44-unit-util-mutations.json and
  tools/math-port/unit-util-mutation-test.cjs. No agents write during this gate.
- Independent Luna reviews declarations/host/gold and all method bodies against
  the pinned TS and locked production CJS. Body hash before fault injection
  equals the restored hash below. The review explicitly excludes whole-parser
  and SVG acceptance.
- Full normal build_test.bat passes, exit 0 (build/p44-full-core.log), including
  moved callable/token-map/handler regressions, prior MML/stack/factories and all
  editor/core tests. UnitUtil and token oracles reproduce with --check. Build
  products are ignored.

Final body SHA-256:
c35d1016fd25324d72a73ef0b0218a2186d2dcfc9ca83ca5f6f138e38eb1d153.
Pattern host SHA-256:
3da6decee219a7be7705d238288de9fe48f661fd907a1b1797664c420a9cbcf3.

## Scope and next boundary

No util/lengths algorithm replaces UnitUtil, no QuickJS is introduced, and no
fallback hides an unbound regex profile. UnitMap mutations are preserved for
normal literal units, although enabled packages make no external writes to
UNIT_CASES/num/regex fields. Arbitrary regex injections and replacement grammars
remain outside the explicitly documented native engine profile.

106 module interfaces are compilation structure, not a completion percentage.
The next actual parser-class ownership decision is p45-parser-module-ownership.md,
which supersedes the unimplemented broad-service plan in p44-parser-services-plan.md.
Actual Tags/configuration/base/AMS/packages, fonts/common layout, SVG parity and
Folia integration remain. Mermaid stays paused; its pending files are preserved.
