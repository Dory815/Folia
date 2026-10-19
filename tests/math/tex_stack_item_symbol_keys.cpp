#include "support/folia_test.hpp"
import folia.math.output.common;
import folia.math.output.svg.render_session;
import folia.math.input.tex.parser;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.tex_parser_context;
import folia.math.util.property_host;
import folia.math.util.native_identities;

namespace {
using namespace boost::ut;
namespace o=folia::math::options;
namespace t=folia::math::tex;
namespace common=folia::math::output::common;
namespace svg=folia::math::output::svg;

common::FontLoadHost fontHost(){
  common::FontLoadHost host;
  host.asyncIsSynchronous=[] {return true;};
  host.warn=[](const std::any&) {};
  return host;
}
t::TeXRef testTeX(svg::NativeRenderSession& session,t::ParserRuntime& runtime){
  auto jax=t::TeX::make(runtime);
  jax->setMmlFactory(folia::math::core::InputMmlFactoryReference{session.services().mmlFactoryOwner()});
  return jax;
}
o::Value startItem(o::Context& context,t::TeXRef jax){
  const auto parsed=jax->parseOptions();
  const auto* options=std::get_if<t::ParseOptionsRef>(&parsed);
  if(!options||!*options)return o::Value(o::Undefined{});
  const auto parser=t::TexParser::make(u"",folia::math::PropertyList{},**options);
  const o::Value stack=context.get(o::Value(parser->instanceIdentity()),o::Key(u"stack"));
  const o::Value entries=context.get(stack,o::Key(u"stack"));
  return context.get(entries,o::Key(u"0"));
}

suite stack_property_symbol_keys=[] {
  "stack_property_record_uses_ordinary_symbol_key_operations"_test=[] {
    svg::NativeRenderSession session(fontHost());
    auto& runtime=session.services().parser();auto& context=session.context();
    const auto item=startItem(context,testTeX(session,runtime));
    expect(std::holds_alternative<o::ObjectRef>(item.data));
    if(!std::holds_alternative<o::ObjectRef>(item.data))return;
    const auto properties=context.get(item,o::Key(u"_properties"));
    expect(std::holds_alternative<o::ObjectRef>(properties.data));
    if(!std::holds_alternative<o::ObjectRef>(properties.data))return;
    const auto objectPrototype=context.intrinsics().objectPrototype;
    const auto inheritedToString=context.get(o::Value(objectPrototype),o::Key(u"toString"));
    expect(context.callMember(inheritedToString,properties)==o::Value(u"[object Object]"));
    const auto symbolA=context.symbol(u"alpha"),symbolB=context.symbol(u"beta"),symbolC=context.symbol(u"gamma"),absent=context.symbol(u"absent");
    expect(context.get(properties,o::Key(absent))==o::Value(o::Undefined{}));
    expect(!context.hasOwn(properties,o::Key(absent)));
    t::StackPropertyList nativeOnly;
    nativeOnly.setOwn(o::Key(symbolA),t::StackProperty(1.0));
    t::StackPropertyStorage::Attributes readOnly;
    readOnly.key=o::Key(symbolA);readOnly.writable=false;
    nativeOnly.storage()->attributes.push_back(readOnly);
    bool readOnlyRejected=false;
    try{nativeOnly.set(o::Key(symbolA),t::StackProperty(2.0));}catch(const std::exception&){readOnlyRejected=true;}
    expect(readOnlyRejected);
    t::StackPropertyList detachedString;
    detachedString.setOwn("locked",t::StackProperty(1.0));
    t::StackPropertyStorage::Attributes locked;
    locked.key=o::Key(u"locked");locked.writable=false;
    detachedString.storage()->attributes.push_back(locked);
    bool detachedStringReadOnlyRejected=false;
    try{detachedString.set(o::Key(u"locked"),t::StackProperty(2.0));}catch(const std::exception&){detachedStringReadOnlyRejected=true;}
    expect(detachedStringReadOnlyRejected);

    const auto getProperty=context.get(item,o::Key(u"getProperty"));
    const auto setProperty=context.get(item,o::Key(u"setProperty"));
    const auto setProperties=context.get(item,o::Key(u"setProperties"));
    const std::array<o::Value,2> setSymbolArgs{o::Value(symbolA),o::Value(17.0)};
    expect(context.callMember(setProperty,item,setSymbolArgs)==item);
    const std::array<o::Value,1> getSymbolArgs{o::Value(symbolA)};
    expect(context.callMember(getProperty,item,getSymbolArgs)==o::Value(17.0));

    std::vector<std::string> evaluationOrder;
    const auto propertiesGetter=context.function(u"get _properties",[&](o::Context&,o::Value,std::span<const o::Value>){
      evaluationOrder.push_back("properties");return properties;
    });
    context.define(std::get<o::ObjectRef>(item.data),o::Key(u"_properties"),o::Descriptor::access(propertiesGetter,std::nullopt,true,true));
    const auto keyObject=context.object();
    const auto toPropertyKey=context.function(u"toPropertyKey",[&](o::Context&,o::Value,std::span<const o::Value>){
      evaluationOrder.push_back("key");return o::Value(symbolA);
    });
    context.define(keyObject,o::Key(context.intrinsics().symbolToPrimitive),o::Descriptor::data(o::Value(toPropertyKey),true,true,true));
    const std::array<o::Value,1> convertedGetArgs{o::Value(keyObject)};
    expect(context.callMember(getProperty,item,convertedGetArgs)==o::Value(17.0));
    expect(evaluationOrder==std::vector<std::string>{"properties","key"});

    context.set(properties,o::Key(u"z"),o::Value(u"z-value"));
    context.set(properties,o::Key(u"2"),o::Value(u"two"));
    context.set(properties,o::Key(u"1"),o::Value(u"one"));
    std::vector<std::string> typedReadOrder;
    const auto stringGetter=context.function(u"get z",[&](o::Context&,o::Value,std::span<const o::Value>){
      typedReadOrder.push_back("z");return o::Value(u"z-value");
    });
    context.define(std::get<o::ObjectRef>(properties.data),o::Key(u"z"),o::Descriptor::access(stringGetter,std::nullopt,true,true));
    context.define(std::get<o::ObjectRef>(properties.data),o::Key(symbolA),o::Descriptor::data(o::Value(17.0),true,true,true));
    int nonEnumerableGetterCalls=0;
    const auto nonEnumerableGetter=context.function(u"get beta",[&](o::Context&,o::Value,std::span<const o::Value>){
      ++nonEnumerableGetterCalls;return o::Value(23.0);
    });
    context.define(std::get<o::ObjectRef>(properties.data),o::Key(symbolB),o::Descriptor::access(nonEnumerableGetter,std::nullopt,false,true));
    const auto symbolGetter=context.function(u"get gamma",[&](o::Context&,o::Value,std::span<const o::Value>){
      typedReadOrder.push_back("gamma");return o::Value(31.0);
    });
    context.define(std::get<o::ObjectRef>(properties.data),o::Key(symbolC),o::Descriptor::access(symbolGetter,std::nullopt,true,true));
    int symbolSetterCalls=0;double symbolSetterValue=0;
    const auto symbolSetter=context.function(u"set delta",[&](o::Context&,o::Value,std::span<const o::Value> args){
      ++symbolSetterCalls;symbolSetterValue=std::get<double>(args[0].data);return o::Value(o::Undefined{});
    });
    const auto symbolD=context.symbol(u"delta");
    context.define(std::get<o::ObjectRef>(properties.data),o::Key(symbolD),o::Descriptor::access(std::nullopt,symbolSetter,true,true));
    const std::array<o::Value,2> setAccessorArgs{o::Value(symbolD),o::Value(37.0)};
    expect(context.callMember(setProperty,item,setAccessorArgs)==item);
    expect(symbolSetterCalls==1&&symbolSetterValue==37.0);

    const auto propertiesObject=std::get<o::ObjectRef>(properties.data);
    auto* propertyOwner=runtime.stackPropertyIdentities.resolve(propertiesObject);
    const auto* propertyBinding=std::any_cast<folia::math::NativeBindingPayload<t::StackPropertyStorage*>>(&context.payload(propertiesObject));
    expect(propertyBinding!=nullptr);
    if(propertyBinding){
      auto propertyStorage=std::shared_ptr<t::StackPropertyStorage>(propertyBinding->lifetime,propertyOwner);
      auto nativeProperties=t::StackPropertyList::fromStorage(std::move(propertyStorage));
      nativeProperties.set(o::Key(symbolD),t::StackProperty(39.0));
      typedReadOrder.clear();
      nonEnumerableGetterCalls=0;
      t::StackPropertyList typedAssigned;
      folia::math::tex::stack_property_host::assign(typedAssigned,t::StackProperty(nativeProperties));
      expect(typedReadOrder==std::vector<std::string>{"z","gamma"});
      expect(nonEnumerableGetterCalls==0);
      expect(typedAssigned.get(o::Key(u"z"))==t::StackProperty(std::string("z-value")));
      expect(typedAssigned.get(o::Key(symbolC))==t::StackProperty(31.0));
      expect(!typedAssigned.hasOwn(o::Key(symbolB)));
    }
    expect(symbolSetterCalls==2&&symbolSetterValue==39.0);

    const auto assignedSource=context.object();
    context.define(assignedSource,o::Key(symbolA),o::Descriptor::data(o::Value(41.0),true,true,true));
    context.define(assignedSource,o::Key(symbolB),o::Descriptor::data(o::Value(43.0),true,false,true));
    context.define(assignedSource,o::Key(symbolD),o::Descriptor::data(o::Value(47.0),true,true,true));
    const std::array<o::Value,1> assignArgs{o::Value(assignedSource)};
    expect(context.callMember(setProperties,item,assignArgs)==item);
    expect(context.get(properties,o::Key(symbolA))==o::Value(41.0));
    expect(context.get(properties,o::Key(symbolB))==o::Value(23.0));
    expect(symbolSetterCalls==3&&symbolSetterValue==47.0);

    expect(context.get(properties,o::Key(symbolA))==o::Value(41.0));
    expect(context.get(properties,o::Key(symbolC))==o::Value(31.0));
    expect(context.descriptor(properties,o::Key(symbolC))->accessor);
    expect(context.descriptor(properties,o::Key(symbolA))->enumerable);
    expect(!context.descriptor(properties,o::Key(symbolB))->enumerable);
    const auto keys=context.ownKeys(properties);
    expect(keys.size()==7u);
    if(keys.size()==7u){
      expect(keys[0]==o::Key(u"1"));expect(keys[1]==o::Key(u"2"));expect(keys[2]==o::Key(u"z"));
      expect(keys[3]==o::Key(symbolA));expect(keys[4]==o::Key(symbolB));expect(keys[5]==o::Key(symbolC));expect(keys[6]==o::Key(symbolD));
    }
    const auto ownSymbols=context.ownSymbols(properties);
    expect(ownSymbols.size()==4u);
    if(ownSymbols.size()==4u){expect(ownSymbols[0]==symbolA);expect(ownSymbols[1]==symbolB);expect(ownSymbols[2]==symbolC);expect(ownSymbols[3]==symbolD);}

    // Object.assign copies the enumerable own string and Symbol keys through
    // the same Context operations used by Source property assignment.
    const auto target=context.object();
    for(const auto& key:context.ownKeys(properties)){
      const auto descriptor=context.descriptor(properties,key);
      if(descriptor&&descriptor->enumerable)context.set(o::Value(target),key,context.get(properties,key));
    }
    expect(context.get(o::Value(target),o::Key(symbolA))==o::Value(41.0));
    expect(!context.hasOwn(o::Value(target),o::Key(symbolB)));
    expect(context.get(o::Value(target),o::Key(symbolC))==o::Value(31.0));
    expect(context.get(o::Value(target),o::Key(u"z"))==o::Value(u"z-value"));

    // Assignment into another live StackProperty record writes the same
    // Symbol-key storage that reflection reads.
    const auto secondItem=startItem(context,testTeX(session,runtime));
    const auto secondProperties=context.get(secondItem,o::Key(u"_properties"));
    for(const auto& key:context.ownKeys(o::Value(target)))
      context.set(secondProperties,key,context.get(o::Value(target),key));
    expect(context.get(secondProperties,o::Key(symbolA))==o::Value(41.0));
    expect(context.ownSymbols(secondProperties).size()==3u);

    // Typed Object.assign snapshots keys, then rechecks each live descriptor.
    // The z getter removes a later Symbol, so its getter must never run.
    const auto secondObject=std::get<o::ObjectRef>(secondProperties.data);
    const auto lateSymbol=context.symbol(u"late");
    std::vector<std::string> mutationOrder;
    const auto deletingGetter=context.function(u"get z and delete late",[&](o::Context& cx,o::Value,std::span<const o::Value>){
      mutationOrder.push_back("z");cx.erase(secondObject,o::Key(lateSymbol));return o::Value(u"z-value");
    });
    const auto lateGetter=context.function(u"get late",[&](o::Context&,o::Value,std::span<const o::Value>){
      mutationOrder.push_back("late");return o::Value(61.0);
    });
    context.define(secondObject,o::Key(u"z"),o::Descriptor::access(deletingGetter,std::nullopt,true,true));
    context.define(secondObject,o::Key(lateSymbol),o::Descriptor::access(lateGetter,std::nullopt,true,true));
    auto* secondOwner=runtime.stackPropertyIdentities.resolve(secondObject);
    const auto* secondBinding=std::any_cast<folia::math::NativeBindingPayload<t::StackPropertyStorage*>>(&context.payload(secondObject));
    expect(secondBinding!=nullptr);
    if(secondBinding){
      auto secondStorage=std::shared_ptr<t::StackPropertyStorage>(secondBinding->lifetime,secondOwner);
      auto secondNative=t::StackPropertyList::fromStorage(std::move(secondStorage));
      t::StackPropertyList mutationTarget;
      folia::math::tex::stack_property_host::assign(mutationTarget,t::StackProperty(secondNative));
      expect(mutationOrder==std::vector<std::string>{"z"});
      expect(!mutationTarget.hasOwn(o::Key(lateSymbol)));
    }

    context.erase(std::get<o::ObjectRef>(properties.data),o::Key(symbolA));
    expect(!context.hasOwn(properties,o::Key(symbolA)));
    expect(context.get(properties,o::Key(symbolA))==o::Value(o::Undefined{}));
    expect(context.ownSymbols(properties).size()==3u);
  };
};
}
