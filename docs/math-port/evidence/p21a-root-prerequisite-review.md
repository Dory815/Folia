# P21a root prerequisite review — NodeUtil and child arrays

Root has inspected pinned NodeUtil.ts and the native declarations identified
in both read-only audits. This review does not accept NodeUtil, freeze a new
storage API or authorize agent edits to the node model. Source acceptance
remains 53/246; existing dense-node evidence cannot prove sparse behavior.

NodeUtil exports 24 methods and one live mutable attrs Set. Its direct child
assignment and live alias behavior cannot be translated into append/replace
tree methods. getChildAt reads sparse/absent slots as undefined; copyChildren
reads them and writes present undefined. Superscript-first source writes index
2 while index 1 can be absent; other TeX paths insert explicit null. Native
shared<vector<Product*>> aliases correctly but loses hole/present-undefined/
null distinctions, and indexed writes do not grow it. This is a real required
boundary correction, not a fallback or source-text special case.

The single authoritative replacement must retain live identity/rebinding,
length and slot presence, four distinguishable read/presence states, correct
indexed assignment, iteration and relevant array operations. Root must freeze
its full API and actual-Source observations before implementation and migrate
the existing FactoryChildren boundary, all node callers and tests together.
Do not keep parallel dense/sparse stores or add a compatibility selector.

The existing PropertyArray already models presence and undefined in the
Property graph, but Property currently cannot carry node identity. Reusing
that mechanism would require a deliberate node/object identity and import
boundary decision; the audit does not authorize adding a second typed mirror
or silently converting all falsey values into nullptr. All core modules must
remain portable C++23 modules with import std; platform APIs stay elsewhere.

Root corrected an initial agent claim about String.fromCodePoint(NaN) using
actual Node: it throws RangeError, not NUL. The revised report correctly maps
createEntity to existing parseInt(Hex) and unicodeString(single number). This
does not require a new numeric host or speculative conversion adapter.

The source-object receiver matters: copyChildren dispatches this.setChild;
getForm/getMoAttribute dispatch this.getOp; attrs is live mutable state.
getOp reads raw static OPTABLE and must not route through constructor-specific
lookup/range/default algorithms. copyAttributes aliases Attributes before its
property-entry snapshot; setProperties snapshots keys and reads values live.
The empty/Text/XML RTTI family also matters for isNode. These responsibilities
remain for a separately frozen NodeUtil implementation with independent
Source cases and all existing node-model regression checks.
