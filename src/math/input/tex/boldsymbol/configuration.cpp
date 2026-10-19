// Native implementation of MathJax 4.1.3 BoldsymbolConfiguration.ts.
module folia.math.input.tex.boldsymbol.configuration;
import folia.math.input.tex.handler_types;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.node_factory;
import folia.math.input.tex.node_util;
import folia.math.input.tex.stack;
import folia.math.input.tex.tex_constants;
import folia.math.input.tex.token_map;
import folia.math.core.input_jax;
import folia.math.util.option_functions;
import folia.math.util.function_types;
import folia.math.util.property_host;
import std;

namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view name){return Key(std::u16string(name));}
Value functionLength(ParserRuntime& runtime,Value function,double length){
  if(const auto* object=std::get_if<ObjectRef>(&function.data))
    runtime.context().define(*object,key(u"length"),Descriptor::data(Value(length),false,false,true));
  return function;
}
Value array(Context& cx,std::initializer_list<Value> values){
  const ObjectRef out=cx.array(static_cast<std::uint32_t>(values.size()));
  std::uint32_t i=0;
  for(const auto& value:values){const auto n=std::to_string(i++);cx.set(Value(out),key(std::u16string(n.begin(),n.end())),value);}
  return Value(out);
}
Value object(Context& cx,std::initializer_list<std::pair<std::u16string_view,Value>> fields){
  const ObjectRef out=cx.object();for(const auto& [name,value]:fields)cx.set(Value(out),key(name),value);return Value(out);
}
Property boldVariants(){
  return Property(PropertyList{{"normal","bold"},{"italic","bold-italic"},
    {"fraktur","bold-fraktur"},{"script","bold-script"},
    {"sans-serif","bold-sans-serif"},{"-tex-calligraphic","-tex-bold-calligraphic"},
    {"-tex-oldstyle","-tex-bold-oldstyle"},{"-tex-mathit","bold-italic"}});
}
Value tokenCreator(ParserRuntime& runtime){
  auto function=std::make_shared<Function>();
  function->call=[](const FunctionReceiver&,FunctionArgs args)->std::any{
    // NodeFactory.create forwards the arguments actually supplied by the
    // caller. Source lets omitted JavaScript parameters remain undefined;
    // notably some token creators omit the text argument.
    if(args.empty())throw Exception("TypeError",u"Cannot read properties of undefined (reading 'create')");
    const auto* identity=std::any_cast<NativeObjectRef>(&args[0]);
    const Property missing=Missing{};
    const auto propertyAt=[&](std::size_t index)->const Property*{
      return index<args.size()?std::any_cast<Property>(&args[index]):&missing;
    };
    const auto* kind=propertyAt(1);
    const auto* definition=propertyAt(2);
    const auto* text=propertyAt(3);
    const auto factory=identity?std::dynamic_pointer_cast<NodeFactory>(*identity):nullptr;
    if(!factory||!kind||!definition||!text)throw Exception("TypeError",u"createBoldToken arguments are invalid");
    const mml::NodeValue token=NodeFactory::createToken(*factory,*kind,*definition,*text);
    auto* configuration=std::get_if<NodeFactoryConfiguration*>(&factory->configuration);
    if(property_host::debugText(property_host::string(*kind))!="mtext"&&configuration&&*configuration){
      auto* parseOptions=dynamic_cast<ParseOptions*>(*configuration);
      if(!parseOptions)throw Exception("TypeError",u"factory.configuration is not ParseOptions");
      const auto parserValue=parseOptions->parser();
      const auto* parser=std::get_if<std::shared_ptr<TexParser>>(&parserValue);
      if(!parser||!*parser)throw Exception("TypeError",u"Cannot read properties of null or undefined (reading 'stack')");
      const auto* stack=std::get_if<std::shared_ptr<Stack>>(&(*parser)->stack);
      if(!stack||!*stack)throw Exception("TypeError",u"Cannot read properties of null or undefined (reading 'env')");
      if(property_host::truthy(property_host::get((*stack)->env(),"boldsymbol"))){
        NodeUtil::instance().setProperty(token,"fixBold",Property(true));
        parseOptions->addNode(Property("fixBold"),token);
      }
    }
    return std::any(token);
  };
  OptionCallBinding binding;
  binding.receiver=[](Context&,Value receiver){return FunctionReceiver(std::move(receiver));};
  binding.arguments=[](Context&,std::span<const Value>)->std::vector<std::any>{
    throw Exception("Unimplemented",u"createBoldToken Context calls require the canonical NodeFactory instance binding");
  };
  binding.result=[](Context&,const std::any&)->Value{
    throw Exception("Unimplemented",u"createBoldToken Context results require the canonical MML node identity binding");
  };
  return functionLength(runtime,runtime.functions().bind(std::move(function),std::move(binding),u"createBoldToken"),4);
}
Value rewriteFilter(ParserRuntime& runtime){
  auto function=std::make_shared<Function>();
  function->sourceMetadata=FunctionSourceMetadata{u"rewriteBoldTokens",1};
  function->call=[&runtime](const FunctionReceiver&,FunctionArgs args)->std::any{
    ParseOptionsRef parse;
    if(!args.empty()){
      if(const auto* filter=std::any_cast<core::InputJaxFilterDataRef>(&args[0]);filter&&*filter){
        const auto current=(*filter)->currentData();
        if(const auto* value=std::any_cast<ParseOptionsRef>(&current))parse=*value;
      }else if(const auto* value=std::any_cast<ParseOptionsRef>(&args[0])){
        // Context FunctionList calls convert the actual record.data member
        // before entering this same Source callback.
        parse=*value;
      }
    }
    if(!parse)throw Exception("TypeError",u"rewriteBoldTokens data is not ParseOptions");
    const mml::Children nodes=parse->getList(u"fixBold");
    for(std::size_t i=0;i<nodes->size();++i){
      const mml::NodeValue node=nodes->get(i);
      if(!property_host::truthy(NodeUtil::instance().getProperty(node,"fixBold")))continue;
      const Property variant=NodeUtil::instance().getAttribute(node,"mathvariant");
      const Property replacement=property_host::get(boldVariants(),property_host::debugText(property_host::string(variant)));
      NodeUtil::instance().setAttribute(node,"mathvariant",property_host::truthy(replacement)?replacement:variant);
      const std::array<std::string,1> properties{"fixBold"};
      NodeUtil::instance().removeProperties(node,properties);
    }
    return std::any(Undefined{});
  };
  OptionCallBinding binding;
  binding.receiver=[](Context&,Value receiver){return FunctionReceiver(std::move(receiver));};
  binding.arguments=[&runtime](Context& cx,std::span<const Value> args){
    if(args.empty())throw Exception("TypeError",u"Cannot read properties of undefined (reading 'data')");
    const Value data=cx.get(args[0],key(u"data"));
    const auto* identity=std::get_if<ObjectRef>(&data.data);
    if(!identity)throw Exception("TypeError",u"rewriteBoldTokens data is not ParseOptions");
    return std::vector<std::any>{std::any(runtime.parseOptionsIdentities.resolve(*identity))};
  };
  binding.result=[](Context&,const std::any&){return Value(Undefined{});};
  return functionLength(runtime,runtime.functions().bind(std::move(function),std::move(binding),u"rewriteBoldTokens"),1);
}
} // namespace

ConfigurationRef BoldsymbolConfiguration::initializeModule(ParserRuntime& runtime){
  const auto existing=runtime.configurationHandler.get(u"boldsymbol");
  if(const auto* value=std::get_if<ConfigurationRef>(&existing);value&&*value)return *value;
  const ParseMethod boldsymbol=makeParseCallable([](const FunctionReceiver&,ParserContext& context,
      const ParseAtom& atom,ParseRest)->ParseResult{
    auto* parser=dynamic_cast<TexParser*>(&context);
    if(!parser)throw Exception("TypeError",u"boldsymbol requires a TexParser");
    std::u16string name;
    if(const auto* property=std::get_if<Property>(&atom))name=property_host::string(*property);
    else if(const auto* token=std::get_if<TokenRef>(&atom);token&&*token)name=property_host::string((*token)->token());
    else throw Exception("TypeError",u"boldsymbol name is not a token");
    const auto* stack=std::get_if<std::shared_ptr<Stack>>(&parser->stack);
    if(!stack||!*stack)throw Exception("TypeError",u"Cannot read properties of null or undefined (reading 'env')");
    Property env=(*stack)->env();
    const Property saved=property_host::get(env,"boldsymbol");
    property_host::set(env,"boldsymbol",Property(true));
    const mml::NodeValue result=parser->ParseArg(name);
    property_host::set(env,"boldsymbol",saved);
    parser->Push(result);
    return Property(Missing{});
  });
  // Keep the one Source ParseMethod identity available through the canonical
  // Context callable registry as well. TokenMap stores this same FunctionRef;
  // binding it supplies its real name/length metadata without a parallel body.
  (void)functionLength(runtime,BaseMethods::bind(runtime,std::get<FunctionRef>(boldsymbol),u"Boldsymbol"),2);
  PropertyList methods;methods.setOwn("boldsymbol",functions::toProperty(boldsymbol));
  (void)TokenMapHost::create<CommandMap>(runtime.mapHandler,Property("boldsymbol"),Property(methods));
  Context& cx=runtime.context();
  const Value handlers=object(cx,{{u"macro",array(cx,{Value(u"boldsymbol")})}});
  const Value nodes=object(cx,{{u"token",tokenCreator(runtime)}});
  const Value filters=array(cx,{rewriteFilter(runtime)});
  const Value definition=object(cx,{{u"handler",handlers},{u"nodes",nodes},{u"postprocessors",filters}});
  return Configuration::create(runtime,u"boldsymbol",definition);
}
} // namespace folia::math::tex
