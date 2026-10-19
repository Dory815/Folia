// Real native parser observations for BBM, BBoldx and Dsfont packages.
// This candidate emits real result/error/packageData observations. Token-map
// reflection remains separate strict-observer work.
import std;
import folia.math.input.tex.parser;
import folia.math.input.tex.base_configuration;
import folia.math.input.tex.bbm.configuration;
import folia.math.input.tex.bboldx.configuration;
import folia.math.input.tex.dsfont.configuration;
import folia.math.input.tex.textmacros.configuration;
import folia.math.input.tex.tex_error;
import folia.math.input.tex.map_handler;
import folia.math.input.tex.token_map;
import folia.math.input.tex.token;
import folia.math.core.math_item;
import folia.math.core.math_render_actions;
import folia.math.handlers.html.html_math_document;
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.mml_node;
import folia.math.core.find_math_host;
import folia.math.core.input_jax_host;
import folia.math.input.tex.find_tex_host;
import folia.math.util.regexp_host;
import folia.math.util.unicode_normalization;
import folia.math.util.options;
import folia.math.util.property_host;
import folia.math.util.map_bindings;
import folia.math.util.option_functions;
import folia.math.output.svg.native_document;
#include "support/standalone_abstract_math_item.hpp"

namespace {
namespace tex=folia::math::tex;
namespace opts=folia::math::options;
namespace common=folia::math::output::common;
namespace svg=folia::math::output::svg;
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
    tex::ParserRuntime* runtime=nullptr;
    std::string observationPath;
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
        // The pinned Source oracle keeps one WeakSet for the whole plain()
        // traversal and does not remove objects after visiting them. Preserve
        // that alias marker behavior across sibling packageData paths.
        if(seen){if(std::find(seen->begin(),seen->end(),*object)!=seen->end())return "{\"$\":\"cycle\"}";seen->push_back(*object);}
        if(context->kind(*object)==opts::ObjectKind::array){const auto length=context->arrayLength(*object);std::string out="[";for(std::uint32_t i=0;i<length;++i){if(i)out+=',';const auto index=std::to_string(i);const opts::Key k(std::u16string(index.begin(),index.end()));out+=context->hasOwn(*object,k)?value(context->get(*object,k),seen):"{\"$\":\"hole\"}";}return out+"]";}
        if(const auto* map=std::any_cast<opts::MapStorageRef>(&context->payload(*object));map&&*map){std::string out="{\"$\":\"map\",\"entries\":[";bool first=true;for(const auto& entry:(*map)->entries()){if(!entry.live)continue;if(!first)out+=',';first=false;out+='[';out+=value(entry.key,seen)+','+value(entry.value,seen)+']';}return out+"]}";}
        std::string out="{";bool first=true;for(const auto& key:context->ownKeys(opts::Value(*object))){if(!first)out+=',';first=false;out+=q(text(context->keyString(key)))+":"+value(context->get(opts::Value(*object),key),seen);}return out+"}";
    }
    std::string packageData(tex::TeX& jax){
        const auto options=jax.parseOptions();const auto* parsed=std::get_if<tex::ParseOptionsRef>(&options);if(!parsed||!*parsed)return "null";
        const auto data=(*parsed)->packageData;const auto* map=std::get_if<std::shared_ptr<SourceStringMap<opts::Value>>>(&data);if(!map||!*map)return "null";
        std::vector<opts::ObjectRef> seen;
        // Source plain() visits the root Map before its entry values. Its
        // shared aliases must therefore encounter that same visited identity.
        if(!runtime)throw std::runtime_error("ParserRuntime is required for packageData Map identity");
        seen.push_back(runtime->maps.bind((*map)->storage()));
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
            const auto identity=functions->find(*f);if(!identity)throw std::runtime_error("Unbound FunctionRef in token map at "+observationPath);
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
        if(entries)for(const auto&[key,value]:*entries){if(!first)out+=',';first=false;
            const auto previous=observationPath;observationPath=text(property_host::string(key));
            try{out+='['+property(key)+','+lookup(value)+']';}
            catch(...){observationPath=previous;throw;}
            observationPath=previous;
        }
        out+="]}";
        const auto parse=parsed->parser();std::string functionName="\"\"",functionLength;
        if(const auto* body=std::get_if<FunctionRef>(&parse);body&&*body){
            const auto previous=observationPath;observationPath+=".parser";
            const auto identity=functions->find(*body);if(!identity){observationPath=previous;throw std::runtime_error("Unbound FunctionRef in token map parser at "+previous);}
            const auto nameValue=context->get(*identity,opts::Key(u"name"));
            const auto* name=std::get_if<std::u16string>(&nameValue.data);
            if(!name)throw std::runtime_error("Token-map parser function name is missing");functionName=q(text(*name));
            const auto lengthValue=context->get(*identity,opts::Key(u"length"));
            if(const auto* length=std::get_if<double>(&lengthValue.data))functionLength=",\"functionLength\":"+std::to_string(*length);
        }else if(const auto* value=std::get_if<Property>(&parse);value&&!property_host::nullish(*value))throw std::runtime_error("Token-map parser is non-callable");
        return "{\"name\":"+property(parsed->name())+",\"map\":"+out+",\"functionName\":"+functionName+functionLength+"}";
    }
    std::string maps(tex::ParserRuntime& runtime){
        constexpr std::string_view names[]{"bbm","bboldx-mathchar0miNormal","bboldx-delimiterNormal","bboldx-mathchar0miBold","bboldx-delimiterBold","bboldx","text-bboldx-mathchar0miNormal","text-bboldx-delimiterNormal","text-bboldx-mathchar0miBold","text-bboldx-delimiterBold","text-bboldx","dsfont"};
        std::string out="{";bool first=true;for(const auto name:names){const auto map=runtime.mapHandler.getMap(Property(std::string(name)));if(!map)continue;if(!first)out+=',';first=false;const auto previous=observationPath;observationPath=std::string(name);out+=q(name)+":";try{out+=tokenMap(map);}catch(const std::exception& error){out+="{\"$observerError\":"+q(error.what())+"}";}catch(...){out+="{\"$observerError\":\"unclassified native observer failure\"}";}observationPath=previous;}return out+"}";
    }
    static FunctionRef ref(const tex::ParseMethod& method){
        if(const auto* f=std::get_if<FunctionRef>(&method))return *f;
        if(const auto* p=std::get_if<Property>(&method))if(const auto* f=std::get_if<FunctionRef>(&p->value))return *f;
        return {};
    }
    FunctionRef macroFunction(std::string_view mapName,std::string_view token){
        const auto map=runtime->mapHandler.getMap(Property(std::string(mapName)));if(!map)return {};
        const auto item=map->lookup(Property(std::string(token)));const auto* macro=std::get_if<tex::MacroRef>(&item);
        return macro&&*macro?ref((*macro)->func()):FunctionRef{};
    }
    FunctionRef characterParser(std::string_view mapName){
        const auto map=runtime->mapHandler.getMap(Property(std::string(mapName)));return map?ref(map->parser()):FunctionRef{};
    }
    std::string aliases(){
        const auto same=[](const FunctionRef& a,const FunctionRef& b){return a&&b&&a==b;};
        const auto base=[&](std::string_view name){return runtime?folia::math::functions::reference(folia::math::functions::fromProperty(property_host::get(runtime->baseMethods,std::string(name)))):FunctionRef{};};
        const auto macro=base("Macro"),mathFont=base("MathFont");
        const auto boolText=[](bool value){return value?"true":"false";};
        return "{\"bbmChooseFontShared\":"+std::string(boolText(same(macroFunction("bbm","mathbbm"),macroFunction("bbm","mathbbmss"))))+
            ",\"bbmBaseMathFontShared\":"+boolText(same(macroFunction("bbm","mathbbmtt"),mathFont))+
            ",\"bboldxNormalCharacterShared\":"+boolText(same(characterParser("bboldx-mathchar0miNormal"),characterParser("text-bboldx-mathchar0miNormal")))+
            ",\"bboldxNormalDelimiterShared\":"+boolText(same(characterParser("bboldx-delimiterNormal"),characterParser("text-bboldx-delimiterNormal")))+
            ",\"bboldxBoldCharacterShared\":"+boolText(same(characterParser("bboldx-mathchar0miBold"),characterParser("text-bboldx-mathchar0miBold")))+
            ",\"bboldxBoldDelimiterShared\":"+boolText(same(characterParser("bboldx-delimiterBold"),characterParser("text-bboldx-delimiterBold")))+
            ",\"bboldxChooseFontShared\":"+boolText(same(macroFunction("bboldx","mathbb"),macroFunction("bboldx","mathbfbb")))+
            ",\"bboldxTextFontShared\":"+boolText(same(macroFunction("text-bboldx","textbb"),macroFunction("text-bboldx","textbfbb")))+
            ",\"bboldxMacroAlias\":"+boolText(same(macroFunction("bboldx","imathbb"),macro))+
            ",\"dsfontChooseFont\":"+boolText(static_cast<bool>(macroFunction("dsfont","mathds")))+"}";
    }
    std::string node(mml::NodeValue value){
        if(!value.isNode())return "null";auto* n=value.requireNode();
        const auto attrs=n->attributes();std::string out="{\"kind\":"+q(n->kind());
        if(const auto* textNode=dynamic_cast<mml::TextNode*>(n))out+=",\"text\":"+q(text(textNode->getText()));
        else if(const auto* token=dynamic_cast<mml::AbstractMmlTokenNode*>(n))out+=",\"text\":"+q(text(token->getText()));
        out+=",\"attrs\":"+record(attrs?attrs->getAllAttributes():PropertyList{})+",\"properties\":"+record(n->getAllProperties())+",\"children\":[";
        const auto children=n->childNodes;if(children)for(std::size_t i=0;i<children->size();++i){if(i)out+=',';out+=node(children->get(i));}return out+"]}";
    }
};
opts::Value strings(opts::Context& cx,std::span<const std::u16string_view> values){
    const auto a=cx.array(static_cast<std::uint32_t>(values.size()));std::uint32_t i=0;
    for(auto s:values){const auto n=std::to_string(i++);cx.set(a,opts::Key(std::u16string(n.begin(),n.end())),opts::Value(std::u16string(s)));}return a;
}
constexpr std::u16string_view baseBbm[]{u"base",u"bbm"},baseBboldx[]{u"base",u"bboldx"},baseTextBboldx[]{u"base",u"textmacros",u"bboldx"},baseDsfont[]{u"base",u"dsfont"};
struct Case{const char* name;std::u16string_view input;std::span<const std::u16string_view> packages;const char* optionGroup;const char* option;bool value;const char* option2=nullptr;bool value2=false;};
constexpr Case cases[]={
 {"bbm-default",u"\\mathbbm{ABC} + \\mathbbmss{xyz} + \\mathbbmtt{12}",baseBbm,nullptr,nullptr,false},
 {"bbm-bold-switch",u"\\mathbbm{A}\\mathversion{bold}\\mathbbm{B}\\mathversion{regular}\\mathbbm{C}",baseBbm,nullptr,nullptr,false},
 {"bbm-missing-argument",u"\\mathbbm",baseBbm,nullptr,nullptr,false},
 {"bboldx-normal",u"\\mathbb{R}+\\bbGamma+\\bbalpha+\\bbLparen",baseBboldx,nullptr,nullptr,false},
 {"bboldx-bold-option",u"\\mathbb{R}+\\mathbfbb{C}+\\bfbbGamma",baseBboldx,"bboldx","bfbb",true},
 {"bboldx-light-option",u"\\mathbb{R}+\\bbGamma",baseBboldx,"bboldx","light",true},
 {"bboldx-dotless",u"\\imathbb+\\jmathbb+\\imathbfbb+\\jmathbfbb",baseBboldx,nullptr,nullptr,false},
 {"bboldx-text",u"\\text{\\textbb{A} \\textbfbb{B} \\txtbbalpha \\txtbfbbalpha}",baseTextBboldx,nullptr,nullptr,false},
 {"bboldx-missing-argument",u"\\mathbb",baseBboldx,nullptr,nullptr,false},
 {"dsfont-default",u"\\mathds{1}+\\mathds{A}",baseDsfont,nullptr,nullptr,false},
 {"dsfont-sans-option",u"\\mathds{1}+\\mathds{A}",baseDsfont,"dsfont","sans",true},
 {"dsfont-missing-argument",u"\\mathds",baseDsfont,nullptr,nullptr,false},
 {"bbm-bold-option",u"\\mathbbm{A}+\\mathbbmss{B}",baseBbm,"bbm","bold",true},
 {"bboldx-option-precedence",u"\\mathbb{A}+\\bbGamma+\\bfbbGamma",baseBboldx,"bboldx","bfbb",true,"light",true},
 {"bboldx-text-dotless-aliases",u"\\text{\\itextbb\\jtextbb\\itextbfbb\\jtextbfbb}",baseTextBboldx,nullptr,nullptr,false},
 {"bboldx-text-light-option",u"\\text{\\textbb{A} \\textbfbb{B} \\txtbbalpha \\txtbfbbalpha}",baseTextBboldx,"bboldx","light",true}
};
struct Fixture{
    static common::FontLoadHost fontHost(){
        common::FontLoadHost host;host.asyncIsSynchronous=[] {return true;};
        host.warn=[](const std::any&){};return host;
    }
    svg::NativeRenderSession session{fontHost()};
    opts::Context& cx;
    OptionFunctionBindings& functions;
    tex::ParserRuntime& runtime;
    std::unique_ptr<standalone_math_item::Binding> mathItems;
    opts::ObjectRef inputJaxIdentity{};
    Fixture():cx(session.context()),functions(session.services().functions()),
        runtime(session.services().parser()),
        mathItems(std::make_unique<standalone_math_item::Binding>(cx,runtime.properties,runtime)){
        (void)tex::BasePackage::initializeModule(runtime);
        (void)tex::BbmConfiguration::initializeModule(runtime);
        (void)tex::BboldxConfiguration::initializeModule(runtime);
        (void)tex::DsfontConfiguration::initializeModule(runtime);
        (void)tex::TextMacrosConfiguration::initializeModule(runtime);
    }
    tex::TeXRef jax(const Case& item){
        const auto config=cx.object();cx.set(config,opts::Key(u"packages"),strings(cx,item.packages));
        if(item.option){const auto section=cx.object();cx.set(section,opts::Key(std::u16string(item.option,item.option+std::strlen(item.option))),opts::Value(item.value));
            if(item.option2)cx.set(section,opts::Key(std::u16string(item.option2,item.option2+std::strlen(item.option2))),opts::Value(item.value2));
            const std::u16string optionGroup(item.optionGroup,item.optionGroup+std::strlen(item.optionGroup));cx.set(config,opts::Key(optionGroup),opts::Value(section));}
        auto jax=tex::TeX::make(runtime,opts::Value(config));
        jax->setMmlFactory(session.services().mmlFactoryOwner());
        const auto identity=runtime.texIdentities.find(jax);
        if(!identity)throw opts::Exception("TypeError",u"Standalone TeX owner has no canonical Source identity");
        inputJaxIdentity=*identity;
        return jax;
    }
    mml::NodeValue compile(std::u16string_view source,tex::TeX& jax){
        if(inputJaxIdentity.context==0)throw std::logic_error("Standalone InputJax identity is missing");
        const std::array<opts::Value,3> arguments{opts::Value(std::u16string(source)),opts::Value(inputJaxIdentity),opts::Value(true)};
        const auto identity=cx.construct(mathItems->constructor(),arguments);
        const auto* payload=std::any_cast<std::shared_ptr<NativeObject>>(&cx.payload(identity));
        const auto owner=payload?std::dynamic_pointer_cast<core::MathItem>(*payload):nullptr;
        if(!owner)throw opts::Exception("TypeError",u"Standalone AbstractMathItem has no canonical native owner");
        return jax.compile(*owner,core::MathDocumentReference{Missing{}});
    }
};

}
int main(int argc,char** argv){
    try{
        Fixture fixture;Serializer serializer{&fixture.cx,&fixture.functions,&fixture.runtime};
        // Focused parser mode deliberately skips global token-map reflection:
        // it exercises the same initialized runtime and real TeX compile path,
        // while isolating a single case whose map metadata may be independently
        // under investigation.
        if(argc==3&&std::string_view(argv[1])=="--case"){
            const auto found=std::find_if(std::begin(cases),std::end(cases),[&](const Case& item){return item.name==std::string_view(argv[2]);});
            if(found==std::end(cases)){std::cerr<<"unknown focused case: "<<argv[2]<<'\n';return 2;}
            std::cerr<<"[focused-parser] "<<found->name<<" compile begin\n";
            const auto jax=fixture.jax(*found);
            try{
                const auto result=fixture.compile(found->input,*jax);
                std::cout<<"{\"name\":"<<q(found->name)<<",\"input\":"<<q(text(found->input))
                    <<",\"result\":"<<serializer.node(result)<<",\"error\":null}\n";
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
            const Case parseCase{"parse",input,packageViews,nullptr,nullptr,false};
            const auto jax=fixture.jax(parseCase);
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
        std::cout<<"{\"schema\":1,\"cases\":[";
        bool first=true;for(const auto& item:cases){if(!first)std::cout<<',';first=false;std::cout<<"{\"name\":"<<q(item.name)<<",\"input\":"<<q(text(item.input))<<",\"packages\":[";
            for(std::size_t package=0;package<item.packages.size();++package){if(package)std::cout<<',';std::cout<<q(text(item.packages[package]));}
            std::cout<<"],\"options\":"<<(item.option?(std::string("{\"")+item.optionGroup+"\":{" )+std::string("\"")+item.option+"\":"+(item.value?"true":"false")+(item.option2?std::string(",\"")+item.option2+"\":"+(item.value2?"true":"false"):std::string{})+"}}":"{}")<<",\"result\":";
            const auto jax=fixture.jax(item);
            std::string resultJson="null",errorJson="null";
            try{
                const auto result=fixture.compile(item.input,*jax);
                resultJson=serializer.node(result);
            }
            catch(const tex::TexError& e){errorJson="{\"name\":\"TexError\",\"id\":"+q(text(e.id))+",\"message\":"+q(text(e.message))+"}";}
            catch(const options::Exception& e){errorJson="{\"name\":"+q(e.name)+",\"message\":"+q(text(e.message))+"}";}
            catch(const std::exception& e){errorJson="{\"name\":\"NativeException\",\"message\":"+q(e.what())+"}";}
            catch(...){errorJson="{\"name\":\"UnclassifiedNativeException\"}";}
            const auto packageJson=serializer.packageData(*jax);
            const auto mapsJson=serializer.maps(fixture.runtime);
            const auto aliasesJson=serializer.aliases();
            std::cout<<resultJson<<",\"error\":"<<errorJson<<",\"packageData\":"<<packageJson
                <<",\"maps\":"<<mapsJson<<",\"aliases\":"<<aliasesJson<<"}";
        }std::cout<<"]}\n";return 0;
    }catch(const options::Exception& e){std::cerr<<"fixture-init "<<e.name<<": "<<text(e.message)<<'\n';}
     catch(const std::exception& e){std::cerr<<"fixture-init NativeException: "<<e.what()<<'\n';}
     catch(...){std::cerr<<"fixture-init UnclassifiedNativeException\n";}
    return 2;
}
