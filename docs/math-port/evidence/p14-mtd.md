# P14 `MmlMtd` method mapping

Pinned source: MathJax 4.1.3 `ts/core/MmlTree/MmlNodes/mtd.ts`, SHA256
`8b888aec4069818407079e84c42247495e04bd93ff52a3fd28bff65ca7635cfb`.
The root-frozen inventory records `verifyChildren` at lines 84–94 (member
SHA256 `12eb4f8cf2cef7befcbd1708f039314a70abcfb22b0ab6237e9ec23577320d44`)
and `setTeXclass` at lines 99–103 (member SHA256
`0a1ac83d27f23dc8d3baaf996e919f671232137dd4c1e4a492532309c846f472`).

| Source method | Native mapping |
| --- | --- |
| `verifyChildren(options)` | `src/math/core/mml_tree/mml_nodes/mtd.cpp:12-23`. Under `TreeCallGuard`, test the direct `parent` pointer and call its factory-backed `isKind("mtr")`. For a non-null invalid parent, build the source message using the current `kind()`, call `mError` with the same `PropertyList` handle wrapped in `ErrorOptions` and `shortError=true`, then return without superclass traversal. For null or a valid parent, call the inherited `AbstractMmlBaseNode::verifyChildren(options)`. Since `mlabeledtr` derives from `mtr`, factory class identity makes it pass the same `isKind` test. |
| `setTeXclass(prev)` | `src/math/core/mml_tree/mml_nodes/mtd.cpp:25-31`. Under `TreeCallGuard`, call `getPrevClass(prev)` first; then capture the current child array and require child 0 through the frozen table host; call child 0's virtual `setTeXclass(nullptr)`, ignore its result, and return this cell. A regular cell's child 0 is its inferred row. |

No defaults, getters, table repairs, or geometry are implemented in this file;
root owns those bindings. No tests or builds were run for this assignment. Root
owns source review and acceptance; this mapping is not an acceptance claim.
