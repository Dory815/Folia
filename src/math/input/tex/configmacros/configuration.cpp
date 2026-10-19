// Native implementation of MathJax 4.1.3 ConfigMacrosConfiguration.ts.
module folia.math.input.tex.configmacros.configuration;
import folia.math.input.tex.handler_types;
import folia.math.input.tex.macro;
import folia.math.input.tex.newcommand.items;
import folia.math.input.tex.newcommand.methods;
import folia.math.input.tex.base_parse_methods;
import folia.math.input.tex.parser;
import folia.math.input.tex.token_map;
import folia.math.util.options;
import folia.math.util.property_host;
import std;

namespace folia::math::tex {
namespace {
using namespace options;
constexpr std::u16string_view MACROSMAP=u"configmacros-map";
constexpr std::u16string_view ACTIVEMAP=u"configmacros-active-map";
constexpr std::u16string_view ENVIRONMENTMAP=u"configmacros-env-map";
Key key(std::u16string_view name){return Key(std::u16string(name));}
Value string(std::u16string_view value){return Value(std::u16string(value));}
Value functionLength(ParserRuntime& runtime,Value function,double length){
  if(const auto* object=std::get_if<ObjectRef>(&function.data))
    runtime.context().define(*object,key(u"length"),Descriptor::data(Value(length),false,false,true));
  return function;
}
std::u16string textKey(const Key& value){
  if(const auto* text=std::get_if<std::u16string>(&value.data))return *text;
  property_host::typeError("ConfigMacros definitions require string keys");
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
ParseMethod sourceMethod(ParserRuntime& runtime,std::u16string_view name){
  const Property value=NewcommandMethods::get(runtime,name);
  if(const auto* method=std::get_if<FunctionRef>(&value.value);method&&*method)return *method;
  throw Exception("Unimplemented",u"ConfigMacros canonical Newcommand method is unavailable");
}
ParseOptions& parseOptions(TeX& jax){
  const auto shared=jax.parseOptions();
  const auto* parse=std::get_if<std::shared_ptr<ParseOptions>>(&shared);
  if(!parse||!*parse)property_host::typeError("Cannot read properties of null or undefined (reading 'handlers')");
  return **parse;
}
std::vector<Key> objectKeys(Context& cx,const Value& objectValue){
  std::vector<Key> result;
  if(std::holds_alternative<Null>(objectValue.data)||std::holds_alternative<Undefined>(objectValue.data))
    property_host::typeError("Cannot convert undefined or null to object");
  for(const Key& name:cx.ownKeys(objectValue)){
    if(!std::holds_alternative<std::u16string>(name.data))continue;
    const auto descriptor=cx.descriptor(objectValue,name);
    if(descriptor&&descriptor->enumerable)result.push_back(name);
  }
  return result;
}
Property property(ParserRuntime& runtime,const Value& value){return runtime.properties.from(value);}
void setMacros(ParserRuntime& runtime,std::u16string_view optionName,
               std::u16string_view mapName,TeX& jax){
  Context& cx=runtime.context();
  ParseOptions& options=parseOptions(jax);
  const auto handlers=std::get_if<std::shared_ptr<SubHandlers>>(&options.handlers);
  if(!handlers||!*handlers)property_host::typeError("Cannot read properties of null or undefined (reading 'retrieve')");
  const auto map=(*handlers)->retrieve(Property(property_host::debugText(mapName)));
  auto handler=std::dynamic_pointer_cast<MacroMap>(map);
  if(!handler)property_host::typeError("Cannot read properties of undefined (reading 'add')");
  const Value definitions=cx.get(options.options,key(optionName));
  for(const Key& name:objectKeys(cx,definitions)){
    // The Source conditional reads macros[cs] once for typeof and again in
    // either branch. Keep both Gets observable for accessors/proxies.
    const Value typeProbe=cx.get(definitions,name);
    const Value definition=cx.get(definitions,name);
    Value args=definition;
    if(std::holds_alternative<std::u16string>(typeProbe.data))args=array(cx,{definition});
    ParseMethod callback;
    Value third=cx.get(args,key(u"2"));
    const auto* definitionArray=std::get_if<ObjectRef>(&args.data);
    const auto* templateArray=std::get_if<ObjectRef>(&third.data);
    if(definitionArray&&cx.kind(*definitionArray)==ObjectKind::array&&templateArray&&
       cx.kind(*templateArray)==ObjectKind::array){
      // The Source constructor arguments are evaluated left to right: the
      // static callback is captured before def.slice/concat are read or run.
      callback=sourceMethod(runtime,u"MacroWithTemplate");
      const Value slice=cx.get(args,key(u"slice"));
      const auto* sliceFunction=std::get_if<ObjectRef>(&slice.data);
      if(!sliceFunction||cx.kind(*sliceFunction)!=ObjectKind::function)
        property_host::typeError("def.slice is not a function");
      const std::array<Value,2> sliceArgs{Value(0),Value(2)};
      const Value prefix=cx.call(*sliceFunction,args,sliceArgs);
      const Value concat=cx.get(prefix,key(u"concat"));
      const auto* concatFunction=std::get_if<ObjectRef>(&concat.data);
      if(!concatFunction||cx.kind(*concatFunction)!=ObjectKind::function)
        property_host::typeError("def.slice(...).concat is not a function");
      const Value templateValue=cx.get(args,key(u"2"));
      const std::array<Value,1> concatArgs{templateValue};
      args=cx.call(*concatFunction,prefix,concatArgs);
    }else callback=sourceMethod(runtime,u"Macro");
    const std::u16string macroName=textKey(name);
    auto macro=std::make_shared<Macro>(property_host::debugText(macroName),
                                       std::move(callback),property(runtime,args));
    handler->add(Property(property_host::debugText(macroName)),
                 TokenLookup{std::move(macro)});
  }
}
void configActives(ParserRuntime& runtime,TeX& jax){setMacros(runtime,u"active",ACTIVEMAP,jax);}
void configMacros(ParserRuntime& runtime,TeX& jax){setMacros(runtime,u"macros",MACROSMAP,jax);}
void configEnvironments(ParserRuntime& runtime,TeX& jax){
  Context& cx=runtime.context();ParseOptions& options=parseOptions(jax);
  const auto handlers=std::get_if<std::shared_ptr<SubHandlers>>(&options.handlers);
  if(!handlers||!*handlers)property_host::typeError("Cannot read properties of null or undefined (reading 'retrieve')");
  const auto map=(*handlers)->retrieve(Property(property_host::debugText(ENVIRONMENTMAP)));
  auto handler=std::dynamic_pointer_cast<EnvironmentMap>(map);
  if(!handler)property_host::typeError("Cannot read properties of undefined (reading 'add')");
  const Value environments=cx.get(options.options,key(u"environments"));
  for(const Key& name:objectKeys(cx,environments)){
    const std::u16string environment=textKey(name);
    // Preserve `new Macro(env, NewcommandMethods.BeginEnv, ...)` argument
    // evaluation: resolve BeginEnv before creating/evaluating concat args.
    const ParseMethod begin=sourceMethod(runtime,u"BeginEnv");
    const Value args=array(cx,{Value(true)});
    const Value concat=cx.get(args,key(u"concat"));
    const auto* function=std::get_if<ObjectRef>(&concat.data);
    if(!function||cx.kind(*function)!=ObjectKind::function)property_host::typeError("args.concat is not a function");
    const Value value=cx.get(environments,name);
    const std::array<Value,1> concatArgs{value};
    const Value actualArgs=cx.call(*function,args,concatArgs);
    handler->add(Property(property_host::debugText(environment)),
      TokenLookup{std::make_shared<Macro>(property_host::debugText(environment),begin,property(runtime,actualArgs))});
  }
}
Value configFunction(ParserRuntime& runtime){
  auto body=std::make_shared<Function>();
  body->sourceMetadata=FunctionSourceMetadata{u"configmacrosConfig",2};
  body->call=[&runtime](const FunctionReceiver&,FunctionArgs args)->std::any{
    if(args.size()<2)throw Exception("TypeError",u"ConfigMacros config requires configuration and TeX");
    const auto* config=std::any_cast<ParserConfigurationRef>(&args[0]);
    const auto* jax=std::any_cast<TeX*>(&args[1]);
    if(!config||!*config||!jax||!*jax)throw Exception("TypeError",u"ConfigMacros config arguments are invalid");
    configActives(runtime,**jax);configMacros(runtime,**jax);configEnvironments(runtime,**jax);
    return std::any(Undefined{});
  };
  OptionCallBinding binding;
  binding.receiver=[](Context&,Value receiver){return FunctionReceiver(std::move(receiver));};
  binding.arguments=[&runtime](Context&,std::span<const Value> args){
    if(args.size()<2)throw Exception("TypeError",u"ConfigMacros config requires (config, jax)");
    const auto* config=std::get_if<ObjectRef>(&args[0].data);const auto* jax=std::get_if<ObjectRef>(&args[1].data);
    if(!config||!jax)throw Exception("TypeError",u"ConfigMacros config identities are invalid");
    return std::vector<std::any>{std::any(runtime.parserConfigurationIdentities.resolve(*config)),
                                 std::any(runtime.texIdentities.resolve(*jax).get())};
  };
  binding.result=[](Context&,const std::any&){return Value(Undefined{});};
  return functionLength(runtime,runtime.functions().bind(std::move(body),std::move(binding),u"ConfigMacrosConfig"),2);
}
Value initFunction(ParserRuntime& runtime){
  auto body=std::make_shared<Function>();
  body->sourceMetadata=FunctionSourceMetadata{u"configmacrosInit",1};
  body->call=[&runtime](const FunctionReceiver&,FunctionArgs args)->std::any{
    if(args.empty())throw Exception("TypeError",u"ConfigMacros init requires ParserConfiguration");
    const auto* configuration=std::any_cast<ParserConfigurationRef>(&args[0]);
    if(!configuration||!*configuration)throw Exception("TypeError",u"ConfigMacros init argument is invalid");
    Context& cx=runtime.context();
    const ParseMethod environment=functions::fromProperty(property_host::get(runtime.parseMethods,property_host::debugText(u"environment")));
    (void)TokenMapHost::create<MacroMap>(runtime.mapHandler,Property(property_host::debugText(ACTIVEMAP)),Property(PropertyList{}));
    (void)TokenMapHost::create<CommandMap>(runtime.mapHandler,Property(property_host::debugText(MACROSMAP)),Property(PropertyList{}));
    (void)TokenMapHost::create<EnvironmentMap>(runtime.mapHandler,Property(property_host::debugText(ENVIRONMENTMAP)),environment,Property(PropertyList{}));
    const Value config=object(cx,{{u"handler",object(cx,{
      {u"character",array(cx,{string(ACTIVEMAP)})},
      {u"macro",array(cx,{string(MACROSMAP)})},
      {u"environment",array(cx,{string(ENVIRONMENTMAP)})}})}, {u"priority",Value(3)}});
    const ConfigurationRef local=Configuration::local(runtime,config);
    // Source calls the public ParserConfiguration.append. The native API is
    // currently protected; Root owns exposing that exact existing method.
    (*configuration)->append(local);
    return std::any(Undefined{});
  };
  OptionCallBinding binding;
  binding.receiver=[](Context&,Value receiver){return FunctionReceiver(std::move(receiver));};
  binding.arguments=[&runtime](Context&,std::span<const Value> args){
    if(args.empty())throw Exception("TypeError",u"ConfigMacros init requires ParserConfiguration");
    const auto* identity=std::get_if<ObjectRef>(&args[0].data);
    if(!identity)throw Exception("TypeError",u"ConfigMacros init identity is invalid");
    return std::vector<std::any>{std::any(runtime.parserConfigurationIdentities.resolve(*identity))};
  };
  binding.result=[](Context&,const std::any&){return Value(Undefined{});};
  return functionLength(runtime,runtime.functions().bind(std::move(body),std::move(binding),u"ConfigMacrosInit"),1);
}
} // namespace

ConfigurationRef ConfigMacrosConfiguration::initializeModule(ParserRuntime& runtime){
  // ConfigMacros imports these Source modules directly; their methods/items
  // exist independently of whether the Newcommand package is selected.
  NewcommandItems::initializeModule(runtime);
  (void)NewcommandMethods::initializeModule(runtime);
  const auto loaded=runtime.configurationHandler.get(u"configmacros");
  if(const auto* existing=std::get_if<ConfigurationRef>(&loaded);existing&&*existing)return *existing;
  Context& cx=runtime.context();
  const Value newcommandItems=runtime.newcommandItemsModule;
  const Value beginEnv=cx.get(newcommandItems,key(u"BeginEnvItem"));
  const Value beginEnvPrototype=cx.get(beginEnv,key(u"prototype"));
  const Value beginEnvKind=cx.get(beginEnvPrototype,key(u"kind"));
  const auto* itemKey=std::get_if<std::u16string>(&beginEnvKind.data);
  if(!itemKey)property_host::typeError("BeginEnvItem.prototype.kind is not a string");
  const Value items=object(cx,{{*itemKey,beginEnv}});
  const Value options=object(cx,{{u"active",expandable(cx,Value(cx.object()))},
      {u"macros",expandable(cx,Value(cx.object()))},
      {u"environments",expandable(cx,Value(cx.object()))}});
  const Value definition=object(cx,{{u"init",initFunction(runtime)},
      {u"config",configFunction(runtime)},{u"items",items},{u"options",options}});
  return Configuration::create(runtime,u"configmacros",definition);
}
} // namespace folia::math::tex
