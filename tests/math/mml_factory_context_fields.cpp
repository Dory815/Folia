#include "support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.core.mml_tree.mml_factory_context_values;
import folia.math.core.mml_tree.mml_nodes.mrow;
import folia.math.core.mml_tree.mml_nodes.mi;
import folia.math.core.mml_tree.mml_nodes.mtable;
import folia.math.core.mml_tree.mml_nodes.mtr;
import folia.math.core.mml_tree.mml_nodes.mtd;
import folia.math.util.property_host;

namespace {
using namespace boost::ut;
namespace t=folia::math::tex;
namespace o=folia::math::options;
namespace m=folia::math::mml;
namespace tree=folia::math::tree;

template<class T>bool equalVector(const std::vector<T>& lhs,const std::vector<T>& rhs){return lhs==rhs;}

template<class Action>void diagnoseMmlFailure(const char* stage,Action&& action){
  try{action();}
  catch(const o::Exception& error){
    std::cerr<<"[MML diagnostic "<<stage<<"] "<<error.name<<": "
             <<folia::math::property_host::debugText(error.message)<<'\n';
    throw;
  }
}

struct Graph {
    o::Context context;
    folia::math::OptionFunctionBindings functions{context};
    folia::math::regexp_host::RegExpBindings regexp{context};
    folia::math::Unicode16Normalization unicode;
    folia::math::core::InputJaxModule input{context};
    folia::math::core::FindMathModule findMath{context};
    t::FindTeXModule findTeX{context,findMath,regexp};
    t::ParserRuntime runtime{context,functions,regexp,unicode,input,findTeX};
};

struct SharedRuntimeGraph {
    o::Context context;
    folia::math::OptionFunctionBindings functions{context};
    folia::math::regexp_host::RegExpBindings regexp{context};
    folia::math::Unicode16Normalization unicode;
    folia::math::core::InputJaxModule input{context};
    folia::math::core::FindMathModule findMath{context};
    t::FindTeXModule findTeX{context,findMath,regexp};
    folia::math::util::RuntimeModules modules{context};
    std::unique_ptr<t::ParserRuntime> makeRuntime(){
        return std::make_unique<t::ParserRuntime>(context,functions,regexp,unicode,input,findTeX,&modules);
    }
};

suite mmlFactoryContextFields = [] {
  "bound_native_mml_constructor_preserves_token_children_and_attributes"_test = [] {
    Graph graph;
    auto factory=std::make_shared<m::MmlFactory>();
    (void)graph.runtime.bindMmlFactory(factory);
    auto* text=dynamic_cast<m::TextNode*>(factory->create("text",tree::FactoryArgs{}));
    expect(text!=nullptr);
    if(!text)return;
    text->setText(u"\u221a");
    auto children=std::make_shared<tree::ChildArray<m::MmlNode>>();
    children->push_back(m::NodeValue(text));
    folia::math::PropertyList properties;
    properties.set("stretchy",folia::math::Property(true));
    const std::array<std::any,2> args{std::any(properties),std::any(m::Children(children))};
    auto* token=dynamic_cast<m::AbstractMmlTokenNode*>(factory->create("mo",tree::FactoryArgs(args)));
    expect(token!=nullptr);
    if(!token)return;
    expect(token->attributes()->get("stretchy")==folia::math::Property(true));
    expect(token->childArrayOwner()->size()==1u);
    expect(token->childArrayOwner()->get(0)==m::NodeValue(text));
    expect(token->getText()==u"\u221a");
  };
  "mml_attributes_native_cell_activates_at_the_source_constructor_assignment"_test = [] {
    Graph graph;
    auto factory=std::make_shared<m::MmlFactory>();
    const auto factoryIdentity=graph.runtime.bindMmlFactory(factory);
    const auto map=std::get<o::ObjectRef>(graph.context.get(o::Value(factoryIdentity),o::Key(u"nodeMap")).data);
    const auto constructor=std::get<o::ObjectRef>(graph.runtime.maps.resolve(map)->get(o::Value(u"mrow")).data);
    const auto prototype=std::get<o::ObjectRef>(graph.context.get(o::Value(constructor),o::Key(u"prototype")).data);
    const auto receiver=graph.context.objectWithPrototype(prototype);
    const auto originalGetClass=graph.context.get(o::Value(factoryIdentity),o::Key(u"getNodeClass"));
    std::size_t beforeAttributes=0,setListCalls=0;
    const auto getClass=graph.context.function(u"getNodeClass",
      [&](o::Context& context,o::Value self,std::span<const o::Value> args){
        expect(!context.hasOwn(o::Value(receiver),o::Key(u"attributes")))
          << "class defaults are read before the Source attributes assignment";
        ++beforeAttributes;
        return context.call(std::get<o::ObjectRef>(originalGetClass.data),self,args);
      });
    graph.context.set(o::Value(factoryIdentity),o::Key(u"getNodeClass"),o::Value(getClass));
    const auto attributesPrototype=std::get<o::ObjectRef>(graph.context.get(
      o::Value(graph.runtime.attributeBindings.constructor()),o::Key(u"prototype")).data);
    const auto originalSetList=graph.context.get(o::Value(attributesPrototype),o::Key(u"setList"));
    const auto setList=graph.context.function(u"setList",
      [&](o::Context& context,o::Value self,std::span<const o::Value> args){
        ++setListCalls;
        auto* node=graph.runtime.mmlNodeIdentities.resolve(receiver);
        const auto native=node->attributes();
        expect(native!=nullptr) << "the native owner is available during Source setList";
        const auto* identity=std::get_if<o::ObjectRef>(&self.data);
        expect(identity!=nullptr);
        if(identity)expect(native==graph.runtime.attributeBindings.resolve(*identity))
          << "Source and native access share the exact Attributes owner";
        return context.call(std::get<o::ObjectRef>(originalSetList.data),self,args);
      });
    graph.context.set(o::Value(attributesPrototype),o::Key(u"setList"),o::Value(setList));
    const std::array<o::Value,1> arguments{o::Value(factoryIdentity)};
    (void)graph.context.call(constructor,o::Value(receiver),arguments);
    expect(beforeAttributes==2u);
    expect(setListCalls==1u);
  };

  "mml_class_statics_survive_other_parser_runtime_retirement"_test = [] {
    diagnoseMmlFailure("shared class runtime lifetime",[&]{
    SharedRuntimeGraph graph;
    auto firstRuntime=graph.makeRuntime();
    auto firstFactory=std::make_shared<m::MmlFactory>();
    const auto firstIdentity=firstRuntime->bindMmlFactory(firstFactory);
    const auto firstMap=std::get<o::ObjectRef>(graph.context.get(
        o::Value(firstIdentity),o::Key(u"nodeMap")).data);
    const auto firstCtor=std::get<o::ObjectRef>(firstRuntime->maps.resolve(firstMap)->get(o::Value(u"mrow")).data);
    const auto firstDefaults=std::get<o::ObjectRef>(graph.context.get(
        o::Value(firstCtor),o::Key(u"defaults")).data);
    graph.context.set(o::Value(firstDefaults),o::Key(u"mathvariant"),o::Value(u"bold"));

    auto laterRuntime=graph.makeRuntime();
    auto laterFactory=std::make_shared<m::MmlFactory>();
    const auto laterIdentity=laterRuntime->bindMmlFactory(laterFactory);
    const auto laterMap=std::get<o::ObjectRef>(graph.context.get(
        o::Value(laterIdentity),o::Key(u"nodeMap")).data);
    const auto laterCtor=std::get<o::ObjectRef>(laterRuntime->maps.resolve(laterMap)->get(o::Value(u"mrow")).data);
    expect(firstCtor==laterCtor);
    laterRuntime.reset();

    auto* firstNode=dynamic_cast<m::AbstractMmlNode*>(firstFactory->create("mrow",tree::FactoryArgs{}));
    expect(firstNode!=nullptr);
    if(firstNode)expect(firstNode->attributes()->getDefault("mathvariant")==folia::math::Property(std::string("bold")));

    auto newestRuntime=graph.makeRuntime();
    auto newestFactory=std::make_shared<m::MmlFactory>();
    const auto newestIdentity=newestRuntime->bindMmlFactory(newestFactory);
    const auto newestMap=std::get<o::ObjectRef>(graph.context.get(
        o::Value(newestIdentity),o::Key(u"nodeMap")).data);
    const auto newestCtor=std::get<o::ObjectRef>(newestRuntime->maps.resolve(newestMap)->get(o::Value(u"mrow")).data);
    expect(newestCtor==firstCtor);
    firstRuntime.reset();
    auto* newestNode=dynamic_cast<m::AbstractMmlNode*>(newestFactory->create("mrow",tree::FactoryArgs{}));
    expect(newestNode!=nullptr);
    if(newestNode)expect(newestNode->attributes()->getDefault("mathvariant")==folia::math::Property(std::string("bold")));
    });
  };

  "mml_class_static_defaults_and_accents_are_per_context_live_class_owners"_test = [] {
    Graph graph;
    auto firstFactory=std::make_shared<m::MmlFactory>();
    auto secondFactory=std::make_shared<m::MmlFactory>();
    const auto firstIdentity=graph.runtime.bindMmlFactory(firstFactory);
    const auto secondIdentity=graph.runtime.bindMmlFactory(secondFactory);
    const auto firstMap=std::get<o::ObjectRef>(graph.context.get(o::Value(firstIdentity),o::Key(u"nodeMap")).data);
    const auto secondMap=std::get<o::ObjectRef>(graph.context.get(o::Value(secondIdentity),o::Key(u"nodeMap")).data);
    auto constructor=[&](o::ObjectRef map,std::u16string_view kind){
      return std::get<o::ObjectRef>(graph.runtime.maps.resolve(map)->get(o::Value(std::u16string(kind))).data);
    };
    const auto mrow=constructor(firstMap,u"mrow");
    const auto inferred=constructor(secondMap,u"inferredMrow");
    expect(mrow==constructor(secondMap,u"mrow"));
    expect(graph.context.get(o::Value(mrow),o::Key(u"defaults"))==
           graph.context.get(o::Value(inferred),o::Key(u"defaults")));
    const auto defaultsDescriptor=graph.context.descriptor(o::Value(mrow),o::Key(u"defaults"));
    expect(defaultsDescriptor.has_value());
    if(defaultsDescriptor)
      expect(!defaultsDescriptor->accessor&&
             defaultsDescriptor->enumerable&&defaultsDescriptor->writable&&defaultsDescriptor->configurable);

    const auto munder=constructor(firstMap,u"munder");
    const auto munderover=constructor(secondMap,u"munderover");
    const auto mover=constructor(firstMap,u"mover");
    expect(!graph.context.hasOwn(o::Value(munder),o::Key(u"ACCENTS")));
    expect(graph.context.prototype(munder).has_value()&&graph.context.prototype(munder).value()==munderover);
    expect(graph.context.get(o::Value(munder),o::Key(u"ACCENTS"))==
           graph.context.get(o::Value(munderover),o::Key(u"ACCENTS")));
    expect(graph.context.hasOwn(o::Value(mover),o::Key(u"ACCENTS")));
    const auto accentsDescriptor=graph.context.descriptor(o::Value(mover),o::Key(u"ACCENTS"));
    expect(accentsDescriptor.has_value());
    if(accentsDescriptor)
      expect(!accentsDescriptor->accessor&&accentsDescriptor->enumerable&&
             accentsDescriptor->writable&&accentsDescriptor->configurable);
    expect(!graph.context.hasOwn(o::Value(constructor(firstMap,u"mlabeledtr")),o::Key(u"defaults")));
    expect(!graph.context.hasOwn(o::Value(constructor(firstMap,u"text")),o::Key(u"defaults")));
    expect(!graph.context.hasOwn(o::Value(constructor(firstMap,u"XML")),o::Key(u"defaults")));
    expect(!graph.context.hasOwn(o::Value(constructor(firstMap,u"html")),o::Key(u"defaults")));

    const auto defaults=std::get<o::ObjectRef>(graph.context.get(o::Value(mrow),o::Key(u"defaults")).data);
    graph.context.set(o::Value(defaults),o::Key(u"mathvariant"),o::Value(u"bold"));
    auto* typed=dynamic_cast<m::AbstractMmlNode*>(firstFactory->create("mrow",tree::FactoryArgs{}));
    expect(typed!=nullptr);
    if(typed)expect(typed->attributes()->getDefault("mathvariant")==folia::math::Property(std::string("bold")));

    const auto replacement=graph.context.object();
    graph.context.set(o::Value(replacement),o::Key(u"mathvariant"),o::Value(u"script"));
    graph.context.set(o::Value(mrow),o::Key(u"defaults"),o::Value(replacement));
    auto* replaced=dynamic_cast<m::AbstractMmlNode*>(firstFactory->create("mrow",tree::FactoryArgs{}));
    expect(replaced!=nullptr);
    if(replaced)expect(replaced->attributes()->getDefault("mathvariant")==folia::math::Property(std::string("script")));

    Graph isolated;
    auto isolatedFactory=std::make_shared<m::MmlFactory>();
    const auto isolatedIdentity=isolated.runtime.bindMmlFactory(isolatedFactory);
    const auto isolatedMap=std::get<o::ObjectRef>(isolated.context.get(o::Value(isolatedIdentity),o::Key(u"nodeMap")).data);
    const auto isolatedMrow=std::get<o::ObjectRef>(isolated.runtime.maps.resolve(isolatedMap)->get(o::Value(u"mrow")).data);
    expect(isolatedMrow!=mrow);
    expect(isolated.context.get(o::Value(isolatedMrow),o::Key(u"defaults"))!=
           graph.context.get(o::Value(mrow),o::Key(u"defaults")));
  };

  "mml_creator_record_enumerability_reads_only_live_descriptor_metadata"_test = [] {
    Graph graph;
    auto factory=std::make_shared<m::MmlFactory>();
    const auto factoryIdentity=graph.runtime.bindMmlFactory(factory);
    const auto recordValue=graph.context.get(factoryIdentity,o::Key(u"node"));
    const auto* record=std::get_if<o::ObjectRef>(&recordValue.data);
    expect(record!=nullptr);
    if(!record)return;

    std::size_t getterCalls=0;
    const auto getter=graph.context.function(u"getProbe",[&](o::Context&,o::Value,
        std::span<const o::Value>){++getterCalls;return o::Value(31.0);});
    const auto enumerableKey=o::Key(u"metadata-enumerable");
    const auto hiddenKey=o::Key(u"metadata-hidden");
    graph.context.define(*record,enumerableKey,o::Descriptor::access(getter,std::nullopt,true,true));
    graph.context.define(*record,hiddenKey,o::Descriptor::access(getter,std::nullopt,false,true));

    expect(graph.context.ownPropertyEnumerable(recordValue,enumerableKey));
    expect(!graph.context.ownPropertyEnumerable(recordValue,hiddenKey));
    expect(getterCalls==0u);
    expect(graph.context.get(recordValue,enumerableKey)==o::Value(31.0));
    expect(getterCalls==1u);
  };

  "factory_arguments_unwrap_live_context_property_records_without_coercing_other_values"_test = [] {
    Graph graph;
    const auto source=graph.context.object();
    const std::array<o::Value,2> recordArgs{o::Value(source),o::Value(o::Undefined{})};
    auto native=m::mmlFactoryContextArguments(graph.runtime.properties,graph.runtime.nodeArrayIdentities,
                                                graph.runtime.treeNodeBindings,recordArgs);
    auto* record=std::any_cast<folia::math::PropertyList>(&native[0]);
    expect(record!=nullptr);
    if(record){
      record->set("native-write",folia::math::Property(17.0));
      expect(graph.context.get(o::Value(source),o::Key(u"native-write"))==o::Value(17.0));
      graph.context.set(o::Value(source),o::Key(u"context-write"),o::Value(23.0));
      expect(record->get("context-write")==folia::math::Property(23.0));
    }

    const std::array<o::Value,2> nullArgs{o::Value(o::Null{}),o::Value(o::Undefined{})};
    auto nullNative=m::mmlFactoryContextArguments(graph.runtime.properties,graph.runtime.nodeArrayIdentities,
                                                  graph.runtime.treeNodeBindings,nullArgs);
    const auto* nullValue=std::any_cast<folia::math::Property>(&nullNative[0]);
    expect(nullValue&&std::holds_alternative<folia::math::PropertyNull>(nullValue->value));

    const std::array<o::Value,2> primitiveArgs{o::Value(u"abc"),o::Value(o::Undefined{})};
    auto primitiveNative=m::mmlFactoryContextArguments(graph.runtime.properties,graph.runtime.nodeArrayIdentities,
                                                        graph.runtime.treeNodeBindings,primitiveArgs);
    const auto* primitive=std::any_cast<folia::math::Property>(&primitiveNative[0]);
    expect(primitive&&std::get_if<std::string>(&primitive->value)!=nullptr);
    if(primitive)if(const auto* text=std::get_if<std::string>(&primitive->value))expect(*text=="abc");
  };

  "bound_native_initialization_converts_lexical_factory_and_constructor_arguments_once"_test = [] {
    diagnoseMmlFailure("25 bound native initialization",[&]{
    Graph graph;
    auto factory=std::make_shared<m::MmlFactory>();
    const auto factoryIdentity=graph.runtime.bindMmlFactory(factory);
    auto otherFactory=std::make_shared<m::MmlFactory>();
    const auto otherFactoryIdentity=graph.runtime.bindMmlFactory(otherFactory);
    std::size_t factoryGetterCalls=0;
    const auto factoryGetter=graph.context.function(u"factory getter",
        [&factoryGetterCalls,otherFactoryIdentity](o::Context&,o::Value,std::span<const o::Value>){
          ++factoryGetterCalls;return o::Value(otherFactoryIdentity);
        });
    const auto factorySetter=graph.context.function(u"factory setter",
        [](o::Context&,o::Value,std::span<const o::Value>){return o::Value(o::Undefined{});});
    graph.context.define(graph.runtime.mmlNodeContextMethods.mmlNodePrototype(),o::Key(u"factory"),
        o::Descriptor::access(factoryGetter,factorySetter));

    auto* child=dynamic_cast<m::AbstractMmlNode*>(factory->create("mi",tree::FactoryArgs{}));
    expect(child!=nullptr) << "MML test 25 assertion at source line 224";
    if(!child)return;
    expect(std::isinf(child->arity())) << "MML test 25 assertion at source line 226";
    expect(child->childArrayOwner()->empty()) << "MML test 25 assertion at source line 227";

    folia::math::PropertyList attributes;
    attributes.set("mathcolor",folia::math::Property(std::string("red")));
    auto children=std::make_shared<tree::ChildArray<m::MmlNode>>();
    (void)children->push_back(tree::ChildValue<m::MmlNode>(child));
    auto* node=dynamic_cast<m::AbstractMmlNode*>(factory->create("merror",attributes,children));
    expect(node!=nullptr) << "MML test 25 assertion at source line 234";
    if(!node)return;
    expect(node->arity()==-1.0) << "MML test 25 assertion at source line 236";
    expect(node->attributes()->get("mathcolor")==folia::math::Property(std::string("red"))) << "MML test 25 assertion at source line 237";
    expect(factoryGetterCalls==0u) << "MML test 25 assertion at source line 238";

    expect(node->childArrayOwner()->size()==1u) << "MML test 25 assertion at source line 240";
    if(node->childArrayOwner()->size()==1u){
      auto* inferred=node->childArrayOwner()->get(0).nodeOrNull();
      expect(inferred!=nullptr) << "MML test 25 assertion at source line 243";
      if(inferred){
        expect(inferred->childArrayOwner()->size()==1u) << "MML test 25 assertion at source line 245";
        if(inferred->childArrayOwner()->size()==1u)
          expect(inferred->childArrayOwner()->get(0).nodeOrNull()==child) << "MML test 25 assertion at source line 247";
      }
    }

    const auto identity=graph.runtime.mmlNodeIdentities.findPointee(node);
    expect(identity.has_value()) << "MML test 25 assertion at source line 252";
    if(!identity)return;
    const auto nodePrototype=graph.context.prototype(*identity);
    expect(nodePrototype.has_value()) << "MML test 25 bound node has a canonical class prototype";
    expect(!graph.context.hasOwn(o::Value(*identity),o::Key(u"factory")))
        << "MML test 25 inherited factory accessor remains visible";
    const auto rawChildren=graph.context.get(o::Value(*identity),o::Key(u"childNodes"));
    const auto* array=std::get_if<o::ObjectRef>(&rawChildren.data);
    expect(array!=nullptr) << "MML test 25 assertion at source line 256";
    if(array)expect(graph.runtime.treeNodeBindings.arrayView(o::Value(*array))==node->childArrayOwner()) << "MML test 25 assertion at source line 257";
    expect(factoryGetterCalls==0u) << "MML test 25 assertion at source line 258";
    expect(graph.context.get(o::Value(*identity),o::Key(u"factory"))==o::Value(otherFactoryIdentity)) << "MML test 25 assertion at source line 259";
    expect(factoryGetterCalls==1u) << "MML test 25 assertion at source line 260";
    expect(factoryIdentity!=otherFactoryIdentity) << "MML test 25 assertion at source line 261";
    });
  };

  "bound_native_children_use_live_core_tree_setChildren_and_canonical_array_identity"_test = [] {
    Graph graph;
    auto factory=std::make_shared<m::MmlFactory>();
    const auto factoryIdentity=graph.runtime.bindMmlFactory(factory);
    const auto nodeMap=std::get<o::ObjectRef>(graph.context.get(factoryIdentity,o::Key(u"nodeMap")).data);
    const auto miCtor=std::get<o::ObjectRef>(graph.runtime.maps.resolve(nodeMap)->get(o::Value(u"mi")).data);
    const std::array<o::Value,3> ctorArgs{o::Value(factoryIdentity),o::Value(graph.context.object()),
                                          o::Value(graph.context.array(0))};
    const auto identity=graph.context.construct(miCtor,ctorArgs);
    auto* node=dynamic_cast<m::AbstractMmlNode*>(graph.runtime.mmlNodeIdentities.resolve(identity));
    expect(node!=nullptr);
    if(!node)return;

    const auto corePrototype=graph.runtime.treeNodeClasses.abstractNodePrototype();
    const auto original=graph.context.get(o::Value(corePrototype),o::Key(u"setChildren"));
    const auto children=std::make_shared<tree::ChildArray<m::MmlNode>>();
    auto* child=factory->create("mn");
    (void)children->push_back(tree::ChildValue<m::MmlNode>(child));
    const auto expectedArray=t::bindNodeArray(graph.runtime,children);
    std::size_t calls=0;
    const auto replacement=graph.context.function(u"setChildren",
        [&calls,identity,expectedArray](o::Context&,o::Value receiver,std::span<const o::Value> args){
            ++calls;
            expect(receiver==o::Value(identity));
            expect(!args.empty()&&args.front()==expectedArray);
            return o::Value(u"source-sentinel");
        });
    graph.context.set(o::Value(corePrototype),o::Key(u"setChildren"),o::Value(replacement));
    node->setChildren(children);
    expect(calls==1u);
    graph.context.set(o::Value(corePrototype),o::Key(u"setChildren"),original);
  };

  "native_unbound_mml_append_keeps_foreign_child_arrays_and_symbol_properties_live"_test = [] {
    Graph graph;
    auto factory=std::make_shared<m::MmlFactory>();
    auto* owner=dynamic_cast<m::AbstractMmlNode*>(factory->create("msup"));
    expect(owner!=nullptr);
    if(!owner)return;

    const auto symbol=graph.context.symbol(u"foreign-property");
    const auto foreign=graph.context.object();
    graph.context.set(o::Value(foreign),o::Key(u"isInferred"),o::Value(true));
    const auto rawChildren=graph.context.array(0);
    graph.context.set(o::Value(foreign),o::Key(u"childNodes"),o::Value(rawChildren));
    const auto names=graph.context.array(1);
    graph.context.set(o::Value(names),o::Key(u"0"),o::Value(symbol));
    const auto getNames=graph.context.function(u"getPropertyNames",
        [names](o::Context&,o::Value,std::span<const o::Value>){return o::Value(names);});
    const auto getProperty=graph.context.function(u"getProperty",
        [symbol](o::Context&,o::Value,std::span<const o::Value> args){
            return !args.empty()&&args.front()==o::Value(symbol)?o::Value(37.0):o::Value(o::Undefined{});
        });
    graph.context.set(o::Value(foreign),o::Key(u"getPropertyNames"),o::Value(getNames));
    graph.context.set(o::Value(foreign),o::Key(u"getProperty"),o::Value(getProperty));

    const auto child=graph.runtime.treeNodeBindings.childValue(o::Value(foreign));
    const auto foreignAccess=child.sourceAccess();
    const auto replacement=owner->appendChild(child);
    auto* row=dynamic_cast<m::AbstractMmlNode*>(replacement.nodeOrNull());
    expect(row!=nullptr);
    if(row){
        expect(row->propertyOwner().getKey(folia::math::Property(symbol))==folia::math::Property(37.0));
        auto view=row->nativeOperationView();
        const auto receiver=view.toDynamic(tree::ChildValue<m::MmlNode>(row));
        const auto getProperty=view.get(receiver,u"getProperty");
        const std::array<m::NativeMmlValue,1> args{m::NativeMmlValue(folia::math::Property(symbol))};
        const auto result=view.invoke(getProperty,receiver,args);
        const auto* property=std::get_if<folia::math::Property>(&result.data());
        expect(property&&*property==folia::math::Property(37.0));
    }
  };

  "native_foreign_property_name_coerces_once_before_property_assignment"_test = [] {
    Graph graph;
    auto factory=std::make_shared<m::MmlFactory>();
    auto* owner=dynamic_cast<m::AbstractMmlNode*>(factory->create("msup"));
    expect(owner!=nullptr);
    if(!owner)return;

    std::size_t coercions=0;
    const auto key=graph.context.object();
    const auto toPrimitive=graph.context.function(u"toPrimitive",
        [&coercions](o::Context&,o::Value,std::span<const o::Value> args){
            expect(!args.empty()&&args.front()==o::Value(u"string"));
            ++coercions;
            return o::Value(coercions==1?u"once":u"twice");
        });
    graph.context.set(o::Value(key),o::Key(graph.context.intrinsics().symbolToPrimitive),
                      o::Value(toPrimitive));
    const auto names=graph.context.array(1);
    graph.context.set(o::Value(names),o::Key(u"0"),o::Value(key));
    const auto foreign=graph.context.object();
    graph.context.set(o::Value(foreign),o::Key(u"isInferred"),o::Value(true));
    graph.context.set(o::Value(foreign),o::Key(u"childNodes"),o::Value(graph.context.array(0)));
    graph.context.set(o::Value(foreign),o::Key(u"getPropertyNames"),o::Value(graph.context.function(u"getPropertyNames",
        [names](o::Context&,o::Value,std::span<const o::Value>){return o::Value(names);} )));
    graph.context.set(o::Value(foreign),o::Key(u"getProperty"),o::Value(graph.context.function(u"getProperty",
        [key](o::Context&,o::Value,std::span<const o::Value> args){
            return !args.empty()&&args.front()==o::Value(key)?o::Value(47.0):o::Value(o::Undefined{});
        } )));

    const auto child=graph.runtime.treeNodeBindings.childValue(o::Value(foreign));
    const auto replacement=owner->appendChild(child);
    auto* row=dynamic_cast<m::AbstractMmlNode*>(replacement.nodeOrNull());
    expect(row!=nullptr);
    if(row)expect(row->propertyOwner().get("once")==folia::math::Property(47.0));
    expect(coercions==1u);
  };

  "bound_property_records_preserve_symbol_owner_in_prototypes_copy_and_assign"_test = [] {
    Graph graph;
    const auto symbol=graph.context.symbol(u"bound-owner-symbol");
    const auto sourceObject=graph.context.object();
    graph.context.define(sourceObject,o::Key(symbol),o::Descriptor::data(o::Value(17.0)));
    const auto source=graph.runtime.properties.from(o::Value(sourceObject));
    const auto sourceList=folia::math::PropertyList::record(source);
    expect(sourceList.has_value());
    if(!sourceList)return;

    // A native record whose prototype is Context-backed must resolve the
    // inherited Symbol on the same canonical Context owner.
    auto derived=folia::math::PropertyList::withPrototype(*sourceList);
    expect(derived.getKey(folia::math::Property(symbol))==folia::math::Property(17.0));

    const auto copied=graph.runtime.properties.copyOwnEnumerableProperties(source);
    expect(folia::math::PropertyList::record(copied)->getKey(folia::math::Property(symbol))==
           folia::math::Property(17.0));

    const auto targetObject=graph.context.object();
    const auto target=graph.runtime.properties.from(o::Value(targetObject));
    auto targetList=folia::math::PropertyList::record(target);
    expect(targetList.has_value());
    if(!targetList)return;
    targetList->assign(*sourceList);
    expect(graph.context.get(o::Value(targetObject),o::Key(symbol))==o::Value(17.0));
  };

  "native_property_storage_projects_symbol_keys_through_context_and_arrays"_test = [] {
    Graph graph;
    const auto symbol=graph.context.symbol(u"native-storage-symbol");
    folia::math::PropertyList record;
    record.set("alpha",folia::math::Property(1.0));
    record.setKey(folia::math::Property(symbol),folia::math::Property(7.0));
    const auto recordValue=graph.runtime.properties.to(folia::math::Property(record));
    const auto* recordObject=std::get_if<o::ObjectRef>(&recordValue.data);
    expect(recordObject!=nullptr);
    if(recordObject){
        expect(graph.context.get(*recordObject,o::Key(symbol))==o::Value(7.0));
        const auto keys=graph.context.ownKeys(o::Value(*recordObject));
        expect(keys.size()==2);
        expect(keys.size()<2||keys[0]==o::Key(u"alpha"));
        expect(keys.size()<2||keys[1]==o::Key(symbol));
        graph.context.set(o::Value(*recordObject),o::Key(symbol),o::Value(9.0));
        expect(record.getKey(folia::math::Property(symbol))==folia::math::Property(9.0));
        graph.context.erase(*recordObject,o::Key(symbol));
        expect(record.getKey(folia::math::Property(symbol)).missing());
    }

    folia::math::PropertyArray array{folia::math::Property(3.0)};
    auto arrayRecord=array.record();
    arrayRecord.setKey(folia::math::Property(symbol),folia::math::Property(11.0));
    const auto arrayValue=graph.runtime.properties.to(folia::math::Property(array));
    const auto* arrayObject=std::get_if<o::ObjectRef>(&arrayValue.data);
    expect(arrayObject!=nullptr);
    if(arrayObject){
        expect(graph.context.get(*arrayObject,o::Key(symbol))==o::Value(11.0));
        const auto keys=graph.context.ownKeys(o::Value(*arrayObject));
        expect(std::find(keys.begin(),keys.end(),o::Key(symbol))!=keys.end());
        graph.context.set(o::Value(*arrayObject),o::Key(symbol),o::Value(12.0));
        expect(arrayRecord.getKey(folia::math::Property(symbol))==folia::math::Property(12.0));
    }

    folia::math::PropertyList defaults;
    defaults.setKey(folia::math::Property(symbol),folia::math::Property(19.0));
    const auto defaultsValue=graph.runtime.properties.to(folia::math::Property(defaults));
    const std::array<o::Value,2> attributesArgs{defaultsValue,o::Value(o::Null{})};
    const auto attributes=graph.context.construct(graph.runtime.attributeBindings.constructor(),attributesArgs);
    const auto defaultsLayer=graph.context.get(attributes,o::Key(u"defaults"));
    const auto* defaultsObject=std::get_if<o::ObjectRef>(&defaultsLayer.data);
    expect(defaultsObject!=nullptr);
    if(defaultsObject){
        expect(graph.context.get(*defaultsObject,o::Key(symbol))==o::Value(19.0));
        const auto own=graph.context.ownKeys(defaultsLayer);
        expect(std::find(own.begin(),own.end(),o::Key(symbol))!=own.end());
        expect(graph.context.enumerableStringKeys(*defaultsObject).empty());
    }
  };

  "native_mml_property_key_uses_string_hint_and_preserves_symbol_result"_test = [] {
    Graph graph;
    auto factory=std::make_shared<m::MmlFactory>();
    auto* parent=dynamic_cast<m::AbstractMmlNode*>(factory->create("msup"));
    expect(parent!=nullptr);
    if(!parent)return;
    const auto expected=graph.context.symbol(u"to-property-key");
    const auto keyObject=graph.context.object();
    std::vector<std::u16string> hints;
    const auto toPrimitive=graph.context.function(u"toPrimitive",
        [&hints,expected](o::Context&,o::Value,std::span<const o::Value> args){
            hints.push_back(args.empty()?u"missing":std::get<std::u16string>(args.front().data));
            return o::Value(expected);
        });
    graph.context.set(o::Value(keyObject),o::Key(graph.context.intrinsics().symbolToPrimitive),
                      o::Value(toPrimitive));
    const auto foreign=graph.context.object();
    const auto names=graph.context.array(1);
    graph.context.set(o::Value(names),o::Key(u"0"),o::Value(keyObject));
    const auto getNames=graph.context.function(u"getPropertyNames",
        [names](o::Context&,o::Value,std::span<const o::Value>){return o::Value(names);});
    const auto getProperty=graph.context.function(u"getProperty",
        [keyObject](o::Context&,o::Value,std::span<const o::Value> args){
            return !args.empty()&&args.front()==o::Value(keyObject)?o::Value(23.0):o::Value(o::Undefined{});
        });
    graph.context.set(o::Value(foreign),o::Key(u"isInferred"),o::Value(true));
    graph.context.set(o::Value(foreign),o::Key(u"childNodes"),o::Value(graph.context.array(0)));
    graph.context.set(o::Value(foreign),o::Key(u"attributes"),o::Value(o::Null{}));
    graph.context.set(o::Value(foreign),o::Key(u"getPropertyNames"),o::Value(getNames));
    graph.context.set(o::Value(foreign),o::Key(u"getProperty"),o::Value(getProperty));
    const auto foreignAccess=graph.runtime.treeNodeBindings.nodeAccess(o::Value(keyObject));
    (void)parent->appendChild(graph.runtime.treeNodeBindings.childValue(o::Value(foreign)));
    auto* replacement=dynamic_cast<m::AbstractMmlNode*>(parent->childArrayOwner()->get(0).nodeOrNull());
    expect(replacement!=nullptr);
    if(replacement)expect(replacement->propertyOwner().getKey(folia::math::Property(expected))==
                           folia::math::Property(23.0));
    if(replacement){
      auto view=replacement->nativeOperationView();
      const auto receiver=view.toDynamic(tree::ChildValue<m::MmlNode>(replacement));
      const auto getProperty=view.get(receiver,u"getProperty");
      const std::array<m::NativeMmlValue,1> args{
          m::NativeMmlValue(m::MmlSourceValue<m::MmlNode>{o::Value(keyObject),foreignAccess})};
      const auto found=view.invoke(getProperty,receiver,args);
      const auto* property=std::get_if<folia::math::Property>(&found.data());
      expect(property&&*property==folia::math::Property(23.0));
    }
    expect(equalVector(hints,std::vector<std::u16string>{u"string",u"string"}));
  };

  "inferred_mml_child_operations_discard_core_override_returns_as_source_does"_test = [] {
    Graph graph;
    auto factory=std::make_shared<m::MmlFactory>();
    const auto factoryIdentity=graph.runtime.bindMmlFactory(factory);
    const auto nodeMap=std::get<o::ObjectRef>(graph.context.get(factoryIdentity,o::Key(u"nodeMap")).data);
    const auto mathCtor=std::get<o::ObjectRef>(graph.runtime.maps.resolve(nodeMap)->get(o::Value(u"math")).data);
    const std::array<o::Value,3> ctorArgs{o::Value(factoryIdentity),o::Value(graph.context.object()),
                                          o::Value(graph.context.array(0))};
    const auto math=graph.context.construct(mathCtor,ctorArgs);
    const auto childNodes=std::get<o::ObjectRef>(graph.context.get(math,o::Key(u"childNodes")).data);
    const auto first=std::get<o::ObjectRef>(graph.context.get(o::Value(childNodes),o::Key(u"0")).data);
    const auto appendSentinel=graph.context.object();
    const auto replaceSentinel=graph.context.object();
    const auto setChildrenSentinel=graph.context.object();
    const auto append=graph.context.function(u"appendChild",
        [appendSentinel](o::Context&,o::Value,std::span<const o::Value>){return o::Value(appendSentinel);});
    const auto replace=graph.context.function(u"replaceChild",
        [replaceSentinel](o::Context&,o::Value,std::span<const o::Value>){return o::Value(replaceSentinel);});
    const auto setChildren=graph.context.function(u"setChildren",
        [setChildrenSentinel](o::Context&,o::Value,std::span<const o::Value>){return o::Value(setChildrenSentinel);});
    graph.context.set(o::Value(first),o::Key(u"appendChild"),o::Value(append));
    graph.context.set(o::Value(first),o::Key(u"replaceChild"),o::Value(replace));
    graph.context.set(o::Value(first),o::Key(u"setChildren"),o::Value(setChildren));
    const auto input=graph.context.object();
    const auto appendMethod=graph.context.get(math,o::Key(u"appendChild"));
    const std::array<o::Value,1> appendArgs{o::Value(input)};
    expect(graph.context.callMember(appendMethod,math,appendArgs)==o::Value(input));
    const auto replaceMethod=graph.context.get(math,o::Key(u"replaceChild"));
    const std::array<o::Value,2> replaceArgs{o::Value(input),o::Value(first)};
    expect(graph.context.callMember(replaceMethod,math,replaceArgs)==o::Value(input));
    const auto setChildrenMethod=graph.context.get(math,o::Key(u"setChildren"));
    const std::array<o::Value,1> childrenArgs{o::Value(graph.context.array(0))};
    expect(graph.context.callMember(setChildrenMethod,math,childrenArgs)==o::Value(setChildrenSentinel));
  };

  "mml_copy_reads_single_child_index_again_when_unwrapping_inferred_row"_test = [] {
    Graph graph;
    auto factory=std::make_shared<m::MmlFactory>();
    const auto factoryIdentity=graph.runtime.bindMmlFactory(factory);
    const auto nodeMap=std::get<o::ObjectRef>(graph.context.get(factoryIdentity,o::Key(u"nodeMap")).data);
    const auto miCtor=std::get<o::ObjectRef>(graph.runtime.maps.resolve(nodeMap)->get(o::Value(u"mi")).data);
    const std::array<o::Value,3> ctorArgs{o::Value(factoryIdentity),o::Value(graph.context.object()),
                                          o::Value(graph.context.array(0))};
    const auto node=graph.context.construct(miCtor,ctorArgs);
    const auto conditionNode=graph.context.object();
    graph.context.set(o::Value(conditionNode),o::Key(u"isInferred"),o::Value(true));
    const auto copiedChild=graph.context.object();
    graph.context.set(o::Value(copiedChild),o::Key(u"isInferred"),o::Value(false));
    const auto copyFunction=graph.context.function(u"copy",
        [copiedChild](o::Context&,o::Value,std::span<const o::Value>){return o::Value(copiedChild);});
    const auto branchChild=graph.context.object();
    const auto branchChildren=graph.context.array(1);
    graph.context.set(o::Value(branchChildren),o::Key(u"0"),o::Value(branchChild));
    graph.context.set(o::Value(branchChild),o::Key(u"copy"),o::Value(copyFunction));
    graph.context.set(o::Value(branchChild),o::Key(u"isInferred"),o::Value(false));
    graph.context.set(o::Value(branchChild),o::Key(u"childNodes"),o::Value(graph.context.array(0)));
    const auto branch=graph.context.object();
    graph.context.set(o::Value(branch),o::Key(u"childNodes"),o::Value(branchChildren));

    std::size_t indexReads=0;
    const auto changingIndex=graph.context.function(u"get",
        [&indexReads,conditionNode,branch](o::Context&,o::Value,std::span<const o::Value>){
            return o::Value(indexReads++==0?conditionNode:branch);
        });
    const auto iteration=graph.context.object();
    graph.context.define(iteration,o::Key(u"length"),o::Descriptor::data(o::Value(1.0)));
    graph.context.define(iteration,o::Key(u"0"),o::Descriptor::access(changingIndex,std::nullopt,true,true));
    graph.context.set(node,o::Key(u"childNodes"),o::Value(iteration));

    const auto copy=graph.context.get(node,o::Key(u"copy"));
    const auto result=graph.context.callMember(copy,node,{});
    expect(indexReads==2u);
    const auto resultChildren=std::get<o::ObjectRef>(graph.context.get(result,o::Key(u"childNodes")).data);
    expect(graph.context.get(o::Value(resultChildren),o::Key(u"0"))==o::Value(copiedChild));
  };

  "mml_factory_projects_exact_fields_and_all_canonical_mml_classes"_test = [] {
    diagnoseMmlFailure("34 canonical MML classes",[&]{
    Graph graph;
    auto factory=std::make_shared<m::MmlFactory>();
    const auto identity=graph.runtime.bindMmlFactory(factory);
    expect(equalVector(graph.context.enumerableStringKeys(identity),
        std::vector<std::u16string>{u"defaultKind",u"nodeMap",u"node"})) << "MML test 34 assertion at source line 584";
    const auto node=graph.context.get(identity,o::Key(u"node"));
    expect(node==graph.context.get(identity,o::Key(u"MML"))) << "MML test 34 assertion at source line 587";
    const auto map=std::get<o::ObjectRef>(graph.context.get(identity,o::Key(u"nodeMap")).data);
    const auto storage=graph.runtime.maps.resolve(map);
    expect(storage->size()==43u) << "MML test 34 assertion at source line 590";
    for(const auto& entry:storage->entries()){
      const auto* name=std::get_if<std::u16string>(&entry.key.data);
      expect(name!=nullptr) << "MML test 34 assertion at source line 593";
      if(!name)continue;
      const auto value=entry.value;
      const auto* constructor=std::get_if<o::ObjectRef>(&value.data);
      expect(constructor!=nullptr) << "MML test 34 assertion at source line 597";
      if(!constructor)continue;
      const auto* descriptor=std::any_cast<tree::FactoryClassRef<m::MmlNode>>(
          &graph.context.payload(*constructor));
      expect(descriptor!=nullptr) << "MML test 34 assertion at source line 601";
      if(!descriptor||!*descriptor)continue;
      const auto* info=std::any_cast<std::shared_ptr<m::MmlClassInfo>>(&(*descriptor)->staticData);
      expect(info!=nullptr&&*info) << "MML test 34 assertion at source line 604";
      if(!info||!*info)continue;
      expect((*info)->sourceKind==*name) << "MML test 34 assertion at source line 606";
      expect((*info)->sourceConstructorName.size()>0u) << "MML test 34 assertion at source line 607";
      const auto length=graph.context.get(o::Value(*constructor),o::Key(u"length"));
      expect(length==o::Value((*info)->sourceConstructorArity)) << "MML test 34 assertion at source line 609";
      const auto prototype=std::get<o::ObjectRef>(graph.context.get(
          o::Value(*constructor),o::Key(u"prototype")).data);
      const auto kind=graph.context.descriptor(o::Value(prototype),o::Key(u"kind"));
      expect(kind.has_value()&&kind->accessor&&!kind->enumerable&&kind->configurable) << "MML test 34 assertion at source line 613";
    const auto actualParent=graph.context.prototype(prototype).value();
      const auto parentConstructor=std::get<o::ObjectRef>(graph.context.get(
        o::Value(actualParent),o::Key(u"constructor")).data);
      expect(graph.context.get(o::Value(parentConstructor),o::Key(u"name"))==
          o::Value((*info)->sourcePrototypeParent)) << "MML test 34 assertion at source line 617";
      expect(graph.context.prototype(*constructor).value()==parentConstructor) << "MML test 34 assertion at source line 619";
      expect(graph.context.get(o::Value(parentConstructor),o::Key(u"prototype"))==
          o::Value(actualParent)) << "MML test 34 assertion at source line 620";
    }
    });
  };

  "native_mml_deferred_forEach_callback_checks_factory_lifetime"_test = [] {
    Graph graph;
    auto owner=std::make_shared<m::MmlFactory>();
    const std::weak_ptr<m::MmlFactory> weakOwner=owner;
    auto* parent=dynamic_cast<m::AbstractMmlNode*>(owner->create("mi"));
    expect(parent!=nullptr);
    if(!parent)return;
    const auto foreign=graph.context.object();
    graph.context.set(o::Value(foreign),o::Key(u"isInferred"),o::Value(true));
    const auto rawChildren=graph.context.object();
    std::optional<o::ObjectRef> deferred;
    const auto forEach=graph.context.function(u"forEach",
        [&deferred](o::Context&,o::Value,std::span<const o::Value> args){
            if(!args.empty())if(const auto* callback=std::get_if<o::ObjectRef>(&args.front().data))deferred=*callback;
            return o::Value(o::Undefined{});
        });
    graph.context.set(o::Value(rawChildren),o::Key(u"forEach"),o::Value(forEach));
    graph.context.set(o::Value(foreign),o::Key(u"childNodes"),o::Value(rawChildren));
    const auto sourceChild=graph.runtime.treeNodeBindings.childValue(o::Value(foreign));
    (void)parent->appendChild(sourceChild);
    expect(deferred.has_value());
    if(!deferred)return;
    const auto deferredChild=graph.context.object();
    const std::array<o::Value,3> args{o::Value(deferredChild),o::Value(0.0),o::Value(rawChildren)};
    // This native receiver has no Context identity. Its retained callback is
    // used here to test the factory lease, before it can borrow the receiver.
    owner.reset();
    expect(weakOwner.expired());
    bool rejected=false;
    try{(void)graph.context.call(*deferred,o::Value(o::Undefined{}),args);}
    catch(const o::Exception& error){
        rejected=error.name=="TypeError"&&error.message==u"Native MML receiver owner has expired";
    }
    expect(rejected);

    // Ordinary successful callback invocation uses actual bound identities,
    // so the foreign child's parent can be the exact Context receiver.
    auto boundOwner=std::make_shared<m::MmlFactory>();
    (void)graph.runtime.bindMmlFactory(boundOwner);
    auto* boundParent=boundOwner->create("mi");
    deferred.reset();
    (void)boundParent->appendChild(sourceChild);
    expect(deferred.has_value());
    if(!deferred)return;
    (void)graph.context.call(*deferred,o::Value(o::Undefined{}),args);
    expect(boundParent->childArrayOwner()->size()==1u);
    expect(graph.context.get(o::Value(deferredChild),o::Key(u"parent"))==boundParent->sourceValue());
  };

  "mml_factory_node_map_replacement_is_raw_live_and_uses_its_actual_method_receiver"_test = [] {
    Graph graph;
    auto factory=std::make_shared<m::MmlFactory>();
    const auto identity=graph.runtime.bindMmlFactory(factory);
    const auto originalMap=std::get<o::ObjectRef>(graph.context.get(identity,o::Key(u"nodeMap")).data);
    const auto mi=graph.runtime.maps.resolve(originalMap)->get(o::Value(u"mi"));
    auto storage=graph.runtime.maps.createStorage();
    for(const auto& entry:graph.runtime.maps.resolve(originalMap)->entries())
        storage->set(entry.key,entry.value);
    storage->set(o::Value(u"mi"),mi);
    const auto replacement=graph.runtime.maps.bind(storage);
    graph.context.set(identity,o::Key(u"nodeMap"),o::Value(replacement));
    expect(graph.context.get(identity,o::Key(u"nodeMap"))==o::Value(replacement));
    auto* node=factory->create("mi");
    expect(node!=nullptr);
    expect(dynamic_cast<m::MmlMi*>(node)!=nullptr);
    const auto expectedFactoryClass=factory->getNodeClass("mi");
    expect(expectedFactoryClass!=nullptr);

    // Later Map-side edits are consumed by the same live typed access path.
    const auto mrow=graph.runtime.maps.resolve(originalMap)->get(o::Value(u"mrow"));
    graph.runtime.maps.resolve(replacement)->set(o::Value(u"mrow"),mrow);
    expect(factory->getNodeClass("mrow")!=nullptr);
    graph.runtime.maps.resolve(replacement)->erase(o::Value(u"mi"));
    expect(!factory->getNodeClass("mi"));
  };

  "mml_factory_node_map_foreign_receiver_methods_are_called_lazily_and_with_same_this"_test = [] {
    Graph graph;
    auto factory=std::make_shared<m::MmlFactory>();
    const auto identity=graph.runtime.bindMmlFactory(factory);
    const auto nativeMap=std::get<o::ObjectRef>(graph.context.get(identity,o::Key(u"nodeMap")).data);
    const auto constructor=graph.runtime.maps.resolve(nativeMap)->get(o::Value(u"mi"));
    const auto foreign=graph.context.object();
    std::vector<std::u16string> calls;
    o::Value mapped=constructor;
    const auto get=graph.context.function(u"get",[&](o::Context&,o::Value receiver,
        std::span<const o::Value> args){
          expect(receiver==o::Value(foreign));calls.push_back(u"get");
          expect(args.size()==1u&&args[0]==o::Value(u"mi"));return mapped;
        });
    const auto set=graph.context.function(u"set",[&](o::Context&,o::Value receiver,
        std::span<const o::Value> args){
          expect(receiver==o::Value(foreign));calls.push_back(u"set");mapped=args[1];return receiver;
        });
    const auto has=graph.context.function(u"has",[&](o::Context&,o::Value receiver,
        std::span<const o::Value>){expect(receiver==o::Value(foreign));calls.push_back(u"has");return o::Value(true);});
    const auto erase=graph.context.function(u"delete",[&](o::Context&,o::Value receiver,
        std::span<const o::Value>){expect(receiver==o::Value(foreign));calls.push_back(u"delete");return o::Value(true);});
    const auto clear=graph.context.function(u"clear",[&](o::Context&,o::Value receiver,
        std::span<const o::Value>){expect(receiver==o::Value(foreign));calls.push_back(u"clear");return o::Value(o::Undefined{});});
    const auto keys=graph.context.function(u"keys",[&](o::Context& context,o::Value receiver,
        std::span<const o::Value>){
          expect(receiver==o::Value(foreign));calls.push_back(u"keys");
          const auto result=context.array(1);context.set(o::Value(result),o::Key(u"0"),o::Value(u"mi"));return o::Value(result);
        });
    graph.context.set(o::Value(foreign),o::Key(u"get"),o::Value(get));
    graph.context.set(o::Value(foreign),o::Key(u"set"),o::Value(set));
    graph.context.set(o::Value(foreign),o::Key(u"has"),o::Value(has));
    graph.context.set(o::Value(foreign),o::Key(u"delete"),o::Value(erase));
    graph.context.set(o::Value(foreign),o::Key(u"clear"),o::Value(clear));
    graph.context.set(o::Value(foreign),o::Key(u"keys"),o::Value(keys));
    graph.context.set(identity,o::Key(u"nodeMap"),o::Value(foreign));
    expect(graph.context.get(identity,o::Key(u"nodeMap"))==o::Value(foreign));
    const auto typedClass=factory->getNodeClass("mi");
    expect(typedClass!=nullptr);
    expect(factory->getNodeClass("mi")==typedClass);
    auto access=factory->nodeMapAccess();
    expect(access->has(o::Value(u"mi")));
    access->set(o::Value(u"mi"),tree::FactoryClassMapValue<m::MmlNode>::native(
        typedClass));
    expect(access->erase(o::Value(u"mi")));
    expect(equalVector(access->keysValues(),std::vector<o::Value>{o::Value(u"mi")}));
    access->clear();
    expect(equalVector(calls,std::vector<std::u16string>{u"get",u"get",u"has",u"set",u"delete",u"keys",u"clear"}));
  };

  "mml_factory_node_accessor_reentry_keeps_selected_owner_and_uses_new_owner_next_call"_test = [] {
    Graph graph;
    auto factory=std::make_shared<m::MmlFactory>();
    const auto identity=graph.runtime.bindMmlFactory(factory);
    const auto oldNode=graph.context.get(identity,o::Key(u"node"));
    auto* anchor=factory->create("mrow");
    const auto anchorIdentity=graph.runtime.mmlNodeIdentities.find(anchor);
    expect(anchorIdentity.has_value());
    if(!anchorIdentity)return;
    const auto nextNode=graph.context.object();
    int nextCalls=0;
    const auto nextCreator=graph.context.function(u"mi",[&](o::Context&,o::Value receiver,
        std::span<const o::Value>){++nextCalls;expect(receiver==o::Value(nextNode));return o::Value(*anchorIdentity);});
    graph.context.set(o::Value(nextNode),o::Key(u"mi"),o::Value(nextCreator));
    int reads=0;
    const auto get=graph.context.function(u"get",[&](o::Context& context,o::Value,
        std::span<const o::Value>){
          ++reads;
          if(reads==1)context.define(identity,o::Key(u"node"),o::Descriptor::data(o::Value(nextNode)));
          return oldNode;
        });
    graph.context.define(identity,o::Key(u"node"),o::Descriptor::access(get,std::nullopt,true,true));
    auto* selected=dynamic_cast<m::MmlMi*>(factory->create("mi"));
    expect(selected!=nullptr);
    expect(reads==1);
    expect(factory->create("mi")==anchor);
    expect(nextCalls==1);

    graph.context.erase(identity,o::Key(u"node"));
    bool failedAtSourceRead=false;
    try{(void)factory->create("missing");}
    catch(const o::Exception& error){failedAtSourceRead=error.name=="TypeError";}
    expect(failedAtSourceRead);
  };

  "mml_constructor_is_canonical_and_rejects_expired_runtime_lease"_test = [] {
    o::Context context;
    folia::math::OptionFunctionBindings functions{context};
    folia::math::regexp_host::RegExpBindings regexp{context};
    folia::math::Unicode16Normalization unicode;
    folia::math::core::InputJaxModule input{context};
    folia::math::core::FindMathModule findMath{context};
    t::FindTeXModule findTeX{context,findMath,regexp};
    auto runtime=std::make_unique<t::ParserRuntime>(context,functions,regexp,unicode,input,findTeX);
    auto factory=std::make_shared<m::MmlFactory>();
    const auto identity=runtime->bindMmlFactory(factory);
    const auto map=std::get<o::ObjectRef>(context.get(identity,o::Key(u"nodeMap")).data);
    const auto ctor=std::get<o::ObjectRef>(runtime->maps.resolve(map)->get(o::Value(u"mi")).data);
    const auto second=runtime->maps.resolve(map)->get(o::Value(u"mi"));
    expect(second==o::Value(ctor));
    auto storage=runtime->maps.resolve(map);
    std::weak_ptr<o::MapStorage> weakStorage=storage;
    auto access=factory->nodeMapAccess();
    const auto storageIdentity=access->identity();
    storage.reset();
    auto* node=factory->create("mi");
    expect(dynamic_cast<m::MmlMi*>(node)!=nullptr);
    runtime.reset();
    expect(!weakStorage.expired());
    expect(access->identity()==storageIdentity);
    expect(access->has(o::Value(u"mi")))
        << "Retained nodeMap storage supports pure membership after ParserRuntime retirement";
    bool rejected=false;
    try{(void)context.get(identity,o::Key(u"nodeMap"));}
    catch(const o::Exception& error){rejected=error.name=="TypeError";}
    expect(rejected) << "Factory native field requires a live ParserRuntime";
    bool constructorRejected=false;
    const std::array<o::Value,1> constructorArgs{o::Value(identity)};
    try{(void)context.construct(ctor,constructorArgs);}
    catch(const o::Exception& error){constructorRejected=error.name=="TypeError";}
    expect(constructorRejected) << "MML constructor requires a live native allocation provider";
  };

  "mml_projected_map_identity_outlives_provider_but_live_operations_do_not"_test = [] {
    std::shared_ptr<tree::FactoryClassMapAccess<m::MmlNode>> access;
    std::weak_ptr<o::MapStorage> weakStorage;
    const void* canonicalIdentity=nullptr;
    {
      o::Context context;
      folia::math::OptionFunctionBindings functions{context};
      folia::math::regexp_host::RegExpBindings regexp{context};
      folia::math::Unicode16Normalization unicode;
      folia::math::core::InputJaxModule input{context};
      folia::math::core::FindMathModule findMath{context};
      t::FindTeXModule findTeX{context,findMath,regexp};
      auto runtime=std::make_unique<t::ParserRuntime>(context,functions,regexp,unicode,input,findTeX);
      auto factory=std::make_shared<m::MmlFactory>();
      const auto identity=runtime->bindMmlFactory(factory);
      const auto originalMap=std::get<o::ObjectRef>(context.get(identity,o::Key(u"nodeMap")).data);
      const auto mi=runtime->maps.resolve(originalMap)->get(o::Value(u"mi"));
      auto replacementStorage=runtime->maps.createStorage();
      replacementStorage->set(o::Value(u"mi"),mi);
      const auto replacement=runtime->maps.bind(replacementStorage);
      context.set(identity,o::Key(u"nodeMap"),o::Value(replacement));
      auto storage=runtime->maps.resolve(replacement);
      weakStorage=storage;
      access=factory->nodeMapAccess();
      canonicalIdentity=storage->identity();
      expect(access->identity()==canonicalIdentity);
      replacementStorage.reset();
      storage.reset();
    }
    expect(!weakStorage.expired());
    expect(access->identity()==canonicalIdentity);
    bool rejected=false;
    try{(void)access->has(o::Value(u"mi"));}
    catch(const o::Exception& error){rejected=error.name=="TypeError";}
    expect(rejected);
    access.reset();
    expect(weakStorage.expired());
  };

  "mml_constructed_nodes_keep_the_actual_registered_class_prototype"_test = [] {
    diagnoseMmlFailure("41 constructed node prototype",[&]{
    Graph graph;
    auto factory=std::make_shared<m::MmlFactory>();
    const auto factoryIdentity=graph.runtime.bindMmlFactory(factory);
    const auto map=std::get<o::ObjectRef>(graph.context.get(factoryIdentity,o::Key(u"nodeMap")).data);
    const auto constructor=std::get<o::ObjectRef>(graph.runtime.maps.resolve(map)->get(o::Value(u"mi")).data);
    const auto prototype=std::get<o::ObjectRef>(graph.context.get(o::Value(constructor),o::Key(u"prototype")).data);
    const auto properties=graph.context.object();
    const auto children=graph.context.array(0);
    const std::array<o::Value,3> args{o::Value(factoryIdentity),o::Value(properties),o::Value(children)};
    const auto created=graph.context.construct(constructor,args);
    expect(graph.context.prototype(created)==prototype) << "MML test 41 assertion at source line 861";
    auto* createdNode=graph.runtime.mmlNodeIdentities.resolve(created);
    createdNode->setProperty("stale",17);
    createdNode->childArrayOwner()->push_back(m::NodeValue(factory->create("mi",tree::FactoryArgs{})));
    std::size_t setChildrenCalls=0;
    std::vector<o::Value> seenChildrenArguments;
    const auto originalSetChildren=graph.context.get(o::Value(created),o::Key(u"setChildren"));
    const auto overrideSetChildren=graph.context.function(u"",
        [&setChildrenCalls,&seenChildrenArguments,originalSetChildren](o::Context& context,o::Value receiver,
                                                                       std::span<const o::Value> childArgs){
            ++setChildrenCalls;
            seenChildrenArguments.push_back(childArgs.empty()?o::Value(o::Undefined{}):childArgs.front());
            return context.callMember(originalSetChildren,std::move(receiver),childArgs,u"setChildren");
        });
    graph.context.set(o::Value(created),o::Key(u"setChildren"),o::Value(overrideSetChildren));
    const auto previousAttributes=graph.context.get(o::Value(created),o::Key(u"attributes"));
    const auto replacementProperties=graph.context.object();
    graph.context.set(o::Value(replacementProperties),o::Key(u"id"),o::Value(u"reinitialized"));
    const std::array<o::Value,3> callArgs{o::Value(factoryIdentity),o::Value(replacementProperties),o::Value(children)};
    const auto callResult=graph.context.call(constructor,o::Value(created),callArgs);
    expect(callResult==o::Value(created)) << "MML test 41 assertion at source line 881";
    expect(createdNode->getPropertyNames().empty()) << "MML test 41 assertion at source line 882";
    expect(createdNode->childArrayOwner()->size()==0u) << "MML test 41 assertion at source line 883";
    expect(setChildrenCalls==1u) << "MML test 41 assertion at source line 884";
    const std::array<o::Value,1> omittedChildrenCall{o::Value(factoryIdentity)};
    expect(graph.context.call(constructor,o::Value(created),omittedChildrenCall)==o::Value(created)) << "MML test 41 assertion at source line 886";
    expect(setChildrenCalls==2u) << "MML test 41 assertion at source line 887";
    const auto defaultChildren=std::get<o::ObjectRef>(seenChildrenArguments.back().data);
    expect(graph.context.kind(defaultChildren)==o::ObjectKind::array) << "MML test 41 assertion at source line 889";
    expect(graph.context.arrayLength(defaultChildren)==0u) << "MML test 41 assertion at source line 890";
    expect(graph.context.get(o::Value(created),o::Key(u"attributes"))!=previousAttributes) << "MML test 41 assertion at source line 891";

    // The CJS function is callable on Object.create(MML.mi.prototype). Its
    // native owner is allocated from the real descriptor and bound to this
    // exact Context receiver, with no intermediate object identity.
    const auto plainReceiver=graph.context.object();
    graph.context.setPrototype(plainReceiver,prototype);
    const auto originalPrototype=graph.context.prototype(plainReceiver);
    const auto plainProperties=graph.context.object();
    graph.context.set(o::Value(plainProperties),o::Key(u"id"),o::Value(u"plain-receiver"));
    const std::array<o::Value,3> plainCallArgs{o::Value(factoryIdentity),o::Value(plainProperties),o::Value(children)};
    const auto handlesBefore=graph.runtime.mmlNodeIdentities.canonicalHandleCount();
    const auto plainResult=graph.context.call(constructor,o::Value(plainReceiver),plainCallArgs);
    expect(plainResult==o::Value(plainReceiver)) << "MML test 41 assertion at source line 904";
    expect(graph.context.prototype(plainReceiver)==originalPrototype) << "MML test 41 assertion at source line 905";
    expect(graph.runtime.mmlNodeIdentities.canonicalHandleCount()==handlesBefore+1) << "MML test 41 assertion at source line 906";
    auto* plainNode=graph.runtime.mmlNodeIdentities.resolve(plainReceiver);
    expect(dynamic_cast<m::MmlMi*>(plainNode)!=nullptr) << "MML test 41 assertion at source line 908";
    expect(graph.runtime.mmlNodeIdentities.find(plainNode)==plainReceiver) << "MML test 41 assertion at source line 909";
    expect(graph.context.get(o::Value(plainReceiver),o::Key(u"attributes"))!=o::Value(o::Null{})) << "MML test 41 assertion at source line 910";
    const auto plainAttributes=std::get<o::ObjectRef>(graph.context.get(o::Value(plainReceiver),o::Key(u"attributes")).data);
    const std::array<o::Value,1> idArgument{o::Value(u"id")};
    const auto attributesGet=graph.context.get(o::Value(plainAttributes),o::Key(u"get"));
    expect(graph.context.callMember(attributesGet,o::Value(plainAttributes),idArgument,u"attributes.get")==o::Value(u"plain-receiver"))
        << "MML constructor preserves id in the Attributes collection";

    const auto derivedMiPrototype=graph.context.object();
    graph.context.setPrototype(derivedMiPrototype,prototype);
    const auto derivedMiReceiver=graph.context.object();
    graph.context.setPrototype(derivedMiReceiver,derivedMiPrototype);
    const auto derivedPrototypeBefore=graph.context.prototype(derivedMiReceiver);
    expect(graph.context.call(constructor,o::Value(derivedMiReceiver),plainCallArgs)==o::Value(derivedMiReceiver)) << "MML test 41 assertion at source line 919";
    expect(graph.context.prototype(derivedMiReceiver)==derivedPrototypeBefore) << "MML test 41 assertion at source line 920";
    expect(graph.runtime.mmlNodeIdentities.find(graph.runtime.mmlNodeIdentities.resolve(derivedMiReceiver))==derivedMiReceiver) << "MML test 41 assertion at source line 921";

    const auto accessorReceiver=graph.context.object();
    graph.context.setPrototype(accessorReceiver,prototype);
    std::size_t accessorWrites=0;
    const auto getter=graph.context.function(u"getFactory",[&](o::Context&,o::Value,std::span<const o::Value>){return o::Value(o::Null{});});
    const auto setter=graph.context.function(u"setFactory",[&](o::Context&,o::Value,std::span<const o::Value> args){++accessorWrites;return args.empty()?o::Value(o::Undefined{}):args[0];});
    graph.context.define(accessorReceiver,o::Key(u"factory"),o::Descriptor::access(getter,setter,false,true));
    expect(graph.context.call(constructor,o::Value(accessorReceiver),plainCallArgs)==o::Value(accessorReceiver)) << "MML test 41 assertion at source line 929";
    expect(accessorWrites==1u) << "MML test 41 assertion at source line 930";
    expect(graph.context.descriptor(o::Value(accessorReceiver),o::Key(u"factory"))->accessor) << "MML test 41 assertion at source line 931";
    bool accessorFactoryRejected=false;
    try{(void)graph.runtime.mmlNodeIdentities.resolve(accessorReceiver)->isKind("mi");}
    catch(const o::Exception& error){accessorFactoryRejected=error.name=="TypeError";}
    expect(accessorFactoryRejected) << "MML test 41 assertion at source line 935";

    const auto readonlyParent=graph.context.object();
    graph.context.setPrototype(readonlyParent,prototype);
    graph.context.define(readonlyParent,o::Key(u"parent"),o::Descriptor::data(o::Value(u"unchanged"),false,false,false));
    bool readonlyParentRejected=false;
    try{(void)graph.context.call(constructor,o::Value(readonlyParent),plainCallArgs);}
    catch(const o::Exception& error){readonlyParentRejected=error.name=="TypeError";}
    expect(readonlyParentRejected) << "MML test 41 assertion at source line 943";
    expect(equalVector(graph.context.ownKeys(o::Value(readonlyParent)),
        std::vector<o::Key>{o::Key(u"parent"),o::Key(u"factory")})) << "MML test 41 assertion at source line 944";
    expect(graph.context.get(o::Value(readonlyParent),o::Key(u"parent"))==o::Value(u"unchanged")) << "MML test 41 assertion at source line 946";

    const auto invalidPlain=graph.context.object();
    graph.context.setPrototype(invalidPlain,prototype);
    const auto rawInvalidFactory=graph.context.object();
    const std::array<o::Value,3> invalidPlainArgs{o::Value(rawInvalidFactory),o::Value(plainProperties),o::Value(children)};
    bool invalidPlainRejected=false;
    try{(void)graph.context.call(constructor,o::Value(invalidPlain),invalidPlainArgs);}
    catch(const o::Exception& error){invalidPlainRejected=error.name=="TypeError";}
    expect(invalidPlainRejected) << "MML test 41 assertion at source line 955";
    expect(equalVector(graph.context.ownKeys(o::Value(invalidPlain)),std::vector<o::Key>{
        o::Key(u"factory"),o::Key(u"parent"),o::Key(u"properties"),o::Key(u"childNodes"),
        o::Key(u"prevClass"),o::Key(u"prevLevel"),o::Key(u"texclass")})) << "MML test 41 assertion at source line 956";
    expect(graph.context.get(o::Value(invalidPlain),o::Key(u"factory"))==o::Value(rawInvalidFactory)) << "MML test 41 assertion at source line 959";

    const auto math=std::get<o::ObjectRef>(graph.runtime.maps.resolve(map)->get(o::Value(u"math")).data);
    const auto mathPrototype=std::get<o::ObjectRef>(graph.context.get(o::Value(math),o::Key(u"prototype")).data);
    expect(graph.context.get(o::Value(graph.context.objectWithPrototype(mathPrototype)),o::Key(u"arity"))==o::Value(-1.0)) << "MML test 41 assertion at source line 963";
    const auto mfrac=std::get<o::ObjectRef>(graph.runtime.maps.resolve(map)->get(o::Value(u"mfrac")).data);
    const auto mfracPrototype=std::get<o::ObjectRef>(graph.context.get(o::Value(mfrac),o::Key(u"prototype")).data);
    expect(graph.context.get(o::Value(graph.context.objectWithPrototype(mfracPrototype)),o::Key(u"arity"))==o::Value(2.0)) << "MML test 41 assertion at source line 966";
    const auto inferredConstructor=std::get<o::ObjectRef>(graph.runtime.maps.resolve(map)->get(o::Value(u"inferredMrow")).data);
    const auto inferredPrototype=std::get<o::ObjectRef>(graph.context.get(o::Value(inferredConstructor),o::Key(u"prototype")).data);
    expect(graph.context.get(o::Value(graph.context.objectWithPrototype(inferredPrototype)),o::Key(u"isInferred"))==o::Value(true)) << "MML test 41 assertion at source line 969";
    const auto textConstructor=std::get<o::ObjectRef>(graph.runtime.maps.resolve(map)->get(o::Value(u"text")).data);
    const auto textPrototype=std::get<o::ObjectRef>(graph.context.get(o::Value(textConstructor),o::Key(u"prototype")).data);
    expect(graph.context.get(o::Value(graph.context.objectWithPrototype(textPrototype)),o::Key(u"arity"))==o::Value(0.0)) << "MML test 41 assertion at source line 972";
    expect(graph.context.get(o::Value(graph.context.objectWithPrototype(textPrototype)),o::Key(u"isInferred"))==o::Value(false)) << "MML test 41 assertion at source line 973";
    const auto invalidInferred=graph.context.object();
    graph.context.setPrototype(invalidInferred,mathPrototype);
    std::size_t invalidInferredSetChildren=0;
    const auto ownSetChildren=graph.context.function(u"setChildren",
        [&invalidInferredSetChildren](o::Context&,o::Value,std::span<const o::Value>){
            ++invalidInferredSetChildren;return o::Value(o::Undefined{});});
    graph.context.set(o::Value(invalidInferred),o::Key(u"setChildren"),o::Value(ownSetChildren));
    const std::array<o::Value,3> invalidInferredArgs{o::Value(o::Null{}),o::Value(graph.context.object()),o::Value(graph.context.array(0))};
    bool invalidInferredRejected=false;
    try{(void)graph.context.call(math,o::Value(invalidInferred),invalidInferredArgs);}
    catch(const o::Exception& error){invalidInferredRejected=error.name=="TypeError";}
    expect(invalidInferredRejected) << "MML test 41 assertion at source line 985";
    expect(invalidInferredSetChildren==0u) << "MML test 41 assertion at source line 986";
    const auto inferredChildren=std::get<o::ObjectRef>(graph.context.get(
        o::Value(invalidInferred),o::Key(u"childNodes")).data);
    expect(graph.context.arrayLength(inferredChildren)==0u) << "MML test 41 assertion at source line 989";

    const auto inferredAccessor=graph.context.object();
    graph.context.setPrototype(inferredAccessor,mathPrototype);
    const auto liveInferredChildren=graph.context.array(0);
    std::size_t inferredChildWrites=0;
    const auto getChildren=graph.context.function(u"getChildren",
        [liveInferredChildren](o::Context&,o::Value,std::span<const o::Value>){
            return o::Value(liveInferredChildren);
        });
    const auto setChildren=graph.context.function(u"setChildren",
        [&inferredChildWrites](o::Context&,o::Value,std::span<const o::Value>){
            ++inferredChildWrites;return o::Value(o::Undefined{});
        });
    graph.context.define(inferredAccessor,o::Key(u"childNodes"),
        o::Descriptor::access(getChildren,setChildren,true,true));
    bool inferredAccessorRejected=false;
    try{(void)graph.context.call(math,o::Value(inferredAccessor),plainCallArgs);}
    catch(const o::Exception& error){inferredAccessorRejected=error.name=="TypeError";}
    expect(inferredAccessorRejected) << "MML test 41 assertion at source line 1008";
    expect(inferredChildWrites==2u) << "MML test 41 assertion at source line 1009";
    expect(graph.context.arrayLength(liveInferredChildren)==0u) << "MML test 41 assertion at source line 1010";

    // An existing receiver's Source factory field is writable and is the
    // owner used by later native create/copy/isKind operations.
    auto replacementFactory=std::make_shared<m::MmlFactory>();
    const auto replacementFactoryIdentity=graph.runtime.bindMmlFactory(replacementFactory);
    const std::array<o::Value,3> replacementCallArgs{
      o::Value(replacementFactoryIdentity),o::Value(replacementProperties),o::Value(children)};
    expect(graph.context.call(constructor,o::Value(created),replacementCallArgs)==o::Value(created)) << "MML test 41 assertion at source line 1018";
    expect(graph.context.get(o::Value(created),o::Key(u"factory"))==o::Value(replacementFactoryIdentity)) << "MML test 41 assertion at source line 1019";
    auto* rebound=graph.runtime.mmlNodeIdentities.resolve(created);
    expect(&rebound->factoryOwner()==replacementFactory.get()) << "MML test 41 assertion at source line 1021";
    const auto productCount=replacementFactory->productCountForDiagnostics();
    (void)rebound->copy();
    expect(replacementFactory->productCountForDiagnostics()==productCount+1) << "MML test 41 assertion at source line 1024";

    // Assignment of an invalid Source factory Value is visible before native
    // consumption reports that the MML algorithms cannot use it.
    const auto invalidFactory=graph.context.object();
    const std::array<o::Value,3> invalidCallArgs{
      o::Value(invalidFactory),o::Value(replacementProperties),o::Value(children)};
    bool invalidRejected=false;
    try{(void)graph.context.call(constructor,o::Value(created),invalidCallArgs);}
    catch(const o::Exception& error){invalidRejected=error.name=="TypeError";}
    expect(invalidRejected) << "MML test 41 assertion at source line 1034";
    expect(graph.context.get(o::Value(created),o::Key(u"factory"))==o::Value(invalidFactory)) << "MML test 41 assertion at source line 1035";

    const auto parentPrototype=graph.context.prototype(prototype).value();
    expect(parentPrototype==graph.runtime.mmlNodeContextMethods.tokenPrototype()) << "MML test 41 assertion at source line 1038";
    const auto nodeMethods=graph.runtime.mmlNodeContextMethods.nodePrototype();
    const auto toStringDescriptor=graph.context.descriptor(o::Value(nodeMethods),o::Key(u"toString"));
    expect(toStringDescriptor.has_value()&&!toStringDescriptor->accessor&&
           toStringDescriptor->enumerable&&toStringDescriptor->writable&&toStringDescriptor->configurable) << "MML test 41 assertion at source line 1041";
    const o::Value createdValue(created);
    const auto toString=graph.context.get(createdValue,o::Key(u"toString"));
    expect(graph.context.call(std::get<o::ObjectRef>(toString.data),createdValue,
                              std::span<const o::Value>{})==o::Value(u"mi()")) << "MML test 41 assertion at source line 1045";
    const auto getText=graph.context.get(createdValue,o::Key(u"getText"));
    expect(std::holds_alternative<o::ObjectRef>(getText.data)) << "MML test 41 assertion at source line 1048";
    if(const auto* fn=std::get_if<o::ObjectRef>(&getText.data)){
      expect(graph.context.call(*fn,createdValue,std::span<const o::Value>{})==o::Value(u"")) << "MML test 41 assertion at source line 1050";
    }

    const std::array<o::Value,1> textConstructArgs{o::Value(factoryIdentity)};
    const auto textReceiver=graph.context.construct(textConstructor,textConstructArgs);
    graph.context.set(o::Value(textReceiver),o::Key(u"text"),o::Value(u"stale text"));
    expect(graph.context.call(textConstructor,o::Value(textReceiver),textConstructArgs)==o::Value(textReceiver)) << "MML test 41 assertion at source line 1056";
    expect(graph.context.get(o::Value(textReceiver),o::Key(u"text"))==o::Value(u"")) << "MML test 41 assertion at source line 1057";

    // Derived public constructors inherit through the canonical constructor
    // owner, independent of Map lookup performed after construction.
    const auto inferred=std::get<o::ObjectRef>(graph.runtime.maps.resolve(map)->get(o::Value(u"inferredMrow")).data);
    const auto mrow=std::get<o::ObjectRef>(graph.runtime.maps.resolve(map)->get(o::Value(u"mrow")).data);
    const auto inferredProto=std::get<o::ObjectRef>(graph.context.get(o::Value(inferred),o::Key(u"prototype")).data);
    const auto mrowProto=std::get<o::ObjectRef>(graph.context.get(o::Value(mrow),o::Key(u"prototype")).data);
    expect(graph.context.prototype(inferredProto)==mrowProto) << "MML test 41 assertion at source line 1065";
    expect(graph.context.prototype(inferred)==mrow) << "MML test 41 assertion at source line 1066";
    });
  };

  "mml_core_base_constructor_owners_keep_native_and_tree_identity"_test = [] {
    Graph graph;
    auto factory=std::make_shared<m::MmlFactory>();
    const auto factoryIdentity=graph.runtime.bindMmlFactory(factory);
    const auto map=std::get<o::ObjectRef>(graph.context.get(factoryIdentity,o::Key(u"nodeMap")).data);
    const auto miCtor=std::get<o::ObjectRef>(graph.runtime.maps.resolve(map)->get(o::Value(u"mi")).data);
    const auto miProto=std::get<o::ObjectRef>(graph.context.get(o::Value(miCtor),o::Key(u"prototype")).data);
    const auto tokenProto=graph.context.prototype(miProto).value();
    const auto abstractMmlProto=graph.context.prototype(tokenProto).value();
    const auto treeNodeProto=graph.context.prototype(abstractMmlProto).value();
    const auto treeNodeCtor=std::get<o::ObjectRef>(graph.context.get(o::Value(treeNodeProto),o::Key(u"constructor")).data);
    expect(graph.context.get(o::Value(treeNodeCtor),o::Key(u"name"))==o::Value(u"AbstractNode"));
    expect(graph.context.get(o::Value(treeNodeCtor),o::Key(u"length"))==o::Value(3));
    const auto treeNodeCtorPrototype=graph.context.prototype(treeNodeCtor).value();
    const auto functionCtor=std::get<o::ObjectRef>(graph.context.get(
        o::Value(treeNodeCtorPrototype),o::Key(u"constructor")).data);
    expect(graph.context.get(o::Value(functionCtor),o::Key(u"name"))==o::Value(u"Function"));

    const auto treeProperties=graph.context.object();
    graph.context.set(o::Value(treeProperties),o::Key(u"id"),o::Value(u"base"));
    const auto treeChild=graph.context.object();
    graph.context.set(o::Value(treeChild),o::Key(u"parent"),o::Value(o::Null{}));
    const auto treeChildren=graph.context.array(1);
    graph.context.set(o::Value(treeChildren),o::Key(u"0"),o::Value(treeChild));
    const std::array<o::Value,3> treeArgs{o::Value(factoryIdentity),o::Value(treeProperties),o::Value(treeChildren)};
    const auto treeNode=graph.context.construct(treeNodeCtor,treeArgs);
    expect(graph.context.prototype(treeNode).value()==treeNodeProto);
    const auto ownProperties=std::get<o::ObjectRef>(graph.context.get(o::Value(treeNode),o::Key(u"properties")).data);
    expect(graph.context.get(o::Value(ownProperties),o::Key(u"id"))==o::Value(u"base"));
    expect(graph.context.get(o::Value(treeChild),o::Key(u"parent"))==o::Value(treeNode));

    const auto textCtor=std::get<o::ObjectRef>(graph.runtime.maps.resolve(map)->get(o::Value(u"text")).data);
    const auto textProto=std::get<o::ObjectRef>(graph.context.get(o::Value(textCtor),o::Key(u"prototype")).data);
    const auto emptyProto=graph.context.prototype(textProto).value();
    const auto emptyCtor=std::get<o::ObjectRef>(graph.context.get(o::Value(emptyProto),o::Key(u"constructor")).data);
    expect(graph.context.get(o::Value(emptyCtor),o::Key(u"name"))==o::Value(u"AbstractMmlEmptyNode"));
    const auto properties=graph.context.object();
    const auto children=graph.context.array(0);
    const std::array<o::Value,3> emptyArgs{o::Value(factoryIdentity),o::Value(properties),o::Value(children)};
    const auto emptyNode=graph.context.construct(emptyCtor,emptyArgs);
    expect(graph.context.prototype(emptyNode).value()==emptyProto);
  };

  "mml_mml_late_field_assignments_obey_ordinary_descriptors_and_accessors"_test = [] {
    diagnoseMmlFailure("43 late field assignment",[&]{
    Graph graph;
    auto factory=std::make_shared<m::MmlFactory>();
    const auto factoryIdentity=graph.runtime.bindMmlFactory(factory);
    const auto map=std::get<o::ObjectRef>(graph.context.get(factoryIdentity,o::Key(u"nodeMap")).data);
    const auto mi=std::get<o::ObjectRef>(graph.runtime.maps.resolve(map)->get(o::Value(u"mi")).data);
    const auto miPrototype=std::get<o::ObjectRef>(graph.context.get(o::Value(mi),o::Key(u"prototype")).data);

    const auto readonly=graph.context.object();
    graph.context.setPrototype(readonly,miPrototype);
    graph.context.define(readonly,o::Key(u"attributes"),
        o::Descriptor::data(o::Value(17.0),false,true,true));
    bool readonlyRejected=false;
    try{const std::array<o::Value,3> args{o::Value(factoryIdentity),o::Value(graph.context.object()),o::Value(graph.context.array(0))};
        (void)graph.context.call(mi,o::Value(readonly),args);}
    catch(const o::Exception& error){readonlyRejected=error.name=="TypeError";}
    expect(readonlyRejected) << "MML test 43 assertion at source line 1130";
    expect(graph.context.get(o::Value(readonly),o::Key(u"attributes"))==o::Value(17.0)) << "MML test 43 assertion at source line 1131";
    expect(equalVector(graph.context.ownKeys(o::Value(readonly)),std::vector<o::Key>{
        o::Key(u"attributes"),o::Key(u"factory"),o::Key(u"parent"),o::Key(u"properties"),
        o::Key(u"childNodes"),o::Key(u"prevClass"),o::Key(u"prevLevel"),o::Key(u"texclass")})) << "MML test 43 assertion at source line 1132";

    const auto existing=factory->create("mi");
    const auto existingIdentity=graph.runtime.mmlNodeIdentities.find(existing);
    expect(existingIdentity.has_value()) << "MML test 43 assertion at source line 1138";
    if(!existingIdentity)return;
    const auto retainedAttributes=graph.context.get(o::Value(*existingIdentity),o::Key(u"attributes"));
    const auto accessor=graph.context.object();
    graph.context.setPrototype(accessor,miPrototype);
    std::size_t writes=0;
    const auto getter=graph.context.function(u"getAttributes",
        [retainedAttributes](o::Context&,o::Value,std::span<const o::Value>){return retainedAttributes;});
    const auto setter=graph.context.function(u"setAttributes",
        [&writes](o::Context&,o::Value,std::span<const o::Value>){++writes;return o::Value(o::Undefined{});});
    graph.context.define(accessor,o::Key(u"attributes"),o::Descriptor::access(getter,setter,true,true));
    const auto inputProperties=graph.context.object();
    graph.context.set(o::Value(inputProperties),o::Key(u"id"),o::Value(u"accessor-attributes"));
    const std::array<o::Value,3> args{o::Value(factoryIdentity),o::Value(inputProperties),o::Value(graph.context.array(0))};
    expect(graph.context.call(mi,o::Value(accessor),args)==o::Value(accessor)) << "MML test 43 assertion at source line 1152";
    expect(writes==1u) << "MML test 43 assertion at source line 1153";
    const std::array<o::Value,1> idArgument{o::Value(u"id")};
    const auto getAttribute=graph.context.get(retainedAttributes,o::Key(u"get"));
    expect(graph.context.callMember(getAttribute,retainedAttributes,idArgument,u"attributes.get")==o::Value(u"accessor-attributes"))
        << "MML constructor writes id to the accessor's Attributes collection";

    const auto text=std::get<o::ObjectRef>(graph.runtime.maps.resolve(map)->get(o::Value(u"text")).data);
    const auto textPrototype=std::get<o::ObjectRef>(graph.context.get(o::Value(text),o::Key(u"prototype")).data);
    const auto textReceiver=graph.context.object();
    graph.context.setPrototype(textReceiver,textPrototype);
    std::u16string textValue=u"old";
    std::size_t textWrites=0;
    const auto textGetter=graph.context.function(u"getText",
        [&textValue](o::Context&,o::Value,std::span<const o::Value>){return o::Value(textValue);});
    const auto textSetter=graph.context.function(u"setText",
        [&textValue,&textWrites](o::Context&,o::Value,std::span<const o::Value> args){
            ++textWrites;if(!args.empty())textValue=std::get<std::u16string>(args[0].data);return o::Value(o::Undefined{});});
    graph.context.define(textReceiver,o::Key(u"text"),o::Descriptor::access(textGetter,textSetter,true,true));
    const std::array<o::Value,1> textArgs{o::Value(factoryIdentity)};
    expect(graph.context.call(text,o::Value(textReceiver),textArgs)==o::Value(textReceiver)) << "MML test 43 assertion at source line 1169";
    expect(textWrites==1u&&textValue.empty()) << "MML test 43 assertion at source line 1170";
    });
  };

  "mml_native_operations_use_current_accessor_backed_properties_and_children"_test = [] {
    diagnoseMmlFailure("44 live accessor-backed operations",[&]{
    Graph graph;
    auto factory=std::make_shared<m::MmlFactory>();
    const auto factoryIdentity=graph.runtime.bindMmlFactory(factory);
    const auto map=std::get<o::ObjectRef>(graph.context.get(factoryIdentity,o::Key(u"nodeMap")).data);
    const auto mrowCtor=std::get<o::ObjectRef>(graph.runtime.maps.resolve(map)->get(o::Value(u"mrow")).data);
    const auto miCtor=std::get<o::ObjectRef>(graph.runtime.maps.resolve(map)->get(o::Value(u"mi")).data);
    const auto mrowPrototype=std::get<o::ObjectRef>(graph.context.get(o::Value(mrowCtor),o::Key(u"prototype")).data);

    const auto receiver=graph.context.object();
    graph.context.setPrototype(receiver,mrowPrototype);
    auto liveProperties=graph.context.object();
    std::optional<o::Value> propertiesGetterOverride;
    graph.context.set(o::Value(liveProperties),o::Key(u"sentinel"),o::Value(u"live"));
    auto liveChildren=graph.context.array(0);
    std::size_t propertyWrites=0,childWrites=0;
    const auto propertiesGetter=graph.context.function(u"getProperties",
        [&liveProperties,&propertiesGetterOverride](o::Context&,o::Value,std::span<const o::Value>){
            return propertiesGetterOverride.value_or(o::Value(liveProperties));});
    const auto propertiesSetter=graph.context.function(u"setProperties",
        [&propertyWrites](o::Context&,o::Value,std::span<const o::Value>){++propertyWrites;return o::Value(o::Undefined{});});
    const auto childrenGetter=graph.context.function(u"getChildren",
        [&liveChildren](o::Context&,o::Value,std::span<const o::Value>){return o::Value(liveChildren);});
    const auto childrenSetter=graph.context.function(u"setChildren",
        [&childWrites](o::Context&,o::Value,std::span<const o::Value>){++childWrites;return o::Value(o::Undefined{});});
    graph.context.define(receiver,o::Key(u"properties"),o::Descriptor::access(propertiesGetter,propertiesSetter,true,true));
    graph.context.define(receiver,o::Key(u"childNodes"),o::Descriptor::access(childrenGetter,childrenSetter,true,true));
    const std::array<o::Value,3> constructorArgs{
        o::Value(factoryIdentity),o::Value(graph.context.object()),o::Value(graph.context.array(0))};
    expect(graph.context.call(mrowCtor,o::Value(receiver),constructorArgs)==o::Value(receiver)) << "MML test 44 assertion at source line 1202";
    expect(propertyWrites==1u&&childWrites==2u) << "MML test 44 assertion at source line 1203";

    auto* node=graph.runtime.mmlNodeIdentities.resolve(receiver);
    expect(node->getProperty("sentinel")==graph.runtime.properties.from(o::Value(u"live"))) << "MML test 44 assertion at source line 1206";
    node->setProperty("written",graph.runtime.properties.from(o::Value(17.0)));
    expect(graph.context.get(o::Value(liveProperties),o::Key(u"written"))==o::Value(17.0)) << "MML test 44 assertion at source line 1208";

    const auto firstProperties=liveProperties;
    liveProperties=graph.context.object();
    graph.context.set(o::Value(liveProperties),o::Key(u"sentinel"),o::Value(u"replacement"));
    expect(node->getProperty("sentinel")==graph.runtime.properties.from(o::Value(u"replacement"))) << "MML test 44 assertion at source line 1213";
    node->setProperty("written",graph.runtime.properties.from(o::Value(23.0)));
    expect(graph.context.get(o::Value(liveProperties),o::Key(u"written"))==o::Value(23.0)) << "MML test 44 assertion at source line 1215";
    expect(graph.context.get(o::Value(firstProperties),o::Key(u"written"))==o::Value(17.0)) << "MML test 44 assertion at source line 1216";
    expect(graph.context.get(o::Value(receiver),o::Key(u"arity"))==o::Value(std::numeric_limits<double>::infinity())) << "MML test 44 assertion at source line 1217";
    expect(graph.context.get(o::Value(receiver),o::Key(u"isInferred"))==o::Value(false)) << "MML test 44 assertion at source line 1218";

    auto* child1=factory->create("mi");
    auto* child2=factory->create("mn");
    auto* child3=factory->create("mo");
    expect(node->appendChild(m::NodeValue(child1))==m::NodeValue(child1)) << "MML test 44 assertion at source line 1223";
    expect(graph.context.arrayLength(liveChildren)==1u) << "MML test 44 assertion at source line 1224";
    const auto firstChildren=liveChildren;
    const auto replacementChildren=std::make_shared<tree::ChildArray<m::MmlNode>>();
    liveChildren=t::bindNodeArray(graph.runtime,replacementChildren);
    expect(node->appendChild(m::NodeValue(child2))==m::NodeValue(child2)) << "MML test 44 assertion at source line 1228";
    expect(graph.context.arrayLength(firstChildren)==1u) << "MML test 44 assertion at source line 1229";
    expect(graph.context.arrayLength(liveChildren)==1u) << "MML test 44 assertion at source line 1230";
    expect(node->replaceChild(m::NodeValue(child3),m::NodeValue(child2))==m::NodeValue(child3)) << "MML test 44 assertion at source line 1231";
    const auto first=std::get<o::ObjectRef>(graph.context.get(o::Value(liveChildren),o::Key(u"0")).data);
    expect(graph.runtime.mmlNodeIdentities.resolve(first)==child3) << "MML test 44 assertion at source line 1233";

    auto* copy=node->copy();
    expect(copy->getProperty("sentinel")==graph.runtime.properties.from(o::Value(u"replacement"))) << "MML test 44 assertion at source line 1236";
    expect(copy->getProperty("written")==graph.runtime.properties.from(o::Value(23.0))) << "MML test 44 assertion at source line 1237";
    expect(copy->childArrayOwner()->size()==1u) << "MML test 44 assertion at source line 1238";
    std::size_t visited=0;
    const tree::TreeVisitor<m::MmlNode> visitor=[&](m::MmlNode*,const std::any&){++visited;};
    (void)node->walkTree(visitor);
    expect(visited==2u) << "MML test 44 assertion at source line 1242";

    // MmlNode.copy reads isInferred and calls copy on each original child.
    // A Source child can implement those operations without being registered
    // as a native MmlNode; preserve that receiver and the returned identity.
    const auto foreignChild=graph.context.object();
    graph.context.set(o::Value(foreignChild),o::Key(u"isInferred"),o::Value(false));
    const auto copiedForeignChild=graph.context.object();
    std::size_t foreignCopyCalls=0;
    const auto foreignCopy=graph.context.function(u"copy",
        [&foreignCopyCalls,copiedForeignChild](o::Context&,o::Value receiver,std::span<const o::Value>){
            ++foreignCopyCalls;
            return o::Value(copiedForeignChild);
        });
    graph.context.set(o::Value(foreignChild),o::Key(u"copy"),o::Value(foreignCopy));
    const auto foreignChildren=graph.context.array(1);
    graph.context.set(o::Value(foreignChildren),o::Key(u"0"),o::Value(foreignChild));
    liveChildren=foreignChildren;
    auto* foreignCopyOwner=node->copy();
    expect(foreignCopyCalls==1u) << "MML test 44 assertion at source line 1261";
    expect(foreignCopyOwner->childArrayOwner()->size()==1u) << "MML test 44 assertion at source line 1262";
    expect(foreignCopyOwner->childArrayOwner()->get(0).sourceValue()==o::Value(copiedForeignChild)) << "MML test 44 assertion at source line 1263";
    expect(graph.context.get(o::Value(copiedForeignChild),o::Key(u"parent"))==
           o::Value(graph.runtime.mmlNodeIdentities.findPointee(foreignCopyOwner).value())) << "MML test 44 assertion at source line 1264";

    const auto foreignInferredChild=graph.context.object();
    graph.context.set(o::Value(foreignInferredChild),o::Key(u"isInferred"),o::Value(true));
    const auto foreignAppendChildren=graph.context.array(1);
    graph.context.set(o::Value(foreignAppendChildren),o::Key(u"0"),
                      o::Value(graph.runtime.mmlNodeIdentities.findPointee(child1).value()));
    graph.context.set(o::Value(foreignInferredChild),o::Key(u"childNodes"),o::Value(foreignAppendChildren));
    const auto beforeAppend=graph.context.arrayLength(liveChildren);
    const auto appendedSource=graph.runtime.treeNodeBindings.childValue(o::Value(foreignInferredChild));
    expect(node->appendChild(appendedSource)==appendedSource) << "MML test 44 assertion at source line 1275";
    expect(graph.context.arrayLength(liveChildren)==beforeAppend+1u) << "MML test 44 assertion at source line 1276";
    expect(graph.context.get(o::Value(graph.runtime.mmlNodeIdentities.findPointee(child1).value()),o::Key(u"parent"))==o::Value(receiver)) << "MML test 44 assertion at source line 1277";

    auto* finiteOwner=dynamic_cast<m::AbstractMmlNode*>(factory->create("msup"));
    expect(finiteOwner!=nullptr) << "MML test 44 assertion at source line 1280";
    const auto foreignAttributes=graph.context.get(o::Value(receiver),o::Key(u"attributes"));
    graph.context.set(o::Value(foreignInferredChild),o::Key(u"attributes"),foreignAttributes);
    const auto foreignPropertyObject=graph.context.object();
    const auto propertyNames=graph.context.array(2);
    graph.context.set(o::Value(propertyNames),o::Key(u"0"),o::Value(u"foreign"));
    graph.context.set(o::Value(propertyNames),o::Key(u"1"),o::Value(u"foreignObject"));
    const auto getPropertyNames=graph.context.function(u"getPropertyNames",
        [propertyNames](o::Context&,o::Value,std::span<const o::Value>){return o::Value(propertyNames);});
    const auto getProperty=graph.context.function(u"getProperty",
        [foreignPropertyObject](o::Context&,o::Value,std::span<const o::Value> args){
            if(!args.empty()&&args.front()==o::Value(u"foreignObject"))return o::Value(foreignPropertyObject);
            return o::Value(41.0);
        });
    graph.context.set(o::Value(foreignInferredChild),o::Key(u"getPropertyNames"),o::Value(getPropertyNames));
    graph.context.set(o::Value(foreignInferredChild),o::Key(u"getProperty"),o::Value(getProperty));
    const auto finiteChildren=graph.context.array(1);
    graph.context.set(o::Value(finiteChildren),o::Key(u"0"),
                      o::Value(graph.runtime.mmlNodeIdentities.findPointee(child2).value()));
    graph.context.set(o::Value(foreignInferredChild),o::Key(u"childNodes"),o::Value(finiteChildren));
    const auto converted=finiteOwner->appendChild(graph.runtime.treeNodeBindings.childValue(o::Value(foreignInferredChild)));
    auto* convertedNode=dynamic_cast<m::AbstractMmlNode*>(converted.nodeOrNull());
    expect(convertedNode!=nullptr) << "MML test 44 assertion at source line 1302";
    if(convertedNode){
        expect(convertedNode->kind()=="mrow") << "MML test 44 assertion at source line 1304";
        expect(convertedNode->childArrayOwner()->size()==1u) << "MML test 44 assertion at source line 1305";
        expect(convertedNode->getProperty("foreign")==graph.runtime.properties.from(o::Value(41.0))) << "MML test 44 assertion at source line 1306";
        const auto convertedIdentity=graph.runtime.mmlNodeIdentities.findPointee(convertedNode);
        expect(convertedIdentity.has_value()) << "finite child conversion has a canonical identity";
        if(convertedIdentity){
            const auto properties=graph.context.get(o::Value(*convertedIdentity),o::Key(u"properties"));
            expect(graph.context.get(properties,o::Key(u"foreignObject"))==o::Value(foreignPropertyObject))
                << "finite child conversion preserves raw property object identity";
        }
        const auto typedForeignObject=convertedNode->getProperty("foreignObject");
        expect(graph.runtime.properties.to(typedForeignObject)==o::Value(foreignPropertyObject))
            << "finite child property preserves object identity across typed conversion";
        expect(convertedNode->getProperty("foreignObject")==graph.runtime.properties.from(o::Value(foreignPropertyObject))) << "MML test 44 assertion at source line 1307";
    }

    const auto copyMethod=graph.context.get(o::Value(receiver),o::Key(u"copy"));
    expect(graph.context.get(copyMethod,o::Key(u"length"))==o::Value(1)) << "MML test 44 assertion at source line 1311";
    const auto contextCopy=graph.context.call(std::get<o::ObjectRef>(copyMethod.data),o::Value(receiver),std::span<const o::Value>{});
    const auto contextCopyIdentity=std::get<o::ObjectRef>(contextCopy.data);
    auto* contextCopyNode=graph.runtime.mmlNodeIdentities.resolve(contextCopyIdentity);
    expect(contextCopyNode->kind()=="mrow") << "MML test 44 assertion at source line 1315";
    expect(contextCopyNode->childArrayOwner()->size()==2u) << "MML test 44 assertion at source line 1316";

    const auto dynamicKindGetter=graph.context.function(u"getKind",
        [](o::Context&,o::Value,std::span<const o::Value>){return o::Value(u"mo");});
    graph.context.define(receiver,o::Key(u"kind"),o::Descriptor::access(dynamicKindGetter,std::nullopt,true,true));
    auto* dynamicKindResult=node->copy();
    expect(dynamicKindResult->kind()=="mo") << "MML test 44 assertion at source line 1322";

    // Source `properties` accepts null, and Object.assign({}, null) produces
    // an empty record during copy rather than rejecting the field assignment.
    // This receiver has an accessor with a no-op setter, so change its getter
    // backing value explicitly after observing the setter call.
    graph.context.set(o::Value(receiver),o::Key(u"properties"),o::Value(o::Null{}));
    propertiesGetterOverride=o::Value(o::Null{});
    expect(graph.context.get(o::Value(receiver),o::Key(u"properties"))==o::Value(o::Null{})) << "MML test 44 assertion at source line 1327";
    auto* nullPropertiesCopy=node->copy();
    expect(nullPropertiesCopy->getPropertyNames().empty()) << "MML test 44 assertion at source line 1329";
    const auto symbolKey=graph.context.symbol(u"copied-property");
    const auto symbolProperties=graph.context.object();
    graph.context.set(o::Value(symbolProperties),o::Key(symbolKey),o::Value(123.0));
    graph.context.set(o::Value(receiver),o::Key(u"properties"),o::Value(symbolProperties));
    propertiesGetterOverride=o::Value(symbolProperties);
    auto* symbolPropertiesCopy=node->copy();
    const auto symbolCopyIdentity=graph.runtime.mmlNodeIdentities.findPointee(symbolPropertiesCopy);
    expect(symbolCopyIdentity.has_value()) << "MML test 44 assertion at source line 1336";
    if(symbolCopyIdentity){
        const auto copiedProperties=graph.context.get(o::Value(*symbolCopyIdentity),o::Key(u"properties"));
        expect(graph.context.get(copiedProperties,o::Key(symbolKey))==o::Value(123.0))
            << "MML copy preserves Symbol keys inside the properties collection";
    }

    const auto falseyChildren=graph.context.array(2);
    graph.context.set(o::Value(falseyChildren),o::Key(u"0"),o::Value(o::Null{}));
    graph.context.set(o::Value(falseyChildren),o::Key(u"1"),o::Value(false));
    graph.context.set(o::Value(receiver),o::Key(u"childNodes"),o::Value(falseyChildren));
    liveChildren=falseyChildren;
    auto* falseyChildCopy=node->copy();
    expect(falseyChildCopy->childArrayOwner()->size()==2u) << "MML test 44 assertion at source line 1345";
    expect(!falseyChildCopy->childArrayOwner()->get(0).truthy()) << "MML test 44 assertion at source line 1346";
    expect(!falseyChildCopy->childArrayOwner()->get(1).truthy()) << "MML test 44 assertion at source line 1347";

    // MmlNode.copy calls the current attributes.getAllAttributes and copies
    // its enumerable string properties into the new native Attributes owner.
    const auto rawForeignAttributes=graph.context.object();
    const auto rawAttributeList=graph.context.object();
    graph.context.set(o::Value(rawAttributeList),o::Key(u"mathcolor"),o::Value(u"red"));
    const auto getAllAttributes=graph.context.function(u"getAllAttributes",
        [rawAttributeList](o::Context&,o::Value,std::span<const o::Value>){return o::Value(rawAttributeList);});
    graph.context.set(o::Value(rawForeignAttributes),o::Key(u"getAllAttributes"),o::Value(getAllAttributes));
    graph.context.set(o::Value(receiver),o::Key(u"attributes"),o::Value(rawForeignAttributes));
    auto* foreignAttributesCopy=node->copy();
    expect(foreignAttributesCopy->attributes()->get("mathcolor")==folia::math::Property(std::string("red"))) << "MML test 44 assertion at source line 1359";
    });
  };

  "mml_child_position_matches_canonical_nodes_in_context_arrays_through_inferred_rows"_test = [] {
    Graph graph;
    auto factory=std::make_shared<m::MmlFactory>();
    (void)graph.runtime.bindMmlFactory(factory);
    auto* table=factory->create("mtable",tree::FactoryArgs{});
    auto* row=factory->create("mtr",tree::FactoryArgs{});
    auto* cell=factory->create("mtd",tree::FactoryArgs{});
    auto* token=factory->create("mi",tree::FactoryArgs{});
    expect(table!=nullptr&&row!=nullptr&&cell!=nullptr&&token!=nullptr);
    if(!table||!row||!cell||!token)return;

    auto inferredValue=cell->childArrayOwner()->get(0);
    auto* inferred=inferredValue.nodeOrNull();
    expect(inferred!=nullptr);
    if(!inferred)return;

    auto identityOf=[&](m::MmlNode* node){
      const auto identity=graph.runtime.mmlNodeIdentities.findPointee(node);
      expect(identity.has_value());
      return identity.value();
    };
    auto arrayOf=[&](std::initializer_list<m::MmlNode*> nodes){
      const auto array=graph.context.array(nodes.size());
      std::size_t index=0;
      for(auto* node:nodes){
        const auto identity=identityOf(node);
        graph.context.set(o::Value(array),o::Key(folia::math::property_host::text(std::to_string(index))),
                          o::Value(identity));
        ++index;
      }
      return array;
    };

    // Install the same canonical Context child arrays and parent identities
    // that Source traversal uses, without manufacturing a parallel list.
    const auto tableId=identityOf(table),rowId=identityOf(row),cellId=identityOf(cell);
    const auto inferredId=identityOf(inferred),tokenId=identityOf(token);
    graph.context.set(o::Value(tableId),o::Key(u"childNodes"),o::Value(arrayOf({row})));
    graph.context.set(o::Value(rowId),o::Key(u"childNodes"),o::Value(arrayOf({cell})));
    graph.context.set(o::Value(cellId),o::Key(u"childNodes"),o::Value(arrayOf({inferred})));
    graph.context.set(o::Value(inferredId),o::Key(u"childNodes"),o::Value(arrayOf({token})));
    graph.context.set(o::Value(rowId),o::Key(u"parent"),o::Value(tableId));
    graph.context.set(o::Value(cellId),o::Key(u"parent"),o::Value(rowId));
    graph.context.set(o::Value(inferredId),o::Key(u"parent"),o::Value(cellId));
    graph.context.set(o::Value(tokenId),o::Key(u"parent"),o::Value(inferredId));

    const auto rowChildren=graph.runtime.treeNodeBindings.arrayView(
      graph.context.get(o::Value(rowId),o::Key(u"childNodes")));
    const auto cellChildren=graph.runtime.treeNodeBindings.arrayView(
      graph.context.get(o::Value(cellId),o::Key(u"childNodes")));
    expect(row->childArrayOwner()==rowChildren);
    expect(cell->childArrayOwner()==cellChildren);

    const auto rowCell=rowChildren->get(0);
    const auto cellInferred=cellChildren->get(0);
    expect(rowCell.isSourceBacked()&&rowCell.nodeOrNull()==cell);
    expect(cellInferred.isSourceBacked()&&cellInferred.nodeOrNull()==inferred);
    expect(m::NodeValue(cell)==rowCell);
    expect(rowCell==m::NodeValue(cell));
    expect(m::NodeValue(inferred)==cellInferred);
    expect(cellInferred==m::NodeValue(inferred));
    expect(!(m::NodeValue(folia::math::PropertyNull{})==rowCell));
    expect(!(rowCell==m::NodeValue(folia::math::PropertyNull{})));
    const auto foreign=graph.context.object();
    const auto foreignAccess=m::NodeValue(graph.runtime.treeNodeBindings.nodeAccess(o::Value(foreign)));
    expect(!(foreignAccess==m::NodeValue(cell)));
    expect(!(m::NodeValue(cell)==foreignAccess));
    expect(!(foreignAccess==m::NodeValue(folia::math::PropertyNull{})));
    expect(cell->childPosition()==std::optional<std::size_t>(0));
    expect(token->childPosition()==std::optional<std::size_t>(0));

    auto* other=factory->create("mtd",tree::FactoryArgs{});
    expect(other!=nullptr);
    if(other){
      const auto otherIdentity=graph.runtime.mmlNodeIdentities.findPointee(other);
      expect(otherIdentity.has_value());
      if(otherIdentity){
        const auto otherArray=graph.context.array(1);
        graph.context.set(o::Value(otherArray),o::Key(u"0"),o::Value(*otherIdentity));
        const auto otherView=graph.runtime.treeNodeBindings.arrayView(o::Value(otherArray));
        const auto otherAccess=otherView->get(0);
        expect(otherAccess.nodeOrNull()==other);
        expect(!(m::NodeValue(cell)==otherAccess));
        expect(!(otherAccess==m::NodeValue(cell)));
      }
    }
  };
  "mml_native_operations_resolve_parent_properties_and_children_after_bind"_test = [] {
    // Source witness: core/Tree/Node.ts declares these as ordinary own fields;
    // its methods read/write this.parent, this.properties, and this.childNodes
    // at operation time. An accessor redefinition therefore participates in
    // native methods exactly as it does in the pinned JavaScript implementation.
    Graph graph;
    auto factory=std::make_shared<m::MmlFactory>();
    (void)graph.runtime.bindMmlFactory(factory);
    auto* node=factory->create("mrow");
    const auto identity=graph.runtime.mmlNodeIdentities.findPointee(node);
    expect(identity.has_value());
    if(!identity)return;

    auto liveProperties=graph.context.object();
    graph.context.set(o::Value(liveProperties),o::Key(u"marker"),o::Value(1.0));
    auto replacementProperties=graph.context.object();
    graph.context.set(o::Value(replacementProperties),o::Key(u"marker"),o::Value(2.0));
    std::size_t propertyGets=0,propertySets=0;
    const auto propertiesGetter=graph.context.function(u"getLiveProperties",
        [&liveProperties,&propertyGets](o::Context&,o::Value,std::span<const o::Value>){++propertyGets;return o::Value(liveProperties);});
    const auto propertiesSetter=graph.context.function(u"setLiveProperties",
        [&propertySets](o::Context&,o::Value,std::span<const o::Value>){++propertySets;return o::Value(o::Undefined{});});
    graph.context.define(*identity,o::Key(u"properties"),o::Descriptor::access(propertiesGetter,propertiesSetter,true,true));
    expect(node->getProperty("marker")==graph.runtime.properties.from(o::Value(1.0)));
    liveProperties=replacementProperties;
    expect(node->getProperty("marker")==graph.runtime.properties.from(o::Value(2.0)));
    node->setProperty("nativeWrite",graph.runtime.properties.from(o::Value(3.0)));
    const auto replacementRecord=folia::math::PropertyList::record(graph.runtime.properties.from(o::Value(replacementProperties)));
    expect(replacementRecord.has_value());
    if(replacementRecord)node->propertyOwner(*replacementRecord);
    expect(propertyGets>0u&&propertySets>0u);
    expect(graph.context.get(o::Value(replacementProperties),o::Key(u"nativeWrite"))==o::Value(3.0));

    auto liveChildren=graph.context.array(0);
    std::size_t childGets=0,childSets=0;
    const auto childrenGetter=graph.context.function(u"getLiveChildren",
        [&liveChildren,&childGets](o::Context&,o::Value,std::span<const o::Value>){++childGets;return o::Value(liveChildren);});
    const auto childrenSetter=graph.context.function(u"setLiveChildren",
        [&childSets](o::Context&,o::Value,std::span<const o::Value>){++childSets;return o::Value(o::Undefined{});});
    graph.context.define(*identity,o::Key(u"childNodes"),o::Descriptor::access(childrenGetter,childrenSetter,true,true));
    auto replacementOwner=std::make_shared<tree::ChildArray<m::MmlNode>>();
    node->childArrayOwner(replacementOwner);
    liveChildren=t::bindNodeArray(graph.runtime,replacementOwner);
    auto* child=factory->create("mi");
    node->appendChild(m::NodeValue(child));
    expect(childGets>0u&&childSets>0u);
    expect(graph.context.arrayLength(liveChildren)==1u);
    expect(node->childArrayOwner()->size()==1u);

    auto liveParent=graph.context.object();
    std::size_t parentGets=0,parentSets=0;
    const auto parentGetter=graph.context.function(u"getLiveParent",
        [&liveParent,&parentGets](o::Context&,o::Value,std::span<const o::Value>){++parentGets;return o::Value(liveParent);});
    const auto parentSetter=graph.context.function(u"setLiveParent",
        [&parentSets](o::Context&,o::Value,std::span<const o::Value>){++parentSets;return o::Value(o::Undefined{});});
    graph.context.define(*identity,o::Key(u"parent"),o::Descriptor::access(parentGetter,parentSetter,true,true));
    expect(node->getParent().sourceValue()==o::Value(liveParent));
    auto* parent=factory->create("math");
    node->setParent(m::NodeValue(parent));
    expect(parentGets>0u&&parentSets>0u);

    const auto inheritedParent=graph.context.object();
    std::size_t inheritedSets=0;
    const auto inheritedSetter=graph.context.function(u"setInheritedParent",
        [&inheritedSets](o::Context&,o::Value,std::span<const o::Value>){++inheritedSets;return o::Value(o::Undefined{});});
    graph.context.define(inheritedParent,o::Key(u"parent"),o::Descriptor::access(std::nullopt,inheritedSetter,true,true));
    graph.context.erase(*identity,o::Key(u"parent"));
    graph.context.setPrototype(*identity,inheritedParent);
    node->setParent(m::NodeValue(parent));
    expect(inheritedSets==1u);

    const auto readonlyProto=graph.context.object();
    graph.context.define(readonlyProto,o::Key(u"parent"),o::Descriptor::data(o::Value(liveParent),false,true,true));
    graph.context.setPrototype(*identity,readonlyProto);
    bool readonlyRejected=false;
    try{node->setParent(m::NodeValue(parent));}
    catch(const o::Exception& error){readonlyRejected=error.name=="TypeError";}
    expect(readonlyRejected);
  };
};
}
