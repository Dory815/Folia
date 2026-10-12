# P43 actual parser Stack acceptance

Root accepts the nine responsibilities of input/tex/Stack.ts. This supplies the
real parser stack over the existing actual BaseItem owners and MML graph. It
does not supply the configured base reducers, ParseOptions or TeXParser.

## Evidence

- afbee40 freezes Root-owned contracts, hosts, actual interfaces and independent
  original CJS observations. Compiled stubs fail all 68 initial assertions, with
  zero passed (build/p43-stack-stub-red.log).
- Luna changes only stack.cpp. Root catches an ambiguous native variant Null
  conversion at compilation. Root also requires original constructor-argument
  retention and immediate CheckTuple slot capture before virtual Pop; the
  replacement array itself is spread only after Pop completes. Luna corrects
  these at the implementation boundary. The input span is captured as Source
  rest-argument slots, retaining shared item/node identities.
- Root adds four original observations for tuple-slot rebinding, same-array
  mutation during Pop, input rest snapshots and own-only shared env assignment.
  The generated native Prev observer initially has four C++ operand evaluation
  order errors: it reads Size before Prev. Root sequences those observations
  explicitly; original Source expectations are not altered.
- All 72 original records now pass. Coverage includes constructor/global/env
  identity, falsey inputs, actual MML wrapping, nullable check results,
  success truthiness, recursive replacement/empty list, current globals,
  null-kind env, repeated env getter/copy/source/target order, inherited keys and
  nested aliases, partial effects on errors, inherited accessor deletion,
  Pop removal before getters, callback-mutated live top, Top Null/Missing/numeric
  edges, Prev dispatch and captured-length/live-slot string conversion.
- All sixteen independently injected semantic faults compile and fail the
  original assertions. Baseline and restored files pass. See
  p43-parser-stack-mutations.json and parser-stack-mutation-test.cjs.
- Independent Luna review compares actual TS and locked CJS to the interfaces,
  host and final body. The reviewer reconfirms the restored body hash after
  mutation testing; no substantive mismatch in the enabled typed profile.
- The full normal build_test.bat passes, exit 0
  (build/p43-stack-full-core.log), including prior MML/factory/StackItem/BaseItem
  and all existing editor/core regressions. The original oracle reproduces with
  --check. Build products stay ignored.

Final stack.cpp SHA-256:
0e427803ec399ed0bf326749a2eeb6b63a64c2ba4cf64edf0ecf57170b4e268d.
Root host SHA-256:
cd1ef7ad7cf019bb7c976689dc9d600d6866b275db9107f96a7bce719093852d.

## Scope

There is one parser-stack container of existing StackItemRef owners, separate
from each item's MML node list by the original responsibilities, not a duplicate
node graph. No factory shim, alternate registry, error fallback, temporary
parser, interpreter or SVG path is installed. Explicit typed package boundaries
and shared-owner/session lifetimes remain in p43-parser-stack-contract.md.

102 compiled module interfaces are structural evidence, not a percentage of
working TeX. Real configured base/AMS/enabled-package parsing, mathematical
font/layout, SVG formula parity and Folia native wiring remain required.
Mermaid remains paused with all pending work preserved.
