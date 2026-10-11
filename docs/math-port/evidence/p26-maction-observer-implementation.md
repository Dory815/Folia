# P26 maction observer binding repair

This change is limited to the generated native C++ header emitter in
`tools/math-port/maction-oracle.cjs`. It does not change the JavaScript source
computations, stimulus generation, expected records, value emitter, fixture,
production declarations, or MML algorithms. Root owns fixture regeneration,
original-source replay, compilation, and acceptance.

The oracle's frozen profile is 934 cases / 935 assertions / 2,318 source-value
records. The pinned `MmlMaction` source is MathJax 4.1.3
`ts/core/MmlTree/MmlNodes/maction.ts` (SHA-256
`fa173b7e2b25b90e0f83ac201098357a8abf28462fa86fa69d92b65906527895`); its
source-derived values and case suffix are untouched.

The native `children` fixture helper now creates the existing
`FactoryChildren<MmlNode>` / `ChildArray<MmlNode>` representation from tagged
`NodeValue` entries. This lets the unchanged setups express node and explicit
null entries through the current tree API; no parallel vector is introduced.
The `Receive` probe now implements the existing tagged virtual signatures for
`core`, `coreMO`, and `setTeXclass`. It returns `NodeValue` for node/null
results, while `Probe::updateTeXclass` accepts the existing tagged value. Local
`id` and `label` overloads project a `NodeValue` only at the original observer
boundary, where the JavaScript helper maps a nullish node to its `null` label.
The selected probe still follows its existing `MmlNode*` declaration and now
reads slot zero through `ChildArray::get` rather than vector indexing. Factory
product returns and the `mError` product return retain their existing pointer
interfaces.

The `observe` projection corresponding to JavaScript
`childNodes?.map(id) ?? null` captures the current child-array handle and its
initial length. For each initial index it performs `has` and then a live `get`,
calling `id` only for present properties. It resizes the result first and sets
only mapped slots, so holes remain holes while explicit nullish entries invoke
the source callback and become its `null` result. It does not follow a rebound
`childNodes` handle during the map.

`realNode` corresponds to the source's required `childNodes.map(realNode)`.
It retains the source object-field projection order, captures the child array
and initial length, uses `has`/`get` for each mapped slot, and preserves holes
in the result. A present nullish child reaches the `kind` member access and
raises a TypeError; a hole skips the callback, while inherited indexed values
are observed through `ChildArray`. Its native parameter is tagged so recursion
does not erase a source array slot before the original member access.

The actual JavaScript setup, test operations, callbacks, rebindings, and
expected values remain byte-for-byte outside the native header template. The
existing production `setChildInheritedAttributes` for-of behavior is not
rewritten by this packet; the ChildArray setup/probes now allow it to observe
the original distinctions between captured-array iteration and the observer's
map operations. Root must replay the frozen source profile and verify the
generated fixture and full-graph compilation independently. No build or test
was run here, and this report makes no acceptance claim.
