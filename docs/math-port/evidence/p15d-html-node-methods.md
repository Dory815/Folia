# P15d HtmlNode method evidence

The pinned MathJax 4.1.3 source is `.reference/MathJax-src/ts/core/MmlTree/MmlNodes/HtmlNode.ts`, SHA-256 `1bf0ba981fc72b63a525424fb51e8bc45231fcf768d618fb8c75517bd72ae665`. The implementation is [html_node.cpp](../../../src/math/core/mml_tree/mml_nodes/html_node.cpp). MathJax's Apache-2.0 attribution is retained; see `src/math/LICENSE-MathJax.txt`.

## Method mapping

| Source member | Source lines | Native method and behavior |
| --- | ---: | --- |
| `HtmlNode.getHTML` | 45–47 | `HtmlNode::getHTML()` calls virtual `getXML()` and returns its exact `XmlRef`. No cached or direct-storage substitute is used. |
| `HtmlNode.setHTML` | 54–68 | `HtmlNode::setHTML()` attempts `getAttribute(html, "data-mjx-hdw")` and ignores the raw return. Its catch-all covers only that attempted call. The catch calls `node("span", PropertyList{}, {html})` using a fresh empty property list and the original payload. Errors from `node` escape; `setXML` is invoked outside the try and its actual virtual `XMLNode*` result is returned, including a different XML receiver. |
| `HtmlNode.getSerializedHTML` | 72–74 | Invokes the current `HtmlAdaptor::outerHTML(xml_)` and returns the host result. |
| `HtmlNode.textContent` | 79–81 | Invokes the current `HtmlAdaptor::textContent(xml_)` and returns UTF-16 unchanged, preserving lone surrogates and live host data. |
| `HtmlNode.toString` | 88–91 | Calls `HtmlAdaptor::kind(xml_)` exactly once and formats `HTML=<kind>...</kind>` from that result; it does not call semantic `HtmlNode::kind()`. |
| `HtmlNode.verifyTree` | 96–101 | Reads the direct parent; only a present non-token parent calls virtual `mError` with the exact message, preserved nullable options and `short=true`. It adds no base verifier, adaptor call, traversal, or normalization. |

`kind` at source lines 38–40 is generated/root-owned and is outside this six-method implementation packet. The inherited `AbstractMmlEmptyNode::mError` is the source-equivalent no-op for an HtmlNode; no error tree is fabricated here.

## Host adaptation and call boundaries

The private `htmlAdaptor` helper checks the typed `XmlAdaptor*` boundary with `dynamic_cast<HtmlAdaptor*>`; a null or XML-only adaptor raises the existing typed host `TypeError` at the attempted operation. In `setHTML`, such a failure during `getAttribute` enters the source catch and the fallback `node` access is attempted. If the adaptor is still null or XML-only, that fallback dispatch raises and escapes. Any exception from `getAttribute`, regardless of native exception type, takes the same fallback. An exception from fallback `node` is outside the `try`'s protected expression and propagates; exceptions from virtual `setXML` also propagate because the call is after the catch block.

`XmlRef` remains the shared opaque payload identity held by the inherited XMLNode state. The typed adaptor's five host methods do not add a second tree, cache, or normalized text field. `textContent()` uses UTF-16 because it feeds token text extraction, including lone surrogates. Serialized markup and kind labels use the existing internal `std::string` host boundary, whose string conversion is lossless WTF-8; external UTF-8 encoding/decoding is a separate boundary and must not normalize these values. The wrapper deliberately performs no DOM parsing, texhtml construction, measurement, or serialization policy.

The actual-CJS oracle was frozen by root before dispatch at 306 assertions across 68 cases. I did not edit interfaces, constructor bindings, fixtures, tests, CMake, or scope, and did not build or run tests. This report records source mapping only; root owns independent review, mutations, build checks, and acceptance.
