module folia.math.adaptors.lite.parser;
import folia.math.cancellation;
import folia.math.adaptors.lite_adaptor;
import folia.math.util.entities;
import folia.math.util.js_case;
import folia.math.util.property_host;
import folia.math.util.regexp_host;
import std;

namespace folia::math::adaptors::lite {
namespace {
using regexp_host::Match;

void replaceAll(std::u16string& text,std::u16string_view from,std::u16string_view to){
    std::size_t at=0;
    while((at=text.find(from,at))!=std::u16string::npos){text.replace(at,from.size(),to);at+=to.size();}
}

bool isSpace(char16_t c) { return c == u' ' || c == u'\n'; }
// JavaScript evaluates the source's \uNNNN template-literal escapes before
// RegExp construction. Build those actual UTF-16 code units here.
std::u16string nameClass() {
    std::u16string result=u"[^";
    result.push_back(0); result+=u"-"; result.push_back(0x001F);
    result+=u" \"'>/=";
    result.push_back(0x007F); result+=u"-"; result.push_back(0x009F);
    result.push_back(u']');
    return result;
}
std::u16string spacePattern(){return u"[ \\n]+";}
std::u16string optionalSpacePattern(){return u"[ \\n]*";}
std::u16string tagNamePattern(){return std::u16string(u"[A-Za-z]")+nameClass()+u"*";}
std::u16string attributeNamePattern(){return nameClass()+u"+";}
std::u16string valuePattern(){return u"(?:'[^']*'|\"[^\"]*\"|"+spacePattern()+u")";}
std::u16string valueSplitPattern(){return u"(?:'([^']*)'|\"([^\"]*)\"|("+spacePattern()+u"))";}
std::u16string attributePattern(){return attributeNamePattern()+u"(?:"+optionalSpacePattern()+u"="+optionalSpacePattern()+valuePattern()+u")?";}
std::u16string attrSplitPattern(){return u"("+attributeNamePattern()+u")(?:"+optionalSpacePattern()+u"="+optionalSpacePattern()+valueSplitPattern()+u")?";}
std::u16string tagPattern(){
    const auto space=spacePattern(),optional=optionalSpacePattern();
    const auto tag=tagNamePattern(),attribute=attributePattern();
    return u"(<(?:"+tag+u"(?:"+space+attribute+u")*"+optional+u"/?|/"+tag+
        u"|!--[^]*?--|![^]*?)(?:>|$))";
}

std::vector<std::optional<std::u16string>> splitCaptures(
    std::u16string_view input, std::u16string_view pattern) {
    return regexp_host::split(input,std::u16string(pattern),u"u");
}

std::vector<std::u16string> splitTags(std::u16string_view input) {
    // Source first removes processing instructions with /<\\?.*?\\?>/g.
    std::u16string text(input);
    regexp_host::RegExp pi(u"<\\?.*?\\?>", u"g");
    std::u16string withoutPI;
    std::size_t copied = 0;
    while (const auto match = pi.exec(text)) {
        const auto whole = match->at(0);
        if (!whole) break;
        withoutPI.append(text, copied, match->index - copied);
        copied = match->index + whole->size();
    }
    withoutPI.append(text, copied, std::u16string::npos);

    const auto split=regexp_host::split(withoutPI,tagPattern(),u"u");
    std::vector<std::u16string> parts;
    parts.reserve(split.size());
    for(const auto& part:split){
        if(!part)property_host::typeError("LiteParser tag split produced an undefined capture");
        parts.push_back(*part);
    }
    return parts;
}

std::u16string asciiLower(std::u16string_view value) { return js_case::lowerCase(value); }
std::u16string htmlValue(const Property& value) { return property_host::string(value); }

LiteNodeRef nodeValue(const DOMNodeValue& value, std::string_view member) {
    if (const auto* node = std::get_if<DOMNodeRef>(&value)) return *node;
    if (std::holds_alternative<Missing>(value))
        property_host::typeError("Cannot read properties of undefined (reading '" + std::string(member) + "')");
    if (std::holds_alternative<PropertyNull>(value))
        property_host::typeError("Cannot read properties of null (reading '" + std::string(member) + "')");
    property_host::typeError("Cannot read properties of undefined (reading '" + std::string(member) + "')");
}

bool truthyProperty(const Property& value) {
    if (std::holds_alternative<Missing>(value.value) || std::holds_alternative<PropertyNull>(value.value)) return false;
    return property_host::truthy(value);
}

LiteBase& sourceLiteAdaptor(AbstractDOMAdaptor& adaptor) {
    auto* lite = dynamic_cast<LiteBase*>(&adaptor);
    if (!lite) property_host::typeError("LiteParser requires a LiteAdaptor");
    return *lite;
}

std::u16string joinParts(const std::vector<std::u16string>& values) {
    std::u16string result;
    for (const auto& value : values) result += value;
    return result;
}
}

LiteParser::LiteParser(std::shared_ptr<entities::EntityModule> entities)
    : entities_(entities ? std::move(entities) : std::make_shared<entities::EntityModule>()) {}

void LiteParser::staticPropertyReader(
    std::function<Property(std::u16string_view)> reader,
    std::function<Property(const Property&, std::u16string_view)> element,
    std::function<bool(const Property&, std::u16string_view)> hasOwn) {
    staticProperty_ = std::move(reader);
    staticElement_ = std::move(element);
    staticHasOwn_ = std::move(hasOwn);
}

void LiteParser::contextMethodBridge(
    std::function<options::Value(std::u16string_view,std::span<const options::Value>)> call,
    std::function<options::Value(std::u16string_view)> getMethod,
    std::function<options::Value(options::Value,std::u16string_view)> getProperty,
    std::function<void(options::Value,std::u16string_view,options::Value)> setProperty,
    std::function<options::Value(options::Value,std::span<const options::Value>)> callMethod,
    std::function<options::Value(AbstractDOMAdaptor&,std::u16string_view)> getAdaptorMethod,
    std::function<options::Value(options::Value,options::Value,std::span<const options::Value>)> callFunction,
    std::function<options::Value(std::function<options::Value(options::Value)>)> makeCallback,
    std::function<options::Value(NodeKind,std::span<const options::Value>)> constructNode,
    std::function<options::Value(const Property&)> valueFromProperty,
    std::function<std::u16string(const options::Value&)> stringValue,
    std::function<options::Value(AbstractDOMAdaptor&)> adaptorValue,
    std::function<options::Value(LiteNodeRef)> nodeValue,
    std::function<options::Value(const PropertyArray&)> arrayValue,
    std::function<PropertyArray(options::Value)> arrayFromValue,
    std::function<Property(options::Value)> propertyFromValue,
    std::function<LiteNodeRef(options::Value)> nodeFromValue,
    std::function<void(options::Value,const std::function<void(options::Value)>&)> forEachIterable,
    std::function<bool(options::Value,NodeKind)> instanceOf) {
    methodCall_=std::move(call);methodValue_=std::move(getMethod);
    propertyGet_=std::move(getProperty);propertySet_=std::move(setProperty);parserCall_=std::move(callMethod);
    adaptorMethod_=std::move(getAdaptorMethod);functionCall_=std::move(callFunction);callback_=std::move(makeCallback);
    constructNode_=std::move(constructNode);
    valueFromProperty_=std::move(valueFromProperty);stringValue_=std::move(stringValue);
    adaptorValue_=std::move(adaptorValue);
    nodeValue_=std::move(nodeValue);arrayValue_=std::move(arrayValue);
    arrayFromValue_=std::move(arrayFromValue);
    propertyFromValue_=std::move(propertyFromValue);
    nodeFromValue_=std::move(nodeFromValue);forEachIterable_=std::move(forEachIterable);
    instanceOf_=std::move(instanceOf);
}

namespace {
std::u16string arrayText(const PropertyArray& array,std::size_t index) {
    return property_host::string(array.get(index));
}
Property shiftProperty(PropertyArray& array) {
    const auto length=array.size();if(!length)return Missing{};
    const auto first=array.get(0);
    for(std::size_t i=1;i<length;++i){
        if(array.record().has(std::to_string(i)))array.set(i-1,array.get(i));
        else array.erase(i-1);
    }
    array.erase(length-1);array.resize(length-1);
    return first;
}
std::u16string shiftText(PropertyArray& array) {
    return property_host::string(shiftProperty(array));
}
std::u16string sourceStringArgument(const Property& value,std::string_view method){
    if(const auto* text=std::get_if<std::string>(&value.value))return property_host::text(*text);
    if(value.missing())property_host::typeError("Cannot read properties of undefined (reading '"+std::string(method)+"')");
    if(std::holds_alternative<PropertyNull>(value.value))property_host::typeError("Cannot read properties of null (reading '"+std::string(method)+"')");
    property_host::typeError(std::string(method)+" is not a function");
}
}

LiteNodeRef LiteParser::addText(AbstractDOMAdaptor& adaptor,LiteNodeRef node,std::u16string_view text){
    auto translated=entities_->translate(text);
    if(adaptorMethod_){
        const auto receiver=adaptorValue_(adaptor);
        const auto appendMethod=adaptorMethod_(adaptor,u"append");
        const auto textMethod=adaptorMethod_(adaptor,u"text");
        const std::array<options::Value,1> textArgs{options::Value(translated)};
        const auto textNode=functionCall_(textMethod,receiver,textArgs);
        const std::array<options::Value,2> appendArgs{nodeValue_(node),textNode};
        return nodeFromValue_(functionCall_(appendMethod,receiver,appendArgs));
    }
    return adaptor.append(node,adaptor.text(translated));
}
LiteNodeRef LiteParser::addComment(AbstractDOMAdaptor& adaptor,LiteNodeRef node,std::u16string_view comment){
    if(adaptorMethod_){
        const auto receiver=adaptorValue_(adaptor);
        const auto appendMethod=adaptorMethod_(adaptor,u"append");
        const std::array<options::Value,1> commentArgs{options::Value(std::u16string(comment))};
        const auto commentNode=constructNode_(NodeKind::Comment,commentArgs);
        const std::array<options::Value,2> appendArgs{nodeValue_(node),commentNode};
        return nodeFromValue_(functionCall_(appendMethod,receiver,appendArgs));
    }
    return adaptor.append(node,sourceLiteAdaptor(adaptor).comment(comment));
}
LiteNodeRef LiteParser::closeTag(AbstractDOMAdaptor& adaptor,LiteNodeRef node,std::u16string_view tag){
    const auto kind=asciiLower(tag.substr(2,tag.size()>=3?tag.size()-3:0));
    if(adaptorMethod_){
        const auto receiver=adaptorValue_(adaptor);
        while(true){
            const auto parentMethod=adaptorMethod_(adaptor,u"parent");
            const std::array<options::Value,1> parentArgs{nodeValue_(node)};
            const auto parent=functionCall_(parentMethod,receiver,parentArgs);
            if(!truthyProperty(propertyFromValue_(parent)))break;
            const auto kindMethod=adaptorMethod_(adaptor,u"kind");
            const std::array<options::Value,1> kindArgs{nodeValue_(node)};
            const auto actualKind=functionCall_(kindMethod,receiver,kindArgs);
            const auto* actual=std::get_if<std::u16string>(&actualKind.data);
            if(actual&&*actual==kind)break;
            const auto currentParent=adaptorMethod_(adaptor,u"parent");
            const auto next=functionCall_(currentParent,receiver,parentArgs);
            node=nodeFromValue_(next);
        }
        const auto parentMethod=adaptorMethod_(adaptor,u"parent");
        const std::array<options::Value,1> parentArgs{nodeValue_(node)};
        return nodeFromValue_(functionCall_(parentMethod,receiver,parentArgs));
    }
    while(true){
        const auto parent=adaptor.parent(node);
        if(!std::holds_alternative<LiteNodeRef>(parent)||adaptor.kind(node)==kind)break;
        node=std::get<LiteNodeRef>(parent);
    }
    const auto parent=adaptor.parent(node);
    if(const auto* result=std::get_if<LiteNodeRef>(&parent))return *result;
    if(std::holds_alternative<PropertyNull>(parent))return {};
    if(std::holds_alternative<Missing>(parent))property_host::typeError("Cannot read properties of undefined (reading 'kind')");
    return nodeValue(parent,"kind");
}
void LiteParser::addAttributes(AbstractDOMAdaptor& adaptor,LiteNodeRef node,PropertyArray& attributes){
    while(attributes.size()){
        std::array<Property,5> group{Missing{},Missing{},Missing{},Missing{},Missing{}};
        for(auto& part:group){if(!attributes.size())break;part=shiftProperty(attributes);}
        const auto& rawValue=truthyProperty(group[2])?group[2]:truthyProperty(group[3])?group[3]:truthyProperty(group[4])?group[4]:Property(std::string{});
        const auto value=entities_->translate(property_host::string(rawValue));
        if(adaptorMethod_){
            const auto receiver=adaptorValue_(adaptor);
            const auto setAttribute=adaptorMethod_(adaptor,u"setAttribute");
            const std::array<options::Value,3> args{nodeValue_(node),valueFromProperty_(group[1]),options::Value(value)};
            (void)functionCall_(setAttribute,receiver,args);
        }else adaptor.setAttribute(node,property_host::string(group[1]),Property(property_host::debugText(value)));
    }
}
LiteNodeRef LiteParser::openTag(AbstractDOMAdaptor& adaptor,LiteNodeRef node,std::u16string_view tag,PropertyArray& parts){
    const auto pcdata=staticProperty_?staticProperty_(u"PCDATA"):Property(Missing{});
    const auto selfClosing=staticProperty_?staticProperty_(u"SELF_CLOSING"):Property(Missing{});
    regexp_host::RegExp openTagKind(u"<(.*?)[\\s\\n>/]");
    const auto match=openTagKind.exec(std::u16string(tag));
    if(!match||match->captures.size()<2||!match->captures[1])property_host::typeError("Cannot read properties of null (reading '1')");
    const auto kind=asciiLower(*match->captures[1]);
    LiteNodeRef child;
    if(adaptorMethod_){
        const auto receiver=adaptorValue_(adaptor);
        const auto makeNode=adaptorMethod_(adaptor,u"node");
        const std::array<options::Value,1> args{options::Value(kind)};
        child=nodeFromValue_(functionCall_(makeNode,receiver,args));
    }else child=adaptor.node(kind);
    regexp_host::RegExp attrStartPattern(u"^<.*?[\\s\\n>]");
    const auto attrStart=attrStartPattern.exec(std::u16string(tag));
    const auto attrText=attrStart&&attrStart->captures[0]?tag.substr(attrStart->captures[0]->size()):tag;
    const auto captures=splitCaptures(attrText,attrSplitPattern());
    PropertyArray attributes;
    for(const auto& item:captures)attributes.push(item?Property(property_host::debugText(*item)):Property(Missing{}));
    bool complete=false;
    if(methodCall_){
        const auto attributeArray=arrayValue_(attributes);
        const auto pop=propertyGet_(attributeArray,u"pop");
        const auto last=functionCall_(pop,attributeArray,std::span<const options::Value>{});
        if(const auto* text=std::get_if<std::u16string>(&last.data)){
            regexp_host::RegExp endPattern(u">$");complete=endPattern.exec(*text).has_value();
        }else if(std::holds_alternative<options::Undefined>(last.data))
            property_host::typeError("Cannot read properties of undefined (reading 'match')");
        else if(std::holds_alternative<options::Null>(last.data))
            property_host::typeError("Cannot read properties of null (reading 'match')");
        else property_host::typeError("attributes.pop(...).match is not a function");
    }else{
        complete=!captures.empty()&&captures.back()&&!captures.back()->empty()&&captures.back()->back()==u'>';
        if(attributes.size())attributes.resize(attributes.size()-1);
    }
    if(complete||attributes.size()<5){
        if(methodCall_){
            const auto method=methodValue_(u"addAttributes");
            std::vector<options::Value> args{adaptorValue_(adaptor),nodeValue_(child),arrayValue_(attributes)};
            (void)parserCall_(method,args);
        }else addAttributes(adaptor,child,attributes);
        if(adaptorMethod_){
            const auto receiver=adaptorValue_(adaptor);
            const auto append=adaptorMethod_(adaptor,u"append");
            const std::array<options::Value,2> args{nodeValue_(node),nodeValue_(child)};
            (void)functionCall_(append,receiver,args);
        }else adaptor.append(node,child);
        const bool selfClose=tag.size()>=2&&tag.substr(tag.size()-2)==u"/>";
        const auto closes=staticElement_?staticElement_(selfClosing,kind):Property(Missing{});
        if(!truthyProperty(closes)&&!selfClose){
            const auto isPcdata=staticElement_?staticElement_(pcdata,kind):Property(Missing{});
            if(truthyProperty(isPcdata)){
                    if(methodCall_){
                        const auto method=methodValue_(u"handlePCDATA");
                        const std::array<options::Value,4> args{adaptorValue_(adaptor),nodeValue_(child),options::Value(std::u16string(kind)),arrayValue_(parts)};
                        (void)parserCall_(method,args);
                    } else handlePCDATA(adaptor,child,kind,parts);
            }else node=child;
        }
    }
    return node;
}
void LiteParser::handlePCDATA(AbstractDOMAdaptor& adaptor,LiteNodeRef node,std::u16string_view kind,PropertyArray& parts){
    PropertyArray content;const auto endTag=u"</"+std::u16string(kind)+u">";std::u16string ptag;
    while(parts.size()&&ptag!=endTag){
        content.push(Property(property_host::debugText(ptag)));
        if(parts.size())content.push(Property(property_host::debugText(shiftText(parts))));
        ptag=parts.size()?shiftText(parts):std::u16string{};
    }
    std::u16string text;for(std::size_t i=0;i<content.size();++i)text+=arrayText(content,i);
    if(adaptorMethod_){
        const auto receiver=adaptorValue_(adaptor);
        const auto append=adaptorMethod_(adaptor,u"append");
        const auto makeText=adaptorMethod_(adaptor,u"text");
        const std::array<options::Value,1> textArgs{options::Value(text)};
        const auto textNode=functionCall_(makeText,receiver,textArgs);
        const std::array<options::Value,2> appendArgs{nodeValue_(node),textNode};
        (void)functionCall_(append,receiver,appendArgs);
    }else adaptor.append(node,adaptor.text(text));
}
LiteNodeRef LiteParser::getOnlyChild(AbstractDOMAdaptor& adaptor,LiteNodeRef body){
    if(adaptorMethod_&&forEachIterable_&&instanceOf_){
        struct Finished{};
        const auto receiver=adaptorValue_(adaptor);
        const auto childNodes=adaptorMethod_(adaptor,u"childNodes");
        const std::array<options::Value,1> args{nodeValue_(body)};
        const auto values=functionCall_(childNodes,receiver,args);
        options::Value node=options::Null{};
        try{
            forEachIterable_(values,[&](options::Value child){
                if(!instanceOf_(child,NodeKind::Element))return;
                if(options::truthy(node)) { node=options::Null{};throw Finished{}; }
                node=std::move(child);
            });
        }catch(const Finished&){}
        return nodeFromValue_(std::move(node));
    }
    LiteNodeRef node{};const auto children=adaptor.childNodes(body).slots;
    for(std::size_t i=0;i<children.size();++i){
        const auto childValue=propertyNode(children.get(i));const auto* child=std::get_if<LiteNodeRef>(&childValue);
        if(!child||recordConst(*child).nodeKind!=NodeKind::Element)continue;
        if(node)return{};node=*child;
    }
    return node;
}
std::vector<AttributeData> LiteParser::allAttributes(AbstractDOMAdaptor& adaptor,LiteNodeRef node,bool xml){
    auto attributes=adaptor.allAttributes(node);
    if(!xml)return attributes;
    const auto kind=adaptor.kind(node);
    const auto namespaces=staticProperty_?staticProperty_(u"XMLNS"):Property(Missing{});
    if(!staticHasOwn_||!staticHasOwn_(namespaces,kind))return attributes;
    for(const auto& attribute:attributes)if(attribute.name==u"xmlns")return attributes;
    const auto value=staticElement_?staticElement_(namespaces,kind):Property(Missing{});
    attributes.push_back({u"xmlns",value});
    return attributes;
}
void LiteParser::checkDocument(AbstractDOMAdaptor& adaptor,LiteNodeRef document){
    if(adaptorMethod_&&propertyGet_&&propertySet_&&forEachIterable_&&instanceOf_){
        const auto receiver=adaptorValue_(adaptor);
        // Preserve the Source evaluation sequence: body(), then this.getOnlyChild().
        const auto bodyMethod=adaptorMethod_(adaptor,u"body");
        const std::array<options::Value,1> bodyArgs{nodeValue_(document)};
        const auto body=functionCall_(bodyMethod,receiver,bodyArgs);
        const auto getOnlyChild=methodValue_(u"getOnlyChild");
        const std::array<options::Value,2> soleArgs{receiver,body};
        const auto node=parserCall_(getOnlyChild,soleArgs);
        if(!options::truthy(node))return;

        const auto childNodes=adaptorMethod_(adaptor,u"childNodes");
        const auto loopBodyMethod=adaptorMethod_(adaptor,u"body");
        const std::array<options::Value,1> loopBodyArgs{nodeValue_(document)};
        const auto loopBody=functionCall_(loopBodyMethod,receiver,loopBodyArgs);
        const std::array<options::Value,1> childrenArgs{loopBody};
        const auto children=functionCall_(childNodes,receiver,childrenArgs);
        struct StopChildScan{};
        try{
            forEachIterable_(children,[&](options::Value child){
                if(child==node)throw StopChildScan{};
                if(instanceOf_(child,NodeKind::Comment)){
                    const auto value=propertyGet_(child,u"value");
                    if(const auto* text=std::get_if<std::u16string>(&value.data)){
                        if(text->starts_with(u"<!DOCTYPE"))propertySet_(nodeValue_(document),u"type",value);
                    }else if(std::holds_alternative<options::Undefined>(value.data))
                        property_host::typeError("Cannot read properties of undefined (reading 'match')");
                    else if(std::holds_alternative<options::Null>(value.data))
                        property_host::typeError("Cannot read properties of null (reading 'match')");
                    else property_host::typeError("child.value.match is not a function");
                }
            });
        }catch(const StopChildScan&){}

        const auto kindMethod=adaptorMethod_(adaptor,u"kind");
        const std::array<options::Value,1> kindArgs{node};
        const auto kindValue=functionCall_(kindMethod,receiver,kindArgs);
        const auto* kind=std::get_if<std::u16string>(&kindValue.data);
        if(!kind)return;
        if(*kind==u"html"){
            const auto nodeChildren=propertyGet_(node,u"children");
            forEachIterable_(nodeChildren,[&](options::Value child){
                const auto method=adaptorMethod_(adaptor,u"kind");
                const std::array<options::Value,1> args{child};
                const auto childKind=functionCall_(method,receiver,args);
                const auto* text=std::get_if<std::u16string>(&childKind.data);
                if(!text)return;
                if(*text==u"head")propertySet_(nodeValue_(document),u"head",child);
                if(*text==u"body")propertySet_(nodeValue_(document),u"body",child);
            });
            propertySet_(nodeValue_(document),u"root",node);
            const auto remove=adaptorMethod_(adaptor,u"remove");
            const std::array<options::Value,1> removeArgs{node};
            (void)functionCall_(remove,receiver,removeArgs);
            auto parentMethod=adaptorMethod_(adaptor,u"parent");
            auto bodyValue=propertyGet_(nodeValue_(document),u"body");
            std::array<options::Value,1> parentArgs{bodyValue};
            auto parent=functionCall_(parentMethod,receiver,parentArgs);
            if(parent!=node){
                const auto append=adaptorMethod_(adaptor,u"append");
                const std::array<options::Value,2> args{node,propertyGet_(nodeValue_(document),u"body")};
                (void)functionCall_(append,receiver,args);
            }
            parentMethod=adaptorMethod_(adaptor,u"parent");
            auto headValue=propertyGet_(nodeValue_(document),u"head");
            parentArgs[0]=headValue;
            parent=functionCall_(parentMethod,receiver,parentArgs);
            if(parent!=node){
                const auto insert=adaptorMethod_(adaptor,u"insert");
                const std::array<options::Value,2> args{propertyGet_(nodeValue_(document),u"head"),propertyGet_(nodeValue_(document),u"body")};
                (void)functionCall_(insert,receiver,args);
            }
        }else if(*kind==u"head"){
            const auto replace=adaptorMethod_(adaptor,u"replace");
            const std::array<options::Value,2> args{node,propertyGet_(nodeValue_(document),u"head")};
            const auto replacement=functionCall_(replace,receiver,args);
            propertySet_(nodeValue_(document),u"head",replacement);
        }else if(*kind==u"body"){
            const auto replace=adaptorMethod_(adaptor,u"replace");
            const std::array<options::Value,2> args{node,propertyGet_(nodeValue_(document),u"body")};
            const auto replacement=functionCall_(replace,receiver,args);
            propertySet_(nodeValue_(document),u"body",replacement);
        }
        return;
    }
    const auto body=nodeValue(adaptor.body(DOMNodeValue(document)),"children");
    LiteNodeRef node;
    if(methodCall_){
        const auto method=methodValue_(u"getOnlyChild");
        const std::array<options::Value,2> args{adaptorValue_(adaptor),nodeValue_(body)};
        const auto result=parserCall_(method,args);
        if(!std::holds_alternative<options::Null>(result.data)&&!std::holds_alternative<options::Undefined>(result.data))node=nodeFromValue_(result);
    } else node=getOnlyChild(adaptor,body);
    if(!node)return;
    const auto children=adaptor.childNodes(body).slots;
    for(std::size_t i=0;i<children.size();++i){
        const auto childValue=propertyNode(children.get(i));const auto* child=std::get_if<LiteNodeRef>(&childValue);
        if(child&&*child==node)break;
        if(child&&recordConst(*child).nodeKind==NodeKind::Comment){
            // Source reads LiteComment.value directly here (the complete
            // comment marker), not adaptor.value(), which strips the marker.
            const auto value=sourceStringArgument(recordConst(*child).value,"match");
            if(value.starts_with(u"<!DOCTYPE"))record(document).documentType=recordConst(*child).value;
        }
    }
    const auto kind=adaptor.kind(node);
    if(kind==u"html"){
        const auto htmlChildren=adaptor.childNodes(node).slots;
        for(std::size_t i=0;i<htmlChildren.size();++i){const auto childValue=propertyNode(htmlChildren.get(i));const auto* child=std::get_if<LiteNodeRef>(&childValue);if(!child)continue;const auto childKind=adaptor.kind(*child);if(childKind==u"head")record(document).head=folia::math::nodeProperty(*child);if(childKind==u"body")record(document).body=folia::math::nodeProperty(*child);}
        record(document).root=folia::math::nodeProperty(node);adaptor.remove(node);
        const auto currentBody=nodeValue(adaptor.body(DOMNodeValue(document)),"parent");const auto currentHead=nodeValue(adaptor.head(DOMNodeValue(document)),"parent");
        const auto bodyParent=adaptor.parent(currentBody);if(!std::holds_alternative<LiteNodeRef>(bodyParent)||std::get<LiteNodeRef>(bodyParent)!=node)adaptor.append(node,currentBody);
        const auto headParent=adaptor.parent(currentHead);if(!std::holds_alternative<LiteNodeRef>(headParent)||std::get<LiteNodeRef>(headParent)!=node)adaptor.insert(currentHead,currentBody);
    }else if(kind==u"head"){const auto old=nodeValue(adaptor.head(DOMNodeValue(document)),"parent");const auto replacement=adaptor.replace(node,old);record(document).head=folia::math::nodeProperty(replacement);}
    else if(kind==u"body"){const auto old=nodeValue(adaptor.body(DOMNodeValue(document)),"parent");const auto replacement=adaptor.replace(node,old);record(document).body=folia::math::nodeProperty(replacement);}
}

LiteNodeRef LiteParser::parseFromString(std::u16string_view input, std::u16string_view,
                                        AbstractDOMAdaptor& adaptor) {
    // Source obtains its document from the adaptor argument on every call;
    // this remains correct when parseFromString is invoked with another
    // compatible adaptor.
    LiteNodeRef document;
    LiteNodeRef node;
    if(adaptorMethod_){
        const auto receiver=adaptorValue_(adaptor);
        const auto createDocument=adaptorMethod_(adaptor,u"createDocument");
        document=nodeFromValue_(functionCall_(createDocument,receiver,std::span<const options::Value>{}));
        const auto body=adaptorMethod_(adaptor,u"body");
        const std::array<options::Value,1> bodyArgs{nodeValue_(document)};
        node=nodeFromValue_(functionCall_(body,receiver,bodyArgs));
    }else{
        document=sourceLiteAdaptor(adaptor).createDocument();
        node=nodeValue(adaptor.body(DOMNodeValue(document)), "attributes");
    }
    PropertyArray parts;
    for(auto& part:splitTags(input))parts.push(Property(property_host::debugText(part)));
    const auto partsValue=methodCall_?arrayValue_(parts):options::Value(options::Undefined{});
    auto sourceShift=[&]()->options::Value{
        const auto shift=propertyGet_(partsValue,u"shift");
        return functionCall_(shift,partsValue,std::span<const options::Value>{});
    };
    while (methodCall_?options::truthy(propertyGet_(partsValue,u"length")):parts.size()!=0) {
        options::Value textValue=options::Undefined{},tagValue=options::Undefined{};
        Property text,tag;
        if(methodCall_){textValue=sourceShift();tagValue=sourceShift();text=propertyFromValue_(textValue);tag=propertyFromValue_(tagValue);}
        else{text=shiftProperty(parts);tag=parts.size()?shiftProperty(parts):Property(Missing{});}
        const bool hasText=methodCall_?options::truthy(textValue):truthyProperty(text);
        const bool hasTag=methodCall_?options::truthy(tagValue):truthyProperty(tag);
        if (hasText) {
            if(methodCall_){const auto method=methodValue_(u"addText");const std::array<options::Value,3> args{adaptorValue_(adaptor),nodeValue_(node),textValue};(void)parserCall_(method,args);}
            else addText(adaptor,node,sourceStringArgument(text,"replace"));
        }
        if (hasTag) {
            const auto tagText=sourceStringArgument(tag,"charAt");
            if (!tagText.empty() && tagText.back() == u'>') {
                if (tagText.size() > 1 && tagText[1] == u'!') {
                    if(methodCall_){const auto method=methodValue_(u"addComment");const std::array<options::Value,3> args{adaptorValue_(adaptor),nodeValue_(node),tagValue};(void)parserCall_(method,args);}
                    else addComment(adaptor,node,tagText);
                } else if (tagText.size() > 1 && tagText[1] == u'/') {
                    if(methodCall_){const auto method=methodValue_(u"closeTag");const std::array<options::Value,3> args{adaptorValue_(adaptor),nodeValue_(node),tagValue};node=nodeFromValue_(parserCall_(method,args));}
                    else node=closeTag(adaptor,node,tagText);
                } else {
                    if(methodCall_){const auto method=methodValue_(u"openTag");const std::array<options::Value,4> args{adaptorValue_(adaptor),nodeValue_(node),tagValue,arrayValue_(parts)};node=nodeFromValue_(parserCall_(method,args));}
                    else node=openTag(adaptor,node,tagText,parts);
                }
            }
        }
    }
    if(methodCall_){const auto method=methodValue_(u"checkDocument");const std::array<options::Value,2> args{adaptorValue_(adaptor),nodeValue_(document)};(void)parserCall_(method,args);}
    else checkDocument(adaptor,document);
    return document;
}

std::u16string LiteParser::serialize(AbstractDOMAdaptor& adaptor, LiteNodeRef node, bool xml) {
    cancellation::checkpoint(cancellation::CheckpointStage::Serialization);
    const auto selfClosing = staticProperty_ ? staticProperty_(u"SELF_CLOSING") : Property(Missing{});
    std::u16string tag;
    options::Value tagValue;
    options::Value attributesValue;
    options::Value attributeTextValue;
    if(methodCall_){
        const auto kind=adaptorMethod_(adaptor,u"kind");
        const std::array<options::Value,1> kindArgs{nodeValue_(node)};
        tagValue=functionCall_(kind,adaptorValue_(adaptor),kindArgs);
        const auto allAttributes=methodValue_(u"allAttributes");
        const std::array<options::Value,3> args{adaptorValue_(adaptor),nodeValue_(node),options::Value(xml)};
        attributesValue=parserCall_(allAttributes,args);
        const auto map=propertyGet_(attributesValue,u"map");
        const auto callback=callback_([this,xml](options::Value item)->options::Value{
            cancellation::checkpoint(cancellation::CheckpointStage::Serialization);
            const auto name=propertyGet_(item,u"name");
            const auto nameText=stringValue_(name);
            const auto protect=methodValue_(u"protectAttribute");
            const auto value=propertyGet_(item,u"value");
            const std::array<options::Value,2> protectArgs{value,options::Value(xml)};
            const auto protectedValue=parserCall_(protect,protectArgs);
            return options::Value(nameText+u"=\""+stringValue_(protectedValue)+u"\"");
        });
        const std::array<options::Value,1> mapArgs{callback};
        const auto mapped=functionCall_(map,attributesValue,mapArgs);
        const auto join=propertyGet_(mapped,u"join");
        const std::array<options::Value,1> joinArgs{options::Value(u" ")};
        attributeTextValue=functionCall_(join,mapped,joinArgs);
    }else{
        tag=adaptor.kind(node);tagValue=options::Value(tag);
        auto attributes=adaptor.allAttributes(node);
        if(xml){
            const auto namespaceKind=adaptor.kind(node);
            const auto xmlns=staticProperty_ ? staticProperty_(u"XMLNS") : Property(Missing{});
            if(staticHasOwn_ && staticHasOwn_(xmlns,namespaceKind)){
                const auto nsValue=staticElement_?staticElement_(xmlns,namespaceKind):Property(Missing{});
                bool hasXmlns=false;for(const auto& attribute:attributes)if(attribute.name==u"xmlns"){hasXmlns=true;break;}
                if(!hasXmlns)attributes.push_back({u"xmlns",nsValue});
            }
        }
        std::vector<std::u16string> rendered;
        for(const auto& attribute:attributes){
            cancellation::checkpoint(cancellation::CheckpointStage::Serialization);
            const auto escaped=protectAttribute(htmlValue(attribute.value),xml);
            rendered.push_back(attribute.name+u"=\""+escaped+u"\"");
        }
        std::u16string joined;
        for(std::size_t i=0;i<rendered.size();++i){if(i)joined.push_back(u' ');joined+=rendered[i];}
        attributeTextValue=options::Value(joined);
    }
    options::Value contentValue;
    if(methodCall_){
        const auto method=methodValue_(u"serializeInner");
        const std::array<options::Value,3> args{adaptorValue_(adaptor),nodeValue_(node),options::Value(xml)};
        contentValue=parserCall_(method,args);
    }else contentValue=options::Value(serializeInner(adaptor,node,xml));
    const bool hasContent=!xml||options::truthy(contentValue);
    const std::u16string openTag=methodCall_?stringValue_(tagValue):tag;
    const bool hasAttributes=options::truthy(attributeTextValue);
    const std::u16string attrText=hasAttributes
        ? (methodCall_?stringValue_(attributeTextValue):std::get<std::u16string>(attributeTextValue.data))
        : std::u16string{};
    const auto closes = staticElement_ ? staticElement_(selfClosing,methodCall_?std::u16string_view(stringValue_(tagValue)):std::u16string_view(tag)) : Property(Missing{});
    std::u16string html=u"<"+openTag+(hasAttributes?u" "+attrText:u"");
    if(hasContent&&!truthyProperty(closes)){
        const std::u16string closeTag=methodCall_?stringValue_(tagValue):tag;
        const auto content=methodCall_?stringValue_(contentValue):std::get<std::u16string>(contentValue.data);
        html+=u">"+content+u"</"+closeTag+u">";
    }else if(xml)html+=u"/>";
    else html+=u">";
    return html;
}

std::u16string LiteParser::serializeInner(AbstractDOMAdaptor& adaptor, LiteNodeRef node, bool xml) {
    cancellation::checkpoint(cancellation::CheckpointStage::Serialization);
    if(methodCall_&&propertyGet_&&callback_){
        const auto pcdata=staticProperty_?staticProperty_(u"PCDATA"):Property(Missing{});
        const auto nodeKind=propertyGet_(nodeValue_(node),u"kind");
        const bool rawText=staticHasOwn_&&staticHasOwn_(pcdata,stringValue_(nodeKind));
        const auto receiver=adaptorValue_(adaptor);
        const auto childNodes=adaptorMethod_(adaptor,u"childNodes");
        const std::array<options::Value,1> childArgs{nodeValue_(node)};
        const auto children=functionCall_(childNodes,receiver,childArgs);
        const auto map=propertyGet_(children,u"map");
        const auto callback=callback_([this,&adaptor,receiver,xml,rawText](options::Value child)->options::Value{
            cancellation::checkpoint(cancellation::CheckpointStage::Serialization);
            if(rawText){
                const auto valueMethod=adaptorMethod_(adaptor,u"value");
                const std::array<options::Value,1> args{child};
                return functionCall_(valueMethod,receiver,args);
            }
            const auto kindMethod=adaptorMethod_(adaptor,u"kind");
            const std::array<options::Value,1> kindArgs{child};
            const auto kind=functionCall_(kindMethod,receiver,kindArgs);
            if(kind==options::Value(u"#text")){
                const auto protect=methodValue_(u"protectHTML");
                const auto valueMethod=adaptorMethod_(adaptor,u"value");
                const std::array<options::Value,1> valueArgs{child};
                const auto value=functionCall_(valueMethod,receiver,valueArgs);
                const std::array<options::Value,1> protectArgs{value};
                return parserCall_(protect,protectArgs);
            }
            if(kind==options::Value(u"#comment"))return propertyGet_(child,u"value");
            const auto serialize=methodValue_(u"serialize");
            const std::array<options::Value,3> args{receiver,child,options::Value(xml)};
            return parserCall_(serialize,args);
        });
        const std::array<options::Value,1> mapArgs{callback};
        const auto mapped=functionCall_(map,children,mapArgs);
        const auto join=propertyGet_(mapped,u"join");
        const std::array<options::Value,1> joinArgs{options::Value(u"")};
        return stringValue_(functionCall_(join,mapped,joinArgs));
    }
    const auto pcdata = staticProperty_ ? staticProperty_(u"PCDATA") : Property(Missing{});
    const auto kind=adaptor.kind(node);
    if(staticHasOwn_ && staticHasOwn_(pcdata,kind)){
        const auto children=adaptor.childNodes(node).slots;
        const auto length=children.size();
        std::u16string result;
        for(std::size_t i=0;i<length;++i){
            cancellation::checkpoint(cancellation::CheckpointStage::Serialization);
            if(!children.has(i))continue;
            const auto child=propertyNode(children.get(i));
            result+=adaptor.value(nodeValue(child,"value"));
        }
        return result;
    }
    const auto children=adaptor.childNodes(node).slots;
    const auto length=children.size();
    std::u16string result;
    for(std::size_t i=0;i<length;++i){
        cancellation::checkpoint(cancellation::CheckpointStage::Serialization);
        if(!children.has(i))continue;
        const auto childValue=propertyNode(children.get(i));
        const auto child=nodeValue(childValue,"kind");
        const auto childKind=adaptor.kind(child);
        if(childKind==u"#text"){
            if(methodCall_){
                const auto protect=methodValue_(u"protectHTML");
                const auto value=adaptor.value(child);
                const std::array<options::Value,1> args{options::Value(value)};
                result+=stringValue_(parserCall_(protect,args));
            }else result+=protectHTML(adaptor.value(child));
        }
        else if(childKind==u"#comment"){
            if(methodCall_){
                const auto raw=propertyGet_(nodeValue_(child),u"value");
                if(!std::holds_alternative<options::Undefined>(raw.data)&&!std::holds_alternative<options::Null>(raw.data))result+=stringValue_(raw);
            }else result+=adaptor.getProperty(child,"value").missing()?adaptor.value(child):htmlValue(adaptor.getProperty(child,"value"));
        }
        else if(methodCall_){
            const auto method=methodValue_(u"serialize");
            const std::array<options::Value,3> args{adaptorValue_(adaptor),nodeValue_(child),options::Value(xml)};
            result+=stringValue_(parserCall_(method,args));
        }else result+=serialize(adaptor,child,xml);
    }
    return result;
}

std::u16string LiteParser::protectHTML(std::u16string_view input) const {
    std::u16string text(input);
    replaceAll(text, u"&", u"&amp;");
    replaceAll(text, u"<", u"&lt;");
    replaceAll(text, u">", u"&gt;");
    return text;
}

std::u16string LiteParser::protectAttribute(std::u16string_view input, bool xml) const {
    std::u16string text(input);
    replaceAll(text, u"&", u"&amp;");
    replaceAll(text, u"\"", u"&quot;");
    if (xml) { replaceAll(text,u"<",u"&lt;"); replaceAll(text,u">",u"&gt;"); }
    return text;
}
}
