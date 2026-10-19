#include "support/folia_test.hpp"
import folia.math.output.common;
import folia.math.output.svg.render_session;
import folia.math.input.tex.parser;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.tex_parser_context;
import folia.math.input.tex.bussproofs.items;
import folia.math.input.tex.bussproofs.methods;
import folia.math.util.property_host;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace o=folia::math::options;
namespace t=folia::math::tex;
namespace common=folia::math::output::common;
namespace svg=folia::math::output::svg;

void defineProperty(o::Context& cx,o::ObjectRef target,const o::Key& name,o::Descriptor descriptor){
  cx.define(target,name,std::move(descriptor));
}
void defineProperty(o::Context& cx,const o::Value& target,const o::Key& name,o::Descriptor descriptor){
  defineProperty(cx,std::get<o::ObjectRef>(target.data),name,std::move(descriptor));
}

common::FontLoadHost fontHost(){
  common::FontLoadHost host;
  host.asyncIsSynchronous=[] {return true;};
  host.warn=[](const std::any&) {};
  return host;
}

t::TeXRef testTeX(svg::NativeRenderSession& session,t::ParserRuntime& runtime){
  auto jax=t::TeX::make(runtime);
  jax->setMmlFactory(core::InputMmlFactoryReference{session.services().mmlFactoryOwner()});
  return jax;
}

suite tex_stack_item_raw_source_fields=[] {
  "tex_start_stack_item_exposes_source_own_storage_fields"_test=[] {
    svg::NativeRenderSession session(fontHost());
    auto& runtime=session.services().parser();
    auto& context=session.context();
    const auto jax=testTeX(session,runtime);
    const auto parsed=jax->parseOptions();
    const auto* options=std::get_if<t::ParseOptionsRef>(&parsed);
    expect(options!=nullptr&&*options);
    if(!options||!*options)return;
    const auto parser=t::TexParser::make(u"",PropertyList{},**options);
    const o::Value stack=context.get(o::Value(parser->instanceIdentity()),o::Key(u"stack"));
    const o::Value entries=context.get(stack,o::Key(u"stack"));
    const o::Value start=context.get(entries,o::Key(u"0"));
    std::vector<std::u16string> keys;
    for(const auto& key:context.ownKeys(start))
      if(const auto* text=std::get_if<std::u16string>(&key.data))keys.push_back(*text);
    const std::vector<std::u16string> expected{u"_nodes",u"startStr",u"startI",u"stopI",u"factory",u"global",u"_properties",u"_env"};
    expect(keys==expected);
    expect(context.hasOwn(start,o::Key(u"_env")));
    expect(context.get(start,o::Key(u"_env"))==context.get(start,o::Key(u"env")));
    const auto nodes=context.get(start,o::Key(u"_nodes"));
    const auto properties=context.get(start,o::Key(u"_properties"));
    expect(std::holds_alternative<o::ObjectRef>(nodes.data));
    expect(std::holds_alternative<o::ObjectRef>(properties.data));
    const auto propertiesObject=std::get<o::ObjectRef>(properties.data);
    const auto objectPrototype=context.intrinsics().objectPrototype;
    expect(context.prototype(propertiesObject)==objectPrototype);
    expect(context.get(properties,o::Key(u"missing"))==o::Value(o::Undefined{}));
    const auto inheritedToString=context.get(o::Value(objectPrototype),o::Key(u"toString"));
    expect(context.get(properties,o::Key(u"toString"))==inheritedToString);
    expect(context.callMember(inheritedToString,properties)==o::Value(u"[object Object]"));
  };

  "stack_item_env_reads_live_source_descriptors"_test=[] {
    svg::NativeRenderSession session(fontHost());auto& runtime=session.services().parser();auto& context=session.context();
    const auto jax=testTeX(session,runtime);const auto parsed=jax->parseOptions();const auto* options=std::get_if<t::ParseOptionsRef>(&parsed);
    expect(options!=nullptr&&*options);if(!options||!*options)return;
    const auto parser=t::TexParser::make(u"",PropertyList{},**options);
    const auto stack=context.get(o::Value(parser->instanceIdentity()),o::Key(u"stack"));
    const auto entries=context.get(stack,o::Key(u"stack"));const auto itemValue=context.get(entries,o::Key(u"0"));
    const auto itemObject=std::get<o::ObjectRef>(itemValue.data);
    const auto itemRef=runtime.stackItemIdentities.resolve(itemObject);auto* item=dynamic_cast<t::BaseItem*>(itemRef.get());
    expect(item!=nullptr);if(!item)return;
    const o::Key environmentKey(u"_env");
    auto typedEnvironment=[&]{return runtime.properties.to(item->env());};

    // Deleting a native-backed own field preserves its declared owner cell;
    // the next Source assignment recreates the own descriptor and identity.
    context.erase(itemObject,environmentKey);
    expect(!context.hasOwn(itemValue,environmentKey));
    expect(std::holds_alternative<o::Undefined>(typedEnvironment().data));
    const auto reassigned=context.object();
    item->setEnv(runtime.properties.from(o::Value(reassigned)));
    expect(context.hasOwn(itemValue,environmentKey));
    expect(context.get(itemValue,environmentKey)==o::Value(reassigned));
    expect(typedEnvironment()==o::Value(reassigned));

    // A Source accessor replaces the native data cell. The typed getter must
    // follow that live descriptor instead of reading the stale backing field.
    const auto getterValue=context.object();
    const auto envGetter=context.function(u"get _env",[getterValue](o::Context&,o::Value,std::span<const o::Value>){return o::Value(getterValue);});
    context.define(itemObject,environmentKey,o::Descriptor::access(envGetter,std::nullopt,true,true));
    expect(context.get(itemValue,environmentKey)==o::Value(getterValue));
    expect(typedEnvironment()==o::Value(getterValue));

    // Replacing the accessor with a data descriptor is observed by the same
    // typed API, including the descriptor's exact Context object identity.
    const auto dataValue=context.object();
    context.define(itemObject,environmentKey,o::Descriptor::data(o::Value(dataValue),true,true,true));
    expect(context.get(itemValue,environmentKey)==o::Value(dataValue));
    expect(typedEnvironment()==o::Value(dataValue));

    context.erase(itemObject,environmentKey);
    expect(!context.hasOwn(itemValue,environmentKey));
    expect(std::holds_alternative<o::Undefined>(typedEnvironment().data));

    // An inherited accessor intercepts assignment as Source does; no own
    // property is created, and BaseItem::env observes the inherited getter.
    const auto oldPrototype=context.prototype(itemObject);
    const auto inheritedPrototype=context.object();
    context.setPrototype(inheritedPrototype,oldPrototype);
    o::Value inheritedValue(o::Undefined{});
    const auto inheritedGetter=context.function(u"get _env",[&](o::Context&,o::Value,std::span<const o::Value>){return inheritedValue;});
    const auto inheritedSetter=context.function(u"set _env",[&](o::Context&,o::Value,std::span<const o::Value> args){inheritedValue=args.empty()?o::Value(o::Undefined{}):args[0];return o::Value(o::Undefined{});});
    context.define(inheritedPrototype,environmentKey,o::Descriptor::access(inheritedGetter,inheritedSetter,true,true));
    context.setPrototype(itemObject,inheritedPrototype);
    const auto inheritedAssigned=context.object();
    item->setEnv(runtime.properties.from(o::Value(inheritedAssigned)));
    expect(inheritedValue==o::Value(inheritedAssigned));
    expect(!context.hasOwn(itemValue,environmentKey));
    expect(typedEnvironment()==o::Value(inheritedAssigned));
  };

  "stack_item_env_preserves_symbol_data_and_getter_values"_test=[] {
    svg::NativeRenderSession session(fontHost());auto& runtime=session.services().parser();auto& context=session.context();
    const auto jax=testTeX(session,runtime);const auto parsed=jax->parseOptions();const auto* options=std::get_if<t::ParseOptionsRef>(&parsed);
    expect(options!=nullptr&&*options);if(!options||!*options)return;
    const auto parser=t::TexParser::make(u"",PropertyList{},**options);
    const auto stack=context.get(o::Value(parser->instanceIdentity()),o::Key(u"stack"));
    const auto entries=context.get(stack,o::Key(u"stack"));const auto itemValue=context.get(entries,o::Key(u"0"));
    const auto itemObject=std::get<o::ObjectRef>(itemValue.data);
    const auto itemRef=runtime.stackItemIdentities.resolve(itemObject);auto* item=dynamic_cast<t::BaseItem*>(itemRef.get());
    expect(item!=nullptr);if(!item)return;
    const o::Key environmentKey(u"_env");auto typedEnvironment=[&]{return runtime.properties.to(item->env());};
    const auto dataSymbol=context.symbol(u"stack item env data");item->setEnv(Property(dataSymbol));
    const auto dataDescriptor=context.descriptor(itemValue,environmentKey);
    expect(dataDescriptor.has_value()&&!dataDescriptor->accessor);expect(context.get(itemValue,environmentKey)==o::Value(dataSymbol));
    expect(item->env()==Property(dataSymbol));expect(typedEnvironment()==o::Value(dataSymbol));
    const auto getterSymbol=context.symbol(u"stack item env getter");
    const auto getter=context.function(u"get _env",[getterSymbol](o::Context&,o::Value,std::span<const o::Value>){return o::Value(getterSymbol);});
    context.define(itemObject,environmentKey,o::Descriptor::access(getter,std::nullopt,true,true));
    expect(context.get(itemValue,environmentKey)==o::Value(getterSymbol));expect(item->env()==Property(getterSymbol));
    expect(typedEnvironment()==o::Value(getterSymbol));
  };

  "closed_stack_item_constructor_call_preserves_live_env_descriptors"_test=[] {
    svg::NativeRenderSession session(fontHost());auto& runtime=session.services().parser();auto& context=session.context();
    const auto jax=testTeX(session,runtime);const auto parsed=jax->parseOptions();const auto* options=std::get_if<t::ParseOptionsRef>(&parsed);
    expect(options!=nullptr&&*options);if(!options||!*options)return;
    const auto parser=t::TexParser::make(u"",PropertyList{},**options);
    const auto stack=context.get(o::Value(parser->instanceIdentity()),o::Key(u"stack"));
    const auto entries=context.get(stack,o::Key(u"stack"));const auto start=context.get(entries,o::Key(u"0"));
    const auto factory=context.get(start,o::Key(u"factory"));
    const auto constructor=std::get<o::ObjectRef>(context.get(runtime.baseItemsModule,o::Key(u"NullItem")).data);
    const std::array<o::Value,1> constructorArgs{o::Value(factory)};const o::Key environmentKey(u"_env");
    const auto nativeInstance=context.construct(constructor,constructorArgs);
    expect(!context.hasOwn(o::Value(nativeInstance),environmentKey));
    expect(context.call(constructor,o::Value(nativeInstance),constructorArgs)==o::Value(nativeInstance));
    expect(!context.hasOwn(o::Value(nativeInstance),environmentKey));
    const auto nativeEnvironment=context.object();context.set(o::Value(nativeInstance),environmentKey,o::Value(nativeEnvironment));
    const auto nativeBefore=context.descriptor(o::Value(nativeInstance),environmentKey);
    expect(nativeBefore.has_value()&&!nativeBefore->accessor);
    expect(context.call(constructor,o::Value(nativeInstance),constructorArgs)==o::Value(nativeInstance));
    const auto nativeAfter=context.descriptor(o::Value(nativeInstance),environmentKey);
    expect(nativeAfter.has_value()&&!nativeAfter->accessor&&nativeAfter->value==nativeBefore->value);
    expect(nativeAfter->enumerable==nativeBefore->enumerable&&nativeAfter->configurable==nativeBefore->configurable&&nativeAfter->writable==nativeBefore->writable);
    const auto prototype=std::get<o::ObjectRef>(context.get(o::Value(constructor),o::Key(u"prototype")).data);
    const auto ordinaryDataReceiver=context.objectWithPrototype(prototype);const auto ordinaryEnvironment=context.object();
    context.define(ordinaryDataReceiver,environmentKey,o::Descriptor::data(o::Value(ordinaryEnvironment),true,true,true));
    const auto dataBefore=context.descriptor(o::Value(ordinaryDataReceiver),environmentKey);
    expect(context.call(constructor,o::Value(ordinaryDataReceiver),constructorArgs)==o::Value(ordinaryDataReceiver));
    const auto dataAfter=context.descriptor(o::Value(ordinaryDataReceiver),environmentKey);
    expect(dataAfter.has_value()&&!dataAfter->accessor&&dataAfter->value==dataBefore->value);
    expect(dataAfter->enumerable==dataBefore->enumerable&&dataAfter->configurable==dataBefore->configurable&&dataAfter->writable==dataBefore->writable);
    const auto ordinaryAccessorReceiver=context.objectWithPrototype(prototype);int getterCalls=0,setterCalls=0;
    const auto getter=context.function(u"get _env",[&](o::Context&,o::Value,std::span<const o::Value>){++getterCalls;return o::Value(ordinaryEnvironment);});
    const auto setter=context.function(u"set _env",[&](o::Context&,o::Value,std::span<const o::Value>){++setterCalls;return o::Value(o::Undefined{});});
    context.define(ordinaryAccessorReceiver,environmentKey,o::Descriptor::access(getter,setter,true,true));
    const auto accessorBefore=context.descriptor(o::Value(ordinaryAccessorReceiver),environmentKey);
    expect(context.call(constructor,o::Value(ordinaryAccessorReceiver),constructorArgs)==o::Value(ordinaryAccessorReceiver));
    const auto accessorAfter=context.descriptor(o::Value(ordinaryAccessorReceiver),environmentKey);
    expect(accessorAfter.has_value()&&accessorAfter->accessor&&accessorAfter->getter==accessorBefore->getter&&accessorAfter->setter==accessorBefore->setter);
    expect(accessorAfter->enumerable==accessorBefore->enumerable&&accessorAfter->configurable==accessorBefore->configurable);
    expect(getterCalls==0&&setterCalls==0);
  };

  "stack_item_raw_source_fields_replace_only_at_the_context_cell"_test=[] {
    svg::NativeRenderSession session(fontHost());
    auto& runtime=session.services().parser();
    auto& context=session.context();
    const auto jax=testTeX(session,runtime);
    const auto parsed=jax->parseOptions();
    const auto* options=std::get_if<t::ParseOptionsRef>(&parsed);
    expect(options!=nullptr&&*options);
    if(!options||!*options)return;
    const auto parser=t::TexParser::make(u"",PropertyList{},**options);
    const o::Value stack=context.get(o::Value(parser->instanceIdentity()),o::Key(u"stack"));
    const o::Value entries=context.get(stack,o::Key(u"stack"));
    const o::Value start=context.get(entries,o::Key(u"0"));
    const o::Value oldNodes=context.get(start,o::Key(u"_nodes"));
    const o::Value oldProperties=context.get(start,o::Key(u"_properties"));
    context.set(oldProperties,o::Key(u"retained"),o::Value(u"old-owner"));
    const auto clear=context.get(start,o::Key(u"Clear"));
    (void)context.callMember(clear,o::Value(start));
    const o::Value clearedNodes=context.get(start,o::Key(u"_nodes"));
    expect(clearedNodes!=oldNodes);
    expect(context.get(clearedNodes,o::Key(u"length"))==o::Value(0.0));

    context.set(start,o::Key(u"_nodes"),o::Value(17.0));
    expect(context.get(start,o::Key(u"_nodes"))==o::Value(17.0));
    expect(context.kind(std::get<o::ObjectRef>(oldNodes.data))==o::ObjectKind::array);

    const auto replacement=context.object();
    context.set(o::Value(replacement),o::Key(u"probe"),o::Value(u"replacement"));
    context.set(start,o::Key(u"_properties"),o::Value(replacement));
    expect(context.get(start,o::Key(u"_properties"))==o::Value(replacement));
    const std::array<o::Value,1> getArgs{o::Value(u"probe")};
    const auto getProperty=context.get(start,o::Key(u"getProperty"));
    expect(context.callMember(getProperty,o::Value(start),getArgs)==o::Value(u"replacement"));
    const std::array<o::Value,2> setArgs{o::Value(u"probe"),o::Value(u"updated")};
    const auto setProperty=context.get(start,o::Key(u"setProperty"));
    (void)context.callMember(setProperty,o::Value(start),setArgs);
    expect(context.get(o::Value(replacement),o::Key(u"probe"))==o::Value(u"updated"));
    expect(context.get(oldProperties,o::Key(u"retained"))==o::Value(u"old-owner"));
    expect(std::holds_alternative<o::Undefined>(context.get(oldProperties,o::Key(u"probe")).data));

  };

  "stack_item_node_and_property_methods_read_the_current_source_descriptors"_test=[] {
    svg::NativeRenderSession session(fontHost());
    auto& runtime=session.services().parser();
    auto& context=session.context();
    const auto jax=testTeX(session,runtime);
    const auto parsed=jax->parseOptions();
    const auto* options=std::get_if<t::ParseOptionsRef>(&parsed);
    expect(options!=nullptr&&*options);
    if(!options||!*options)return;
    const auto parser=t::TexParser::make(u"",PropertyList{},**options);
    const o::Value stack=context.get(o::Value(parser->instanceIdentity()),o::Key(u"stack"));
    const o::Value entries=context.get(stack,o::Key(u"stack"));
    const o::Value start=context.get(entries,o::Key(u"0"));
    const auto foreignNodes=context.array(2);
    context.set(o::Value(foreignNodes),o::Key(u"0"),o::Value(u"first"));
    context.set(o::Value(foreignNodes),o::Key(u"1"),o::Value(u"second"));
    context.set(start,o::Key(u"_nodes"),o::Value(foreignNodes));
    const auto sizeMethod=context.get(start,o::Key(u"Size"));
    expect(context.callMember(sizeMethod,start)==o::Value(2.0));
    context.set(o::Value(foreignNodes),o::Key(u"length"),o::Value(3.0));
    context.set(o::Value(foreignNodes),o::Key(u"2"),o::Value(u"third"));
    expect(context.callMember(sizeMethod,start)==o::Value(3.0));

    int nodeGets=0,nodeSets=0;
    o::Value currentNodes(foreignNodes);
    const auto nodeGetter=context.function(u"get _nodes",[&](o::Context&,o::Value,std::span<const o::Value>){
      ++nodeGets;return currentNodes;
    });
    const auto nodeSetter=context.function(u"set _nodes",[&](o::Context&,o::Value,std::span<const o::Value> args){
      ++nodeSets;currentNodes=args.empty()?o::Value(o::Undefined{}):args[0];return o::Value(o::Undefined{});
    });
    defineProperty(context,start,o::Key(u"_nodes"),o::Descriptor::access(nodeGetter,nodeSetter,true,true));
    expect(context.callMember(sizeMethod,start)==o::Value(3.0));
    expect(nodeGets==1);
    // Source StackItem.ts indexes First with Size()-1; slot 2 is "third" here.
    expect(context.get(start,o::Key(u"First"))==o::Value(u"third"));
    const auto peekMethod=context.get(start,o::Key(u"Peek"));
    const std::array<o::Value,1> peekArgs{o::Value(1.0)};
    const auto peeked=context.callMember(peekMethod,start,peekArgs);
    expect(context.kind(std::get<o::ObjectRef>(peeked.data))==o::ObjectKind::array);
    expect(context.get(peeked,o::Key(u"0"))==o::Value(u"third"));
    const auto popMethod=context.get(start,o::Key(u"Pop"));
    expect(context.callMember(popMethod,start)==o::Value(u"third"));
    const auto pushMethod=context.get(start,o::Key(u"Push"));
    const std::array<o::Value,1> pushArgs{o::Value(u"fourth")};
    (void)context.callMember(pushMethod,start,pushArgs);
    expect(context.get(o::Value(foreignNodes),o::Key(u"2"))==o::Value(u"fourth"));
    const auto clearMethod=context.get(start,o::Key(u"Clear"));
    (void)context.callMember(clearMethod,start);
    const auto cleared=context.get(start,o::Key(u"_nodes"));
    expect(cleared!=o::Value(foreignNodes));
    expect(nodeSets==1);
    expect(context.get(cleared,o::Key(u"length"))==o::Value(0.0));
    expect(context.get(o::Value(foreignNodes),o::Key(u"2"))==o::Value(u"fourth"));

    const auto properties=context.object();
    context.set(o::Value(properties),o::Key(u"probe"),o::Value(u"before"));
    int propertyGets=0,propertySets=0;
    const auto propertyGetter=context.function(u"get _properties",[&](o::Context&,o::Value,std::span<const o::Value>){
      ++propertyGets;return o::Value(properties);
    });
    const auto propertySetter=context.function(u"set _properties",[&](o::Context&,o::Value,std::span<const o::Value> args){
      ++propertySets;
      context.set(o::Value(properties),o::Key(u"replacement"),args.empty()?o::Value(o::Undefined{}):args[0]);
      return o::Value(o::Undefined{});
    });
    defineProperty(context,start,o::Key(u"_properties"),o::Descriptor::access(propertyGetter,propertySetter,true,true));
    const std::array<o::Value,1> getArgs{o::Value(u"probe")};
    const auto getProperty=context.get(start,o::Key(u"getProperty"));
    expect(context.callMember(getProperty,start,getArgs)==o::Value(u"before"));
    const std::array<o::Value,2> setArgs{o::Value(u"replacement"),o::Value(u"after")};
    const auto setProperty=context.get(start,o::Key(u"setProperty"));
    (void)context.callMember(setProperty,start,setArgs);
    expect(context.get(o::Value(properties),o::Key(u"replacement"))==o::Value(u"after"));
    expect(propertyGets>0&&propertySets==0);
    context.set(start,o::Key(u"_properties"),o::Value(context.object()));
    expect(propertySets==1);
    expect(context.get(start,o::Key(u"_properties"))==o::Value(properties));
    defineProperty(context,start,o::Key(u"_properties"),o::Descriptor::data(o::Value(u"raw replacement"),true,true,true));
    expect(context.get(start,o::Key(u"_properties"))==o::Value(u"raw replacement"));
  };

  "stack_peek_normalizes_nullish_count_before_live_nodes_get"_test=[] {
    svg::NativeRenderSession session(fontHost());auto& runtime=session.services().parser();auto& context=session.context();const auto jax=testTeX(session,runtime);
    const auto parsed=jax->parseOptions();const auto* options=std::get_if<t::ParseOptionsRef>(&parsed);expect(options!=nullptr&&*options);if(!options||!*options)return;
    const auto parser=t::TexParser::make(u"",PropertyList{},**options);const auto stack=context.get(o::Value(parser->instanceIdentity()),o::Key(u"stack"));const auto entries=context.get(stack,o::Key(u"stack"));const auto start=context.get(entries,o::Key(u"0"));
    const auto nodes=context.object();std::vector<std::string> trace;
    const auto slice=context.function(u"slice",[&](o::Context& cx,o::Value receiver,std::span<const o::Value> args){trace.push_back("slice-call");expect(receiver==o::Value(nodes));expect(args.size()==1&&args[0]==o::Value(2.0));return o::Value(cx.array(0));});
    const auto sliceGetter=context.function(u"get slice",[&](o::Context&,o::Value,std::span<const o::Value>){trace.push_back("slice-get");return o::Value(slice);});defineProperty(context,o::Value(nodes),o::Key(u"slice"),o::Descriptor::access(sliceGetter,std::nullopt,true,true));
    const auto nodesGetter=context.function(u"get _nodes",[&](o::Context&,o::Value,std::span<const o::Value>){trace.push_back("nodes-get");return o::Value(nodes);});defineProperty(context,start,o::Key(u"_nodes"),o::Descriptor::access(nodesGetter,std::nullopt,true,true));
    const auto size=context.function(u"Size",[&](o::Context&,o::Value,std::span<const o::Value>){trace.push_back("size-call");return o::Value(3.0);});
    const auto sizeGetter=context.function(u"get Size",[&](o::Context&,o::Value,std::span<const o::Value>){trace.push_back("size-get");return o::Value(size);});defineProperty(context,start,o::Key(u"Size"),o::Descriptor::access(sizeGetter,std::nullopt,true,true));
    const auto peek=context.get(start,o::Key(u"Peek"));
    for(const auto count:{o::Value(o::Undefined{}),o::Value(o::Null{})}){trace.clear();const std::array<o::Value,1> args{count};(void)context.callMember(peek,start,args);expect((trace==std::vector<std::string>{"nodes-get","slice-get","size-get","size-call","slice-call"}));}
  };

  "stack_context_pop_and_peek_preserve_raw_sparse_slots"_test=[] {
    svg::NativeRenderSession session(fontHost());
    auto& runtime=session.services().parser();
    auto& context=session.context();
    const auto jax=testTeX(session,runtime);
    const auto parsed=jax->parseOptions();
    const auto* options=std::get_if<t::ParseOptionsRef>(&parsed);
    expect(options!=nullptr&&*options);
    if(!options||!*options)return;
    const auto parser=t::TexParser::make(u"",PropertyList{},**options);
    const o::Value stack=context.get(o::Value(parser->instanceIdentity()),o::Key(u"stack"));
    const o::Value entries=context.get(stack,o::Key(u"stack"));
    const o::Value start=context.get(entries,o::Key(u"0"));
    const auto sparse=context.array(3);
    context.set(o::Value(sparse),o::Key(u"1"),o::Value(o::Undefined{}));
    const o::Value rawSlice=context.get(o::Value(sparse),o::Key(u"slice"));
    const o::Value rawPop=context.get(o::Value(sparse),o::Key(u"pop"));
    bool sliceReceiver=false,popReceiver=false;
    const auto sliceOverride=context.function(u"slice",[&](o::Context& cx,o::Value thisValue,std::span<const o::Value> args){
      sliceReceiver=thisValue==o::Value(sparse);
      return cx.callMember(rawSlice,std::move(thisValue),args);
    });
    const auto popOverride=context.function(u"pop",[&](o::Context& cx,o::Value thisValue,std::span<const o::Value> args){
      popReceiver=thisValue==o::Value(sparse);
      return cx.callMember(rawPop,std::move(thisValue),args);
    });
    context.set(o::Value(sparse),o::Key(u"slice"),o::Value(sliceOverride));
    context.set(o::Value(sparse),o::Key(u"pop"),o::Value(popOverride));
    context.set(start,o::Key(u"_nodes"),o::Value(sparse));

    const auto peek=context.get(start,o::Key(u"Peek"));
    const std::array<o::Value,1> count{o::Value(3.0)};
    const o::Value copy=context.callMember(peek,start,count);
    expect(context.kind(std::get<o::ObjectRef>(copy.data))==o::ObjectKind::array);
    expect(context.get(copy,o::Key(u"length"))==o::Value(3.0));
    expect(!context.hasOwn(copy,o::Key(u"0")));
    expect(context.hasOwn(copy,o::Key(u"1")));
    expect(std::holds_alternative<o::Undefined>(context.get(copy,o::Key(u"1")).data));
    expect(!context.hasOwn(copy,o::Key(u"2")));
    expect(sliceReceiver);

    const auto pop=context.get(start,o::Key(u"Pop"));
    expect(std::holds_alternative<o::Undefined>(context.callMember(pop,start).data));
    expect(popReceiver);
    expect(context.get(o::Value(sparse),o::Key(u"length"))==o::Value(2.0));
    // Pop removes the trailing hole; index 0 was a hole before and remains one.
    expect(!context.hasOwn(o::Value(sparse),o::Key(u"0")));
    expect(context.hasOwn(o::Value(sparse),o::Key(u"1")));
    expect(std::holds_alternative<o::Undefined>(context.callMember(pop,start).data));
    expect(context.get(o::Value(sparse),o::Key(u"length"))==o::Value(1.0));
    expect(!context.hasOwn(o::Value(sparse),o::Key(u"0")));
  };

  "stack_context_to_mml_uses_strict_source_length_equality"_test=[] {
    svg::NativeRenderSession session(fontHost());
    auto& runtime=session.services().parser();
    auto& context=session.context();
    const auto jax=testTeX(session,runtime);
    const auto parsed=jax->parseOptions();
    const auto* options=std::get_if<t::ParseOptionsRef>(&parsed);
    expect(options!=nullptr&&*options);
    if(!options||!*options)return;
    const auto parser=t::TexParser::make(u"",PropertyList{},**options);
    const o::Value stack=context.get(o::Value(parser->instanceIdentity()),o::Key(u"stack"));
    const o::Value entries=context.get(stack,o::Key(u"stack"));
    const o::Value start=context.get(entries,o::Key(u"0"));
    const auto foreign=context.object();
    const auto lengthGetter=context.function(u"get length",[](o::Context&,o::Value,std::span<const o::Value>){return o::Value(u"1");});
    defineProperty(context,o::Value(foreign),o::Key(u"length"),o::Descriptor::access(lengthGetter,std::nullopt,true,true));
    const auto rawNodes=context.array(0);
    context.set(start,o::Key(u"_nodes"),o::Value(rawNodes));
    int nodesGetterCalls=0;
    const auto nodesGetter=context.function(u"get nodes",[&](o::Context&,o::Value,std::span<const o::Value>){++nodesGetterCalls;return o::Value(foreign);});
    defineProperty(context,start,o::Key(u"nodes"),o::Descriptor::access(nodesGetter,std::nullopt,true,true));
    int createCalls=0;
    bool keptRawArray=false;
    const auto create=context.function(u"create",[&](o::Context&,o::Value,std::span<const o::Value> args){
      ++createCalls;
      keptRawArray=args.size()==4&&args[2]==o::Value(rawNodes);
      return o::Value(u"created");
    });
    context.set(start,o::Key(u"create"),o::Value(create));
    const auto toMml=context.get(start,o::Key(u"toMml"));
    expect(context.callMember(toMml,start)==o::Value(u"created"));
    // Source MmlStack.toMml reads and forwards _nodes directly, not the nodes accessor.
    expect(createCalls==1&&keptRawArray&&nodesGetterCalls==0);
  };

  "native_source_nodes_property_reads_configurable_receiver_getter"_test=[] {
    svg::NativeRenderSession session(fontHost());
    auto& runtime=session.services().parser();
    auto& context=session.context();
    const auto jax=testTeX(session,runtime);
    const auto parsed=jax->parseOptions();
    const auto* options=std::get_if<t::ParseOptionsRef>(&parsed);
    expect(options!=nullptr&&*options);
    if(!options||!*options)return;
    const auto parser=t::TexParser::make(u"",PropertyList{},**options);
    const o::Value stack=context.get(o::Value(parser->instanceIdentity()),o::Key(u"stack"));
    const o::Value entries=context.get(stack,o::Key(u"stack"));
    const o::Value start=context.get(entries,o::Key(u"0"));
    const auto* object=std::get_if<o::ObjectRef>(&start.data);
    expect(object!=nullptr);
    if(!object)return;
    const auto reference=runtime.stackItemIdentities.resolve(*object);
    auto* item=reference?dynamic_cast<t::BaseItem*>(reference.get()):nullptr;
    expect(item!=nullptr);
    if(!item)return;
    const auto expected=context.array(1);
    int getterCalls=0;
    bool getterReceiver=false;
    const auto getter=context.function(u"get nodes",[&](o::Context&,o::Value receiver,std::span<const o::Value>){
      ++getterCalls;getterReceiver=receiver==start;return o::Value(expected);
    });
    defineProperty(context,start,o::Key(u"nodes"),o::Descriptor::access(getter,std::nullopt,true,true));
    expect(item->sourceNodesPropertyValue()==o::Value(expected));
    expect(getterCalls==1&&getterReceiver);
  };
  "native_stack_item_string_uses_current_raw_nodes_owner"_test=[] {
    svg::NativeRenderSession session(fontHost());
    auto& runtime=session.services().parser();
    auto& context=session.context();
    const auto jax=testTeX(session,runtime);
    const auto parsed=jax->parseOptions();
    const auto* options=std::get_if<t::ParseOptionsRef>(&parsed);
    expect(options!=nullptr&&*options);
    if(!options||!*options)return;
    const auto parser=t::TexParser::make(u"",PropertyList{},**options);
    const o::Value stack=context.get(o::Value(parser->instanceIdentity()),o::Key(u"stack"));
    const o::Value entries=context.get(stack,o::Key(u"stack"));
    const o::Value start=context.get(entries,o::Key(u"0"));
    const auto foreign=context.array(2);
    context.set(o::Value(foreign),o::Key(u"0"),o::Value(u"first"));
    context.set(o::Value(foreign),o::Key(u"1"),o::Value(u"second"));
    bool joinReceiver=false;
    const auto join=context.function(u"join",[&](o::Context&,o::Value thisValue,std::span<const o::Value> args){
      joinReceiver=thisValue==o::Value(foreign)&&args.size()==1&&args[0]==o::Value(u"; ");
      return o::Value(u"joined raw values");
    });
    context.set(o::Value(foreign),o::Key(u"join"),o::Value(join));
    context.set(start,o::Key(u"_nodes"),o::Value(foreign));
    const auto* object=std::get_if<o::ObjectRef>(&start.data);
    expect(object!=nullptr);
    if(!object)return;
    const auto itemRef=runtime.stackItemIdentities.resolve(*object);
    auto* item=itemRef?dynamic_cast<t::BaseItem*>(itemRef.get()):nullptr;
    expect(item!=nullptr);
    if(!item)return;
    std::size_t nodesGetterCalls=0;
    const auto nodesGetter=context.function(u"get nodes",[&](o::Context&,o::Value,std::span<const o::Value>){
      ++nodesGetterCalls;return o::Value(foreign);
    });
    defineProperty(context,start,o::Key(u"nodes"),o::Descriptor::access(nodesGetter,std::nullopt,false,true));
    // TexParser's constructor has already pushed the stop item. The StartItem
    // has therefore finalized into an MmlItem at stack[0] before this fixture
    // obtains it from the real parser stack.
    expect(item->toString()=="mml[joined raw values]");
    expect(joinReceiver&&nodesGetterCalls==1);
  };

  "native_stack_item_to_string_uses_source_addition_coercion_and_order"_test=[] {
    svg::NativeRenderSession session(fontHost());auto& runtime=session.services().parser();auto& context=session.context();const auto jax=testTeX(session,runtime);
    const auto parsed=jax->parseOptions();const auto* options=std::get_if<t::ParseOptionsRef>(&parsed);expect(options!=nullptr&&*options);if(!options||!*options)return;
    const auto parser=t::TexParser::make(u"",PropertyList{},**options);const auto stack=context.get(o::Value(parser->instanceIdentity()),o::Key(u"stack"));const auto entries=context.get(stack,o::Key(u"stack"));const auto start=context.get(entries,o::Key(u"0"));
    const auto identity=std::get<o::ObjectRef>(start.data);const auto ref=runtime.stackItemIdentities.resolve(identity);auto* item=dynamic_cast<t::BaseItem*>(ref.get());expect(item!=nullptr);if(!item)return;
    std::vector<std::string> trace;const auto kind=context.object(),joinResult=context.object(),nodes=context.object();
    const auto kindPrimitive=context.function(u"kind primitive",[&](o::Context&,o::Value,std::span<const o::Value> args){expect(args.size()==1&&args[0]==o::Value(u"default"));trace.push_back("kind-primitive");return o::Value(u"custom-kind");});
    defineProperty(context,o::Value(kind),o::Key(context.intrinsics().symbolToPrimitive),o::Descriptor::data(o::Value(kindPrimitive),true,true,true));defineProperty(context,start,o::Key(u"kind"),o::Descriptor::data(o::Value(kind),true,true,true));
    const auto resultPrimitive=context.function(u"join primitive",[&](o::Context&,o::Value,std::span<const o::Value> args){expect(args.size()==1&&args[0]==o::Value(u"default"));trace.push_back("join-primitive");return o::Value(false);});
    defineProperty(context,o::Value(joinResult),o::Key(context.intrinsics().symbolToPrimitive),o::Descriptor::data(o::Value(resultPrimitive),true,true,true));
    const auto join=context.function(u"join",[&](o::Context&,o::Value receiver,std::span<const o::Value> args){trace.push_back("join-call");expect(receiver==o::Value(nodes));expect(args.size()==1&&args[0]==o::Value(u"; "));return o::Value(joinResult);});
    const auto joinGetter=context.function(u"get join",[&](o::Context&,o::Value,std::span<const o::Value>){trace.push_back("join-get");return o::Value(join);});defineProperty(context,o::Value(nodes),o::Key(u"join"),o::Descriptor::access(joinGetter,std::nullopt,true,true));
    const auto nodesGetter=context.function(u"get _nodes",[&](o::Context&,o::Value,std::span<const o::Value>){trace.push_back("nodes-get");return o::Value(nodes);});defineProperty(context,start,o::Key(u"_nodes"),o::Descriptor::access(nodesGetter,std::nullopt,true,true));
    expect(item->toString()=="custom-kind[false]");expect((trace==std::vector<std::string>{"kind-primitive","nodes-get","join-get","join-call","join-primitive"}));

    const auto overConstructor=std::get<o::ObjectRef>(context.get(runtime.baseItemsModule,o::Key(u"OverItem")).data);
    const auto factory=context.get(start,o::Key(u"factory"));const std::array<o::Value,1> ctorArgs{factory};const auto overObject=context.construct(overConstructor,ctorArgs);
    const auto overIdentity=runtime.stackItemIdentities.resolve(overObject);auto* over=dynamic_cast<t::OverItem*>(overIdentity.get());expect(over!=nullptr);if(!over)return;
    const auto numerator=context.object();const auto numeratorPrimitive=context.function(u"numerator primitive",[&](o::Context&,o::Value,std::span<const o::Value> args){expect(args.size()==1&&args[0]==o::Value(u"default"));return o::Value(5.0);});
    defineProperty(context,o::Value(numerator),o::Key(context.intrinsics().symbolToPrimitive),o::Descriptor::data(o::Value(numeratorPrimitive),true,true,true));
    const auto overProperties=context.get(overObject,o::Key(u"_properties"));context.set(overProperties,o::Key(u"num"),o::Value(numerator));
    const auto overNodes=context.object();const auto overJoin=context.function(u"join",[](o::Context&,o::Value,std::span<const o::Value>){return o::Value(false);});context.set(o::Value(overNodes),o::Key(u"join"),o::Value(overJoin));context.set(overObject,o::Key(u"_nodes"),o::Value(overNodes));
    expect(over->toString()=="over[5 / false]");
  };

  "native_fix_initial_mo_reads_the_captured_raw_array_in_source_order"_test=[] {
    svg::NativeRenderSession session(fontHost());
    auto& runtime=session.services().parser();
    auto& context=session.context();
    const auto jax=testTeX(session,runtime);
    const auto parsed=jax->parseOptions();
    const auto* options=std::get_if<t::ParseOptionsRef>(&parsed);
    expect(options!=nullptr&&*options);
    if(!options||!*options)return;
    const auto parser=t::TexParser::make(u"",PropertyList{},**options);
    const o::Value stack=context.get(o::Value(parser->instanceIdentity()),o::Key(u"stack"));
    const o::Value entries=context.get(stack,o::Key(u"stack"));
    const o::Value start=context.get(entries,o::Key(u"0"));
    const auto* object=std::get_if<o::ObjectRef>(&start.data);
    expect(object!=nullptr);
    if(!object)return;
    const auto reference=runtime.stackItemIdentities.resolve(*object);
    auto* item=reference?dynamic_cast<t::BaseItem*>(reference.get()):nullptr;
    expect(item!=nullptr);
    if(!item)return;
    const auto nodes=context.object();
    defineProperty(context,o::Value(nodes),o::Key(u"length"),o::Descriptor::data(o::Value(2.0),true,true,true));
    std::vector<int> trace;
    const auto first=context.function(u"first",[&](o::Context& cx,o::Value,std::span<const o::Value>){
      trace.push_back(0);
      cx.set(o::Value(nodes),o::Key(u"length"),o::Value(0.0));
      return o::Value(o::Undefined{});
    });
    const auto second=context.function(u"second",[&](o::Context&,o::Value,std::span<const o::Value>){trace.push_back(1);return o::Value(o::Undefined{});});
    defineProperty(context,o::Value(nodes),o::Key(u"0"),o::Descriptor::access(first,std::nullopt,true,true));
    defineProperty(context,o::Value(nodes),o::Key(u"1"),o::Descriptor::access(second,std::nullopt,true,true));
    context.set(start,o::Key(u"_nodes"),o::Value(context.array(0)));
    int nodesGetterCalls=0;
    const auto nodesGetter=context.function(u"get nodes",[&](o::Context&,o::Value,std::span<const o::Value>){++nodesGetterCalls;return o::Value(nodes);});
    defineProperty(context,start,o::Key(u"nodes"),o::Descriptor::access(nodesGetter,std::nullopt,true,true));
    t::ParseUtil::fixInitialMO(**options,*item);
    expect((trace==std::vector<int>{0,1}));
    expect(nodesGetterCalls==1);
    expect(context.get(o::Value(nodes),o::Key(u"length"))==o::Value(0.0));
  };

  "native_bussproofs_peek_reads_childnodes_without_node_guard"_test=[] {
    svg::NativeRenderSession session(fontHost());
    auto& runtime=session.services().parser();
    auto& context=session.context();
    const auto jax=testTeX(session,runtime);
    const auto parsed=jax->parseOptions();
    const auto* options=std::get_if<t::ParseOptionsRef>(&parsed);
    expect(options!=nullptr&&*options);
    if(!options||!*options)return;
    const auto parser=t::TexParser::make(u"",PropertyList{},**options);
    const o::Value stack=context.get(o::Value(parser->instanceIdentity()),o::Key(u"stack"));
    const o::Value entries=context.get(stack,o::Key(u"stack"));
    const o::Value start=context.get(entries,o::Key(u"0"));
    const auto* object=std::get_if<o::ObjectRef>(&start.data);
    expect(object!=nullptr);
    if(!object)return;
    const auto reference=runtime.stackItemIdentities.resolve(*object);
    auto* item=reference?dynamic_cast<t::BaseItem*>(reference.get()):nullptr;
    expect(item!=nullptr);
    if(!item)return;
    bool typedPeekRejected=false;
    try{(void)item->Peek();}catch(const o::Exception& error){typedPeekRejected=error.name=="TypeError";}
    expect(typedPeekRejected);
    int peekCalls=0;
    const auto rawPeek=context.array(1);
    const auto peek=context.function(u"Peek",[&](o::Context&,o::Value,std::span<const o::Value>){++peekCalls;return o::Value(rawPeek);});
    defineProperty(context,start,o::Key(u"Peek"),o::Descriptor::data(o::Value(peek),true,true,true));
    bool typeError=false;
    try{
      auto& access=item->sourceAccess();const auto receiver=access.receiver();const auto method=access.getReceiver(u"Peek");
      const auto result=access.callMember(method,receiver,{});const auto first=access.getIndex(result,0);
      const auto childNodes=access.get(first,o::Key(u"childNodes"));(void)access.get(childNodes,o::Key(u"length"));
    }catch(const o::Exception& error){typeError=error.name=="TypeError";}
    expect(typeError&&peekCalls==1);
  };

  "native_peek_read_uses_bounded_context_iterator_and_return"_test=[] {
    svg::NativeRenderSession session(fontHost());
    auto& runtime=session.services().parser();
    auto& context=session.context();
    const auto jax=testTeX(session,runtime);
    const auto parsed=jax->parseOptions();
    const auto* options=std::get_if<t::ParseOptionsRef>(&parsed);
    expect(options!=nullptr&&*options);
    if(!options||!*options)return;
    const auto parser=t::TexParser::make(u"",PropertyList{},**options);
    const o::Value stack=context.get(o::Value(parser->instanceIdentity()),o::Key(u"stack"));
    const o::Value entries=context.get(stack,o::Key(u"stack"));
    const o::Value start=context.get(entries,o::Key(u"0"));
    const auto* object=std::get_if<o::ObjectRef>(&start.data);
    expect(object!=nullptr);
    if(!object)return;
    const auto reference=runtime.stackItemIdentities.resolve(*object);
    auto* item=reference?dynamic_cast<t::BaseItem*>(reference.get()):nullptr;
    expect(item!=nullptr);
    if(!item)return;
    const auto iterable=context.object();
    const auto iterator=context.object();
    std::vector<int> trace;
    int index=0;
    const auto next=context.function(u"next",[&](o::Context& cx,o::Value,std::span<const o::Value>){
      trace.push_back(index);
      const auto result=cx.object();
      cx.set(o::Value(result),o::Key(u"done"),o::Value(false));
      cx.set(o::Value(result),o::Key(u"value"),o::Value(std::u16string(index==0?u"first":u"second")));
      ++index;
      return o::Value(result);
    });
    const auto close=context.function(u"return",[&](o::Context& cx,o::Value,std::span<const o::Value>){
      trace.push_back(2);
      const auto result=cx.object();cx.set(o::Value(result),o::Key(u"done"),o::Value(true));return o::Value(result);
    });
    defineProperty(context,o::Value(iterator),o::Key(u"next"),o::Descriptor::data(o::Value(next),true,true,true));
    defineProperty(context,o::Value(iterator),o::Key(u"return"),o::Descriptor::data(o::Value(close),true,true,true));
    const auto iteratorMethod=context.function(u"iterator",[&](o::Context&,o::Value,std::span<const o::Value>){return o::Value(iterator);});
    defineProperty(context,o::Value(iterable),o::Key(context.intrinsics().symbolIterator),o::Descriptor::data(o::Value(iteratorMethod),true,true,true));
    const auto peek=context.function(u"Peek",[&](o::Context&,o::Value,std::span<const o::Value>){return o::Value(iterable);});
    defineProperty(context,start,o::Key(u"Peek"),o::Descriptor::data(o::Value(peek),true,true,true));
    const auto values=item->sourceReadPeek(o::Value(2.0),2);
    const auto* valuesObject=std::get_if<o::ObjectRef>(&values.data);
    expect(valuesObject!=nullptr&&context.get(values,o::Key(u"length"))==o::Value(2.0));
    expect(context.get(values,o::Key(u"0"))==o::Value(u"first"));
    expect(context.get(values,o::Key(u"1"))==o::Value(u"second"));
    expect((trace==std::vector<int>{0,1,2}));
  };

  "native_prime_check_item_uses_live_peek_iterator_owner"_test=[] {
    svg::NativeRenderSession session(fontHost());auto& runtime=session.services().parser();auto& context=session.context();
    const auto jax=testTeX(session,runtime);const auto parsed=jax->parseOptions();const auto* options=std::get_if<t::ParseOptionsRef>(&parsed);
    expect(options!=nullptr&&*options);if(!options||!*options)return;
    const auto parser=t::TexParser::make(u"",PropertyList{},**options);
    const auto stack=context.get(o::Value(parser->instanceIdentity()),o::Key(u"stack"));const auto entries=context.get(stack,o::Key(u"stack"));const auto start=context.get(entries,o::Key(u"0"));
    const auto startObject=std::get<o::ObjectRef>(start.data);const auto startRef=runtime.stackItemIdentities.resolve(startObject);auto* startItem=dynamic_cast<t::BaseItem*>(startRef.get());expect(startItem!=nullptr);if(!startItem)return;
    const auto constructorValue=context.get(runtime.baseItemsModule,o::Key(u"PrimeItem"));const auto constructor=std::get<o::ObjectRef>(constructorValue.data);
    const auto factory=context.get(start,o::Key(u"factory"));const std::array<o::Value,1> constructorArgs{factory};const auto primeObject=context.construct(constructor,constructorArgs);
    const auto primeRef=runtime.stackItemIdentities.resolve(primeObject);auto* prime=dynamic_cast<t::PrimeItem*>(primeRef.get());expect(prime!=nullptr);if(!prime)return;
    const auto iterable=context.object(),iterator=context.object();std::vector<int> trace;int index=0;
    const auto next=context.function(u"next",[&](o::Context& cx,o::Value,std::span<const o::Value>){trace.push_back(index);const auto result=cx.object();cx.set(o::Value(result),o::Key(u"done"),o::Value(false));cx.set(o::Value(result),o::Key(u"value"),o::Value(u"not-node"));++index;return o::Value(result);});
    const auto close=context.function(u"return",[&](o::Context& cx,o::Value,std::span<const o::Value>){trace.push_back(2);const auto result=cx.object();cx.set(o::Value(result),o::Key(u"done"),o::Value(true));return o::Value(result);});
    defineProperty(context,o::Value(iterator),o::Key(u"next"),o::Descriptor::data(o::Value(next),true,true,true));defineProperty(context,o::Value(iterator),o::Key(u"return"),o::Descriptor::data(o::Value(close),true,true,true));
    const auto iteratorMethod=context.function(u"iterator",[&](o::Context&,o::Value,std::span<const o::Value>){return o::Value(iterator);});defineProperty(context,o::Value(iterable),o::Key(context.intrinsics().symbolIterator),o::Descriptor::data(o::Value(iteratorMethod),true,true,true));
    const auto peek=context.function(u"Peek",[&](o::Context&,o::Value,std::span<const o::Value> args){expect(args.size()==1&&args[0]==o::Value(2.0));return o::Value(iterable);});defineProperty(context,primeObject,o::Key(u"Peek"),o::Descriptor::data(o::Value(peek),true,true,true));
    bool typeError=false;try{(void)prime->checkItem(startItem);}catch(const o::Exception& error){typeError=error.name=="TypeError";}
    expect(typeError&&(trace==std::vector<int>{0,1,2}));
  };

  "native_base_methods_superscript_uses_production_bounded_peek_consumer"_test=[] {
    svg::NativeRenderSession session(fontHost());auto& runtime=session.services().parser();auto& context=session.context();
    const auto jax=testTeX(session,runtime);const auto parsed=jax->parseOptions();const auto* options=std::get_if<t::ParseOptionsRef>(&parsed);
    expect(options!=nullptr&&*options);if(!options||!*options)return;
    const auto parser=t::TexParser::make(u"",PropertyList{},**options);
    const auto stack=context.get(o::Value(parser->instanceIdentity()),o::Key(u"stack"));const auto entries=context.get(stack,o::Key(u"stack"));const auto start=context.get(entries,o::Key(u"0"));
    const auto constructor=std::get<o::ObjectRef>(context.get(runtime.baseItemsModule,o::Key(u"PrimeItem")).data);const auto factory=context.get(start,o::Key(u"factory"));const std::array<o::Value,1> constructorArgs{factory};const auto primeObject=context.construct(constructor,constructorArgs);
    const auto primeRef=runtime.stackItemIdentities.resolve(primeObject);expect(primeRef!=nullptr);if(!primeRef)return;parser->Push(t::StackEntry(primeRef));
    const auto iterable=context.object(),iterator=context.object();std::vector<int> trace;int index=0;
    const auto next=context.function(u"next",[&](o::Context& cx,o::Value,std::span<const o::Value>){trace.push_back(index);const auto result=cx.object();cx.set(o::Value(result),o::Key(u"done"),o::Value(false));cx.set(o::Value(result),o::Key(u"value"),o::Value(u"not-node"));++index;return o::Value(result);});
    const auto close=context.function(u"return",[&](o::Context& cx,o::Value,std::span<const o::Value>){trace.push_back(2);const auto result=cx.object();cx.set(o::Value(result),o::Key(u"done"),o::Value(true));return o::Value(result);});
    defineProperty(context,o::Value(iterator),o::Key(u"next"),o::Descriptor::data(o::Value(next),true,true,true));defineProperty(context,o::Value(iterator),o::Key(u"return"),o::Descriptor::data(o::Value(close),true,true,true));
    const auto iteratorMethod=context.function(u"iterator",[&](o::Context&,o::Value,std::span<const o::Value>){return o::Value(iterator);});defineProperty(context,o::Value(iterable),o::Key(context.intrinsics().symbolIterator),o::Descriptor::data(o::Value(iteratorMethod),true,true,true));
    const auto peek=context.function(u"Peek",[&](o::Context&,o::Value,std::span<const o::Value> args){expect(args.size()==1&&args[0]==o::Value(2.0));return o::Value(iterable);});defineProperty(context,primeObject,o::Key(u"Peek"),o::Descriptor::data(o::Value(peek),true,true,true));
    const auto table=t::BaseMethods::initializeModule(runtime);const auto method=functions::fromProperty(t::BaseMethods::get(table,u"Superscript"));const t::ParseAtom atom=Property(Missing{});const t::ParseRest rest{};
    bool typeError=false;try{(void)t::invokeParseMethod(method,FunctionReceiver{Missing{}},*parser,atom,rest);}catch(const o::Exception& error){typeError=error.name=="TypeError";}
    expect(typeError&&(trace==std::vector<int>{0,1,2}));
  };

  "native_bussproofs_inference_production_path_preserves_missing_child_error"_test=[] {
    svg::NativeRenderSession session(fontHost());auto& runtime=session.services().parser();auto& context=session.context();
    const auto jax=testTeX(session,runtime);const auto parsed=jax->parseOptions();const auto* options=std::get_if<t::ParseOptionsRef>(&parsed);
    expect(options!=nullptr&&*options);if(!options||!*options)return;
    const auto parser=t::TexParser::make(u"",PropertyList{},**options);
    const auto stack=context.get(o::Value(parser->instanceIdentity()),o::Key(u"stack"));const auto entries=context.get(stack,o::Key(u"stack"));const auto start=context.get(entries,o::Key(u"0"));
    const auto itemClass=t::ProofTreeItem::initializeModule(runtime);const auto constructor=runtime.itemClassIdentities.find(itemClass);expect(constructor.has_value());if(!constructor)return;
    const auto factory=context.get(start,o::Key(u"factory"));const std::array<o::Value,1> constructorArgs{factory};const auto proofObject=context.construct(*constructor,constructorArgs);
    const auto proofRef=runtime.stackItemIdentities.resolve(proofObject);expect(proofRef!=nullptr);if(!proofRef)return;parser->Push(t::StackEntry(proofRef));
    context.set(proofObject,o::Key(u"_nodes"),o::Value(context.array(1)));
    int peekCalls=0;const auto peekArray=context.array(1);const auto peek=context.function(u"Peek",[&](o::Context&,o::Value,std::span<const o::Value>){++peekCalls;return o::Value(peekArray);});defineProperty(context,proofObject,o::Key(u"Peek"),o::Descriptor::data(o::Value(peek),true,true,true));
    const auto methodSet=t::BussproofsMethods::create(runtime);const auto property=property_host::get(methodSet.methods,property_host::debugText(u"Inference"));const auto method=functions::fromProperty(property);
    const t::ParseAtom atom=Property(Missing{});const std::array<std::any,1> restStorage{std::any(Property(1.0))};const t::ParseRest rest(restStorage);
    bool typeError=false;try{(void)t::invokeParseMethod(method,FunctionReceiver{Missing{}},*parser,atom,rest);}catch(const o::Exception& error){typeError=error.name=="TypeError";}
    expect(typeError&&peekCalls==1);
    const auto twoNodes=context.array(2);context.set(proofObject,o::Key(u"_nodes"),o::Value(twoNodes));
    const std::array<std::any,1> binaryRestStorage{std::any(Property(2.0))};const t::ParseRest binaryRest(binaryRestStorage);
    try{(void)t::invokeParseMethod(method,FunctionReceiver{Missing{}},*parser,atom,binaryRest);}catch(const o::Exception&){}
    expect(peekCalls==1);
  };

  "retained_context_stack_value_does_not_keep_stack_owner_alive"_test=[] {
    std::weak_ptr<t::StackItemContext> weakOwner;o::Value retainedRaw;
    {
      svg::NativeRenderSession session(fontHost());auto& runtime=session.services().parser();auto& context=session.context();const auto jax=testTeX(session,runtime);
      const auto parsed=jax->parseOptions();const auto* options=std::get_if<t::ParseOptionsRef>(&parsed);expect(options!=nullptr&&*options);if(!options||!*options)return;
      const auto parser=t::TexParser::make(u"",PropertyList{},**options);const auto stack=context.get(o::Value(parser->instanceIdentity()),o::Key(u"stack"));const auto entries=context.get(stack,o::Key(u"stack"));retainedRaw=context.get(entries,o::Key(u"0"));
      const auto object=std::get<o::ObjectRef>(retainedRaw.data);auto owner=runtime.stackItemIdentities.resolve(object);weakOwner=owner;expect(!weakOwner.expired());
    }
    expect(std::holds_alternative<o::ObjectRef>(retainedRaw.data));expect(weakOwner.expired());
  };

  "stack_source_access_uses_weak_runtime_lease_without_owner_cycle"_test=[] {
    std::weak_ptr<t::StackItemContext> weakOwner;
    std::shared_ptr<t::BaseItem> retainedOwner;
    o::Value retainedRaw;
    {
      svg::NativeRenderSession session(fontHost());auto& runtime=session.services().parser();auto& context=session.context();const auto jax=testTeX(session,runtime);
      const auto parsed=jax->parseOptions();const auto* options=std::get_if<t::ParseOptionsRef>(&parsed);expect(options!=nullptr&&*options);if(!options||!*options)return;
      const auto parser=t::TexParser::make(u"",PropertyList{},**options);const auto stack=context.get(o::Value(parser->instanceIdentity()),o::Key(u"stack"));const auto entries=context.get(stack,o::Key(u"stack"));retainedRaw=context.get(entries,o::Key(u"0"));
      const auto object=std::get<o::ObjectRef>(retainedRaw.data);auto owner=runtime.stackItemIdentities.resolve(object);weakOwner=owner;retainedOwner=std::dynamic_pointer_cast<t::BaseItem>(owner);expect(retainedOwner!=nullptr&&!weakOwner.expired());
    }
    expect(std::holds_alternative<o::ObjectRef>(retainedRaw.data));expect(!weakOwner.expired());
    bool leaseRejected=false;try{(void)retainedOwner->sourceNodesCurrentValue();}catch(const o::Exception& error){leaseRejected=error.name=="TypeError";}
    expect(leaseRejected);
    retainedOwner.reset();expect(weakOwner.expired());
  };

  "stack_context_set_properties_matches_object_assign_nullish_order"_test=[] {
    svg::NativeRenderSession session(fontHost());
    auto& runtime=session.services().parser();
    auto& context=session.context();
    const auto jax=testTeX(session,runtime);
    const auto parsed=jax->parseOptions();
    const auto* options=std::get_if<t::ParseOptionsRef>(&parsed);
    expect(options!=nullptr&&*options);
    if(!options||!*options)return;
    const auto parser=t::TexParser::make(u"",PropertyList{},**options);
    const o::Value stack=context.get(o::Value(parser->instanceIdentity()),o::Key(u"stack"));
    const o::Value entries=context.get(stack,o::Key(u"stack"));
    const o::Value start=context.get(entries,o::Key(u"0"));
    const auto target=context.object();
    context.set(start,o::Key(u"_properties"),o::Value(target));
    const auto setProperties=context.get(start,o::Key(u"setProperties"));
    bool nullSourceIgnored=true;
    try{
      const std::array<o::Value,1> args{o::Value(o::Null{})};
      (void)context.callMember(setProperties,start,args);
    }catch(const o::Exception&){nullSourceIgnored=false;}
    expect(nullSourceIgnored);
    expect(context.ownKeys(o::Value(target)).empty());

    context.set(start,o::Key(u"_properties"),o::Value(o::Null{}));
    const auto empty=context.object();
    bool nullTargetThrows=false;
    try{
      const std::array<o::Value,1> args{o::Value(empty)};
      (void)context.callMember(setProperties,start,args);
    }catch(const o::Exception& error){nullTargetThrows=error.name=="TypeError";}
    expect(nullTargetThrows);
  };
};
} // namespace



