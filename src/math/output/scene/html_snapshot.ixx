/*************************************************************
 * Typed snapshot of a canonical output DOM subtree.
 *************************************************************/
export module folia.math.output.scene.html_snapshot;
export import folia.math.output.scene.html_types;
export import folia.math.core.dom_adaptor;
import folia.math.adaptors.lite_adaptor;
import std;

export namespace folia::math::output::scene {
HtmlNodePrimitive snapshotHtmlNode(AbstractDOMAdaptor const &adaptor, DOMNodeRef source,
                                   std::u16string_view namespaceHint);
} // namespace folia::math::output::scene
