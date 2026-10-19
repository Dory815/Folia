/*************************************************************
 * Native owner resolution for MathJax 4.1.3 TexHtml's live Source adaptor.
 *************************************************************/
module folia.math.input.tex.texhtml.tex_html_services;
import folia.math.util.option_types;
import folia.math.util.property_host;
import std;

namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view name){return Key(std::u16string(name));}
}

TexHtmlServices resolveTexHtmlServices(Context& context,Value adaptor,
    const TexHtmlAdaptorResolver& resolver){
    // Match the nested Source expression's property lookup order. In
    // particular, nullish adaptors fail while reading childNodes before any
    // native owner resolution can replace that observable error.
    Value childNodesMethod=context.get(adaptor,key(u"childNodes"));
    Value bodyMethod=context.get(adaptor,key(u"body"));
    Value parseMethod=context.get(adaptor,key(u"parse"));
    const auto* identity=std::get_if<ObjectRef>(&adaptor.data);
    if(!identity)property_host::typeError("TexHtml requires the active LiteAdaptor instance");
    if(!resolver)property_host::typeError("TexHtml LiteAdaptor resolver is unavailable");
    auto dom=resolver(*identity);
    if(!dom)property_host::typeError("TexHtml adaptor is not a native LiteAdaptor instance");
    auto xml=folia::math::mml::XmlDomAdaptor::create(dom);
    return TexHtmlServices{std::move(dom),std::move(xml),*identity,
        std::move(childNodesMethod),std::move(bodyMethod),std::move(parseMethod)};
}
}
