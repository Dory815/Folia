# P40 parser utility contract

Root owns the architecture, module declarations, host adapters, original
observations, Native tests and acceptance. Mermaid remains paused. This packet
does not implement TeXParser, layout, SVG or Folia integration.

The source is pinned MathJax 4.1.3 (`fb987178c2d279a99b0db5ea02e751691703955e`):
`ts/input/tex/NodeUtil.ts` and `ts/input/tex/TexError.ts`. The existing MML graph,
Property host, Attributes and child arrays are the only data model. No second
AST, interpreter, generic JavaScript engine or formula-specific shortcuts.

## File ownership

- Root: `node_util.ixx`, its local receiver/property-access helpers, attrs membership and
  shared service; `tex_error.ixx`; complete fixed-pattern host
  `tex_error_pattern.ixx`; CMake; all fixtures and acceptance documents.
- Luna mml_row_mi: only the 21 newly stubbed semantic methods in
  `node_util.cpp`. The three accepted child methods and Root helpers are frozen.
- Luna math_layout: only TexError constructor and processString bodies in
  `tex_error.cpp`. Preserve the dedicated throwable class and public id/message.
- Luna mml_verify_nodes: independent read-only source/interface review.

Implementation is released after the compiled stub test fails against actual
original output. Agents must report every method/source range and any unmet
semantics, and must not modify tests, expected data, interfaces, hosts or build
files. Root reviews every method before accepting it.

## NodeUtil obligations

All 24 source methods retain separate responsibilities and virtual dispatch.
Enabled package child indices are integers; retain the existing size_t seam.
Parser heterogeneous arguments use canonical NodeValue or MmlNode* in std::any;
isNode tests AbstractMmlNode/AbstractMmlEmptyNode identity, including TextNode.
The eleven case-sensitive attrs names are membership-only in enabled packages;
the unordered native container does not claim Source Set enumeration parity.

setProperties snapshots Object.keys, then reads each current value. texClass is
assigned before setProperty; movablelimits is a property before conditional
mo/mstyle attribute assignment; inferred is ignored. Preserve short-circuit
queries, callbacks, insertion order, nullable values and partial mutations.
copyAttributes installs the same Attributes handle before snapshotting own
property entries, then invokes setters in order; do not pre-copy both objects.

getOp uses global MmlMo::operatorTable, defaults only Missing to infix, captures
the selected form table before invoking mo.getText, and returns truthy data or
Null. getForm invokes this service's getOp over the live forms array. Operator
attribute lookup checks isSet first, then infix/postfix/prefix with both optional
chains, returning the first non-Missing value (including Null/false/zero); only
then re-read the attributes receiver for the fallback get.

## TexError obligations

TexError is a plain distinct error class, not std::exception or MML Error.
Constructor assigns id before processing message. Native strings are UTF-16.
The fixed host implements both complete private patterns, including escaped
braces, backtracking, capture boundaries, non-Unicode dot, and line terminators.
It is not a general RegExp implementation.

processString preserves the source's whole-remainder lexicographic comparison
in the braced branch, its literal plural handling, number conversion timing,
nullish join behavior, malformed tokens and lack of recursive substitution.
Do not replace it with a conventional formatter or special-case one token.

## Acceptance

Original observations execute pinned CJS with normalized TS comparison and hash
checks. Expectations are generated before Native implementation. Compare UTF-16
strings, live identities, raw Property values, errors and callback/partial-write
order, including invalid input reachable through parser services. Run narrow
Source/Native checks and the complete normal core build/tests. Describe bounded
coverage honestly; no renderer acceptance follows from utility tests.
