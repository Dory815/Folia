# P35 Root: subclass verification with raw options

Root assigns Luna only MmlMath::verifyTree in mml_nodes/math.cpp,
HtmlNode::verifyTree in mml_nodes/html_node.cpp and MmlMprescripts::verifyTree /
MmlNone::verifyTree in mml_nodes/mmultiscripts.cpp, plus
p35-node-verification-implementation.md. All paths are under src/math/core/
mml_tree except the report under docs/math-port/evidence. No other methods,
declarations, helpers, fixtures, expectations, manifests, builds/commits/spawns
or Folia edits. In particular mmultiscripts' other two stub bodies stay outside
this packet; do not silently implement them or the table/multiscript host stubs.

Read original MathJax 4.1.3 ts/core/MmlTree/MmlNodes/{math,HTMLNode,
mmultiscripts}.ts and the accepted P30/P32 coordinate/defaulting contracts.
These methods own only their original verification rules; use the current
semantic tree and virtual parent/error access. No render or parser logic.

MmlMath alone has its own Source default: Missing -> local Null. Call qualified
super verifyTree with that local raw value; then re-read current parent and, if
truthy, virtual mError with original message/options/short=true. Exact Null
does not skip this later derived check. Super's returned result is ignored.

HtmlNode does not normalize Missing or call super. Preserve current parent
short-circuit and the second parent read for !isToken, then virtual mError with
the exact HTML message and raw options/short=true, followed by return.

Mprescripts and None have no local Missing default. Their super call defaults
only its own copy, leaving the derived original raw options untouched. Preserve
the original current parent reads and exact isKind condition/error messages.
Use getParent plus tagged requireNode, not the stub multiscript host helpers.
Preserve every virtual callback and exception after already completed effects.
Use existing TreeCallGuard; no optional-object options converter.

Root owns the already-frozen simple-mml/html-node/multiscript Source fixtures
and P30 original options-boundary observations. They reproduce independently.
Root will inspect the current failed baseline, review the bodies, isolate these
methods' Source tests, add raw option witnesses and inject compiling faults.
No full MML/parser/layout/SVG/Folia completion follows from this packet.
