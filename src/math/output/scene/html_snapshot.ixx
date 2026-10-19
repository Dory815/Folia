/*************************************************************
 * Immutable, Context-free capture of a canonical output DOM subtree for a
 * foreignObject. Lite deliberately has no namespace model, so the consumer
 * supplies Source-directed namespace hints at this boundary.
 *************************************************************/
export module folia.math.output.scene.foundation:html_snapshot;
export import :html_types;
export import folia.math.core.dom_adaptor;

export namespace folia::math::output::scene {
HtmlNodePrimitive snapshotHtmlNode(const AbstractDOMAdaptor&,DOMNodeRef,
                                   std::u16string_view namespaceHint);
}
