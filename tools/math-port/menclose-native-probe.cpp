// Native observations of the real MathDocument -> TeX -> SvgMenclose ->
// immutable Scene path. The only MML input is parsed from the TeX test strings.
import std;
import folia.math.output.svg.native_document;
import folia.math.output.svg.output_jax;
import folia.math.output.svg.wrapper_constructors;
import folia.math.output.scene;
import folia.math.util.property_host;

namespace {
using namespace folia::math;
namespace opts=folia::math::options;
namespace svg=folia::math::output::svg;
namespace common=folia::math::output::common;
namespace scene=folia::math::output::scene;

std::string quote(std::string_view text){
    std::string out="\"";
    constexpr char hex[]="0123456789abcdef";
    for(unsigned char c:text){
        if(c=='\\')out+="\\\\";
        else if(c=='\"')out+="\\\"";
        else if(c=='\n')out+="\\n";
        else if(c=='\r')out+="\\r";
        else if(c=='\t')out+="\\t";
        else if(c<0x20){out+="\\u00";out+=hex[c>>4];out+=hex[c&15];}
        else out+=static_cast<char>(c);
    }
    return out+'\"';
}
std::string id(opts::ObjectRef value){return std::to_string(value.context)+":"+std::to_string(value.index);}
std::string valueJson(opts::Context& cx,const opts::Value& value,
                      std::unordered_set<std::string>& visiting){
    if(std::holds_alternative<opts::Undefined>(value.data))return "{\"type\":\"undefined\"}";
    if(std::holds_alternative<opts::Null>(value.data))return "null";
    if(const auto* v=std::get_if<bool>(&value.data))return *v?"true":"false";
    if(const auto* v=std::get_if<double>(&value.data)){
        if(!std::isfinite(*v))return quote(std::isnan(*v)?"NaN":(*v>0?"Infinity":"-Infinity"));
        std::ostringstream out;out<<std::setprecision(std::numeric_limits<double>::max_digits10)<<*v;return out.str();
    }
    if(const auto* v=std::get_if<std::u16string>(&value.data))
        return "{\"type\":\"string\",\"value\":"+quote(property_host::debugText(*v))+"}";
    if(const auto* v=std::get_if<opts::ObjectRef>(&value.data)){
        std::string type="object";
        try{
            const auto kind=cx.kind(*v);
            if(kind==opts::ObjectKind::function)type="function";
            else if(kind==opts::ObjectKind::array)type="array";
        }catch(...){}
        const auto identity=id(*v);
        if(type=="array"){
            if(!visiting.insert(identity).second)
                return "{\"type\":\"array\",\"id\":"+quote(identity)+
                    ",\"length\":"+std::to_string(cx.arrayLength(*v))+",\"circular\":true}";
            const auto length=cx.arrayLength(*v);
            std::string out="{\"type\":\"array\",\"id\":"+quote(identity)+
                ",\"length\":"+std::to_string(length)+",\"items\":[";
            for(std::uint32_t i=0;i<length;++i){
                if(i)out+=',';
                std::u16string index;
                for(const char c:std::to_string(i))index.push_back(static_cast<char16_t>(c));
                const opts::Key key(std::move(index));
                if(!cx.hasOwn(opts::Value(*v),key))out+="{\"type\":\"hole\"}";
                else out+=valueJson(cx,cx.get(opts::Value(*v),key),visiting);
            }
            visiting.erase(identity);
            return out+"]}";
        }
        if(type=="function")
            return "{\"type\":\"function\",\"id\":"+quote(identity)+"}";
        std::string out="{\"type\":"+quote(type)+",\"id\":"+quote(identity);
        if(!visiting.insert(identity).second)return out+",\"circular\":true}";
        out+=",\"fields\":{";
        bool first=true;
        for(const auto& key:cx.enumerableStringKeys(opts::Value(*v))){
            if(!first)out+=',';first=false;
            out+=quote(property_host::debugText(key))+":"+
                valueJson(cx,cx.get(opts::Value(*v),opts::Key(key)),visiting);
        }
        visiting.erase(identity);
        return out+"}}";
    }
    return "{\"type\":\"symbol\"}";
}
std::string valueJson(opts::Context& cx,const opts::Value& value){
    std::unordered_set<std::string> visiting;
    return valueJson(cx,value,visiting);
}
struct Event {std::string method,receiver,result;std::vector<std::string> args;};
struct HtmlContract {
    std::string name;
    bool length=false,prototypeDescriptor=false,nameDescriptor=false,lengthDescriptor=false;
    bool defaultArgs=false,receiver=false,returnIdentity=false,rawNulls=false,dynamicReplacement=false;
    bool callPropertyIgnored=false,exception=false,restored=false;
};
std::string htmlContractJson(const HtmlContract& value){
    return "{\"name\":"+quote(value.name)+
        ",\"length\":"+(value.length?"true":"false")+
        ",\"prototypeDescriptor\":"+(value.prototypeDescriptor?"true":"false")+
        ",\"nameDescriptor\":"+(value.nameDescriptor?"true":"false")+
        ",\"lengthDescriptor\":"+(value.lengthDescriptor?"true":"false")+
        ",\"defaultArgs\":"+(value.defaultArgs?"true":"false")+
        ",\"receiver\":"+(value.receiver?"true":"false")+
        ",\"returnIdentity\":"+(value.returnIdentity?"true":"false")+
        ",\"rawNulls\":"+(value.rawNulls?"true":"false")+
        ",\"dynamicReplacement\":"+(value.dynamicReplacement?"true":"false")+
        ",\"callPropertyIgnored\":"+(value.callPropertyIgnored?"true":"false")+
        ",\"exception\":"+(value.exception?"true":"false")+
        ",\"restored\":"+(value.restored?"true":"false")+"}";
}
HtmlContract testHtmlBinding(opts::Context& cx,opts::ObjectRef jax,opts::ObjectRef adaptor){
    HtmlContract result;
    std::optional<opts::ObjectRef> prototype=cx.prototype(jax);
    while(prototype&&!cx.hasOwn(opts::Value(*prototype),opts::Key(u"html")))
        prototype=cx.prototype(*prototype);
    if(!prototype)return result;
    const auto methodValue=cx.get(opts::Value(*prototype),opts::Key(u"html"));
    const auto* method=std::get_if<opts::ObjectRef>(&methodValue.data);
    if(!method||cx.kind(*method)!=opts::ObjectKind::function)return result;
    const auto name=cx.get(opts::Value(*method),opts::Key(u"name"));
    const auto length=cx.get(opts::Value(*method),opts::Key(u"length"));
    if(const auto* text=std::get_if<std::u16string>(&name.data))result.name=property_host::debugText(*text);
    result.length=std::holds_alternative<double>(length.data)&&std::get<double>(length.data)==4;
    const auto htmlDescriptor=cx.descriptor(opts::Value(*prototype),opts::Key(u"html"));
    const auto nameDescriptor=cx.descriptor(opts::Value(*method),opts::Key(u"name"));
    const auto lengthDescriptor=cx.descriptor(opts::Value(*method),opts::Key(u"length"));
    result.prototypeDescriptor=htmlDescriptor&&htmlDescriptor->writable&&htmlDescriptor->enumerable&&htmlDescriptor->configurable;
    result.nameDescriptor=nameDescriptor&&!nameDescriptor->writable&&!nameDescriptor->enumerable&&nameDescriptor->configurable;
    result.lengthDescriptor=lengthDescriptor&&!lengthDescriptor->writable&&!lengthDescriptor->enumerable&&lengthDescriptor->configurable;

    const opts::Value originalNode=cx.get(opts::Value(adaptor),opts::Key(u"node"));
    std::vector<opts::Value> captured;
    opts::ObjectRef capturedReceiver{};
    const auto marker=cx.object();
    const auto first=cx.function(u"firstNode",[&](opts::Context&,opts::Value receiver,
        std::span<const opts::Value> args)->opts::Value{
        if(const auto* object=std::get_if<opts::ObjectRef>(&receiver.data))capturedReceiver=*object;
        captured.assign(args.begin(),args.end());return marker;
    });
    const auto fakeCall=cx.function(u"fakeCall",[](opts::Context&,opts::Value,
        std::span<const opts::Value>)->opts::Value{
        throw opts::Exception("HtmlCallPropertyError",u"function.call property must not be read");
    });
    cx.define(first,opts::Key(u"call"),opts::Descriptor::data(opts::Value(fakeCall),true,true,true));
    cx.set(opts::Value(adaptor),opts::Key(u"node"),opts::Value(first));
    const std::array<opts::Value,1> defaults{opts::Value(u"mjx-probe")};
    bool defaultCallSucceeded=false;
    try{
        const auto returned=cx.call(*method,opts::Value(jax),defaults);
        defaultCallSucceeded=returned==opts::Value(marker);
    }catch(const opts::Exception&){}
    result.callPropertyIgnored=defaultCallSucceeded;
    result.returnIdentity=defaultCallSucceeded;
    result.receiver=capturedReceiver==adaptor;
    result.defaultArgs=defaultCallSucceeded&&captured.size()==4&&
        std::holds_alternative<std::u16string>(captured[0].data)&&std::get<std::u16string>(captured[0].data)==u"mjx-probe"&&
        std::holds_alternative<opts::ObjectRef>(captured[1].data)&&cx.kind(std::get<opts::ObjectRef>(captured[1].data))==opts::ObjectKind::object&&
        std::holds_alternative<opts::ObjectRef>(captured[2].data)&&cx.kind(std::get<opts::ObjectRef>(captured[2].data))==opts::ObjectKind::array&&
        std::holds_alternative<opts::Undefined>(captured[3].data);

    const auto secondMarker=cx.object();
    const auto second=cx.function(u"secondNode",[&](opts::Context&,opts::Value receiver,
        std::span<const opts::Value> args)->opts::Value{
        capturedReceiver=std::get<opts::ObjectRef>(receiver.data);
        captured.assign(args.begin(),args.end());return secondMarker;
    });
    cx.set(opts::Value(adaptor),opts::Key(u"node"),opts::Value(second));
    const std::array<opts::Value,4> raw{opts::Value(u"mjx-probe"),opts::Null{},opts::Null{},opts::Null{}};
    const auto rawResult=cx.call(*method,opts::Value(jax),raw);
    result.rawNulls=rawResult==opts::Value(secondMarker)&&capturedReceiver==adaptor&&captured.size()==4&&
        std::all_of(captured.begin()+1,captured.end(),[](const auto& value){return std::holds_alternative<opts::Null>(value.data);});
    result.dynamicReplacement=cx.get(opts::Value(adaptor),opts::Key(u"node"))==opts::Value(second);

    const auto throwing=cx.function(u"throwingNode",[](opts::Context&,opts::Value,
        std::span<const opts::Value>)->opts::Value{
        throw opts::Exception("HtmlProbeError",u"node method test exception");
    });
    cx.set(opts::Value(adaptor),opts::Key(u"node"),opts::Value(throwing));
    try{(void)cx.call(*method,opts::Value(jax),defaults);}
    catch(const opts::Exception& error){
        result.exception=error.name=="HtmlProbeError"&&error.message==u"node method test exception";
    }
    cx.set(opts::Value(adaptor),opts::Key(u"node"),originalNode);
    result.restored=cx.get(opts::Value(adaptor),opts::Key(u"node"))==originalNode;
    return result;
}
std::string eventsJson(opts::Context& cx,const std::vector<Event>& events){
    std::string out="[";
    for(std::size_t i=0;i<events.size();++i){
        if(i)out+=',';
        const auto& e=events[i];
        out+="{\"method\":"+quote(e.method)+",\"receiver\":"+quote(e.receiver)+",\"args\":[";
        for(std::size_t j=0;j<e.args.size();++j){if(j)out+=',';out+=e.args[j];}
        out+="],\"result\":"+e.result+"}";
    }
    return out+"]";
}
opts::ObjectRef prototype(opts::Context& cx,opts::ObjectRef constructor){
    const auto value=cx.get(opts::Value(constructor),opts::Key(u"prototype"));
    if(const auto* object=std::get_if<opts::ObjectRef>(&value.data))return *object;
    throw opts::Exception("TypeError",u"Source constructor has no prototype");
}
opts::ObjectRef constructor(const svg::SvgWrapperConstructorSet& set,std::u16string_view name){
    for(const auto& [key,value]:set.constructors)if(key==name)return value;
    throw std::runtime_error("Required actual SVG constructor is absent");
}
void observe(opts::Context& cx,opts::ObjectRef target,std::u16string_view key,
    std::string method,std::vector<Event>& events,bool& failFirstSvg){
    const auto value=cx.get(opts::Value(target),opts::Key(std::u16string(key)));
    const auto* original=std::get_if<opts::ObjectRef>(&value.data);
    if(!original||cx.kind(*original)!=opts::ObjectKind::function)
        throw opts::Exception("TypeError",u"Required actual Source method is not callable");
    const auto actual=*original;
    const auto observer=cx.function(
        property_host::text(method),[actual,method,&events,&failFirstSvg](opts::Context& context,
        opts::Value receiver,std::span<const opts::Value> args)->opts::Value{
            Event event;event.method=method;
            if(const auto* object=std::get_if<opts::ObjectRef>(&receiver.data))event.receiver=id(*object);
            for(const auto& arg:args)event.args.push_back(valueJson(context,arg));
            bool isMenclosePrimitive=false;
            if(method=="SvgOutputJax.svg"&&!args.empty()){
                if(const auto* tag=std::get_if<std::u16string>(&args.front().data))
                    isMenclosePrimitive=*tag==u"line"||*tag==u"rect"||*tag==u"ellipse"||*tag==u"path";
            }
            if(method=="SvgOutputJax.svg"&&failFirstSvg&&isMenclosePrimitive){
                failFirstSvg=false;
                event.result="{\"type\":\"throws\",\"name\":\"ProbeInputError\",\"message\":\"one-time SVG method test input\"}";
                events.push_back(std::move(event));
                throw opts::Exception("ProbeInputError",u"one-time SVG method test input");
            }
            const auto result=context.call(actual,receiver,args);
            event.result=valueJson(context,result);
            events.push_back(std::move(event));
            return result;
        });
    if(method=="LiteAdaptor.setAttribute"){
        const auto fakeCall=cx.function(u"fakeSetAttributeCall",[](opts::Context&,opts::Value,
            std::span<const opts::Value>)->opts::Value{
            throw opts::Exception("SetAttributeCallPropertyError",u"setAttribute.call property must not be read");
        });
        cx.define(observer,opts::Key(u"call"),opts::Descriptor::data(opts::Value(fakeCall),true,true,true));
    }
    cx.set(opts::Value(target),opts::Key(std::u16string(key)),opts::Value(observer));
}
std::size_t countNodes(const scene::FrozenNodeRef& node){
    if(!node)return 0;std::size_t count=1;
    for(const auto& child:node->children)count+=countNodes(child);
    return count;
}
std::string errorJson(const std::exception& error){
    if(const auto* value=dynamic_cast<const opts::Exception*>(&error))
        return "{\"name\":"+quote(value->name)+",\"message\":"+
            quote(property_host::debugText(value->message))+"}";
    return "{\"name\":\"std::exception\",\"message\":"+quote(error.what())+"}";
}
}

int main(){
    try{
        common::FontLoadHost host;
        host.asyncIsSynchronous=[] {return true;};
        host.warn=[](const std::any&){
            throw opts::Exception("UnexpectedWarning",u"Unexpected font warning during Menclose probe");
        };
        host.syncLoad=[](common::FontData&,const common::DynamicFileRef&,std::string){
            throw opts::Exception("Unimplemented",u"Menclose tests do not load dynamic font files");
        };
        host.asyncLoad=[](std::shared_ptr<common::FontData>,common::DynamicFileRef,std::string)
            ->async::PromiseRef{
            throw opts::Exception("Unimplemented",u"Menclose tests do not load dynamic font files");
        };
        auto session=std::make_unique<svg::NativeRenderSession>(std::move(host));
        auto& cx=session->context();
        const auto jaxOptions=cx.object();
        cx.set(jaxOptions,opts::Key(u"fontCache"),opts::Value(u"none"));
        auto jax=session->construct(opts::Value(jaxOptions));
        const auto htmlContract=testHtmlBinding(cx,jax->instanceIdentity(),session->adaptorObject());
        const auto inputOptions=cx.object(),packages=cx.array(2);
        cx.set(opts::Value(packages),opts::Key(u"0"),opts::Value(u"base"));
        cx.set(opts::Value(packages),opts::Key(u"1"),opts::Value(u"enclose"));
        cx.set(inputOptions,opts::Key(u"packages"),opts::Value(packages));
        std::vector<std::string> packageNames;
        for(std::uint32_t i=0;i<cx.arrayLength(packages);++i){
            std::u16string index;for(const char c:std::to_string(i))index.push_back(static_cast<char16_t>(c));
            const auto value=cx.get(opts::Value(packages),opts::Key(std::move(index)));
            const auto* name=std::get_if<std::u16string>(&value.data);
            if(!name)throw opts::Exception("TypeError",u"Input Jax package name is not a string");
            packageNames.push_back(property_host::debugText(*name));
        }
        auto document=svg::createNativeSvgDocument(*session,opts::Value(opts::Undefined{}),
                                                   opts::Value(inputOptions));
        auto& context=document->sourceContext();
        std::vector<Event> events;
        bool failFirstSvg=true;
        const auto wrappers=session->services().wrapperSet();
        const auto basePrototype=prototype(context,constructor(wrappers,u"SvgWrapper"));
        const auto closeConstructor=constructor(wrappers,u"SvgMenclose");
        const auto closePrototype=prototype(context,closeConstructor);
        const auto jaxPrototype=prototype(context,session->services().svgJax().constructor());
        const auto adaptor=session->adaptorObject();
        observe(context,basePrototype,u"svg","SvgWrapper.svg",events,failFirstSvg);
        observe(context,basePrototype,u"fixed","SvgWrapper.fixed",events,failFirstSvg);
        for(const auto& name:{u"line",u"box",u"ellipse",u"path",u"fill",u"arrow"})
            observe(context,closePrototype,name,"SvgMenclose."+property_host::debugText(name),events,failFirstSvg);
        observe(context,jaxPrototype,u"svg","SvgOutputJax.svg",events,failFirstSvg);
        observe(context,jaxPrototype,u"fixed","SvgOutputJax.fixed",events,failFirstSvg);
        observe(context,adaptor,u"append","LiteAdaptor.append",events,failFirstSvg);
        observe(context,adaptor,u"setAttribute","LiteAdaptor.setAttribute",events,failFirstSvg);

        constexpr std::array<std::u16string_view,29> notations={
            u"top",u"right",u"bottom",u"left",u"actuarial",u"madruwb",
            u"updiagonalstrike",u"downdiagonalstrike",u"horizontalstrike",u"verticalstrike",
            u"box",u"roundedbox",u"circle",u"phasorangle",u"uparrow",u"downarrow",
            u"leftarrow",u"rightarrow",u"updownarrow",u"leftrightarrow",u"updiagonalarrow",
            u"northeastarrow",u"southeastarrow",u"northwestarrow",u"southwestarrow",
            u"northeastsouthwestarrow",u"northwestsoutheastarrow",u"longdiv",u"radical"};
        const auto notationValue=context.get(opts::Value(closeConstructor),opts::Key(u"notations"));
        const auto* notationMap=std::get_if<opts::ObjectRef>(&notationValue.data);
        if(!notationMap)throw opts::Exception("TypeError",u"SvgMenclose.notations is not the actual Map");
        const auto notationStorage=session->services().mapBindings().resolve(*notationMap);
        std::vector<std::string> actualNotationNames;
        for(const auto& entry:notationStorage->entries()){
            if(!entry.live)continue;
            const auto* key=std::get_if<std::u16string>(&entry.key.data);
            if(!key)throw opts::Exception("TypeError",u"SvgMenclose notation Map key is not a string");
            actualNotationNames.push_back(property_host::debugText(*key));
        }
        if(actualNotationNames.size()!=notations.size())
            throw opts::Exception("TypeError",u"SvgMenclose notation Map does not contain the full Source set");
        for(std::size_t i=0;i<notations.size();++i)
            if(actualNotationNames[i]!=property_host::debugText(notations[i]))
                throw opts::Exception("TypeError",u"SvgMenclose notation Map order differs from Source");
        auto detached=std::make_unique<scene::Scene>();
        std::weak_ptr<opts::Context> weakContext=session->contextOwner();
        std::weak_ptr<svg::SvgRenderServiceGraph> weakServices=session->services().shared_from_this();
        std::cout<<"{\"schema\":1,\"htmlContract\":"<<htmlContractJson(htmlContract)<<",\"packageList\":[";
        for(std::size_t i=0;i<packageNames.size();++i){if(i)std::cout<<',';std::cout<<quote(packageNames[i]);}
        std::cout<<"],\"notationNames\":[";
        for(std::size_t i=0;i<actualNotationNames.size();++i){if(i)std::cout<<',';std::cout<<quote(actualNotationNames[i]);}
        std::cout<<"],\"cases\":[";
        for(std::size_t index=0;index<=notations.size();++index){
            if(index)std::cout<<',';
            const bool retry=index==1;
            const auto notation=notations[retry?0:(index==0?0:index-1)];
            const std::string name=retry?"top-retry":property_host::debugText(notation);
            const auto tex=std::u16string(u"\\enclose{")+std::u16string(notation)+u"}{x}";
            events.clear();std::string error="null";
            const auto settings=context.object();
            context.set(settings,opts::Key(u"display"),opts::Value(false));
            context.set(settings,opts::Key(u"em"),opts::Value(16.0));
            context.set(settings,opts::Key(u"ex"),opts::Value(8.0));
            context.set(settings,opts::Key(u"containerWidth"),opts::Value(1280.0));
            try{(void)document->convert(opts::Value(tex),opts::Value(settings));}
            catch(const std::exception& caught){error=errorJson(caught);}
            const auto* completed=jax->completedScene();
            std::size_t nodeCount=0;
            if(completed){
                for(const auto& root:completed->roots())nodeCount+=countNodes(root);
                if(index==notations.size())*detached=*completed;
            }
            std::cout<<"{\"name\":"<<quote(name)<<",\"error\":"<<error
                <<",\"sceneNodeCount\":"<<nodeCount<<",\"events\":"
                <<eventsJson(context,events)<<"}";
        }
        std::cout<<"],\"contextReleased\":";
        document.reset();jax.reset();session.reset();
        const bool released=weakContext.expired()&&weakServices.expired();
        std::size_t retained=0;for(const auto& root:detached->roots())retained+=countNodes(root);
        std::cout<<(released?"true":"false")<<",\"detachedSceneNodeCount\":"<<retained<<"}\n";
        return released&&retained>0?0:3;
    }catch(const std::exception& error){std::cerr<<errorJson(error)<<'\n';return 1;}
}
