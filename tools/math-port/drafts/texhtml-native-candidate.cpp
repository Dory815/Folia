// Native observations for MathJax 4.1.3 TexHtmlConfiguration and HTMLDomStrings.
// The parser and DOM are the session's actual TeX, MML, Lite and HTML owners.
import std;
import folia.math.input.tex.parser;
import folia.math.input.tex.base_configuration;
import folia.math.input.tex.html.configuration;
import folia.math.input.tex.setoptions.set_options_configuration;
import folia.math.input.tex.texhtml.tex_html_configuration;
import folia.math.input.tex.tex_error;
import folia.math.handlers.html.html_dom_strings;
import folia.math.output.common.font_data;
import folia.math.output.svg.native_document;
import folia.math.util.string_map;
import folia.math.core.mml_tree.mml_node;
import folia.math.core.mml_tree.mml_nodes.html_node;
import folia.math.core.mml_tree.xml_dom_adaptor;
import folia.math.output.svg.render_session;
import folia.math.util.option_functions;
import folia.math.util.option_types;
import folia.math.util.property_host;
import folia.math.util.regexp_binding;
import folia.math.util.runtime_modules;

namespace {
namespace tex=folia::math::tex;
namespace opts=folia::math::options;
namespace html=folia::math::handlers::html;
namespace svg=folia::math::output::svg;
namespace common=folia::math::output::common;
using namespace folia::math;
using namespace opts;

std::string q(std::string_view value){
    std::string out="\"";constexpr char h[]="0123456789abcdef";
    for(unsigned char c:value){switch(c){case '\\':out+="\\\\";break;case '"':out+="\\\"";break;
    case '\n':out+="\\n";break;case '\r':out+="\\r";break;case '\t':out+="\\t";break;
    default:if(c<32){out+="\\u00";out+=h[c>>4];out+=h[c&15];}else out+=static_cast<char>(c);}}
    return out+'"';
}
std::string text(std::u16string_view value){return property_host::debugText(value);}
std::string num(double value){if(std::isnan(value))return "{\"$\":\"NaN\"}";if(std::isinf(value))return value<0?"{\"$\":\"-Infinity\"}":"{\"$\":\"Infinity\"}";if(value==0&&std::signbit(value))return "{\"$\":\"-0\"}";std::ostringstream out;out<<std::setprecision(17)<<value;return out.str();}
struct Serializer;
struct Serializer{
    opts::Context* context=nullptr;
    OptionFunctionBindings* functions=nullptr;
    mml::HtmlAdaptor* htmlAdaptor=nullptr;
    std::string value(const opts::Value& v,std::vector<opts::ObjectRef>* seen=nullptr){
        if(std::holds_alternative<opts::Undefined>(v.data))return "{\"$\":\"undefined\"}";
        if(std::holds_alternative<opts::Null>(v.data))return "null";
        if(const auto* b=std::get_if<bool>(&v.data))return *b?"true":"false";
        if(const auto* n=std::get_if<double>(&v.data))return num(*n);
        if(const auto* s=std::get_if<std::u16string>(&v.data))return q(text(*s));
        if(std::holds_alternative<opts::SymbolRef>(v.data))return "{\"$\":\"symbol\"}";
        const auto* object=std::get_if<opts::ObjectRef>(&v.data);if(!object)return "null";
        if(!context)throw std::runtime_error("Context is required for object serialization");
        if(context->kind(*object)==opts::ObjectKind::function){
            const auto name=context->get(*object,opts::Key(u"name"));const auto length=context->get(*object,opts::Key(u"length"));
            const auto* n=std::get_if<std::u16string>(&name.data);const auto* l=std::get_if<double>(&length.data);
            return "{\"$\":\"function\",\"name\":"+q(n?text(*n):"")+",\"length\":"+num(l?*l:0)+"}";
        }
        if(seen){if(std::find(seen->begin(),seen->end(),*object)!=seen->end())return "{\"$\":\"cycle\"}";seen->push_back(*object);}
        if(context->kind(*object)==opts::ObjectKind::array){const auto length=context->arrayLength(*object);std::string out="[";for(std::uint32_t i=0;i<length;++i){if(i)out+=',';const auto index=std::to_string(i);const opts::Key k(std::u16string(index.begin(),index.end()));out+=context->hasOwn(*object,k)?value(context->get(*object,k),seen):"{\"$\":\"hole\"}";}return out+"]";}
        if(const auto* map=std::any_cast<opts::MapStorageRef>(&context->payload(*object));map&&*map){std::string out="{\"$\":\"map\",\"entries\":[";bool first=true;for(const auto& entry:(*map)->entries()){if(!entry.live)continue;if(!first)out+=',';first=false;out+='[';out+=value(entry.key,seen)+','+value(entry.value,seen)+']';}return out+"]}";}
        std::string out="{";bool first=true;for(const auto& key:context->ownKeys(opts::Value(*object))){if(!first)out+=',';first=false;out+=q(text(context->keyString(key)))+":"+value(context->get(opts::Value(*object),key),seen);}return out+"}";
    }
    std::string packageData(tex::TeX& jax){
        const auto options=jax.parseOptions();const auto* parsed=std::get_if<tex::ParseOptionsRef>(&options);if(!parsed||!*parsed)return "null";
        const auto data=(*parsed)->packageData;const auto* map=std::get_if<std::shared_ptr<SourceStringMap<opts::Value>>>(&data);if(!map||!*map)return "null";
        std::vector<opts::ObjectRef> seen;
        std::string out="{\"$\":\"map\",\"entries\":[";bool first=true;for(auto it=(*map)->keys();auto keyValue=it.next();){if(!first)out+=',';first=false;out+='['+q(text(*keyValue))+','+value((*map)->get(*keyValue).value(),&seen)+']';}return out+"]}";
    }
    std::string property(const Property& p){
        if(std::holds_alternative<Missing>(p.value))return "{\"$\":\"undefined\"}";
        if(std::holds_alternative<PropertyNull>(p.value))return "null";
        if(const auto* v=std::get_if<bool>(&p.value))return *v?"true":"false";
        if(const auto* v=std::get_if<double>(&p.value))return num(*v);
        if(const auto* v=std::get_if<std::string>(&p.value))return q(*v);
        if(std::holds_alternative<options::SymbolRef>(p.value))return "{\"$\":\"symbol\"}";
        if(const auto* f=std::get_if<FunctionRef>(&p.value)){
            if(!functions)throw std::runtime_error("FunctionRef serializer has no bindings");
            const auto identity=functions->find(*f);if(!identity)throw std::runtime_error("Unbound FunctionRef in token map");
            const auto name=context->get(*identity,opts::Key(u"name")),length=context->get(*identity,opts::Key(u"length"));
            const auto* n=std::get_if<std::u16string>(&name.data);const auto* l=std::get_if<double>(&length.data);
            if(!n||!l)throw std::runtime_error("FunctionRef metadata missing");
            return "{\"$\":\"function\",\"name\":"+q(text(*n))+",\"length\":"+num(*l)+"}";
        }
        if(std::holds_alternative<BuiltinProperty>(p.value))return "{\"$\":\"builtin\"}";
        const auto storage=std::get<std::shared_ptr<PropertyStorage>>(p.value);if(!storage)return "null";
        if(storage->array){auto a=PropertyArray::array(p);std::string out="[";for(std::size_t i=0;i<a->size();++i){if(i)out+=',';out+=a->has(i)?property(a->get(i)):"{\"$\":\"hole\"}";}return out+"]";}
        auto r=PropertyList::record(p);if(!r)return "null";return record(*r);
    }
    std::string record(const PropertyList& value){std::string out="{";bool first=true;for(const auto& k:value.keys()){if(!first)out+=',';first=false;out+=q(k)+":"+property(value.getOwn(k));}return out+"}";}
    std::string lookup(const tex::TokenLookup& item){
        if(const auto* p=std::get_if<Property>(&item))return property(*p);
        if(const auto* m=std::get_if<tex::MacroRef>(&item)){
            if(!*m)throw std::runtime_error("null macro in token map");const auto method=(*m)->func();
            const auto fn=std::get_if<FunctionRef>(&method)?property(Property(*std::get_if<FunctionRef>(&method))):property(std::get<Property>(method));
            return "{\"_token\":"+property((*m)->token())+",\"_func\":"+fn+",\"_args\":"+property((*m)->args())+"}";
        }
        if(const auto* t=std::get_if<tex::TokenRef>(&item)){
            if(!*t)throw std::runtime_error("null token in token map");
            return "{\"_token\":"+property((*t)->token())+",\"_char\":"+property((*t)->character())+",\"_attributes\":"+property((*t)->attributes())+"}";
        }
        throw std::runtime_error("unsupported token map lookup variant");
    }
    std::string tokenMap(const tex::TokenMapRef& source){
        const auto parsed=std::dynamic_pointer_cast<tex::AbstractParseMap>(source);
        if(!parsed)throw std::runtime_error("registered map is not an AbstractParseMap");
        const auto& entries=parsed->mapStorage();std::string out="{\"$\":\"map\",\"entries\":[";bool first=true;
        if(entries)for(const auto&[key,value]:*entries){if(!first)out+=',';first=false;out+='['+property(key)+','+lookup(value)+']';}
        return out+"]}";
    }
    std::string maps(tex::ParserRuntime& runtime){
        constexpr std::string_view names[]{"action-macros","bbox","Braket-macros","Braket-characters","cancel","enclose","color"};
        std::string out="{";bool first=true;for(const auto name:names){const auto map=runtime.mapHandler.getMap(Property(std::string(name)));if(!map)continue;if(!first)out+=',';first=false;out+=q(name)+":"+tokenMap(map);}return out+"}";
    }
    std::string node(mml::NodeValue value){
        if(!value.isNode())return "null";auto* n=value.requireNode();
        const auto attrs=n->attributes();std::string out="{\"kind\":"+q(n->kind())+
            ",\"attrs\":"+record(attrs?attrs->getAllAttributes():PropertyList{})+
            ",\"properties\":"+record(n->getAllProperties())+",\"children\":[";
        const auto children=n->childNodes;
        if(children)for(std::size_t i=0;i<children->size();++i){if(i)out+=',';out+=node(children->get(i));}
        out+=']';
        if(const auto* textNode=dynamic_cast<mml::TextNode*>(n))out+=",\"text\":"+q(text(textNode->getText()));
        else if(const auto* token=dynamic_cast<mml::AbstractMmlTokenNode*>(n))out+=",\"text\":"+q(text(token->getText()));
        if(const auto* html=dynamic_cast<mml::HtmlNode*>(n)){
            if(!htmlAdaptor)throw std::runtime_error("HTML node serializer needs the actual XML adaptor");
            out+=",\"htmlKind\":"+q(htmlAdaptor->kind(html->getHTML()));
            out+=",\"html\":"+q(html->getSerializedHTML());
            out+=",\"textContent\":"+q(text(html->textContent()));
        }
        return out+"}";
    }
};

common::FontLoadHost fontHost(){common::FontLoadHost host;host.asyncIsSynchronous=[] {return true;};host.warn=[](const std::any&){};return host;}
Value makeRecord(Context& cx,std::initializer_list<std::pair<std::u16string_view,Value>> fields){const auto result=cx.object();for(const auto&[name,value]:fields)cx.set(result,Key(std::u16string(name)),value);return Value(result);}
std::string config(tex::ParserRuntime& runtime,Serializer& serializer){
    const auto selected=runtime.configurationHandler.get(u"texhtml");
    const auto* owner=std::get_if<tex::ConfigurationRef>(&selected);if(!owner||!*owner)return "null";
    return "{\"name\":"+q(text((*owner)->name()))+",\"options\":"+serializer.value((*owner)->field(u"options"))+",\"handler\":"+
        serializer.value((*owner)->field(u"handler"))+",\"preprocessors\":"+serializer.value((*owner)->field(u"preprocessors"))+",\"postprocessors\":"+
        serializer.value((*owner)->field(u"postprocessors"))+"}";
}
std::string mapObservation(tex::ParserRuntime& runtime,Serializer& serializer){
    const auto map=runtime.mapHandler.getMap(Property("tex-html"));if(!map)return "null";
    const auto parser=map->parser();
    const std::string parserJson=std::visit([&](const auto& method)->std::string{using T=std::decay_t<decltype(method)>;if constexpr(std::is_same_v<T,FunctionRef>)return serializer.property(Property(method));else return serializer.property(method);},parser);
    return "{\"name\":"+serializer.property(map->name())+",\"constructor\":\"MacroMap\",\"parser\":"+parserJson+",\"entries\":"+serializer.tokenMap(map)+"}";
}
std::string errorRecord(const tex::TexError& error){return "{\"name\":\"TexError\",\"id\":"+q(text(error.id))+",\"message\":"+q(text(error.message))+"}";}
std::string errorRecord(const Exception& error){return "{\"name\":"+q(error.name)+",\"id\":null,\"message\":"+q(text(error.message))+"}";}

struct Session {
    svg::NativeRenderSession native{fontHost()};
    Context& cx=native.context();
    tex::ParserRuntime& runtime=native.services().parser();
    util::RuntimeModules& modules=native.services().modules();
    Serializer serializer{&cx,&native.services().functions()};
    std::string domStringsDefaults;
    tex::TexHtmlAdaptorResolver resolver=[this](ObjectRef identity){return native.services().lite().resolveAdaptor(identity);};
    Session(){
        native.construct();
        (void)tex::BasePackage::initializeModule(runtime);
        (void)tex::HtmlConfiguration::initializeModule(runtime);
        (void)tex::SetOptionsConfiguration::initializeModule(runtime);
        (void)tex::TexHtmlConfiguration::initializeModule(runtime,modules,resolver);
        const auto domStrings=html::HTMLDomStringsModule::initializeModule(
            modules,runtime.properties,runtime.regexp());
        domStringsDefaults=serializer.value(cx.get(Value(domStrings->identity()),Key(u"OPTIONS")));
    }
};

std::string parserCase(Session& session,std::string_view name,std::u16string source,bool enabled,bool setOptions){
    auto& cx=session.cx;auto& runtime=session.runtime;auto& serializer=session.serializer;
    const Value packageNames=cx.array(setOptions?4:3);
    const std::array<std::u16string_view,4> names{u"base",u"html",u"texhtml",u"setoptions"};
    for(std::size_t i=0;i<(setOptions?4:3);++i){const auto index=std::to_string(i);cx.set(packageNames,Key(std::u16string(index.begin(),index.end())),Value(std::u16string(names[i])));}
    const Value options=makeRecord(cx,{{u"packages",packageNames},{u"allowTexHTML",Value(enabled)}});
    auto jax=tex::TeX::make(runtime,options);
    jax->setMmlFactory(core::InputMmlFactoryReference{session.native.services().mmlFactoryOwner()});
    const auto parseOptions=jax->parseOptions();const auto* optionsOwner=std::get_if<tex::ParseOptionsRef>(&parseOptions);
    if(!optionsOwner||!*optionsOwner)throw Exception("TypeError",u"TexHTML observation has no ParseOptions owner");
    const auto* packageDataField=std::get_if<std::shared_ptr<SourceStringMap<Value>>>(&(*optionsOwner)->packageData);
    if(!packageDataField||!*packageDataField)throw Exception("TypeError",u"TexHTML observation has no Source packageData Map");
    auto packageData=*packageDataField;
    packageData->set(u"texhtml",makeRecord(cx,{{u"adaptor",Value(session.native.adaptorObject())}}));
    std::string parsed="null",error="null";tex::TexParserRef parser;
    try{
        parser=tex::TexParser::make(source,Property(PropertyList{}),**optionsOwner);
        const auto result=parser->mml();
        auto xml=mml::XmlDomAdaptor::create(std::static_pointer_cast<AbstractDOMAdaptor>(session.native.adaptor()));
        serializer.htmlAdaptor=xml.get();
        parsed=serializer.node(result);
    }catch(const tex::TexError& caught){error=errorRecord(caught);}
     catch(const Exception& caught){error=errorRecord(caught);}
    std::string parserJson="null";
    if(parser)parserJson="{\"i\":"+num(parser->i)+",\"string\":"+q(text(parser->string()))+",\"macroCount\":"+num(parser->macroCount)+
        ",\"allowTexHTML\":"+serializer.value(cx.get(parser->options(),Key(u"allowTexHTML")))+"}";
    const auto packageRecord=packageData->get(u"texhtml");
    const Value adaptorValue=packageRecord?cx.get(*packageRecord,Key(u"adaptor")):Value(Undefined{});
    const auto adaptorClass=[&](){const auto* object=std::get_if<ObjectRef>(&adaptorValue.data);if(!object)return std::string("null");const auto ctor=cx.get(*object,Key(u"constructor"));const auto nameValue=cx.get(ctor,Key(u"name"));const auto* s=std::get_if<std::u16string>(&nameValue.data);return s?text(*s):std::string("null");}();
    return "{\"name\":"+q(name)+",\"tex\":"+q(text(source))+
        ",\"parser\":"+parserJson+",\"result\":"+parsed+",\"error\":"+error+",\"texhtmlPackageData\":{\"adaptorClass\":"+q(adaptorClass)+
        "},\"configuration\":"+config(runtime,serializer)+",\"macroMap\":"+mapObservation(runtime,serializer)+"}";
}
Value invoke(Context& cx,Value receiver,std::u16string_view method,std::vector<Value> arguments={}){
    const Value callable=cx.get(receiver,Key(std::u16string(method)));
    return cx.callMember(callable,std::move(receiver),arguments,u"adaptor method");
}
DOMNodeRef sourceNode(ContextPropertyBindings& properties,const Value& source){
    const auto converted=propertyNode(properties.from(source));
    if(const auto* node=std::get_if<DOMNodeRef>(&converted))return *node;
    throw Exception("TypeError",u"Lite source method did not return a DOM node");
}
Value nodeIdentity(ContextPropertyBindings& properties,const DOMNodeRef& node){return properties.to(nodeProperty(node));}
std::pair<std::string,std::string> domStringsCases(Session& session){
    auto& cx=session.cx;auto& runtime=session.runtime;auto& serializer=session.serializer;
    auto module=html::HTMLDomStringsModule::initializeModule(session.modules,runtime.properties,runtime.regexp());
    const Value constructor(module->identity());
    const Value staticOptions=cx.get(constructor,Key(u"OPTIONS"));
    const std::string defaults=session.domStringsDefaults;
    const Value include=cx.get(staticOptions,Key(u"includeHtmlTags"));
    const Value custom=cx.function(u"custom",[](Context& c,Value,std::span<const Value> args){
        if(args.size()<2)throw Exception("TypeError",u"custom HTML tag requires the Source adaptor");
        const Value method=c.get(args[1],Key(u"innerHTML"));const std::array<Value,1> forwarded{args[0]};
        const Value inner=c.callMember(method,args[1],forwarded,u"adaptor.innerHTML");
        const auto* string=std::get_if<std::u16string>(&inner.data);if(!string)throw Exception("TypeError",u"Lite innerHTML returned a non-string");
        return Value(u"<custom>"+*string+u"</custom>");
    });
    cx.define(std::get<ObjectRef>(custom.data),Key(u"length"),Descriptor::data(Value(2),false,false,true));
    cx.set(include,Key(u"tex-html"),custom);
    auto configOwner=runtime.configurationHandler.get(u"texhtml");const auto* configRef=std::get_if<tex::ConfigurationRef>(&configOwner);
    if(!configRef||!*configRef)throw Exception("TypeError",u"TexHtml configuration is unavailable for DOM hook observation");
    const Value configMethod=(*configRef)->config();
    const auto* configFunction=std::get_if<ObjectRef>(&configMethod.data);
    if(!configFunction)throw Exception("TypeError",u"TexHtml CONFIG hook is not a Context function");
    (void)cx.call(*configFunction,Value(Undefined{}),{});
    const Value afterCustom=cx.get(include,Key(u"tex-html"));
    const std::string replacement=std::string("{\"customPreserved\":")+(afterCustom==custom?"true":"false")+",\"installed\":"+serializer.value(afterCustom)+"}";
    cx.set(include,Key(u"tex-html"),Value(Null{}));
    (void)cx.call(*configFunction,Value(Undefined{}),{});
    const std::string registered=serializer.value(cx.get(include,Key(u"tex-html")));
    const auto instance=module->create(Value(Undefined{}));
    instance->setAdaptor(Value(session.native.adaptorObject()),session.native.adaptor());
    const Value adaptor(session.native.adaptorObject());
    const Value parsed=invoke(cx,adaptor,u"parse",{Value(u"<div>before<br>middle<tex-html><b>inner</b><tex-html><i>deep</i></tex-html></tex-html>after</div>")});
    const Value body=invoke(cx,adaptor,u"body",{parsed});
    const Value children=invoke(cx,adaptor,u"childNodes",{body});
    if(cx.arrayLength(std::get<ObjectRef>(children.data))==0)throw Exception("TypeError",u"HTMLDomStrings fixture has no root node");
    const Value root=cx.get(children,Key(u"0"));
    const Value find=cx.get(Value(instance->identity()),Key(u"find"));
    const std::array<Value,1> findArgs{root};const Value result=cx.call(std::get<ObjectRef>(find.data),Value(instance->identity()),findArgs);
    const Value strings=cx.get(result,Key(u"0")),nodeLists=cx.get(result,Key(u"1"));
    std::string nested="[";const auto rowCount=cx.arrayLength(std::get<ObjectRef>(strings.data));
    for(std::uint32_t i=0;i<rowCount;++i){if(i)nested+=',';const auto idx=std::to_string(i);const Key rowKey(std::u16string(idx.begin(),idx.end()));
        const Value stringValue=cx.get(strings,rowKey),list=cx.get(nodeLists,rowKey);nested+="{\"string\":"+serializer.value(stringValue)+",\"nodes\":[";
        const auto pairCount=cx.arrayLength(std::get<ObjectRef>(list.data));for(std::uint32_t j=0;j<pairCount;++j){if(j)nested+=',';const auto jdx=std::to_string(j);const Value pair=cx.get(list,Key(std::u16string(jdx.begin(),jdx.end())));
            const Value node=cx.get(pair,Key(u"0")),length=cx.get(pair,Key(u"1"));const auto kind=invoke(cx,adaptor,u"kind",{node});const auto* kindText=std::get_if<std::u16string>(&kind.data);
            if(!kindText)throw Exception("TypeError",u"Lite kind returned a non-string");const bool isText=*kindText==u"#text";
            const Value htmlValue=isText?Value(Null{}):invoke(cx,adaptor,u"outerHTML",{node});
            const Value value=isText?invoke(cx,adaptor,u"value",{node}):Value(Null{});
            const Value nodeText=isText?value:invoke(cx,adaptor,u"textContent",{node});
            nested+="{\"kind\":"+serializer.value(kind)+",\"outerHTML\":"+serializer.value(htmlValue)+",\"value\":"+serializer.value(value)+",\"text\":"+serializer.value(nodeText)+",\"length\":"+serializer.value(length)+"}";
        }nested+="]}";
    }nested+="]";
    const std::string configRecord="{\"defaultsBefore\":"+defaults+",\"configReplacement\":"+replacement+",\"registeredAfterConfig\":"+registered+",\"instanceOptions\":"+serializer.value(instance->optionsValue())+"}";
    const std::string traversalRecord="{\"nestedMarkers\":"+nested+"}";
    return {configRecord,traversalRecord};
}
}

int main(){
    try{
        Session session;std::vector<std::string> cases;
        cases.push_back(parserCase(session,"texhtml-default-disabled",u"<tex-html><b>hello</b></tex-html>",false,false));
        cases.push_back(parserCase(session,"texhtml-profile-enabled",u"\\setOptions{allowTexHTML=true}<tex-html><b data-probe=\"x\">hello &amp; world</b></tex-html>\\setOptions{allowTexHTML=false}",true,true));
        cases.push_back(parserCase(session,"texhtml-multiple-roots",u"<tex-html><b>bold</b><i>italic</i></tex-html>",true,false));
        cases.push_back(parserCase(session,"texhtml-empty",u"<tex-html>   </tex-html>",true,false));
        cases.push_back(parserCase(session,"texhtml-nested-marked",u"<tex-html n=\"2\"><span>outer<tex-html><b>inner</b></tex-html></span><!2></tex-html>",true,false));
        cases.push_back(parserCase(session,"texhtml-missing-end",u"<tex-html><b>open</b>",true,false));
        const auto dom=domStringsCases(session);
        std::cout<<"{\"schema\":1,\"reference\":\"MathJax 4.1.3\",\"commit\":\"fb987178c2d279a99b0db5ea02e751691703955e\",\"scopeSha256\":\"9705d389538b2fc8bf782feea4372ace639b3727304f2971937bbcfb0042a0d1\",\"sourceHashes\":["
          <<"{\"file\":\"ts/handlers/html/HTMLDomStrings.ts\",\"sha256\":\"e4a7da03465d6e69ebdca829b007d9905f180b0d4107ea5a1c5a44737c2712c8\"},"
          <<"{\"file\":\"ts/input/tex/texhtml/TexHtmlConfiguration.ts\",\"sha256\":\"e31f1539452eba33048d8e7bcf87fcf9690f23be1412799ddf2a6f43beab4a3b\"},"
          <<"{\"file\":\"ts/output/common/Wrappers/XmlNode.ts\",\"sha256\":\"aa03369e8f9e4b36154c9e3d4b7ad9bed94fa0713c8ead00467a289e42654dea\"},"
          <<"{\"file\":\"ts/output/svg/Wrappers/semantics.ts\",\"sha256\":\"6a68dd39649fcf05407dd12a700a294c78e9be140b7ceefe518523efae4bdb6e\"},"
          <<"{\"file\":\"ts/output/svg/Wrappers/HtmlNode.ts\",\"sha256\":\"902225b6605078eea19a893fa355875edb7adc5c5312924097e0c8d335b76b45\"}],\"texhtml\":{\"cases\":[";
        for(std::size_t i=0;i<cases.size();++i){if(i)std::cout<<',';std::cout<<cases[i];}
        std::cout<<"]},\"domStrings\":{\"cases\":["<<dom.first<<','<<dom.second<<"]}}\n";return 0;
    }catch(const opts::Exception& error){
        std::cerr<<"Native MathJax option exception "<<error.name<<": "<<text(error.message)<<'\n';
        return 1;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
