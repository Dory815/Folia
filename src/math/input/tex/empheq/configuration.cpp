// Native port of the parser methods and configuration in EmpheqConfiguration.ts.
module folia.math.input.tex.empheq.configuration;
import folia.math.input.tex.empheq.util;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.base_parse_methods;
import folia.math.input.tex.stack_host;
import folia.math.input.tex.handler_types;
import folia.math.input.tex.token_map;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view value){return Key(std::u16string(value));}
Value array(Context& cx,std::initializer_list<Value> values){const ObjectRef result=cx.array(static_cast<std::uint32_t>(values.size()));std::size_t i=0;for(const auto& value:values){const std::string index=std::to_string(i++);cx.set(Value(result),key(std::u16string(index.begin(),index.end())),value);}return Value(result);}
Value object(Context& cx,std::initializer_list<std::pair<std::u16string_view,Value>> fields){const ObjectRef result=cx.object();for(const auto& [name,value]:fields)cx.set(Value(result),key(name),value);return Value(result);}
TexParser& parser(ParserContext& source){auto* result=dynamic_cast<TexParser*>(&source);if(!result)property_host::typeError("Empheq method requires a TexParser");return *result;}
Stack& stack(TexParser& parser){const auto* value=std::get_if<std::shared_ptr<Stack>>(&parser.stack);if(!value||!*value)property_host::typeError("Cannot read properties of null or undefined (reading 'env')");return **value;}
StackItemFactoryService& factory(TexParser& parser){const auto shared=parser.itemFactory();const auto* value=std::get_if<std::shared_ptr<StackItemFactory>>(&shared);if(!value||!*value)property_host::typeError("Cannot read properties of null or undefined (reading 'create')");return **value;}
std::u16string atomText(const ParseAtom& atom){if(const auto* value=std::get_if<Property>(&atom))return property_host::string(*value);if(const auto* token=std::get_if<TokenRef>(&atom);token&&*token)return property_host::string((*token)->character());property_host::typeError("Empheq token is not a string");}
std::u16string restText(ParseRest rest,std::size_t index){if(index>=rest.size())return {};if(const auto* value=std::any_cast<Property>(&rest[index]))return property_host::string(*value);if(const auto* value=std::any_cast<std::u16string>(&rest[index]))return *value;property_host::typeError("Cannot convert empheq argument to string");}
ParseResult empheq(const FunctionReceiver&,ParserContext& source,const ParseAtom& atom,ParseRest){
    auto& p=parser(source);const auto* beginRef=std::get_if<StackItemRef>(&atom);if(!beginRef||!*beginRef)property_host::typeError("Cannot read properties of null or undefined (reading 'getName')");
    BaseItem& begin=stack_host::item(StackEntry(*beginRef),"getName");
    const Property environment=stack(p).env();const Property closing=property_host::get(environment,"closing");
    if(closing==stack_property_host::scalar(begin.getName())){
        if(auto env=PropertyList::record(environment))env->erase("closing");
        auto& nestedItems=factory(p);BaseItem* nestedEnd=nestedItems.itemFactory().create("end",{});
        if(!nestedEnd)property_host::typeError("EndItem constructor is not registered");
        nestedEnd->setProperty("name",StackProperty(property_host::get(stack(p).global,"empheq")));
        p.Push(nestedItems.reference(nestedEnd));
        property_host::set(stack(p).global,"empheq",Property(""));
        const auto* config=std::get_if<ParseOptions*>(&p.configuration);if(!config||!*config)property_host::typeError("Cannot read properties of null or undefined (reading 'runtime')");
        const StackEntry top=stack(p).Top();const auto* itemRef=std::get_if<StackItemRef>(&top);if(!itemRef||!*itemRef)property_host::typeError("Empheq begin item has no Context identity");
        auto& runtime=(**config).runtime();const auto identity=runtime.stackItemIdentities.find(*itemRef);if(!identity)throw Exception("Unimplemented",u"Empheq begin item has no canonical Context identity");const std::array<Value,2> adjustArgs{Value(*identity),Value(p.instanceIdentity())};(void)EmpheqUtil::call(runtime,u"adjustTable",adjustArgs);
        auto& closingItems=factory(p);BaseItem* end=closingItems.itemFactory().create("end",{});if(!end)property_host::typeError("EndItem constructor is not registered");end->setProperty("name",StackProperty(Property("empheq")));p.Push(closingItems.reference(end));return Property(Missing{});
    }
    ParseUtil::checkEqnEnv(p);const Property optionsValue=p.GetBrackets(u"\\begin{"+property_host::string(stack_property_host::scalar(begin.getName()))+u"}");
    const std::u16string argument=property_host::string(p.GetArgument(u"\\begin{"+property_host::string(stack_property_host::scalar(begin.getName()))+u"}"));
    // Source destructures String.split(/=/) into exactly its first two items;
    // additional equals segments are ignored and an empty second item is
    // falsy when rebuilding the nested environment argument.
    const auto equal=argument.find(u'=');const std::u16string env=argument.substr(0,equal);
    std::u16string number;
    if(equal!=std::u16string::npos){const auto second=argument.find(u'=',equal+1);number=argument.substr(equal+1,second==std::u16string::npos?std::u16string::npos:second-equal-1);}
    const auto* options=std::get_if<ParseOptions*>(&p.configuration);if(!options||!*options)property_host::typeError("Cannot read properties of null or undefined (reading 'runtime')");
    if(!EmpheqUtil::checkEnv((**options).runtime(),env)){const std::array<Property,2> args{Property(property_host::debugText(env)),stack_property_host::scalar(begin.getName())};throw TexError(u"EmpheqInvalidEnv",u"Invalid environment \"%1\" for %2",args);}
    begin.setProperty("nestStart",StackProperty(Property(true)));
    if(property_host::truthy(optionsValue)){ParseOptions* config=nullptr;if(const auto* pconfig=std::get_if<ParseOptions*>(&p.configuration))config=*pconfig;if(!config)property_host::typeError("Cannot read properties of null or undefined (reading 'runtime')");const Property allowed(PropertyList{{"left",1},{"right",1}});const std::array<Value,2> splitArgs{Value(property_host::string(optionsValue)),config->runtime().properties.to(allowed)};begin.setProperties(StackProperty(config->runtime().properties.from(EmpheqUtil::call(config->runtime(),u"splitOptions",splitArgs))));}
    property_host::set(stack(p).global,"empheq",Property(property_host::debugText(env)));
    const std::u16string input=p.string();const auto offset=std::isfinite(p.i)&&p.i>0?std::min(input.size(),static_cast<std::size_t>(p.i)):std::size_t{};
    p.setString(u"\\begin{"+env+u"}"+(!number.empty()?u"{"+number+u"}":std::u16string{})+input.substr(offset));p.i=0;p.Push(*beginRef);return Property(Missing{});
}
ParseResult empheqMo(const FunctionReceiver&,ParserContext& source,const ParseAtom&,ParseRest rest){auto& p=parser(source);const auto character=restText(rest,0);const std::array<std::any,4> args{std::any(Property("mo")),std::any(Property(PropertyList{})),std::any(Property(property_host::debugText(character))),std::any(Property(Missing{}))};p.Push(p.create(u"token",args));return Property(Missing{});}
ParseResult empheqDelim(const FunctionReceiver&,ParserContext& source,const ParseAtom& atom,ParseRest){auto& p=parser(source);const auto name=atomText(atom);const Property delimiter=p.GetDelimiter(name);const std::array<std::any,4> args{std::any(Property("mo")),std::any(Property(PropertyList{{"stretchy",true},{"symmetric",true}})),std::any(delimiter),std::any(Property(Missing{}))};p.Push(p.create(u"token",args));return Property(Missing{});}
}
Property EmpheqMethods::initializeModule(ParserRuntime& runtime){
    if(!runtime.empheqMethods.missing())return runtime.empheqMethods;
    auto& cx=runtime.context();
    const auto bindMethod=[&](FunctionRef method,std::u16string_view name,double length){const Value bound=BaseMethods::bind(runtime,method,std::u16string(name));const auto* identity=std::get_if<ObjectRef>(&bound.data);if(!identity)throw Exception("TypeError",u"Empheq method has no Context identity");cx.define(*identity,key(u"length"),Descriptor::data(Value(length),false,false,true));return method;};
    const FunctionRef environment=bindMethod(makeParseCallable(empheq),u"Empheq",2);
    const FunctionRef mo=bindMethod(makeParseCallable(empheqMo),u"EmpheqMO",3);
    const FunctionRef delimiter=bindMethod(makeParseCallable(empheqDelim),u"EmpheqDelim",2);
    runtime.empheqMethods=Property(PropertyList{{"Empheq",Property(environment)},{"EmpheqMO",Property(mo)},{"EmpheqDelim",Property(delimiter)}});
    return runtime.empheqMethods;
}
ConfigurationRef EmpheqConfiguration::initializeModule(ParserRuntime& runtime){
    const auto loaded=runtime.configurationHandler.get(u"empheq");if(const auto* prior=std::get_if<ConfigurationRef>(&loaded);prior&&*prior)return *prior;
    (void)EmpheqUtil::initializeModule(runtime);
    const Property methods=EmpheqMethods::initializeModule(runtime);
    auto& cx=runtime.context();
    const Property parseMethods=ParseMethods::initializeModule(runtime);const ParseMethod environmentParser=functions::fromProperty(property_host::get(parseMethods,"environment"));
    const Property environmentMap(PropertyList{{"empheq",Property(PropertyArray{Property("Empheq"),Property("empheq")})}});
    (void)TokenMapHost::create<EnvironmentMap>(runtime.mapHandler,Property("empheq-env"),environmentParser,environmentMap,methods);
    PropertyList macros;
    constexpr std::array<std::pair<std::string_view,std::u16string_view>,32> symbols{{
        {"empheqlbrace",u"{"},{"empheqrbrace",u"}"},{"empheqlbrack",u"["},{"empheqrbrack",u"]"},{"empheqlangle",u"⟨"},{"empheqrangle",u"⟩"},{"empheqlparen",u"("},{"empheqrparen",u")"},{"empheqlvert",u"|"},{"empheqrvert",u"|"},{"empheqlVert",u"‖"},{"empheqrVert",u"‖"},{"empheqlfloor",u"⌊"},{"empheqrfloor",u"⌋"},{"empheqlceil",u"⌈"},{"empheqrceil",u"⌉"},
        {"empheqbiglbrace",u"{"},{"empheqbigrbrace",u"}"},{"empheqbiglbrack",u"["},{"empheqbigrbrack",u"]"},{"empheqbiglangle",u"⟨"},{"empheqbigrangle",u"⟩"},{"empheqbiglparen",u"("},{"empheqbigrparen",u")"},{"empheqbiglvert",u"|"},{"empheqbigrvert",u"|"},{"empheqbiglVert",u"‖"},{"empheqbigrVert",u"‖"},{"empheqbiglfloor",u"⌊"},{"empheqbigrfloor",u"⌋"},{"empheqbiglceil",u"⌈"},{"empheqbigrceil",u"⌉"}}};
    for(const auto& [name,value]:symbols)macros.setOwn(name,Property(PropertyArray{Property("EmpheqMO"),Property(property_host::debugText(value))}));
    macros.setOwn("empheql",Property("EmpheqDelim"));macros.setOwn("empheqr",Property("EmpheqDelim"));macros.setOwn("empheqbigl",Property("EmpheqDelim"));macros.setOwn("empheqbigr",Property("EmpheqDelim"));
    (void)TokenMapHost::create<CommandMap>(runtime.mapHandler,Property("empheq-macros"),Property(macros),methods);
    const Value handlers=object(cx,{{u"macro",array(cx,{Value(u"empheq-macros")})},{u"environment",array(cx,{Value(u"empheq-env")})}});
    return Configuration::create(runtime,u"empheq",object(cx,{{u"handler",handlers}}));
}
}
