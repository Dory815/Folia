# G170 helper fixture/lite-owner candidate

This candidate is a narrow patch against the G169 tree. It contains two changes required by the remaining assertions in `native_document_typeset_uses_actual_svg_helper_return_identity`:

* The Source-owned `parent` property erase is routed through the bound `LiteNodeOwnerView::removeProperty` operation. Reads and own-property checks already route through the live view, while the old `LiteNodePropertyBinding::erase()` only removed the detached fallback property list. Other names retain the generic property-record erase behavior.
* The custom `@@iterator` fixture now creates a fresh iterator and a fresh per-iterator cursor for every traversal. A separate global `nextCalls` counter only measures calls. This matters because `childNodes` and Scene freezing independently consume the same iterable. Each traversal now yields the same two ordered children and calls `next()` three times (two values and `done`), preserving the existing expectations of two getter calls, six `next()` calls, and two frozen children.

Static source path: `LiteAdaptor.childNodes` uses the Source spread operation (`[...node.children]`); `SourceChildOwner::iterable()` calls `Context::readIterable`; Scene snapshotting uses that same owner. The fixture therefore has to be re-iterable, with each iterator starting at index zero.

## Validation and limits

* `git apply --check tools/math-port/candidates/scene-helper-fixture-g170/scene-helper-fixture-g170.patch` passes against the current G169 tree.
* An independent Node fixture exercised the same iterable/cursor logic twice and produced `first=[comment,text]`, `second=[comment,text]`, `iteratorGetterCalls=2`, `nextCalls=6`.
* No C++ build, test run, formal-tree edit, or candidate application was performed. Root must run the focused Foundation test after integrating the patch to verify the narrow parent-owner erase path and all final assertions.
