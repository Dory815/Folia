// Native observations for the pinned MathJax Physics package corpus.
// The output includes all 63 parser cases and all 11 registered maps.
import std;
import folia.math.input.tex.parser;
import folia.math.input.tex.base_configuration;
import folia.math.input.tex.action.configuration;
import folia.math.input.tex.bbox.configuration;
import folia.math.input.tex.braket.configuration;
import folia.math.input.tex.cancel.configuration;
import folia.math.input.tex.enclose.configuration;
import folia.math.input.tex.physics.configuration;
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
        constexpr std::string_view names[]{"Physics-automatic-bracing-macros","Physics-vector-mo","Physics-vector-mi","Physics-vector-macros","Physics-expressions-macros","Physics-quick-quad-macros","Physics-derivative-macros","Physics-bra-ket-macros","Physics-matrix-macros","Physics-aux-envs","Physics-characters"};
        if(!registry)throw std::runtime_error("Physics map observer has no runtime registry");
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
    std::unordered_map<const tex::TeX*,std::shared_ptr<handlers::html::HTMLMathDocument>> documents;
    Fixture():session(std::make_unique<svg::NativeRenderSession>(fontHost())),cx(session->context()),
        functions(session->services().functions()),runtime(session->services().parser()),factory(session->services().mmlFactory()){
        (void)tex::BasePackage::initializeModule(runtime);(void)tex::PhysicsConfiguration::initializeModule(runtime);
    }
    tex::TeXRef jax(bool italicDiff,bool arrowDel,unsigned optionMask){
        const auto config=cx.object();const auto packageArray=cx.array(2);
        cx.set(packageArray,opts::Key(u"0"),opts::Value(std::u16string(u"base")));
        cx.set(packageArray,opts::Key(u"1"),opts::Value(std::u16string(u"physics")));
        cx.set(config,opts::Key(u"packages"),opts::Value(packageArray));
        if(optionMask){const auto physics=cx.object();if(optionMask&1)cx.set(physics,opts::Key(u"italicdiff"),opts::Value(italicDiff));if(optionMask&2)cx.set(physics,opts::Key(u"arrowdel"),opts::Value(arrowDel));cx.set(config,opts::Key(u"physics"),opts::Value(physics));}
        auto document=svg::createNativeSvgDocument(*session,opts::Undefined{},opts::Value(config));
        const auto inputJax=cx.get(opts::Value(document->sourceIdentity()),opts::Key(u"inputJax"));
        const auto input=cx.get(inputJax,opts::Key(u"0"));const auto* identity=std::get_if<opts::ObjectRef>(&input.data);
        if(!identity)throw opts::Exception("TypeError",u"Physics document inputJax[0] is not canonical");
        auto result=runtime.texIdentities.resolve(*identity);result->setMmlFactory(&factory);
        documents.emplace(result.get(),std::move(document));return result;
    }
    mml::NodeValue compile(std::u16string_view source,tex::TeX& jax){
        const auto found=documents.find(&jax);if(found==documents.end())throw std::runtime_error("Physics HTMLMathDocument owner is missing");
        auto& document=*found->second;auto& context=document.sourceContext();const opts::Value documentValue(document.sourceIdentity());
        const auto options=context.get(documentValue,opts::Key(u"options"));const auto constructor=context.get(options,opts::Key(u"MathItem"));
        const auto inputJax=context.get(documentValue,opts::Key(u"inputJax"));const auto input=context.get(inputJax,opts::Key(u"0"));
        const std::array<opts::Value,3> args{opts::Value(std::u16string(source)),input,opts::Value(true)};
        const auto* itemConstructor=std::get_if<opts::ObjectRef>(&constructor.data);if(!itemConstructor)throw opts::Exception("TypeError",u"HTMLMathDocument.options.MathItem is not a constructor");
        const auto identity=context.construct(*itemConstructor,args);const auto* payload=std::any_cast<std::shared_ptr<NativeObject>>(&context.payload(identity));
        auto item=payload?std::dynamic_pointer_cast<core::MathItem>(*payload):nullptr;if(!item)throw opts::Exception("TypeError",u"Physics MathItem has no canonical HTML owner");
        return jax.compile(*item,core::MathDocumentReference{&document});
    }
};
struct Case{const char* name;std::u16string_view input;bool italicDiff;bool arrowDel;unsigned optionMask;};
constexpr Case cases[]={
 {"physics-quantity-round",uR"phys(\qty(x))phys",false,false,0},
 {"physics-quantity-braced",uR"phys(\qty{x})phys",false,false,0},
 {"physics-quantity-star",uR"phys(\qty*{x})phys",false,false,0},
 {"physics-quantity-big",uR"phys(\qty\big(x))phys",false,false,0},
 {"physics-quantity-bad-big",uR"phys(\qty\foo(x))phys",false,false,0},
 {"physics-quantity-nested-open",uR"phys(\qty(\qty[x]))phys",false,false,0},
 {"physics-pqty",uR"phys(\pqty{x})phys",false,false,0},
 {"physics-Bqty",uR"phys(\Bqty{x})phys",false,false,0},
 {"physics-norm-order",uR"phys(\order{x})phys",false,false,0},
 {"physics-eval-parentheses",uR"phys(\eval(x))phys",false,false,0},
 {"physics-eval-braced-star",uR"phys(\eval*{x})phys",false,false,0},
 {"physics-commutator",uR"phys(\comm{A}{B})phys",false,false,0},
 {"physics-commutator-star-big",uR"phys(\comm*\Big[A][B])phys",false,false,0},
 {"physics-vector-bold",uR"phys(\vb{abc})phys",false,false,0},
 {"physics-vector-bold-star",uR"phys(\vb*{a\alpha 1})phys",false,false,0},
 {"physics-vector-arrow",uR"phys(\va{v})phys",false,false,0},
 {"physics-gradient-brace",uR"phys(\gradient{f})phys",false,false,0},
 {"physics-curl-round",uR"phys(\curl(v))phys",false,false,0},
 {"physics-laplacian-square",uR"phys(\laplacian[f])phys",false,false,0},
 {"physics-expression-power",uR"phys(\sin^2[x](x))phys",false,false,0},
 {"physics-expression-no-power",uR"phys(\det(A))phys",false,false,0},
 {"physics-named-function",uR"phys(\sine{x})phys",false,false,0},
 {"physics-operator-app-no-fence",uR"phys(\grad x)phys",false,false,0},
 {"physics-qq-star",uR"phys(\qq*{if})phys",false,false,0},
 {"physics-qword",uR"phys(x\qgiven y)phys",false,false,0},
 {"physics-differential-no-argument",uR"phys(\dd)phys",false,false,0},
 {"physics-differential-brace",uR"phys(\dd{x})phys",false,false,0},
 {"physics-differential-round",uR"phys(\dd(x))phys",false,false,0},
 {"physics-diffd-default",uR"phys(\diffd x)phys",false,false,0},
 {"physics-diffd-italic",uR"phys(\diffd x)phys",true,false,1},
 {"physics-vnabla-default",uR"phys(\vnabla)phys",false,false,0},
 {"physics-vnabla-arrowdel",uR"phys(\vnabla)phys",false,true,2},
 {"physics-derivative-one-arg",uR"phys(\dv{x})phys",false,false,0},
 {"physics-derivative-two-args",uR"phys(\dv{f}{x})phys",false,false,0},
 {"physics-derivative-partial-three",uR"phys(\pdv{f}{x}{y})phys",false,false,0},
 {"physics-derivative-star",uR"phys(\dv*{f}{x}(x))phys",false,false,0},
 {"physics-bra-ket-pair",uR"phys(\bra{a}\ket{b})phys",false,false,0},
 {"physics-bra-with-ket-command",uR"phys(\bra{a}\ket{b})phys",false,false,0},
 {"physics-bra-only",uR"phys(\Bra{a})phys",false,false,0},
 {"physics-innerproduct",uR"phys(\ip{a}{b})phys",false,false,0},
 {"physics-ketbra-implicit",uR"phys(\dyad{a})phys",false,false,0},
 {"physics-expectation",uR"phys(\ev{A})phys",false,false,0},
 {"physics-expectation-state",uR"phys(\ev{A}{\psi})phys",false,false,0},
 {"physics-matrix-element",uR"phys(\mel{a}{A}{b})phys",false,false,0},
 {"physics-matrix-quantity-braces",uR"phys(\mqty{a&b\\c&d})phys",false,false,0},
 {"physics-matrix-quantity-parentheses",uR"phys(\mqty(a&b\\c&d))phys",false,false,0},
 {"physics-matrix-quantity-star",uR"phys(\mqty*(a&b))phys",false,false,0},
 {"physics-small-matrix",uR"phys(\smqty{a&b\\c&d})phys",false,false,0},
 {"physics-identity-matrix",uR"phys(\imat{3})phys",false,false,0},
 {"physics-identity-matrix-invalid",uR"phys(\imat{03})phys",false,false,0},
 {"physics-xmatrix-scalar",uR"phys(\xmat{a}{1}{1})phys",false,false,0},
 {"physics-xmatrix-indexed",uR"phys(\xmat*{a}{2}{3})phys",false,false,0},
 {"physics-xmatrix-invalid-dimension",uR"phys(\xmat{a}{2}{03})phys",false,false,0},
 {"physics-pauli-x",uR"phys(\pmat{x})phys",false,false,0},
 {"physics-pauli-unknown",uR"phys(\pmat{q})phys",false,false,0},
 {"physics-diagonal",uR"phys(\dmat{a,b,c})phys",false,false,0},
 {"physics-antidiagonal",uR"phys(\admat{a,b,c})phys",false,false,0},
 {"physics-smallmatrix-environment",uR"phys(\begin{smallmatrix}a&b\\c&d\end{smallmatrix})phys",false,false,0},
 {"physics-auto-close-paren",uR"phys(\qty(x))phys",false,false,0},
 {"physics-auto-close-nested",uR"phys(\qty(\qty[x]))phys",false,false,0},
 {"physics-auto-close-extra",uR"phys(\qty(x)))phys",false,false,0},
 {"physics-auto-close-missing",uR"phys(\qty(x)phys",false,false,0},
 {"physics-unknown-command",uR"phys(\physicsUnknown{x})phys",false,false,0},
};
}
int main(){
    try{
        Fixture fixture;Serializer serializer{&fixture.cx,&fixture.functions,&fixture.runtime.mapHandler};
        std::cout<<"{\"schema\":1,\"source\":\"MathJax-src 4.1.3 fb987178c2d279a99b0db5ea02e751691703955e\",\"files\":["
          <<"{\"ts\":\"ts/input/tex/physics/PhysicsConfiguration.ts\",\"tsSha256\":\"5fcdc5a67af1e7103c819786e00f5648f0478e33900ebc617ddecac3bf073cd9\",\"cjs\":\"cjs/input/tex/physics/PhysicsConfiguration.js\",\"cjsSha256\":\"119e700d70e7f6fed4854af927e93d8d997fa42d0135f308b8a3ef7bfdc18d12\"},"
          <<"{\"ts\":\"ts/input/tex/physics/PhysicsItems.ts\",\"tsSha256\":\"6065e6b2c9c647e86acd50bbf3cca186c4ef545754b80bbd904e659478f1e993\",\"cjs\":\"cjs/input/tex/physics/PhysicsItems.js\",\"cjsSha256\":\"84e9128b2005c18654844902db643a2c351da6fad22b8475f3fda5cdfed2ba64\"},"
          <<"{\"ts\":\"ts/input/tex/physics/PhysicsMappings.ts\",\"tsSha256\":\"bbd198b470045f74ada6987480cb662cd1fe4911efb5830fd43c65b5a4682304\",\"cjs\":\"cjs/input/tex/physics/PhysicsMappings.js\",\"cjsSha256\":\"959950867884100b825474824f34051298e9fe6167d7087ae653dc8470e489ad\"},"
          <<"{\"ts\":\"ts/input/tex/physics/PhysicsMethods.ts\",\"tsSha256\":\"b86d2d3a84c553581aec08ebb46fc50fc6b32f9322dd67e3d00b493ef8acef22\",\"cjs\":\"cjs/input/tex/physics/PhysicsMethods.js\",\"cjsSha256\":\"ddf56512e6f2c45e3d271dc702cc77dd5fc42fdd85ee137165f930cb332f74fa\"}],\"cases\":[";
        bool first=true;
        for(const auto& test:cases){if(!first)std::cout<<',';first=false;const auto jax=fixture.jax(test.italicDiff,test.arrowDel,test.optionMask);
            std::cout<<"{\"name\":"<<q(test.name)<<",\"input\":"<<q(text(test.input))<<",\"options\":";
            if(test.optionMask){std::cout<<"{\"physics\":{";bool firstOption=true;if(test.optionMask&1){std::cout<<"\"italicdiff\":"<<(test.italicDiff?"true":"false");firstOption=false;}if(test.optionMask&2){if(!firstOption)std::cout<<',';std::cout<<"\"arrowdel\":"<<(test.arrowDel?"true":"false");}std::cout<<"}}";}else std::cout<<"{}";
            std::cout<<",\"result\":";
            try{std::cout<<serializer.node(fixture.compile(test.input,*jax))<<",\"error\":null";}
            catch(const tex::TexError& e){std::cout<<"null,\"error\":{\"name\":\"TexError\",\"id\":"<<q(text(e.id))<<",\"message\":"<<q(text(e.message))<<"}";}
            catch(const opts::Exception& e){std::cout<<"null,\"error\":{\"name\":"<<q(e.name)<<",\"id\":null,\"message\":"<<q(text(e.message))<<"}";}
            catch(const std::exception& e){std::cout<<"null,\"error\":{\"name\":\"NativeException\",\"id\":null,\"message\":"<<q(e.what())<<"}";}
            catch(...){std::cout<<"null,\"error\":{\"name\":\"UnclassifiedNativeException\",\"id\":null,\"message\":\"\"}";}
            std::cout<<",\"packageData\":"<<serializer.packageData(*jax)<<"}";
        }
        std::cout<<"],\"maps\":"<<serializer.maps(fixture.runtime)<<"}\n";return 0;
    }catch(const opts::Exception& e){std::cerr<<"fixture-init "<<e.name<<": "<<text(e.message)<<'\n';}
     catch(const std::exception& e){std::cerr<<"fixture-init NativeException: "<<e.what()<<'\n';}
     catch(...){std::cerr<<"fixture-init UnclassifiedNativeException\n";}
    return 2;
}
