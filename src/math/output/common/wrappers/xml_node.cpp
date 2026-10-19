module folia.math.output.common;
import :xml_node;
import :wrapper;
import :output_jax;
import folia.math.core.mml_tree.xml_dom_adaptor;
import folia.math.core.math_item;
import folia.math.util.js_number;
import folia.math.util.option_types;
import folia.math.util.property_host;
import folia.math.util.string;
import folia.math.util.utf16;
import std;

namespace folia::math::output::common {
namespace {
[[noreturn]] void typeError(std::u16string text){throw options::Exception("TypeError",std::move(text));}
std::u16string sourceString(options::Context& context,options::Value value){
    const auto primitive=context.toPrimitive(std::move(value),options::PrimitiveHint::stringHint);
    if(const auto* text=std::get_if<std::u16string>(&primitive.data))return *text;
    if(const auto* number=std::get_if<double>(&primitive.data))
        return property_host::text(js_number::toString(*number));
    if(const auto* boolean=std::get_if<bool>(&primitive.data))return *boolean?u"true":u"false";
    if(std::holds_alternative<options::Null>(primitive.data))return u"null";
    if(std::holds_alternative<options::Undefined>(primitive.data))return u"undefined";
    if(std::holds_alternative<options::SymbolRef>(primitive.data))
        throw options::Exception("TypeError",u"Cannot convert a Symbol value to a string");
    throw options::Exception("TypeError",u"Source fontFamily result did not become a primitive");
}
}

CommonXmlNode::CommonXmlNode(CommonWrapperFactory& factory,mml::MmlNode& source,CommonWrapper* parentValue)
    :CommonWrapper(factory,source,parentValue){}

void CommonXmlNode::initializeSource(){
    CommonWrapper::initializeSource();
    rscale=getRScale();
}
void CommonXmlNode::getStyles(){}
void CommonXmlNode::getScale(){}
void CommonXmlNode::getVariant(){}

Property CommonXmlNode::getHDW(DOMNodeRef xml,std::string_view use,
                               std::optional<std::string_view> force){
    const auto forceValue=force.value_or(use);
    const Property hdw=adaptor().getAttribute(xml,u"data-mjx-hdw");
    const auto optionsValue=jax().optionsContext().get(jax().options,options::Key(u"htmlHDW"));
    const auto useText=utf16::decode(use).text,forceText=utf16::decode(forceValue).text;
    const auto* option=std::get_if<std::u16string>(&optionsValue.data);
    return property_host::truthy(hdw)&&option&&(*option==useText||*option==forceText)
        ?hdw:Property(PropertyNull{});
}

std::array<double,3> CommonXmlNode::splitHDW(const Property& hdw)const{
    const double scale=1/metrics().scale;
    auto values=string_util::split(property_host::string(hdw));
    std::array<double,3> result{};
    for(std::size_t i=0;i<result.size();++i){
        const auto value=i<values.size()&&!values[i].empty()
            ?Property(property_host::debugText(values[i])):Property("0");
        result[i]=length2em(value)*scale;
    }
    return result;
}

Property CommonXmlNode::getFontStyles()const{
    auto& output=const_cast<CommonXmlNode*>(this)->jax();
    auto& context=output.optionsContext();
    const options::Value adaptor=context.get(options::Value(output.instanceIdentity()),options::Key(u"adaptor"));
    const auto& m=metrics();
    std::u16string family;
    if(!parent)typeError(u"Cannot read properties of null (reading 'styles')");
    if(parent->styles)family=parent->styles->get(u"font-family");
    if(family.empty()&&property_host::truthy(m.family))family=property_host::string(m.family);
    if(family.empty()){
        const options::Value fontFamily=context.get(adaptor,options::Key(u"fontFamily"));
        const options::Value parentMethod=context.get(adaptor,options::Key(u"parent"));
        if(!output.math)typeError(u"Cannot read properties of null (reading 'start')");
        auto document=std::get_if<core::MathDocumentService*>(&output.document);
        if(!document||!*document)typeError(u"Cannot read properties of null (reading 'locationService')");
        const auto location=core::readMathLocation(output.math->start,"node",&(*document)->locationService());
        const auto value=propertyNode(location);
        const auto* ref=std::get_if<DOMNodeRef>(&value);
        if(!ref)typeError(u"Math start node is not an output DOM node");
        const std::array<options::Value,1> parentArguments{output.sourceDOMValue(*ref)};
        const auto parentValue=context.callMember(parentMethod,adaptor,parentArguments,u"adaptor.parent");
        const std::array<options::Value,1> familyArguments{parentValue};
        const auto familyValue=context.callMember(fontFamily,adaptor,familyArguments,u"adaptor.fontFamily");
        family=options::truthy(familyValue)?sourceString(context,familyValue):u"initial";
    }
    if(family.empty())family=u"initial";
    PropertyList styles;
    styles.setOwn("font-family",Property(property_host::debugText(family)));
    styles.setOwn("font-size",Property(property_host::debugText(
        utf16::decode(output.sourceFixed(m.em*rscale)+"px").text)));
    return Property(styles);
}

DOMNodeRef CommonXmlNode::getHTML(){
    auto* xmlNode=dynamic_cast<mml::XMLNode*>(node);
    if(!xmlNode)typeError(u"CommonXmlNode requires an XMLNode");
    auto& output=jax();
    auto& adaptor=output.domAdaptor();
    auto html=adaptor.clone(output.htmlAdaptor().resolve(xmlNode->getXML()),true);
    auto styles=PropertyList::record(getFontStyles()).value();
    const auto hdw=getHDW(html,"force");
    const double scale=jax().optionsContext().toNumber(jax().optionsContext().get(jax().options,options::Key(u"scale")));
    if(property_host::truthy(hdw)||scale!=1){
        html=addHDW(std::move(html),styles);
    }
    PropertyList definition;
    if(!parent)typeError(u"Cannot read properties of null (reading 'variant')");
    definition.setOwn("variant",Property(property_host::debugText(utf16::decode(parent->variant).text)));
    definition.setOwn("style",Property(styles));
    PropertyArray children;children.push(Property(nodeProperty(html)));
    return jax().html(u"mjx-html",std::move(definition),std::move(children));
}

BBox CommonXmlNode::measureXmlNode(DOMNodeRef xml){
    auto& output=jax();auto& adaptor=output.domAdaptor();
    const auto content=output.html(u"mjx-xml-block",
        PropertyList{{"style",Property(PropertyList{{"display",Property(std::string("inline-block"))}})}},
        PropertyArray{Property(nodeProperty(adaptor.clone(xml,true)))});
    const auto base=output.html(u"mjx-baseline",
        PropertyList{{"style",Property(PropertyList{{"display",Property(std::string("inline-block"))},
            {"width",Property(0)},{"height",Property(0)}})}},PropertyArray{});
    const auto style=getFontStyles();
    const auto measure=output.html(u"mjx-measure-xml",
        PropertyList{{"style",style}},
        PropertyArray{Property(nodeProperty(base)),Property(nodeProperty(content))});
    if(!output.math)typeError(u"Cannot read properties of null (reading 'start')");
    const auto* document=std::get_if<core::MathDocumentService*>(&output.document);
    if(!document||!*document)typeError(u"Cannot read properties of null (reading 'locationService')");
    const auto location=core::readMathLocation(output.math->start,"node",&(*document)->locationService());
    const auto startNodeValue=propertyNode(location);
    const auto* startNode=std::get_if<DOMNodeRef>(&startNodeValue);
    if(!startNode)typeError(u"Math start node is not an output DOM node");
    const std::array<options::Value,1> parentArguments{output.sourceDOMValue(*startNode)};
    const options::Value parentValue=output.sourceAdaptorCall(u"parent",parentArguments);
    const std::array<options::Value,2> appendContainerArguments{
        parentValue,output.sourceDOMValue(output.container)};
    (void)output.sourceAdaptorCall(u"append",appendContainerArguments);
    const std::array<options::Value,2> appendMeasureArguments{
        output.sourceDOMValue(output.container),output.sourceDOMValue(measure)};
    (void)output.sourceAdaptorCall(u"append",appendMeasureArguments);
    const auto contentBox=adaptor.nodeBBox(content);
    const auto baseBox=adaptor.nodeBBox(base);
    const double emValue=metrics().em*metrics().scale*rscale;
    const double width=(contentBox.right-contentBox.left)/emValue;
    const double height=(baseBox.top-contentBox.top)/emValue;
    const double depth=(contentBox.bottom-contentBox.top)/emValue-height;
    adaptor.remove(output.container);
    adaptor.remove(measure);
    return BBox(BBoxData{.w=width,.h=height,.d=depth});
}

void CommonXmlNode::computeBBox(BBox& target,bool){
    auto* xmlNode=dynamic_cast<mml::XMLNode*>(node);
    if(!xmlNode)typeError(u"CommonXmlNode requires an XMLNode");
    target.empty();
    const auto xml=jax().htmlAdaptor().resolve(xmlNode->getXML());
    const auto hdw=getHDW(xml,"use","force");
    if(property_host::truthy(hdw)){
        const auto [h,d,w]=splitHDW(hdw);target.h=h;target.d=d;target.w=w;
        return;
    }
    target.updateFrom(measureXmlNode(xml));
}
}
