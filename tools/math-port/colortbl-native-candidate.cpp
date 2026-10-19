// Native observations for the pinned Colortbl Source corpus.
// This candidate uses the production parser, MML tree, configuration, and map owners.
import std;
import folia.math.input.tex.parser;
import folia.math.input.tex.base_configuration;
import folia.math.input.tex.color.configuration;
import folia.math.input.tex.colortbl.configuration;
import folia.math.input.tex.colortbl.items;
import folia.math.input.tex.tex_error;
import folia.math.input.tex.map_handler;
import folia.math.input.tex.token_map;
import folia.math.input.tex.token;
import folia.math.core.math_item;
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.mml_node;
import folia.math.handlers.html.html_math_document;
import folia.math.output.common.font_data_surface;
import folia.math.output.svg.native_document;
import folia.math.output.svg.render_session;
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
namespace svg=folia::math::output::svg;
namespace common=folia::math::output::common;
using namespace folia::math;

common::FontLoadHost fontHost(){
    common::FontLoadHost host;host.asyncIsSynchronous=[] {return true;};host.warn=[](const std::any&){};return host;
}

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
    tex::MapHandler* registry=nullptr;
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
        constexpr std::string_view names[]{"colortbl"};
        if(!registry)throw std::runtime_error("Colortbl map observer has no runtime registry");
        std::string out="{";bool first=true;for(const auto name:names){const auto map=registry->getMap(Property(std::string(name)));if(!map)continue;if(!first)out+=',';first=false;out+=q(name)+":"+tokenMap(map);}return out+"}";
    }
    std::string node(mml::NodeValue value){
        if(!value.isNode())return "null";auto* n=value.requireNode();
        const auto attrs=n->attributes();std::string out="{\"kind\":"+q(n->kind());
        out+=",\"attrs\":"+record(attrs?attrs->getAllAttributes():PropertyList{})+",\"properties\":"+record(n->getAllProperties());
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
struct Fixture{
    std::unique_ptr<svg::NativeRenderSession> session;
    opts::Context& cx;OptionFunctionBindings& functions;tex::ParserRuntime& runtime;mml::MmlFactory& factory;
    std::shared_ptr<opts::MapContextBindings> mapBindings=std::make_shared<opts::MapContextBindings>(cx);
    std::unordered_map<const tex::TeX*,std::shared_ptr<handlers::html::HTMLMathDocument>> documents;
    Fixture():session(std::make_unique<svg::NativeRenderSession>(fontHost())),cx(session->context()),
        functions(session->services().functions()),runtime(session->services().parser()),factory(session->services().mmlFactory()),
        mapBindings(std::make_shared<opts::MapContextBindings>(cx)){
        (void)tex::BasePackage::initializeModule(runtime);
        (void)tex::ColorConfiguration::initializeModule(runtime,mapBindings);
        (void)tex::ColortblConfiguration::initializeModule(runtime);}
    
    tex::TeXRef jax(std::span<const std::u16string_view> packages){
        const auto config=cx.object();cx.set(config,opts::Key(u"packages"),strings(cx,packages));
        auto document=svg::createNativeSvgDocument(*session,opts::Undefined{},opts::Value(config));
        const auto inputJax=cx.get(opts::Value(document->sourceIdentity()),opts::Key(u"inputJax"));
        const auto input=cx.get(inputJax,opts::Key(u"0"));const auto* identity=std::get_if<opts::ObjectRef>(&input.data);
        if(!identity)throw opts::Exception("TypeError",u"Colortbl document inputJax[0] is not canonical");
        auto result=runtime.texIdentities.resolve(*identity);result->setMmlFactory(&factory);
        documents.emplace(result.get(),std::move(document));return result;
    }
    mml::NodeValue compile(std::u16string_view source,tex::TeX& jax){
        const auto found=documents.find(&jax);if(found==documents.end())throw std::runtime_error("Colortbl HTMLMathDocument owner is missing");
        auto& document=*found->second;auto& context=document.sourceContext();const opts::Value documentValue(document.sourceIdentity());
        const auto options=context.get(documentValue,opts::Key(u"options"));const auto constructor=context.get(options,opts::Key(u"MathItem"));
        const auto inputJax=context.get(documentValue,opts::Key(u"inputJax"));const auto input=context.get(inputJax,opts::Key(u"0"));
        const std::array<opts::Value,3> args{opts::Value(std::u16string(source)),input,opts::Value(true)};
        const auto* itemConstructor=std::get_if<opts::ObjectRef>(&constructor.data);if(!itemConstructor)throw opts::Exception("TypeError",u"HTMLMathDocument.options.MathItem is not a constructor");
        const auto identity=context.construct(*itemConstructor,args);const auto* payload=std::any_cast<std::shared_ptr<NativeObject>>(&context.payload(identity));
        auto item=payload?std::dynamic_pointer_cast<core::MathItem>(*payload):nullptr;if(!item)throw opts::Exception("TypeError",u"Colortbl MathItem has no canonical HTML owner");
        return jax.compile(*item,core::MathDocumentReference{&document});
    }
};
struct Case{const char* name;std::u16string_view input;std::span<const std::u16string_view> packages;};
constexpr std::u16string_view baseColortbl[]{u"base",u"colortbl"};
constexpr std::u16string_view baseColorColortbl[]{u"base",u"color",u"colortbl"};
constexpr Case cases[]={
 {"cell-basic",uR"(\begin{array}{cc}\cellcolor{red}a&b\\c&d\end{array})",baseColortbl},
 {"cell-all-cells",uR"(\begin{array}{cc}\cellcolor{blue}a&\cellcolor{green}b\\\cellcolor{yellow}c&\cellcolor{cyan}d\end{array})",baseColortbl},
 {"cell-over-row-and-column",uR"(\begin{array}{c}\rowcolor{blue}\columncolor{red}\cellcolor{green}x\\y\end{array})",baseColortbl},
 {"row-first-cell",uR"(\begin{array}{cc}\rowcolor{red}a&b\\c&d\end{array})",baseColortbl},
 {"row-color-resets",uR"(\begin{array}{c}\rowcolor{red}a\\b\\\rowcolor{blue}c\end{array})",baseColortbl},
 {"row-color-after-cell",uR"(\begin{array}{c}a\rowcolor{red}b\end{array})",baseColortbl},
 {"row-color-after-empty-entry",uR"(\begin{array}{c}&\rowcolor{red}a\end{array})",baseColortbl},
 {"column-color-preamble",uR"(\begin{array}{>{\columncolor{red}}c}a\\b\end{array})",baseColortbl},
 {"column-color-before-first-cell",uR"(\begin{array}{c}\columncolor{red}a\\b\end{array})",baseColortbl},
 {"column-color-second-column",uR"(\begin{array}{cc}a&\columncolor{red}b\\c&d\end{array})",baseColortbl},
 {"column-color-same-as-preamble",uR"(\begin{array}{c}\columncolor{red}a&\columncolor{red}b\end{array})",baseColortbl},
 {"column-color-different-after-table-start",uR"(\begin{array}{c}a\\\columncolor{red}b\end{array})",baseColortbl},
 {"column-color-overlap-arguments",uR"(\begin{array}{c}\columncolor{red}[1pt][2pt]a\end{array})",baseColortbl},
 {"cell-color-optional-model",uR"(\begin{array}{c}\cellcolor[rgb]{1,0,0}a\end{array})",baseColortbl},
 {"cell-color-custom-definition",uR"(\definecolor{custom}{rgb}{0,0.5,1}\begin{array}{c}\cellcolor{custom}a\end{array})",baseColorColortbl},
 {"cell-color-unknown-name",uR"(\begin{array}{c}\cellcolor{noSuchColor}a\end{array})",baseColortbl},
 {"cell-color-invalid-model",uR"(\begin{array}{c}\cellcolor[noSuchModel]{x}a\end{array})",baseColortbl},
 {"cell-color-outside-array",uR"(\cellcolor{red}x)",baseColortbl},
 {"row-color-outside-array",uR"(\rowcolor{red}x)",baseColortbl},
 {"column-color-outside-array",uR"(\columncolor{red}x)",baseColortbl},
 {"cell-color-missing-argument",uR"(\begin{array}{c}\cellcolor)",baseColortbl},
 {"row-color-missing-argument",uR"(\begin{array}{c}\rowcolor)",baseColortbl},
 {"column-color-missing-argument",uR"(\begin{array}{c}\columncolor)",baseColortbl},
 {"colored-array-framed",uR"(\begin{array}{|c|}\hline\cellcolor{red}a\\\hline\end{array})",baseColortbl},
 {"colored-array-fenced",uR"(\left(\begin{array}{c}\rowcolor{red}a\\b\end{array}\right))",baseColortbl},
 {"colored-array-in-mstyle",uR"(\scriptstyle\begin{array}{c}\cellcolor{red}x\end{array})",baseColortbl},
 {"color-package-explicit",uR"(\begin{array}{c}\cellcolor{red}x\end{array})",baseColorColortbl},
 {"color-package-not-listed",uR"(\begin{array}{c}\cellcolor{red}x\end{array})",baseColortbl},
 {"column-color-top-row-repeat",uR"(\begin{array}{cc}\columncolor{red}a&\columncolor{red}b\end{array})",baseColortbl}
};
std::string descriptor(Serializer& serializer,opts::ObjectRef object,const opts::Key& propertyKey){
    const auto found=serializer.context->descriptor(opts::Value(object),propertyKey);
    if(!found)return "null";
    const auto& value=*found;
    std::string result="{\"configurable\":"+std::string(value.configurable?"true":"false")+
        ",\"enumerable\":"+std::string(value.enumerable?"true":"false");
    if(value.accessor){
        result+=",\"get\":"+(value.getter?serializer.value(opts::Value(*value.getter)):"null")+
            ",\"set\":"+(value.setter?serializer.value(opts::Value(*value.setter)):"null");
    }else{
        result+=",\"value\":"+serializer.value(value.value)+
            ",\"writable\":"+std::string(value.writable?"true":"false");
    }
    return result+"}";
}
std::string classInfo(Fixture& fixture,Serializer& serializer,tex::TeX& jax){
    const auto parseValue=jax.parseOptions();
    const auto* parse=std::get_if<tex::ParseOptionsRef>(&parseValue);
    if(!parse||!*parse)throw opts::Exception("TypeError",u"Colortbl ParseOptions unavailable");
    const auto* factoryValue=std::get_if<std::shared_ptr<tex::StackItemFactory>>(&(*parse)->itemFactory);
    if(!factoryValue||!*factoryValue)throw opts::Exception("TypeError",u"Colortbl StackItemFactory unavailable");
    auto& factory=**factoryValue;
    const auto classReference=factory.getNodeClass("array");
    const auto constructor=classReference?fixture.runtime.itemClassIdentities.find(classReference):std::optional<opts::ObjectRef>{};
    if(!constructor)throw opts::Exception("TypeError",u"ColorArrayItem constructor identity unavailable");
    const auto prototypeValue=fixture.cx.get(opts::Value(*constructor),opts::Key(u"prototype"));
    const auto* prototype=std::get_if<opts::ObjectRef>(&prototypeValue.data);
    if(!prototype)throw opts::Exception("TypeError",u"ColorArrayItem prototype unavailable");
    const auto className=fixture.cx.get(opts::Value(*constructor),opts::Key(u"name"));
    const auto classLength=fixture.cx.get(opts::Value(*constructor),opts::Key(u"length"));
    std::string output="{\"name\":"+serializer.value(className)+",\"length\":"+serializer.value(classLength)+",\"prototype\":[";
    bool first=true;
    for(const auto& propertyKey:fixture.cx.ownKeys(opts::Value(*prototype))){
        if(!first)output+=',';first=false;
        output+='['+q(text(fixture.cx.keyString(propertyKey)))+','+descriptor(serializer,*prototype,propertyKey)+']';
    }
    auto* raw=factory.create("array",tree::FactoryArgs{});
    auto* item=dynamic_cast<tex::ColorArrayItem*>(raw);
    if(!item)throw opts::Exception("TypeError",u"StackItemFactory did not create ColorArrayItem");
    const auto row=std::get_if<std::shared_ptr<tree::ChildArray<mml::MmlNode>>>(&item->row);
    const auto table=std::get_if<std::shared_ptr<tree::ChildArray<mml::MmlNode>>>(&item->table);
    output+="] ,\"instance\":{\"kind\":"+q(item->kind())+
        ",\"isOpen\":"+std::string(item->isOpen()?"true":"false")+
        ",\"color\":{\"cell\":"+q(text(item->color.cell))+",\"row\":"+q(text(item->color.row))+",\"col\":[";
    for(std::size_t i=0;i<item->color.col.size();++i){if(i)output+=',';output+=item->color.col[i]?q(text(*item->color.col[i])):"{\"$\":\"undefined\"}";}
    output+="]},\"hasColor\":"+std::string(item->hasColor?"true":"false")+
        ",\"rowLength\":"+std::to_string(row&&*row?(*row)->size():0)+
        ",\"tableLength\":"+std::to_string(table&&*table?(*table)->size():0)+"}}";
    return output;
}
std::string configurationInfo(Fixture& fixture,Serializer& serializer){
    const auto found=fixture.runtime.configurationHandler.get(u"colortbl");
    const auto* configuration=std::get_if<tex::ConfigurationRef>(&found);
    if(!configuration||!*configuration)throw opts::Exception("TypeError",u"Colortbl configuration unavailable");
    const auto identity=fixture.runtime.configurationIdentities.find(*configuration);
    if(!identity)throw opts::Exception("TypeError",u"Colortbl configuration identity unavailable");
    const auto field=[&](std::u16string_view name){return serializer.value(fixture.cx.get(opts::Value(*identity),opts::Key(std::u16string(name))));};
    return "{\"name\":"+q(text((*configuration)->name()))+
        ",\"handler\":"+field(u"handler")+
        ",\"items\":"+field(u"items")+
        ",\"priority\":"+field(u"priority")+
        ",\"configMethod\":"+field(u"configMethod")+
        ",\"config\":"+serializer.value((*configuration)->config())+"}";
}
}
int main(){
    try{
        Fixture fixture;
        Serializer serializer{&fixture.cx,&fixture.functions,&fixture.runtime.mapHandler};
        const auto classJax=fixture.jax(baseColortbl);
        std::cout<<"{\"schema\":1,\"source\":\"MathJax-src 4.1.3 fb987178c2d279a99b0db5ea02e751691703955e\",\"files\":["
            <<"{\"ts\":\"ts/input/tex/colortbl/ColortblConfiguration.ts\",\"tsSha256\":\"0a10559b100eb9576ac5d1bcd5db4e5e288144a9de753a866bca7c87f819464c\",\"cjs\":\"cjs/input/tex/colortbl/ColortblConfiguration.js\",\"cjsSha256\":\"b2da866632dcecb74b05559fa13c0fdd3d79c9d2b4a927bb22a587ef6c76227e\"}]"
            <<",\"class\":"<<classInfo(fixture,serializer,*classJax)
            <<",\"configuration\":"<<configurationInfo(fixture,serializer)
            <<",\"maps\":"<<serializer.maps(fixture.runtime)<<",\"cases\":[";
        bool first=true;for(const auto& item:cases){if(!first)std::cout<<',';first=false;std::cout<<"{\"name\":"<<q(item.name)<<",\"input\":"<<q(text(item.input))<<",\"packages\":[";
            for(std::size_t package=0;package<item.packages.size();++package){if(package)std::cout<<',';std::cout<<q(text(item.packages[package]));}
            std::cout<<"],\"settings\":{},\"result\":";
            const auto jax=fixture.jax(item.packages);
            try{std::cout<<serializer.node(fixture.compile(item.input,*jax))<<",\"error\":null,\"packageData\":"<<serializer.packageData(*jax)<<"}";}
            catch(const tex::TexError& e){std::cout<<"null,\"error\":{\"name\":\"TexError\",\"id\":"<<q(text(e.id))<<",\"message\":"<<q(text(e.message))<<"},\"packageData\":"<<serializer.packageData(*jax)<<"}";}
            catch(const options::Exception& e){std::cout<<"null,\"error\":{\"name\":"<<q(e.name)<<",\"id\":null,\"message\":"<<q(text(e.message))<<"},\"packageData\":"<<serializer.packageData(*jax)<<"}";}
            catch(const std::exception& e){std::cout<<"null,\"error\":{\"name\":\"NativeException\",\"message\":"<<q(e.what())<<"},\"packageData\":"<<serializer.packageData(*jax)<<"}";}
            catch(...){std::cout<<"null,\"error\":{\"name\":\"UnclassifiedNativeException\"},\"packageData\":"<<serializer.packageData(*jax)<<"}";}
        }std::cout<<"]}\n";return 0;
    }catch(const options::Exception& e){std::cerr<<"fixture-init "<<e.name<<": "<<text(e.message)<<'\n';}
     catch(const std::exception& e){std::cerr<<"fixture-init NativeException: "<<e.what()<<'\n';}
     catch(...){std::cerr<<"fixture-init UnclassifiedNativeException\n";}
    return 2;
}
