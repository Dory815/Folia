module folia.math.output.scene.foundation;
import :html_snapshot;
import folia.math.core.dom_node;
import folia.math.util.property_host;

namespace folia::math::output::scene {
namespace {
std::u16string childNamespace(std::u16string_view kind,std::u16string_view inherited){
    if(kind==u"svg")return u"http://www.w3.org/2000/svg";
    if(kind==u"math")return u"http://www.w3.org/1998/Math/MathML";
    if(kind==u"foreignObject")return u"http://www.w3.org/1999/xhtml";
    return std::u16string(inherited);
}
}

HtmlNodePrimitive snapshotHtmlNode(const AbstractDOMAdaptor& adaptor,DOMNodeRef source,
                                   std::u16string_view namespaceHint){
    HtmlNodePrimitive result;
    result.kind=adaptor.kind(source);
    result.namespaceHint=childNamespace(result.kind,namespaceHint);
    if(result.kind==u"#comment"){
        // LiteAdaptor.value() intentionally strips comment delimiters. The
        // Source-created comment node's raw `value` field is the serialized
        // payload, so snapshot that field rather than the convenience getter.
        result.text=property_host::string(adaptor.getProperty(source,"value"));
        return result;
    }
    if(result.kind==u"#text"){
        result.text=adaptor.value(source);
        return result;
    }
    for(const auto& attribute:adaptor.allAttributes(source)){
        // property_host::string already performs the Source UTF-16 ToString
        // conversion.  Do not route it through the UTF-8 byte decoder.
        result.attributes.emplace_back(attribute.name,
                                       property_host::string(attribute.value));
    }
    const auto children=adaptor.childNodes(source);
    for(std::size_t index=0;index<children.slots.size();++index){
        const auto value=propertyNode(children.slots.get(index));
        const auto* child=std::get_if<DOMNodeRef>(&value);
        if(!child)property_host::typeError("HTML scene snapshot encountered a non-node child");
        result.children.push_back(snapshotHtmlNode(adaptor,*child,result.namespaceHint));
    }
    return result;
}
}
