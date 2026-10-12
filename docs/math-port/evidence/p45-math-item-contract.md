# P45 core MathItem contract

Root freezes core/MathItem.ts against MathJax 4.1.3 SHA-256
c1c7308320ae15514fd1114c3f94642a4e8a8268bfe6f68192e8a557829626ce.
One real core owner supplies the Source MathItem interface/fields and
AbstractMathItem algorithms, protoItem, STATE and newState. No parser-specific
MathItem, mirror state or dependency on the TeX parser module is allowed.

## Responsibilities

- math_item.ixx: actual core MathItem/AbstractMathItem classes and typed generic
  engine edges. All expression fields live only in MathItem's actual object.
- math_item_bindings.cpp: Root's single mutable STATE table (Source constants).
- math_item_state.cpp: Luna A only: constructor, protoItem, isEscaped, setMetrics,
  state, reset, newState. No calls into another implementation of lifecycle.
- math_item.cpp: Luna B only: render, rerender, convert, compile, typeset,
  updateDocument, removeFromDocument, clear. Original base no-ops stay no-ops.
- math_item_host.ixx: Root's typed nullable member access, current STATE reads,
  ordinary inherited-key membership and primitive comparison helpers. No item
  algorithm, alternate document, fake renderer or extra graph.
- Root alone owns oracles, generated tests, fault injection/builds and acceptance.

The locked production CJS is the independent original observer, reached through
the existing validated source-loader chain. Tests supply actual callback service
instruments; they never implement the MathItem algorithms being compared.

## Native generic binding

Source MmlNode uses the accepted NodeValue and same existing graph. Generic N
typeset output is a tagged Null/Missing/native shared output object. A native
Location may carry any N/T node and optional i/n/delim; the complete supplied
location object is preserved opaque by NativeObjectRef. Ordinary scalar location
records use the existing shared Property graph. AbstractMathItem never inspects
location fields; it stores supplied objects without copying or reducing them.
Missing constructor start/end create distinct fresh scalar defaults; explicit
Null remains Null. protoItem's nested start/end records are Source {n:offset}.

InputJax.compile, OutputJax.typeset/escaped, document renderActions and outputJax
are borrowed native interfaces. Actual future jax/document objects implement
these edges; this packet does not supply a complete MathDocument or output backend.
There is no object named MathDocument pretending to be the original full class.
TeX and Tags consume this same real MathItem via math/display/inputData/state;
BaseMethods mutates display on it, not on a shadow. Browser DOM discovery and
concrete document insertion overrides remain their original separate boundary.

Fields math/display/metrics/inputData/outputData use scalar shared Property records;
display Null is the escaped sentinel. Metrics starts empty and setMetrics rebinds
a fresh ordered record. STATE uses live shared ordinary object semantics,
including prototype names for newState's in check and existing Undefined slots.
Arbitrary evaluated descriptors/functions/Symbols and malicious native pointer
lifetimes are not general JS bindings. All borrowed jax/document edges must remain
alive for the item operations; shared MML/output owners retain their identities.

## Source ordering and gate

Constructor fields are fresh, default parameters apply only to Missing, raw state
initializes from live UNPROCESSED. state(nullish) is getter-only. Setter checks
INSERTED/removal first, then re-reads raw state and current TYPESET/COMPILED before
resetting corresponding data records, finally writes raw target. A virtual removal
may mutate raw state, constants or throw; earlier effects must remain.

render/rerender/convert use live document action receivers; default start/end are
resolved before callbacks. compile/typeset read virtual state before current
threshold; invoke selected service; store successful root/output first; read current
completion threshold and call virtual state setter. Output receiver is resolved
before virtual isEscaped selection, so callbacks cannot redirect it afterward.
reset dispatches virtual state with live UNPROCESSED and normalized restore;
removeFromDocument dispatches virtual clear. No blanket guard or fallback.

Root must prove stubs compile and fail the initial 90 original observations,
then delegate the two exclusive body files. Final acceptance requires original
gold, review, compiling fault detection and full core regressions. This is a core
prerequisite, not actual TeX -> SVG or Folia integration. Mermaid stays paused.
