// Real native parser observations for the six Source TeX package ports.
// This candidate emits real result/error/packageData observations. Token-map
// reflection remains separate strict-observer work.
import std;
import folia.math.input.tex.parser;
import folia.math.input.tex.base_configuration;
import folia.math.input.tex.action.configuration;
import folia.math.input.tex.bbox.configuration;
import folia.math.input.tex.braket.configuration;
import folia.math.input.tex.cancel.configuration;
import folia.math.input.tex.enclose.configuration;
import folia.math.input.tex.color.configuration;
import folia.math.input.tex.tex_error;
import folia.math.input.tex.map_handler;
import folia.math.input.tex.token_map;
import folia.math.input.tex.token;
import folia.math.core.math_item;
import folia.math.core.math_render_actions;
import folia.math.core.math_document;
#include "support/standalone_abstract_math_item.hpp"
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

namespace {
namespace tex=folia::math::tex;
namespace opts=folia::math::options;
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
        struct ActiveObject {
            std::vector<opts::ObjectRef>* stack;
            ~ActiveObject(){if(stack)stack->pop_back();}
        } active{nullptr};
        if(seen){if(std::find(seen->begin(),seen->end(),*object)!=seen->end())return "{\"$\":\"cycle\"}";seen->push_back(*object);active.stack=seen;}
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
struct Fixture{
    opts::Context cx;OptionFunctionBindings functions{cx};regexp_host::RegExpBindings regex{cx};Unicode16Normalization unicode;
    core::InputJaxModule input{cx};core::FindMathModule findMath{cx};tex::FindTeXModule findTeX{cx,findMath,regex};
    tex::ParserRuntime runtime{cx,functions,regex,unicode,input,findTeX};
    std::unique_ptr<standalone_math_item::Binding> mathItems;
    std::unordered_map<const tex::TeX*,opts::ObjectRef> jaxIdentities;
    Fixture(){runtime.initializeParserRuntime();mathItems=std::make_unique<standalone_math_item::Binding>(cx,runtime.properties,runtime);(void)tex::BasePackage::initializeModule(runtime);
        (void)tex::ActionConfiguration::initializeModule(runtime);(void)tex::BboxConfiguration::initializeModule(runtime);
        (void)tex::BraketConfiguration::initializeModule(runtime);(void)tex::CancelConfiguration::initializeModule(runtime);
        (void)tex::EncloseConfiguration::initializeModule(runtime);
        auto maps=std::make_shared<opts::MapContextBindings>(cx);
        (void)tex::ColorConfiguration::initializeModule(runtime,std::move(maps));}
    tex::TeXRef jax(std::span<const std::u16string_view> packages){
        const auto config=cx.object();cx.set(config,opts::Key(u"packages"),strings(cx,packages));
        auto result=tex::TeX::make(runtime,config);
        auto factory=std::make_shared<mml::MmlFactory>();
        result->setMmlFactory(factory);
        const auto identity=runtime.texIdentities.find(result);
        if(!identity)throw opts::Exception("TypeError",u"Standalone TeX owner has no canonical Source identity");
        jaxIdentities.emplace(result.get(),*identity);
        return result;
    }
    mml::NodeValue compile(std::u16string_view source,tex::TeX& jax){
        const auto inputIdentity=jaxIdentities.find(&jax);
        if(inputIdentity==jaxIdentities.end())throw opts::Exception("TypeError",u"Standalone Source TeX has no canonical InputJax identity");
        const std::array<opts::Value,3> arguments{
            opts::Value(std::u16string(source)),opts::Value(inputIdentity->second),opts::Value(true)};
        const opts::ObjectRef itemIdentity=cx.construct(mathItems->constructor(),arguments);
        const auto* native=std::any_cast<std::shared_ptr<NativeObject>>(&cx.payload(itemIdentity));
        const auto item=native?std::dynamic_pointer_cast<core::MathItem>(*native):nullptr;
        if(!item)throw opts::Exception("TypeError",u"Standalone AbstractMathItem has no canonical native owner");
        return jax.compile(*item,core::MathDocumentReference{Missing{}});
    }
};
struct Case{const char* name;std::u16string_view input;std::span<const std::u16string_view> packages;};
constexpr std::u16string_view baseAction[]{u"base",u"action"},baseBbox[]{u"base",u"bbox"},baseBraket[]{u"base",u"braket"},baseCancel[]{u"base",u"cancel"},baseEnclose[]{u"base",u"enclose"},baseColor[]{u"base",u"color"};
constexpr Case cases[]={
 {"action-toggle",u"\\toggle{x}{y}\\endtoggle",baseAction},{"action-toggle-over",u"\\toggle{a\\over b}{c}\\endtoggle",baseAction},{"action-mathtip",u"\\mathtip{x}{tip}",baseAction},{"action-texttip",u"\\texttip{x}{tip}",baseAction},
 {"bbox-empty",u"\\bbox{x}",baseBbox},{"bbox-padding",u"\\bbox[2px]{x}",baseBbox},{"bbox-color-style",u"\\bbox[red,border:1px solid blue]{x}",baseBbox},{"bbox-invalid",u"\\bbox[nope]{x}",baseBbox},{"bbox-duplicate",u"\\bbox[2px,3px]{x}",baseBbox},
 {"braket-basic",u"\\bra{a}\\ket{b}\\braket{a|b}",baseBraket},{"braket-stretchy",u"\\Braket{a|b}",baseBraket},{"braket-over",u"\\Braket{a\\over b|c}",baseBraket},{"braket-nested-set",u"\\Braket{\\psi|A|\\phi}+\\Set{x|x>0}",baseBraket},
 {"cancel-family",u"\\cancel{x}+\\bcancel{y}+\\xcancel{z}",baseCancel},{"cancel-to",u"\\cancelto{0}{x}",baseCancel},{"cancel-options",u"\\cancel[color=red,data-padding=2px]{x}",baseCancel},
 {"enclose-options",u"\\enclose{updiagonalstrike,circle}[mathcolor=red]{x}",baseEnclose},{"enclose-invalid-option",u"\\enclose{box}[unknown=1]{x}",baseEnclose},
 {"color-named",u"\\color{Red}{x}",baseColor},{"color-rgb",u"\\textcolor[rgb]{0.5,0,1}{x}",baseColor},{"color-rgb-invalid",u"\\textcolor[rgb]{2,0,1}{x}",baseColor},{"color-define",u"\\definecolor{custom}{RGB}{32,64,128}\\color{custom}{x}",baseColor},{"colorbox",u"\\colorbox{red}{x}",baseColor},{"fcolorbox",u"\\fcolorbox{blue}{yellow}{x}",baseColor}
};
}
int main(int argc,char** argv){
    try{
        Fixture fixture;Serializer serializer{&fixture.cx,&fixture.functions};
        // Focused parser mode deliberately skips global token-map reflection:
        // it exercises the same initialized runtime and real TeX compile path,
        // while isolating a single case whose map metadata may be independently
        // under investigation.
        if(argc==3&&std::string_view(argv[1])=="--case"){
            const auto found=std::find_if(std::begin(cases),std::end(cases),[&](const Case& item){return item.name==std::string_view(argv[2]);});
            if(found==std::end(cases)){std::cerr<<"unknown focused case: "<<argv[2]<<'\n';return 2;}
            std::cerr<<"[focused-parser] "<<found->name<<" compile begin\n";
            const auto jax=fixture.jax(found->packages);
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
            const auto jax=fixture.jax(packageViews);
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
        bool first=true;
        for(const auto& item:cases){
            std::string packagesJson="[";
            for(std::size_t package=0;package<item.packages.size();++package){if(package)packagesJson+=',';packagesJson+=q(text(item.packages[package]));}
            packagesJson+=']';
            tex::TeXRef jax;
            std::string resultJson="null",errorJson="null";
            try{
                jax=fixture.jax(item.packages);
                const auto result=fixture.compile(item.input,*jax);
                resultJson=serializer.node(result);
            }
            catch(const tex::TexError& e){errorJson="{\"name\":\"TexError\",\"id\":"+q(text(e.id))+",\"message\":"+q(text(e.message))+"}";}
            catch(const options::Exception& e){errorJson="{\"name\":"+q(e.name)+",\"message\":"+q(text(e.message))+"}";}
            catch(const std::exception& e){errorJson="{\"name\":\"NativeException\",\"message\":"+q(e.what())+"}";}
            catch(...){errorJson="{\"name\":\"UnclassifiedNativeException\"}";}
            const std::string mapsJson=serializer.maps(fixture.runtime);
            const std::string packageJson=jax?serializer.packageData(*jax):"null";
            const std::string caseJson="{\"name\":"+q(item.name)+",\"input\":"+q(text(item.input))+
                ",\"packages\":"+packagesJson+",\"result\":"+resultJson+
                ",\"error\":"+errorJson+",\"packageData\":"+packageJson+",\"maps\":"+mapsJson+"}";
            if(!first)std::cout<<',';
            first=false;
            std::cout<<caseJson;
        }
        std::cout<<"]}\n";return 0;
    }catch(const options::Exception& e){std::cerr<<"fixture-init "<<e.name<<": "<<text(e.message)<<'\n';}
     catch(const std::exception& e){std::cerr<<"fixture-init NativeException: "<<e.what()<<'\n';}
     catch(...){std::cerr<<"fixture-init UnclassifiedNativeException\n";}
    return 2;
}
