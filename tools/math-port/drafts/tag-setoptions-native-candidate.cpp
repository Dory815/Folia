// Real native parser observations for the TagFormat and SetOptions Source packets.
import std;
import folia.math.input.tex.parser;
import folia.math.input.tex.base_configuration;
import folia.math.input.tex.tex_error;
import folia.math.input.tex.ams.configuration;
import folia.math.input.tex.cases.configuration;
import folia.math.input.tex.mathtools.configuration;
import folia.math.input.tex.mathtools.tags;
import folia.math.input.tex.tagformat.tag_format_configuration;
import folia.math.input.tex.setoptions.set_options_configuration;
import folia.math.input.tex.map_handler;
import folia.math.input.tex.token_map;
import folia.math.input.tex.token;
import folia.math.core.math_item;
import folia.math.core.mml_tree.mml_node;
import folia.math.core.find_math_host;
import folia.math.core.input_jax_host;
import folia.math.input.tex.find_tex_host;
import folia.math.output.svg.native_document;
import folia.math.output.svg.render_session;
import folia.math.output.common.font_data;
import folia.math.handlers.html.html_math_document;
import folia.math.util.regexp_host;
import folia.math.util.regexp_binding;
import folia.math.util.unicode_normalization;
import folia.math.util.options;
import folia.math.util.property_host;
import folia.math.util.js_number;
import folia.math.util.map_bindings;
import folia.math.util.option_functions;

namespace {
namespace tex=folia::math::tex;
namespace opts=folia::math::options;
namespace common=folia::math::output::common;
using namespace folia::math;

std::string q(std::string_view value){
    std::string out="\"";constexpr char h[]="0123456789abcdef";
    for(unsigned char c:value){switch(c){case '\\':out+="\\\\";break;case '"':out+="\\\"";break;
    case '\n':out+="\\n";break;case '\r':out+="\\r";break;case '\t':out+="\\t";break;
    default:if(c<32){out+="\\u00";out+=h[c>>4];out+=h[c&15];}else out+=static_cast<char>(c);}}
    return out+'"';
}
std::string text(std::u16string_view value){return property_host::debugText(value);}
std::string num(double value){if(std::isnan(value))return "{\"$\":\"NaN\"}";if(std::isinf(value))return value<0?"{\"$\":\"-Infinity\"}":"{\"$\":\"Infinity\"}";if(value==0&&std::signbit(value))return "{\"$\":\"-0\"}";std::ostringstream out;out<<std::setprecision(17)<<value;return out.str();}
struct Serializer{
    opts::Context* context=nullptr;
    OptionFunctionBindings* functions=nullptr;
    folia::math::regexp_host::RegExpBindings* regexp=nullptr;
    std::string observationPath;
    std::string value(const opts::Value& v,std::vector<opts::ObjectRef>* seen=nullptr){
        std::vector<opts::ObjectRef> localSeen;if(!seen)seen=&localSeen;
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
        if(std::find(seen->begin(),seen->end(),*object)!=seen->end())return "{\"$\":\"cycle\"}";seen->push_back(*object);
        if(context->kind(*object)==opts::ObjectKind::array){const auto length=context->arrayLength(*object);std::string out="[";for(std::uint32_t i=0;i<length;++i){if(i)out+=',';const auto index=std::to_string(i);const opts::Key k(std::u16string(index.begin(),index.end()));out+=context->hasOwn(*object,k)?value(context->get(*object,k),seen):"{\"$\":\"hole\"}";}return out+"]";}
        if(regexp){try{const auto expression=regexp->resolve(opts::Value(*object));return "{\"lastIndex\":"+num(expression->lastIndex)+"}";}catch(const opts::Exception&) {}}
        if(const auto* map=std::any_cast<opts::MapStorageRef>(&context->payload(*object));map&&*map){std::string out="{\"$\":\"map\",\"entries\":[";bool first=true;for(const auto& entry:(*map)->entries()){if(!entry.live)continue;if(!first)out+=',';first=false;out+='[';out+=value(entry.key,seen)+','+value(entry.value,seen)+']';}return out+"]}";}
        std::string out="{";bool first=true;for(const auto& key:context->ownKeys(opts::Value(*object))){if(!first)out+=',';first=false;out+=q(text(context->keyString(key)))+":"+value(context->get(opts::Value(*object),key),seen);}return out+"}";
    }
    std::string packageData(tex::TeX& jax){
        const auto options=jax.parseOptions();const auto* parsed=std::get_if<tex::ParseOptionsRef>(&options);if(!parsed||!*parsed)return "null";
        const auto data=(*parsed)->packageData;const auto* map=std::get_if<std::shared_ptr<SourceStringMap<opts::Value>>>(&data);if(!map||!*map)return "null";
        std::vector<opts::ObjectRef> seen;
        std::string out="{\"$\":\"map\",\"entries\":[";bool first=true;for(auto it=(*map)->keys();auto keyValue=it.next();){if(!first)out+=',';first=false;out+='['+q(text(*keyValue))+','+value((*map)->get(*keyValue).value(),&seen)+']';}return out+"]}";
    }
    opts::Value parseOptions(tex::TeX& jax){const auto shared=jax.parseOptions();const auto* owner=std::get_if<tex::ParseOptionsRef>(&shared);if(!owner||!*owner)return opts::Value(opts::Undefined{});return (*owner)->options;}
    std::string tagClass(tex::TeX& jax){const auto shared=jax.parseOptions();const auto* owner=std::get_if<tex::ParseOptionsRef>(&shared);if(!owner||!*owner)return "null";
        const auto tags=std::get_if<std::shared_ptr<tex::Tags>>(&(*owner)->tags);if(!tags||!*tags)return "null";
        const auto ctor=context->get(opts::Value((*tags)->sourceIdentity()),opts::Key(u"constructor"));const auto* object=std::get_if<opts::ObjectRef>(&ctor.data);if(!object)return "null";
        const auto name=context->get(*object,opts::Key(u"name"));return value(name);}
    std::string tagClassName(const tex::TagsRef& tags){if(!tags)return "null";
        const auto ctor=context->get(opts::Value(tags->sourceIdentity()),opts::Key(u"constructor"));const auto* object=std::get_if<opts::ObjectRef>(&ctor.data);if(!object)return "null";
        return value(context->get(*object,opts::Key(u"name")));}
    std::string tagParentName(const tex::TagsRef& tags){if(!tags)return "null";
        const auto tagPrototype=context->prototype(tags->sourceIdentity());if(!tagPrototype)return "null";
        const auto parentPrototype=context->prototype(*tagPrototype);if(!parentPrototype)return "null";
        const auto ctor=context->get(*parentPrototype,opts::Key(u"constructor"));const auto* object=std::get_if<opts::ObjectRef>(&ctor.data);if(!object)return "null";
        return value(context->get(*object,opts::Key(u"name")));}
    std::string tagParent(tex::TeX& jax){const auto shared=jax.parseOptions();const auto* owner=std::get_if<tex::ParseOptionsRef>(&shared);if(!owner||!*owner)return "null";
        const auto tags=std::get_if<std::shared_ptr<tex::Tags>>(&(*owner)->tags);if(!tags||!*tags)return "null";
        const auto tagPrototype=context->prototype((*tags)->sourceIdentity());if(!tagPrototype)return "null";
        const auto parentPrototype=context->prototype(*tagPrototype);if(!parentPrototype)return "null";
        const auto constructor=context->get(*parentPrototype,opts::Key(u"constructor"));
        const auto* function=std::get_if<opts::ObjectRef>(&constructor.data);if(!function)return "null";
        return value(context->get(*function,opts::Key(u"name")));}
    std::string property(const Property& p){
        if(std::holds_alternative<Missing>(p.value))return "{\"$\":\"undefined\"}";
        if(std::holds_alternative<PropertyNull>(p.value))return "null";
        if(const auto* v=std::get_if<bool>(&p.value))return *v?"true":"false";
        if(const auto* v=std::get_if<double>(&p.value))return num(*v);
        if(const auto* v=std::get_if<std::string>(&p.value))return q(*v);
        if(std::holds_alternative<options::SymbolRef>(p.value))return "{\"$\":\"symbol\"}";
        if(const auto* f=std::get_if<FunctionRef>(&p.value)){
            if(!functions)throw std::runtime_error("FunctionRef serializer has no bindings at "+observationPath);
            const auto identity=functions->find(*f);if(!identity)throw std::runtime_error("Unbound FunctionRef in token map entry "+observationPath);
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
        if(entries)for(const auto&[key,value]:*entries){if(!first)out+=',';first=false;const auto previous=observationPath;observationPath=previous+"."+text(property_host::string(key));
            try{out+='['+property(key)+','+lookup(value)+']';}catch(...){observationPath=previous;throw;}observationPath=previous;}
        return out+"]}";
    }
    std::string maps(tex::ParserRuntime& runtime){
        const auto map=runtime.mapHandler.getMap(Property("setoptions"));if(!map)return "null";
        const auto previous=observationPath;observationPath="setoptions";
        try{const auto result=tokenMap(map);observationPath=previous;return result;}
        catch(...){observationPath=previous;throw;}
    }
    std::string node(mml::NodeValue value){
        if(!value.isNode())return "null";auto* n=value.requireNode();
        const auto attrs=n->attributes();std::string out="{\"kind\":"+q(n->kind())+
            ",\"attrs\":"+record(attrs?attrs->getAllAttributes():PropertyList{})+
            ",\"properties\":"+record(n->getAllProperties());
        if(const auto* textNode=dynamic_cast<mml::TextNode*>(n))out+=",\"text\":"+q(text(textNode->getText()));
        else if(const auto* token=dynamic_cast<mml::AbstractMmlTokenNode*>(n))out+=",\"text\":"+q(text(token->getText()));
        out+=",\"children\":[";
        const auto children=n->childNodes;if(children)for(std::size_t i=0;i<children->size();++i){if(i)out+=',';out+=node(children->get(i));}return out+"]}";
    }
};
opts::Value strings(opts::Context& cx,std::span<const std::u16string_view> values){
    const auto a=cx.array(static_cast<std::uint32_t>(values.size()));std::uint32_t i=0;
    for(auto s:values){const auto n=std::to_string(i++);cx.set(a,opts::Key(std::u16string(n.begin(),n.end())),opts::Value(std::u16string(s)));}return a;
}
std::u16string encodeComponent(std::u16string_view input){
    constexpr char hex[]="0123456789ABCDEF";std::u16string out;
    for(const unsigned char byte:property_host::debugText(input)){
        const bool safe=(byte>='A'&&byte<='Z')||(byte>='a'&&byte<='z')||(byte>='0'&&byte<='9')||byte=='-'||byte=='_'||byte=='.'||byte=='!'||byte=='~'||byte=='*'||byte=='\''||byte=='('||byte==')';
        if(safe)out.push_back(static_cast<char16_t>(byte));else{out.push_back(u'%');out.push_back(static_cast<char16_t>(hex[byte>>4]));out.push_back(static_cast<char16_t>(hex[byte&15]));}
    }return out;
}
struct CallbackEvent{std::string name,receiver;std::vector<opts::Value> args;};
common::FontLoadHost fontHost(){
    common::FontLoadHost host;host.asyncIsSynchronous=[] {return true;};host.warn=[](const std::any&){};return host;
}
struct Fixture{
    std::unique_ptr<output::svg::NativeRenderSession> session;
    opts::Context& cx;
    OptionFunctionBindings& functions;
    tex::ParserRuntime& runtime;
    util::RuntimeModules& modules;
    std::shared_ptr<handlers::html::HTMLMathDocument> activeDocument;
    opts::ObjectRef activeInputJaxIdentity{};
    tex::TeXRef activeJax;
    opts::Value currentSettings=opts::Undefined{};
    std::vector<CallbackEvent> callbackEvents;
    opts::ObjectRef callbackTarget{};
    Fixture():session(std::make_unique<output::svg::NativeRenderSession>(fontHost())),cx(session->context()),
        functions(session->services().functions()),runtime(session->services().parser()),modules(session->services().modules()){
        const auto outputOptions=cx.object();cx.set(outputOptions,opts::Key(u"fontCache"),opts::Value(u"none"));
        cx.set(outputOptions,opts::Key(u"exFactor"),opts::Value(0.5));(void)session->construct(opts::Value(outputOptions));
    }
    opts::ObjectRef function(std::u16string name,double length,opts::NativeCall callback){
        const auto result=cx.function(name,std::move(callback));
        cx.define(result,opts::Key(u"name"),opts::Descriptor::data(opts::Value(name),false,false,true));
        cx.define(result,opts::Key(u"length"),opts::Descriptor::data(opts::Value(length),false,false,true));return result;
    }
    opts::Value settings(std::string_view caseName){
        const auto result=cx.object();
        if(caseName=="tagformat-all-parent")cx.set(result,opts::Key(u"tags"),opts::Value(u"all"));
        else if(caseName=="tagformat-cases-parent")cx.set(result,opts::Key(u"tags"),opts::Value(u"cases"));
        else if(caseName=="tagformat-base-parent")cx.set(result,opts::Key(u"tags"),opts::Value(u"base"));
        else if(caseName=="tagformat-ams-parent")cx.set(result,opts::Key(u"tags"),opts::Value(u"ams"));
        if(caseName=="tagformat-raw-callbacks"){
            const auto format=cx.object();
            auto addCallback=[&](std::u16string name,double length,opts::Value resultValue){
                const std::string eventName=property_host::debugText(name);const bool member=eventName!="ref";
                const auto fn=function(name,length,[this,eventName,member,resultValue=std::move(resultValue)](opts::Context&,opts::Value receiver,std::span<const opts::Value> args){
                    const auto* actual=std::get_if<opts::ObjectRef>(&receiver.data);
                    callbackEvents.push_back({eventName,member?(actual&&*actual==callbackTarget?"tagformat":"other"):(std::holds_alternative<opts::Undefined>(receiver.data)?"undefined":"other"),std::vector<opts::Value>(args.begin(),args.end())});
                    return resultValue;
                });cx.set(format,opts::Key(name),opts::Value(fn));
            };
            const auto number=cx.object();cx.set(number,opts::Key(u"kind"),opts::Value(u"raw-number"));
            const auto url=cx.object();cx.set(url,opts::Key(u"kind"),opts::Value(u"raw-url"));
            addCallback(u"number",1,opts::Value(number));addCallback(u"tag",1,opts::Value(opts::Null{}));
            addCallback(u"ref",1,opts::Value(23.0));addCallback(u"id",1,opts::Value(opts::Undefined{}));
            addCallback(u"url",2,opts::Value(url));cx.set(result,opts::Key(u"tagformat"),opts::Value(format));
        }else if(caseName=="tagformat-all-custom"||caseName=="tagformat-custom-no-ref"){
            const auto format=cx.object();
            auto numberFunction=std::make_shared<opts::ObjectRef>();
            *numberFunction=function(u"number",1,[this,numberFunction](opts::Context& context,opts::Value receiver,std::span<const opts::Value> args){
                if(context.get(receiver,opts::Key(u"number"))!=opts::Value(*numberFunction)||args.size()!=1)return opts::Value(u"bad-number");
                const auto* value=args.empty()?nullptr:std::get_if<double>(&args[0].data);
                return opts::Value(std::u16string(caseName_=="tagformat-all-custom"?u"N":u"Q")+
                    (value?property_host::text(js_number::toString(*value)):u"undefined"));
            });cx.set(format,opts::Key(u"number"),opts::Value(*numberFunction));
            auto tagFunction=std::make_shared<opts::ObjectRef>();
            *tagFunction=function(u"tag",1,[this,tagFunction](opts::Context& context,opts::Value receiver,std::span<const opts::Value> args){
                if(context.get(receiver,opts::Key(u"tag"))!=opts::Value(*tagFunction)||args.size()!=1)return opts::Value(u"bad-tag");
                const auto out=context.array(3);context.set(out,opts::Key(u"0"),caseName_=="tagformat-all-custom"?opts::Value(u"["):opts::Value(u"{"));
                context.set(out,opts::Key(u"1"),args[0]);context.set(out,opts::Key(u"2"),caseName_=="tagformat-all-custom"?opts::Value(u"]"):opts::Value(u"}"));return opts::Value(out);
            });cx.set(format,opts::Key(u"tag"),opts::Value(*tagFunction));
            if(caseName=="tagformat-all-custom"){
                auto refFunction=std::make_shared<opts::ObjectRef>();
                *refFunction=function(u"ref",1,[refFunction](opts::Context& context,opts::Value receiver,std::span<const opts::Value> args){
                if(!std::holds_alternative<opts::Undefined>(receiver.data)||args.size()!=1)return opts::Value(u"bad-ref");
                const auto out=context.array(3);context.set(out,opts::Key(u"0"),u"<");context.set(out,opts::Key(u"1"),args.empty()?opts::Value(opts::Undefined{}):args[0]);context.set(out,opts::Key(u"2"),u">");return opts::Value(out);
                });cx.set(format,opts::Key(u"ref"),opts::Value(*refFunction));
            }
            auto idFunction=std::make_shared<opts::ObjectRef>();
            *idFunction=function(u"id",1,[this,idFunction](opts::Context& context,opts::Value receiver,std::span<const opts::Value> args){
                if(context.get(receiver,opts::Key(u"id"))!=opts::Value(*idFunction)||args.size()!=1)return opts::Value(u"bad-id");
                const auto* value=args.empty()?nullptr:std::get_if<std::u16string>(&args[0].data);return opts::Value(std::u16string(caseName_=="tagformat-all-custom"?u"custom:":u"no-ref:")+(value?*value:u"undefined"));
            });cx.set(format,opts::Key(u"id"),opts::Value(*idFunction));
            auto urlFunction=std::make_shared<opts::ObjectRef>();
            *urlFunction=function(u"url",2,[this,urlFunction](opts::Context& context,opts::Value receiver,std::span<const opts::Value> args){
                if(context.get(receiver,opts::Key(u"url"))!=opts::Value(*urlFunction)||args.size()!=2)return opts::Value(u"bad-url");
                const auto* id=args.empty()?nullptr:std::get_if<std::u16string>(&args[0].data);const auto* base=args.size()<2?nullptr:std::get_if<std::u16string>(&args[1].data);
                return opts::Value((base?*base:u"undefined")+(caseName_=="tagformat-all-custom"?u"?id=":u"#")+
                    (id?(caseName_=="tagformat-all-custom"?encodeComponent(*id):*id):u"undefined"));
            });cx.set(format,opts::Key(u"url"),opts::Value(*urlFunction));
            cx.set(result,opts::Key(u"tagformat"),opts::Value(format));
        }else if(caseName=="setoptions-package-default-disabled"){
            const auto set=cx.object();cx.set(set,opts::Key(u"allowPackageDefault"),opts::Value(false));cx.set(result,opts::Key(u"setoptions"),opts::Value(set));
        }else if(caseName=="setoptions-option-default-disabled"){
            const auto set=cx.object();cx.set(set,opts::Key(u"allowOptionsDefault"),opts::Value(false));cx.set(result,opts::Key(u"setoptions"),opts::Value(set));
        }else if(caseName=="setoptions-filter-value"){
            const auto allowed=cx.object(),texOptions=cx.object(),set=cx.object();cx.set(texOptions,opts::Key(u"FindTeX"),opts::Value(true));cx.set(allowed,opts::Key(u"tex"),opts::Value(texOptions));
            cx.set(set,opts::Key(u"allowOptions"),opts::Value(allowed));cx.set(result,opts::Key(u"setoptions"),opts::Value(set));
        }else if(caseName=="setoptions-live-source-change"){
            const auto set=cx.object();auto filterPackageFunction=std::make_shared<opts::ObjectRef>();
            *filterPackageFunction=function(u"filterPackage",2,[filterPackageFunction](opts::Context& context,opts::Value receiver,std::span<const opts::Value> args){
                if(context.get(receiver,opts::Key(u"filterPackage"))!=opts::Value(*filterPackageFunction)||args.size()!=2)return opts::Value(false);
                const auto parser=std::get<opts::ObjectRef>(args[0].data);const auto parserOptions=context.get(parser,opts::Key(u"options"));
                const auto config=context.get(parserOptions,opts::Key(u"setoptions"));const auto allow=context.get(config,opts::Key(u"allowOptions"));
                const auto texOptions=context.get(allow,opts::Key(u"tex"));context.set(texOptions,opts::Key(u"displaystyle"),opts::Value(false));return opts::Value(true);
            });cx.set(set,opts::Key(u"filterPackage"),opts::Value(*filterPackageFunction));cx.set(result,opts::Key(u"setoptions"),opts::Value(set));
        }
        return opts::Value(result);
    }
    std::string caseName_;
    tex::TeXRef jax(std::span<const std::u16string_view> packages,std::string_view caseName){
        caseName_=std::string(caseName);const auto config=cx.object();currentSettings=settings(caseName);
        for(const auto package:packages){
            if(package==u"cases")(void)tex::CasesConfiguration::initializeModule(runtime);
            else if(package==u"mathtools")(void)tex::MathtoolsConfiguration::initializeModule(runtime);
            else if(package==u"tagformat")(void)tex::TagFormatConfiguration::initializeModule(runtime);
            else if(package==u"setoptions")(void)tex::SetOptionsConfiguration::initializeModule(runtime);
        }
        const auto* user=std::get_if<opts::ObjectRef>(&currentSettings.data);
        for(const auto& name:cx.enumerableStringKeys(*user))cx.set(config,opts::Key(name),cx.get(*user,opts::Key(name)));
        cx.set(config,opts::Key(u"packages"),strings(cx,packages));
        activeDocument=output::svg::createNativeSvgDocument(*session,opts::Value(opts::Undefined{}),opts::Value(config));
        const opts::Value documentValue(activeDocument->sourceIdentity());
        const opts::Value inputJaxArray=cx.get(documentValue,opts::Key(u"inputJax"));
        const opts::Value inputJaxValue=cx.get(inputJaxArray,opts::Key(u"0"));
        const auto* identity=std::get_if<opts::ObjectRef>(&inputJaxValue.data);
        if(!identity)throw opts::Exception("TypeError",u"Tag/SetOptions document inputJax[0] is not canonical");
        activeInputJaxIdentity=*identity;activeJax=runtime.texIdentities.resolve(activeInputJaxIdentity);
        if(!activeJax)throw opts::Exception("TypeError",u"Tag/SetOptions document TeX owner is unavailable");
        const auto parse=activeJax->parseOptions();
        if(const auto* owner=std::get_if<tex::ParseOptionsRef>(&parse);owner&&*owner){
            const auto format=cx.get((*owner)->options,opts::Key(u"tagformat"));
            if(const auto* object=std::get_if<opts::ObjectRef>(&format.data))callbackTarget=*object;
        }
        return activeJax;
    }
    mml::NodeValue compile(std::u16string_view source,tex::TeX& jax){
        if(!activeDocument||activeJax.get()!=&jax)throw std::runtime_error("Tag/SetOptions HTMLMathDocument owner is missing");
        const opts::Value documentValue(activeDocument->sourceIdentity());
        const opts::Value documentOptions=cx.get(documentValue,opts::Key(u"options"));
        const opts::Value constructorValue=cx.get(documentOptions,opts::Key(u"MathItem"));
        const auto* constructor=std::get_if<opts::ObjectRef>(&constructorValue.data);
        if(!constructor)throw opts::Exception("TypeError",u"Tag/SetOptions document MathItem constructor is unavailable");
        const std::array<opts::Value,3> arguments{opts::Value(std::u16string(source)),opts::Value(activeInputJaxIdentity),opts::Value(true)};
        const opts::ObjectRef itemIdentity=cx.construct(*constructor,arguments);
        const auto* native=std::any_cast<std::shared_ptr<NativeObject>>(&cx.payload(itemIdentity));
        const auto item=native?std::dynamic_pointer_cast<core::MathItem>(*native):nullptr;
        if(!item)throw opts::Exception("TypeError",u"Tag/SetOptions MathItem has no canonical HTML owner");
        return jax.compile(*item,core::MathDocumentReference{activeDocument.get()});
    }
};
struct Case{const char* name;std::u16string_view input;std::span<const std::u16string_view> packages;};
constexpr std::u16string_view baseAms[]{u"base",u"ams"},baseAmsTag[]{u"base",u"ams",u"tagformat"},baseTag[]{u"base",u"tagformat"},baseCasesTag[]{u"base",u"ams",u"cases",u"tagformat"},baseAmsMathtoolsTag[]{u"base",u"ams",u"mathtools",u"tagformat"},baseSet[]{u"base",u"setoptions"},baseAmsSet[]{u"base",u"ams",u"setoptions"};
constexpr Case cases[]={
 {"tagformat-default",u"\\begin{equation}x\\label{eq:alpha beta}\\end{equation}",baseAmsTag},
 {"tagformat-base-parent",u"\\begin{equation}x\\label{eq:base}\\end{equation}",baseTag},
 {"tagformat-ams-parent",u"\\begin{equation}x\\label{eq:ams}\\end{equation}",baseAmsTag},
 {"tagformat-none-parent",u"\\begin{equation}x\\label{eq:none}\\end{equation}",baseTag},
 {"tagformat-all-parent",u"\\begin{equation}x\\label{eq:all}\\end{equation}",baseAmsTag},
 {"tagformat-cases-parent",u"\\begin{numcases}{x=}a&first\\\\b&second\\end{numcases}",baseCasesTag},
 {"tagformat-mathtools-parent",u"\\begin{equation}x\\label{eq:mathtools}\\end{equation}",baseAmsMathtoolsTag},
 {"tagformat-ref-fallback",u"\\begin{equation}x\\label{eq:ref}\\end{equation}\\ref{eq:ref}",baseAmsTag},
 {"tagformat-raw-callbacks",u"x",baseAmsTag},
 {"tagformat-config-tags-registration",u"x",baseAmsTag},
 {"tagformat-all-custom",u"\\begin{equation}x\\label{eq:custom}\\end{equation}\\ref{eq:custom}",baseAmsTag},
 {"tagformat-custom-no-ref",u"\\begin{equation}x\\label{eq:no-ref}\\end{equation}\\ref{eq:no-ref}",baseAmsTag},
 {"setoptions-tex",u"\\setOptions{displaystyle=true}x",baseSet},
 {"setoptions-package-valid",u"\\setOptions[ams]{multlineWidth=60%}\\begin{multline}x\\\\y\\end{multline}",baseAmsSet},
 {"setoptions-invalid-package",u"\\setOptions[no-such-package]{x=1}x",baseSet},
 {"setoptions-parse-before-package-filter",u"\\setOptions[no-such-package]{=}",baseSet},
 {"setoptions-invalid-key",u"\\setOptions{unknownKey=1}x",baseSet},
 {"setoptions-package-blocked",u"\\setOptions[setoptions]{allowOptionsDefault=false}x",baseSet},
 {"setoptions-package-default-disabled",u"\\setOptions[ams]{multlineWidth=60%}x",baseAmsSet},
 {"setoptions-option-default-disabled",u"\\setOptions{displaystyle=true}x",baseSet},
 {"setoptions-filter-value",u"\\setOptions{FindTeX=/^x$/}x",baseSet},
 {"setoptions-live-source-change",u"\\setOptions{displaystyle=true}x",baseSet},
 {"setoptions-package-data-preserved",u"\\setOptions{displaystyle=false}x",baseAmsSet}
};
}
int main(int argc,char** argv){
    try{
        Fixture fixture;Serializer serializer{&fixture.cx,&fixture.functions,&fixture.runtime.regexp()};
        // Focused parser mode deliberately skips global token-map reflection:
        // it exercises the same initialized runtime and real TeX compile path,
        // while isolating a single case whose map metadata may be independently
        // under investigation.
        if(argc==3&&std::string_view(argv[1])=="--case"){
            const auto found=std::find_if(std::begin(cases),std::end(cases),[&](const Case& item){return item.name==std::string_view(argv[2]);});
            if(found==std::end(cases)){std::cerr<<"unknown focused case: "<<argv[2]<<'\n';return 2;}
            std::cerr<<"[focused-parser] "<<found->name<<" compile begin\n";
            const auto jax=fixture.jax(found->packages,found->name);
            try{
                std::cerr<<"[focused-parser] "<<found->name<<" native compile call\n";
                const auto result=fixture.compile(found->input,*jax);
                std::cerr<<"[focused-parser] "<<found->name<<" compile returned; serialize begin\n";
                const auto serialized=serializer.node(result);
                std::cerr<<"[focused-parser] "<<found->name<<" serialize complete\n";
                std::cout<<"{\"name\":"<<q(found->name)<<",\"input\":"<<q(text(found->input))
                    <<",\"result\":"<<serialized<<",\"error\":null}\n";
            }catch(const tex::TexError& e){
                std::cout<<"{\"name\":"<<q(found->name)<<",\"input\":"<<q(text(found->input))
                    <<",\"result\":null,\"error\":{"<<"\"name\":\"TexError\",\"id\":"<<q(text(e.id))
                    <<",\"message\":"<<q(text(e.message))<<"}}\n";
            }catch(const options::Exception& e){
                std::cout<<"{\"name\":"<<q(found->name)<<",\"input\":"<<q(text(found->input))
                    <<",\"result\":null,\"error\":{"<<"\"name\":"<<q(e.name)
                    <<",\"message\":"<<q(text(e.message))<<"}}\n";
            }catch(const std::exception& e){
                std::cout<<"{\"name\":"<<q(found->name)<<",\"input\":"<<q(text(found->input))
                    <<",\"result\":null,\"error\":{\"name\":\"NativeException\",\"message\":"<<q(e.what())<<"}}\n";
            }catch(...){
                std::cout<<"{\"name\":"<<q(found->name)<<",\"input\":"<<q(text(found->input))
                    <<",\"result\":null,\"error\":{\"name\":\"UnclassifiedNativeException\"}}\n";
            }
            std::cerr<<"[focused-parser] "<<found->name<<" compile end\n";
            return 0;
        }
        if(argc==4&&std::string_view(argv[1])=="--parse"){
            std::vector<std::u16string> packageStorage;
            std::vector<std::u16string_view> packageViews;
            std::string_view packageArg(argv[2]);
            while(!packageArg.empty()){
                const auto comma=packageArg.find(',');const auto part=packageArg.substr(0,comma);
                packageStorage.emplace_back(part.begin(),part.end());
                if(comma==std::string_view::npos)break;packageArg.remove_prefix(comma+1);
            }
            for(const auto& package:packageStorage)packageViews.push_back(package);
            const std::string_view inputArg(argv[3]);
            const std::u16string input(inputArg.begin(),inputArg.end());
            std::cerr<<"[focused-parser] packages="<<argv[2]<<" compile begin\n";
            const auto jax=fixture.jax(packageViews,"focused-parse");
            try{
                const auto result=fixture.compile(input,*jax);
                std::cout<<"{\"input\":"<<q(text(input))<<",\"result\":"<<serializer.node(result)<<",\"error\":null}\n";
            }catch(const tex::TexError& e){
                std::cout<<"{\"input\":"<<q(text(input))<<",\"result\":null,\"error\":{\"name\":\"TexError\",\"id\":"
                    <<q(text(e.id))<<",\"message\":"<<q(text(e.message))<<"}}\n";
            }catch(const options::Exception& e){
                std::cout<<"{\"input\":"<<q(text(input))<<",\"result\":null,\"error\":{\"name\":"
                    <<q(e.name)<<",\"message\":"<<q(text(e.message))<<"}}\n";
            }catch(const std::exception& e){
                std::cout<<"{\"input\":"<<q(text(input))<<",\"result\":null,\"error\":{\"name\":\"NativeException\",\"message\":"
                    <<q(e.what())<<"}}\n";
            }catch(...){
                std::cout<<"{\"input\":"<<q(text(input))<<",\"result\":null,\"error\":{\"name\":\"UnclassifiedNativeException\"}}\n";
            }
            std::cerr<<"[focused-parser] compile end\n";
            return 0;
        }
        std::cout<<"{\"schema\":1,\"scopeSha256\":\"9705d389538b2fc8bf782feea4372ace639b3727304f2971937bbcfb0042a0d1\",\"sourceLockSha256\":\"adff29b700cdc2e84f2451e84f9424f69679a94bd5f3448048bb6a4d3e6ea58d\",\"cases\":[";
        bool first=true;for(const auto& item:cases){if(!first)std::cout<<',';first=false;std::cout<<"{\"name\":"<<q(item.name)<<",\"input\":"<<q(text(item.input))<<",\"packages\":[";
            for(std::size_t package=0;package<item.packages.size();++package){if(package)std::cout<<',';std::cout<<q(text(item.packages[package]));}
            std::cout<<"],\"settings\":";
            const auto jax=fixture.jax(item.packages,item.name);std::cout<<serializer.value(fixture.currentSettings)<<",\"result\":";
            std::string error="null";
            try{std::cout<<serializer.node(fixture.compile(item.input,*jax));}
            catch(const tex::TexError& e){std::cout<<"null";error="{\"name\":\"TexError\",\"id\":"+q(text(e.id))+",\"message\":"+q(text(e.message))+"}";}
            catch(const options::Exception& e){std::cout<<"null";error="{\"name\":"+q(e.name)+",\"id\":null,\"message\":"+q(text(e.message))+"}";}
            catch(const std::exception& e){std::cout<<"null";error="{\"name\":\"NativeException\",\"id\":null,\"message\":"+q(e.what())+"}";}
            catch(...){std::cout<<"null";error="{\"name\":\"UnclassifiedNativeException\",\"id\":null,\"message\":\"\"}";}
            std::string configTagsProbe="null";
            if(std::string_view(item.name)=="tagformat-config-tags-registration"){
                auto probeConfiguration=std::make_shared<tex::ParserConfiguration>(fixture.runtime);
                const auto emptyPackages=fixture.cx.array(0);
                probeConfiguration->initializeParserConfiguration(opts::Value(emptyPackages));
                const auto configurationIdentity=fixture.runtime.parserConfigurationIdentities.find(probeConfiguration);
                if(!configurationIdentity)throw std::runtime_error("probe ParserConfiguration has no Context identity");
                const auto current=serializer.parseOptions(*jax);
                const auto selected=fixture.cx.get(current,opts::Key(u"tags"));
                const auto* selectedName=std::get_if<std::u16string>(&selected.data);
                if(!selectedName)throw std::runtime_error("probe tags option is not a string");
                const auto selectedTags=fixture.runtime.tagsFactory.create(*selectedName);
                const auto selectedConstructor=fixture.cx.get(opts::Value(selectedTags->sourceIdentity()),opts::Key(u"constructor"));
                const auto* parent=std::get_if<opts::ObjectRef>(&selectedConstructor.data);
                if(!parent)throw std::runtime_error("probe selected Tags has no constructor");
                const auto custom=tex::TagsFactory::registerSourceSubclass(fixture.runtime,u"CustomTags",*parent,{},{});
                const auto customTags=fixture.cx.object();
                fixture.cx.set(customTags,opts::Key(u"custom"),opts::Value(custom->constructor));
                fixture.cx.set(opts::Value(*configurationIdentity),opts::Key(u"tags"),opts::Value(customTags));
                fixture.cx.set(current,opts::Key(u"tags"),opts::Value(u"custom"));
                tex::tagformatConfig(fixture.runtime,*probeConfiguration,*jax);
                const auto afterCustom=serializer.parseOptions(*jax);
                const auto customName=fixture.cx.get(afterCustom,opts::Key(u"tags"));
                const auto* customKey=std::get_if<std::u16string>(&customName.data);
                if(!customKey)throw std::runtime_error("custom TagFormat alias is not a string");
                const auto customInstance=fixture.runtime.tagsFactory.create(*customKey);
                const std::string customResult="{\"kind\":\"custom\",\"className\":"+serializer.tagClassName(customInstance)+
                    ",\"parentName\":"+serializer.tagParentName(customInstance)+"}";
                fixture.cx.set(opts::Value(*configurationIdentity),opts::Key(u"tags"),opts::Value(fixture.cx.object()));
                fixture.cx.set(afterCustom,opts::Key(u"tags"),opts::Value(u"missing"));
                tex::tagformatConfig(fixture.runtime,*probeConfiguration,*jax);
                const auto afterMissing=serializer.parseOptions(*jax);
                const auto missingName=fixture.cx.get(afterMissing,opts::Key(u"tags"));
                const auto* missingKey=std::get_if<std::u16string>(&missingName.data);
                if(!missingKey)throw std::runtime_error("missing TagFormat alias is not a string");
                const auto missingInstance=fixture.runtime.tagsFactory.create(*missingKey);
                const std::string missingResult="{\"kind\":\"missing\",\"className\":"+serializer.tagClassName(missingInstance)+
                    ",\"parentName\":"+serializer.tagParentName(missingInstance)+"}";
                configTagsProbe="["+customResult+","+missingResult+"]";
            }
            const auto optionValue=serializer.parseOptions(*jax);const auto tagOptions=fixture.cx.get(optionValue,opts::Key(u"tagformat"));
            std::string callbackProbe="null";
            if(std::string_view(item.name)=="tagformat-raw-callbacks"){
                const auto shared=jax->parseOptions();const auto* parsed=std::get_if<tex::ParseOptionsRef>(&shared);
                if(!parsed||!*parsed)throw std::runtime_error("raw callback case has no ParseOptions");
                const auto* tagOwner=std::get_if<std::shared_ptr<tex::Tags>>(&(*parsed)->tags);
                if(!tagOwner||!*tagOwner)throw std::runtime_error("raw callback case has no Tags owner");
                const opts::Value receiver((*tagOwner)->sourceIdentity());
                const auto invoke=[&](std::u16string_view method,std::vector<opts::Value> args){
                    const opts::Value function=fixture.cx.get(receiver,opts::Key(std::u16string(method)));
                    const auto* callable=std::get_if<opts::ObjectRef>(&function.data);if(!callable)throw std::runtime_error("TagFormat method is missing");
                    return fixture.cx.call(*callable,receiver,args);
                };
                const std::array<opts::Value,1> numberArgs{opts::Value(8.0)},tagArgs{opts::Value(u"tag")},refArgs{opts::Value(u"ref")},idArgs{opts::Value(u"id")};
                const std::array<opts::Value,2> urlArgs{opts::Value(u"id"),opts::Value(u"base")};
                const auto number=invoke(u"formatNumber",std::vector<opts::Value>(numberArgs.begin(),numberArgs.end()));
                const auto tag=invoke(u"formatTag",std::vector<opts::Value>(tagArgs.begin(),tagArgs.end()));
                const auto ref=invoke(u"formatRef",std::vector<opts::Value>(refArgs.begin(),refArgs.end()));
                const auto id=invoke(u"formatId",std::vector<opts::Value>(idArgs.begin(),idArgs.end()));
                const auto url=invoke(u"formatUrl",std::vector<opts::Value>(urlArgs.begin(),urlArgs.end()));
                callbackProbe="{\"values\":["+serializer.value(number)+","+serializer.value(tag)+","+serializer.value(ref)+","+serializer.value(id)+","+serializer.value(url)+"],\"events\":[";
                for(std::size_t event=0;event<fixture.callbackEvents.size();++event){if(event)callbackProbe+=',';const auto& value=fixture.callbackEvents[event];
                    callbackProbe+="{\"name\":"+q(value.name)+",\"receiver\":"+q(value.receiver)+",\"args\":[";
                    for(std::size_t i=0;i<value.args.size();++i){if(i)callbackProbe+=',';callbackProbe+=serializer.value(value.args[i]);}callbackProbe+="]}";
                }callbackProbe+="]}";
            }
            std::cout<<",\"error\":"<<error<<",\"options\":"<<serializer.value(optionValue)<<",\"packageData\":"<<serializer.packageData(*jax)
                <<",\"tagClass\":"<<serializer.tagClass(*jax)<<",\"tagParent\":"<<serializer.tagParent(*jax)<<",\"tagOptions\":"<<serializer.value(tagOptions)<<",\"callbackProbe\":"<<callbackProbe<<",\"configTagsProbe\":"<<configTagsProbe<<",\"setoptionsMap\":"<<serializer.maps(fixture.runtime)<<"}";
        }std::cout<<"]}\n";return 0;
    }catch(const options::Exception& e){std::cerr<<"fixture-init "<<e.name<<": "<<text(e.message)<<'\n';}
     catch(const std::exception& e){std::cerr<<"fixture-init NativeException: "<<e.what()<<'\n';}
     catch(...){std::cerr<<"fixture-init UnclassifiedNativeException\n";}
    return 2;
}
