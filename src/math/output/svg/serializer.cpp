module folia.math.output.svg.serializer;
import folia.math.adaptors.lite_adaptor;
import folia.math.cancellation;
import folia.math.util.utf16;
import std;

namespace folia::math::output::svg {
namespace {
using folia::math::AbstractDOMAdaptor;
using folia::math::DOMAttributes;
using folia::math::DOMNodeRef;
using folia::math::adaptors::lite::LiteAdaptor;
using folia::math::output::scene::HtmlNodePrimitive;

DOMNodeRef materializeHtml(AbstractDOMAdaptor &adaptor, HtmlNodePrimitive const &source) {
    cancellation::checkpoint(cancellation::CheckpointStage::Serialization);
    if (source.kind == u"#text") {
        auto result = adaptor.text(source.text);
        if (!result)
            throw std::logic_error("Lite DOM failed to materialize HTML text");
        return result;
    }
    if (source.kind == u"#comment") {
        auto result = adaptor.comment(source.text);
        if (!result)
            throw std::logic_error("Lite DOM failed to materialize HTML comment");
        return result;
    }

    DOMAttributes attributes;
    attributes.reserve(source.attributes.size());
    for (auto const &[name, value] : source.attributes) {
        cancellation::checkpoint(cancellation::CheckpointStage::Serialization);
        attributes.push_back({name, value});
    }
    std::vector<DOMNodeRef> children;
    children.reserve(source.children.size());
    for (auto const &child : source.children) {
        auto node = materializeHtml(adaptor, child);
        if (!node)
            throw std::logic_error("Lite DOM returned a null HTML child");
        children.push_back(std::move(node));
    }
    auto result = adaptor.node(source.kind, attributes, children, source.namespaceHint);
    if (!result)
        throw std::logic_error("Lite DOM failed to materialize an HTML element");
    return result;
}
} // namespace

std::string serializeHtml(folia::math::output::scene::HtmlNodePrimitive const &source) {
    LiteAdaptor adaptor;
    auto root = materializeHtml(adaptor, source);
    if (!root)
        throw std::logic_error("Lite DOM produced a null HTML root");
    return folia::math::utf16::encode(adaptor.serializeXML(root));
}
} // namespace folia::math::output::svg
