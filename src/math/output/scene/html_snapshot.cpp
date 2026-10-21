module folia.math.output.scene.html_snapshot;
import folia.math.adaptors.lite_adaptor;
import std;

namespace folia::math::output::scene {
namespace {
std::u16string child_namespace(std::u16string_view kind, std::u16string_view inherited) {
    if (kind == u"svg")
        return u"http://www.w3.org/2000/svg";
    if (kind == u"math")
        return u"http://www.w3.org/1998/Math/MathML";
    if (kind == u"foreignObject")
        return u"http://www.w3.org/1999/xhtml";
    return std::u16string(inherited);
}
}

HtmlNodePrimitive snapshotHtmlNode(AbstractDOMAdaptor const &adaptor, DOMNodeRef source,
                                   std::u16string_view namespace_hint) {
    if (!source)
        throw std::invalid_argument("HTML snapshot source node is null");
    HtmlNodePrimitive result;
    result.kind = adaptor.kind(source);
    result.namespaceHint = child_namespace(result.kind, namespace_hint);
    if (result.kind == u"#comment") {
        auto comment =
            std::dynamic_pointer_cast<folia::math::adaptors::lite::LiteComment const>(source);
        if (!comment)
            throw std::invalid_argument("HTML comment snapshot requires a typed LiteComment");
        result.text = comment->value();
        return result;
    }
    if (result.kind == u"#text") {
        result.text = adaptor.value(source);
        return result;
    }
    for (auto const &attribute : adaptor.allAttributes(source))
        result.attributes.emplace_back(attribute.name, attribute.value);
    auto const children = adaptor.childNodes(source);
    for (auto const &child : children) {
        if (!child)
            throw std::invalid_argument("HTML snapshot contains a null child");
        result.children.push_back(snapshotHtmlNode(adaptor, child, result.namespaceHint));
    }
    return result;
}
}
