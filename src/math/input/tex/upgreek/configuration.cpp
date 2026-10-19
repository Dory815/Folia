// Native port of MathJax 4.1.3 input/tex/upgreek/UpgreekConfiguration.ts.
module folia.math.input.tex.upgreek.configuration;
import folia.math.input.tex.handler_types;
import folia.math.input.tex.tex_constants;
import folia.math.input.tex.token_map;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view name){return Key(std::u16string(name));}
TexParser& parser(ParserContext& source){auto* value=dynamic_cast<TexParser*>(&source);if(!value)property_host::typeError("Upgreek requires a TexParser");return *value;}
Value array(Context& cx,std::initializer_list<Value> values){const ObjectRef out=cx.array(static_cast<std::uint32_t>(values.size()));std::uint32_t i=0;for(const auto& value:values){const std::string digits=std::to_string(i++);cx.set(Value(out),key(std::u16string(digits.begin(),digits.end())),value);}return Value(out);}
Value record(Context& cx,std::initializer_list<std::pair<std::u16string_view,Value>> values){Value out(cx.object());for(const auto& [name,value]:values)cx.set(out,key(name),value);return out;}
FunctionRef mathchar0miNormal(){
    return makeParseCallable([](const FunctionReceiver&,ParserContext& source,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& p=parser(source);const auto* token=std::get_if<TokenRef>(&atom);if(!token||!*token)property_host::typeError("Cannot read properties of null or undefined (reading 'attributes')");
        Property attributes=(*token)->attributes();
        if(!property_host::truthy(attributes))attributes=Property(PropertyList{});
        auto record=PropertyList::record(attributes);if(!record)property_host::typeError("Cannot create property 'mathvariant' on a primitive value");
        const Property variant=property_host::get(property_host::get(TexConstant,"Variant"),"NORMAL");
        record->setOwn("mathvariant",variant);
        const std::array<std::any,3> args{std::any(Property("mi")),std::any(attributes),std::any((*token)->character())};
        const auto node=p.create(u"token",FunctionArgs(args));p.Push(node);return Property(Missing{});
    });
}
Property mappings(){return Property(PropertyList{
    {"upalpha",Property("α")},{"upbeta",Property("β")},{"upgamma",Property("γ")},{"updelta",Property("δ")},{"upepsilon",Property("ϵ")},{"upzeta",Property("ζ")},{"upeta",Property("η")},{"uptheta",Property("θ")},{"upiota",Property("ι")},{"upkappa",Property("κ")},{"uplambda",Property("λ")},{"upmu",Property("μ")},{"upnu",Property("ν")},{"upxi",Property("ξ")},{"upomicron",Property("ο")},{"uppi",Property("π")},{"uprho",Property("ρ")},{"upsigma",Property("σ")},{"uptau",Property("τ")},{"upupsilon",Property("υ")},{"upphi",Property("ϕ")},{"upchi",Property("χ")},{"uppsi",Property("ψ")},{"upomega",Property("ω")},
    {"upvarepsilon",Property("ε")},{"upvartheta",Property("ϑ")},{"upvarpi",Property("ϖ")},{"upvarrho",Property("ϱ")},{"upvarsigma",Property("ς")},{"upvarphi",Property("φ")},{"Upgamma",Property("Γ")},{"Updelta",Property("Δ")},{"Uptheta",Property("Θ")},{"Uplambda",Property("Λ")},{"Upxi",Property("Ξ")},{"Uppi",Property("Π")},{"Upsigma",Property("Σ")},{"Upupsilon",Property("Υ")},{"Upphi",Property("Φ")},{"Uppsi",Property("Ψ")},{"Upomega",Property("Ω")}});}
}
ConfigurationRef UpgreekConfiguration::initializeModule(ParserRuntime& runtime){
    const auto loaded=runtime.configurationHandler.get(u"upgreek");if(const auto* prior=std::get_if<ConfigurationRef>(&loaded);prior&&*prior)return *prior;
    (void)TokenMapHost::create<CharacterMap>(runtime.mapHandler,Property("upgreek"),ParseMethod{mathchar0miNormal()},mappings());
    auto& cx=runtime.context();const Value handlers=record(cx,{{u"macro",array(cx,{Value(u"upgreek")})}});
    return Configuration::create(runtime,u"upgreek",record(cx,{{u"handler",handlers}}));
}
}
