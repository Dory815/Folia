module folia.math.core.dom_adaptor;
import folia.math.util.property_host;
import std;
namespace folia::math {
DOMNodeRef AbstractDOMAdaptor::node(std::u16string_view kind, PropertyList def, PropertyArray children, std::u16string_view ns) {
    auto result=create(kind,ns); setAttributes(result,def);
    for(std::size_t i=0;i<children.size();++i){
        auto child=propertyNode(children.get(i));
        if(!std::holds_alternative<DOMNodeRef>(child))property_host::typeError("DOM node child is not a node");
        append(result,std::get<DOMNodeRef>(child));
    }
    return result;
}
void AbstractDOMAdaptor::setAttributes(DOMNodeRef node,const PropertyList& def){
    auto style=def.get("style");
    if(auto styleObject=PropertyList::record(style)) {
        for(const auto& key:styleObject->keys()){
            std::u16string name=property_host::text(key),converted;converted.reserve(name.size());
            for(std::size_t i=0;i<name.size();++i){if(name[i]==u'-'&&i+1<name.size()&&name[i+1]>=u'a'&&name[i+1]<=u'z'){converted.push_back(static_cast<char16_t>(name[i+1]-32));++i;}else converted.push_back(name[i]);}
            setStyle(node,converted,property_host::string(styleObject->get(key)));
        }
    }
    auto properties=def.get("properties");
    if(property_host::truthy(properties)) if(auto object=PropertyList::record(properties)) for(const auto& key:object->keys())setProperty(node,key,object->get(key));
    for(const auto& key:def.keys()){
        if((key!="style"||std::holds_alternative<std::string>(style.value))&&key!="properties")
            setAttribute(node,property_host::text(key),def.get(key));
    }
}
DOMNodeRef AbstractDOMAdaptor::replace(DOMNodeRef node,DOMNodeRef oldNode){insert(node,oldNode);remove(oldNode);return oldNode;}
DOMNodeValue AbstractDOMAdaptor::childNode(DOMNodeRef node,double index)const{auto nodes=childNodes(node);if(index<0||!std::isfinite(index)||std::floor(index)!=index||index>=nodes.slots.size())return Missing{};return propertyNode(nodes.slots.get(static_cast<std::size_t>(index)));}
std::vector<std::u16string> AbstractDOMAdaptor::allClasses(DOMNodeRef node)const{
    auto classes=getAttribute(node,u"class");if(classes.missing()||!property_host::truthy(classes))return{};
    auto text=property_host::string(classes);std::vector<std::u16string> out;std::size_t start=0;
    while(start<text.size()){while(start<text.size()&&text[start]==u' ')++start;if(start==text.size())break;auto end=text.find(u' ',start);if(end==std::u16string::npos)end=text.size();out.emplace_back(text.substr(start,end-start));start=end+1;}return out;
}
std::u16string AbstractDOMAdaptor::cssText(DOMNodeRef node)const{return kind(node)==u"style"?textContent(node):std::u16string{};}
}
