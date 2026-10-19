#include "support/folia_test.hpp"
import folia.math.util.factory_context_field_views;
import folia.math.util.map_bindings;
import folia.math.util.option_types;
import folia.math.core.tree.node_factory;

namespace {
using namespace boost::ut;
namespace o=folia::math::options;
namespace t=folia::math::tree;
struct Product{};
using ClassRef=t::FactoryClassRef<Product>;
using MapValue=t::FactoryClassMapValue<Product>;

class SwitchOnSetMap final:public t::FactoryClassMapAccess<Product>{
public:
  using Entry=folia::math::OrderedMapAccess<o::Value,MapValue>::Entry;
  explicit SwitchOnSetMap(std::shared_ptr<t::FactoryClassMapOwner<Product>> owner)
      :owner_(std::move(owner)){}
  const void* identity()const noexcept override{return owner_->identity();}
  std::size_t size()const noexcept override{return owner_->size();}
  std::size_t slotCount()const noexcept override{return owner_->slotCount();}
  std::optional<Entry> entryAt(std::size_t slot)const override{return owner_->entryAt(slot);}
  std::optional<o::Value> keyAt(std::size_t slot)const override{return owner_->keyAt(slot);}
  std::optional<MapValue> get(const o::Value& key)const override{return owner_->get(key);}
  bool has(const o::Value& key)const override{return owner_->has(key);}
  void set(o::Value key,MapValue value)override{
    owner_->set(std::move(key),std::move(value));
    if(afterSet)afterSet();
  }
  bool erase(const o::Value& key)override{return owner_->erase(key);}
  void clear()noexcept override{owner_->clear();}
  t::FactoryClassRef<Product> resolveValue(const MapValue& value)const override{
    ++resolveCount;
    return owner_->resolveValue(value);
  }
  std::function<void()> afterSet;
  mutable int resolveCount=0;
private:
  std::shared_ptr<t::FactoryClassMapOwner<Product>> owner_;
};

suite factoryContextFieldViews = [] {
  "node_factory_calls_selected_member_directly_before_argument_conversion"_test = [] {
    o::Context context;
    auto runtimeLease=std::make_shared<std::atomic_bool>(true);
    auto ownerLease=std::make_shared<int>(1);
    const auto node=context.object();
    std::vector<std::string> order;
    o::Value observedReceiver=o::Undefined{};
    std::vector<o::Value> observedArguments;
    const auto callable=context.function(u"member",[&](o::Context&,o::Value receiver,
        std::span<const o::Value> arguments){
        order.push_back("call");
        observedReceiver=receiver;
        observedArguments.assign(arguments.begin(),arguments.end());
        return o::Value(u"called");
    });
    context.set(o::Value(node),o::Key(u"member"),o::Value(callable));
    int applyReads=0,applyCalls=0;
    static Product result;
    using Access=t::ContextFactoryCreatorAccess<Product>;
    auto access=std::make_shared<Access>(o::Value(node),ownerLease,runtimeLease,
      [&](const o::Value& receiver,const o::Key& key){order.push_back("get");return context.get(receiver,key);},
      [&](const o::Value& receiver,const o::Key& key,o::Value value){context.set(receiver,key,std::move(value));},
      [&](const o::Value& receiver,const o::Key& key){context.erase(std::get<o::ObjectRef>(receiver.data),key);},
      [&](const o::Value& receiver){return context.ownKeys(receiver);},
      [&](const o::Value& receiver){return context.prototype(std::get<o::ObjectRef>(receiver.data));},
      [&](const o::Value& receiver,std::optional<o::ObjectRef> prototype){context.setPrototype(std::get<o::ObjectRef>(receiver.data),prototype);},
      [&](const o::Value& value){const auto* object=std::get_if<o::ObjectRef>(&value.data);return object&&context.kind(*object)==o::ObjectKind::function;},
      [](const o::Value&){return t::FactoryCreatorRef<Product>{};},
      [](const t::FactoryCreatorRef<Product>&){return o::Value(o::Undefined{});},
      [&](const o::Value&){++applyReads;return o::Value(o::Undefined{});},
      [&](const o::Value&,const o::Value&,t::FactoryArgs)->Product*{++applyCalls;return nullptr;},
      [&](const o::Value& receiver,const o::Value& selected,t::FactoryArgs args)->Product*{
        std::vector<o::Value> values;
        for(const auto& argument:args)values.push_back(std::any_cast<o::Value>(argument));
        const auto* function=std::get_if<o::ObjectRef>(&selected.data);
        if(!function)throw o::Exception("TypeError",u"selected member is not callable");
        expect(context.call(*function,receiver,values)==o::Value(u"called"));
        return &result;
      });
    t::AbstractNodeFactory<Product> factory;
    factory.creatorAccess(access);
    auto* actual=factory.createWithArguments("member",[&]{
      order.push_back("arguments");
      return std::vector<std::any>{std::any(o::Value(17)),std::any(o::Value(u"child"))};
    });
    expect(actual==&result);
    expect(order==std::vector<std::string>{"get","arguments","call"});
    expect(observedReceiver==o::Value(node));
    expect(observedArguments==std::vector<o::Value>{o::Value(17),o::Value(u"child")});
    expect(applyReads==0);
    expect(applyCalls==0);

    context.set(o::Value(node),o::Key(u"member"),o::Value(o::Null{}));
    bool argumentsWereBuilt=false,sourceTypeError=false;
    try{
      (void)factory.createWithArguments("member",[&]{
        argumentsWereBuilt=true;
        return std::vector<std::any>{std::any(o::Value(19))};
      });
    }catch(const o::Exception& error){
      sourceTypeError=error.name=="TypeError"&&
          error.message==u"this.node[kind] is not a function";
    }
    expect(argumentsWereBuilt);
    expect(sourceTypeError);

    bool rawArgumentsWereEvaluated=false,conversionWasDeferred=false;
    try{
      (void)factory.createWithConvertedArguments("member",[&]{
        rawArgumentsWereEvaluated=true;
        return std::vector<std::any>{std::any(o::Value(23))};
      },[&](const std::vector<std::any>& raw){
        conversionWasDeferred=true;
        return raw;
      });
    }catch(const o::Exception& error){
      sourceTypeError=error.name=="TypeError"&&
          error.message==u"this.node[kind] is not a function";
    }
    expect(sourceTypeError);
    expect(rawArgumentsWereEvaluated);
    expect(!conversionWasDeferred);
  };

  "native_factory_null_fallback_throws_when_reading_apply"_test = [] {
    t::AbstractFactory<Product> factory;
    factory.defaultKindValue(o::Value(u"fallback"));
    factory.creatorView()->entries.emplace_back(
        o::Key(u"fallback"),t::FactoryCreatorSlot<Product>{{},{}});
    bool matched=false;
    try{(void)factory.create("missing");}
    catch(const o::Exception& error){
      matched=error.name=="TypeError"&&
          error.message==u"Cannot read properties of null (reading 'apply')";
    }
    expect(matched);
  };

  "factory_node_map_projection_uses_actual_owner_and_live_cursor"_test = [] {
    o::Context context;
    o::MapContextBindings maps(context);
    auto owner=std::make_shared<t::FactoryClassMapOwner<Product>>();
    const ClassRef first=std::make_shared<t::FactoryClass<Product>>();
    const ClassRef second=std::make_shared<t::FactoryClass<Product>>();
    const o::ObjectRef firstClass=context.object();
    const o::ObjectRef secondClass=context.object();
    auto classFrom=[&](const o::Value& value)->MapValue{
      const auto* object=std::get_if<o::ObjectRef>(&value.data);
      if(!object)throw o::Exception("TypeError",u"nodeMap values must be constructors");
      if(*object==firstClass)return MapValue::native(first);
      if(*object==secondClass)return MapValue::native(second);
      throw o::Exception("TypeError",u"Unknown constructor identity");
    };
    auto classTo=[&](const MapValue& value)->o::Value{
      if(const auto* raw=value.sourceValue())return *raw;
      const auto* native=value.nativeClass();
      if(native&&*native==first)return o::Value(firstClass);
      if(native&&*native==second)return o::Value(secondClass);
      throw o::Exception("TypeError",u"Unknown native constructor");
    };
    const auto view=[&](){return t::projectFactoryClassMap<Product>(owner,classTo);};
    owner->setKind("mi",first);
    owner->setKind("mn",second);
    const auto map=maps.bind(view());
    expect(maps.bind(view())==map);
    expect(maps.resolve(map)->identity()==owner->identity());
    const o::ObjectRef mapOnlyClass=context.object();
    maps.set(map,o::Value(u"mapOnly"),o::Value(mapOnlyClass));
    expect(maps.get(map,o::Value(u"mapOnly"))==o::Value(mapOnlyClass));

    const o::Value keysMethod=context.get(o::Value(map),o::Key(u"keys"));
    const auto keys=std::get<o::ObjectRef>(context.call(
        std::get<o::ObjectRef>(keysMethod.data),o::Value(map)).data);
    const auto next=std::get<o::ObjectRef>(context.get(o::Value(keys),o::Key(u"next")).data);
    const auto firstResult=context.call(next,o::Value(keys));
    expect(context.get(firstResult,o::Key(u"value"))==o::Value(u"mi"));

    owner->eraseKind("mn");
    owner->setKind("mn",second);
    const auto liveMapOnly=context.call(next,o::Value(keys));
    expect(context.get(liveMapOnly,o::Key(u"value"))==o::Value(u"mapOnly"));
    const auto reinserted=context.call(next,o::Value(keys));
    expect(context.get(reinserted,o::Key(u"value"))==o::Value(u"mn"));
    expect(owner->slotCount()==4u);

    // Map keys preserve their actual Value identity and type.
    const o::ObjectRef objectKey=context.object();
    owner->set(o::Value(1),t::FactoryClassMapValue<Product>::native(first));
    owner->set(o::Value(u"1"),t::FactoryClassMapValue<Product>::native(second));
    owner->set(o::Value(objectKey),t::FactoryClassMapValue<Product>::native(first));
    expect(owner->resolveClass(o::Value(1))==first);
    expect(owner->resolveClass(o::Value(u"1"))==second);
    expect(owner->resolveClass(o::Value(objectKey))==first);
    expect(owner->getKind("mi")==first);

    // Replacing the field with a generic Context Map keeps that exact owner.
    auto external=maps.createStorage();
    external->set(o::Value(u"external"),o::Value(firstClass));
    const auto externalMap=maps.bind(external);
    std::shared_ptr<t::FactoryClassMapAccess<Product>> rebound=
        t::typedFactoryClassMap<Product>(external,classFrom,classTo);
    expect(rebound->identity()==external->identity());
    expect(rebound->resolveClass(o::Value(u"external"))==first);
    rebound->set(o::Value(u"from-native"),t::FactoryClassMapValue<Product>::native(second));
    expect(external->get(o::Value(u"from-native"))==o::Value(secondClass));
    const auto reboundMap=maps.bind(t::projectFactoryClassMap<Product>(rebound,classTo));
    expect(reboundMap==externalMap);
    external->set(o::Value(u"from-map"),o::Value(secondClass));
    expect(rebound->resolveClass(o::Value(u"from-map"))==second);
  };

  "factory_source_kind_enumeration_copies_keys_without_resolving_values"_test = [] {
    o::Context context;
    o::MapContextBindings maps(context);
    auto external=maps.createStorage();
    const o::ObjectRef objectKey=context.object();
    external->set(o::Value(9),o::Value(u"not-a-registered-class"));
    external->set(o::Value(u"9"),o::Value(o::Null{}));
    external->set(o::Value(objectKey),o::Value(false));
    const auto map=maps.bind(external);
    t::AbstractFactory<Product> factory;
    auto rejectingClassResolver=[](const o::Value&)->MapValue{
      throw o::Exception("TypeError",u"nodeMap entry is not a registered factory class");
    };
    factory.nodeMapAccess(t::typedFactoryClassMap<Product>(
        external,rejectingClassResolver,
        [](const MapValue&)->o::Value{throw o::Exception("TypeError",u"No class conversion expected");}));
    const auto keys=factory.getKindsValues();
    const bool keyValuesMatch=keys==std::vector<o::Value>{o::Value(9),o::Value(u"9"),o::Value(objectKey)};
    expect(keyValuesMatch);
    expect(maps.resolve(map)->size()==3u);
  };

  "set_node_class_reads_current_map_again_and_defers_captured_constructor_resolution"_test = [] {
    auto firstClass=std::make_shared<t::FactoryClass<Product>>();
    auto selectedClass=std::make_shared<t::FactoryClass<Product>>();
    static Product firstProduct,selectedProduct;
    firstClass->construct=[](t::FactoryServices<Product>&,t::FactoryArgs){return &firstProduct;};
    selectedClass->construct=[](t::FactoryServices<Product>&,t::FactoryArgs){return &selectedProduct;};

    auto firstOwner=std::make_shared<t::FactoryClassMapOwner<Product>>();
    auto selectedOwner=std::make_shared<t::FactoryClassMapOwner<Product>>();
    selectedOwner->set(o::Value(u"probe"),MapValue::native(selectedClass));
    auto firstMap=std::make_shared<SwitchOnSetMap>(firstOwner);
    auto selectedMap=std::make_shared<SwitchOnSetMap>(selectedOwner);
    auto currentMap=std::static_pointer_cast<t::FactoryClassMapAccess<Product>>(firstMap);
    int mapReads=0;
    firstMap->afterSet=[&]{currentMap=selectedMap;};

    t::AbstractFactory<Product> factory;
    factory.nodeMapAccess(firstMap);
    factory.sourceNodeMapReader([&]{++mapReads;return currentMap;});
    factory.setNodeClass("probe",firstClass);

    expect(mapReads==2);
    expect(currentMap==selectedMap);
    expect(firstMap->resolveCount==0);
    expect(selectedMap->resolveCount==0);
    expect(factory.create("probe")==&selectedProduct);
    expect(firstMap->resolveCount==0);
    expect(selectedMap->resolveCount==1);
  };

  "factory_creator_replacement_is_the_native_owner_and_reads_apply_dynamically"_test = [] {
    o::Context context;
    auto runtimeLease=std::make_shared<std::atomic_bool>(true);
    auto bindingOwner=std::make_shared<int>(1);
    const o::ObjectRef node=context.object(),nextNode=context.object();
    const o::ObjectRef selected=context.function(u"selected",[](o::Context&,o::Value,
        std::span<const o::Value>){return o::Value{};});
    const o::ObjectRef nextSelected=context.function(u"next-selected",[](o::Context&,o::Value,
        std::span<const o::Value>){return o::Value{};});
    int firstApply=0,secondApply=0;
    const o::ObjectRef applyOne=context.function(u"apply-one",[&](o::Context&,o::Value,
        std::span<const o::Value>){++firstApply;return o::Value{};});
    const o::ObjectRef applyTwo=context.function(u"apply-two",[&](o::Context&,o::Value,
        std::span<const o::Value>){++secondApply;return o::Value{};});
    context.set(o::Value(selected),o::Key(u"apply"),o::Value(applyOne));
    context.set(o::Value(node),o::Key(u"kind"),o::Value(selected));
    context.set(o::Value(nextSelected),o::Key(u"apply"),o::Value(applyTwo));
    context.set(o::Value(nextNode),o::Key(u"kind"),o::Value(nextSelected));
    auto creator=std::make_shared<t::FactoryCreator<Product>>();
    creator->call=[](t::FactoryArgs)->Product*{return nullptr;};
    using Access=t::ContextFactoryCreatorAccess<Product>;
    const auto makeAccess=[&](o::ObjectRef owner,o::ObjectRef callable){
      return std::make_shared<Access>(owner,bindingOwner,runtimeLease,
      [&](const o::Value& object,const o::Key& key){return context.get(object,key);},
      [&](const o::Value& object,const o::Key& key,o::Value value){context.set(object,key,std::move(value));},
      [&](const o::Value& object,const o::Key& key){context.erase(std::get<o::ObjectRef>(object.data),key);},
      [&](const o::Value& object){return context.ownKeys(object);},
      [&](const o::Value& object){return context.prototype(std::get<o::ObjectRef>(object.data));},
      [&](const o::Value& object,std::optional<o::ObjectRef> prototype){context.setPrototype(std::get<o::ObjectRef>(object.data),prototype);},
      [&](const o::Value& value){
        const auto* object=std::get_if<o::ObjectRef>(&value.data);
        return object&&context.kind(*object)==o::ObjectKind::function;
      },
      [&,callable](const o::Value& value)->t::FactoryCreatorRef<Product>{
        const auto* object=std::get_if<o::ObjectRef>(&value.data);
        return object&&*object==callable?creator:t::FactoryCreatorRef<Product>{};
      },
      [callable](const t::FactoryCreatorRef<Product>&){return o::Value(callable);},
      [&](const o::Value& selectedValue){return context.get(selectedValue,o::Key(u"apply"));},
      [&,callable](const o::Value& selectedValue,const o::Value& applyValue,t::FactoryArgs)->Product*{
        expect(selectedValue==o::Value(callable));
        const auto* applyObject=std::get_if<o::ObjectRef>(&applyValue.data);
        if(!applyObject)throw o::Exception("TypeError",u"selected.apply is not callable");
        (void)context.call(*applyObject,selectedValue);
        static Product product;
        return &product;
      },
      [](const o::Value&,const o::Value&,t::FactoryArgs)->Product*{
        throw o::Exception("TypeError",u"unexpected direct invocation");
      });
    };
    auto access=makeAccess(node,selected);
    t::AbstractFactory<Product> factory;
    factory.creatorAccess(access);
    expect(factory.creatorSourceObject()==node);
    const bool creatorKeysMatch=factory.creatorAccess()->ownKeys()==std::vector<o::Key>{o::Key(u"kind")};
    expect(creatorKeysMatch);
    const auto nextAccess=makeAccess(nextNode,nextSelected);
    const auto firstResult=factory.createWithArguments("kind",[&]{
      // The selected .apply value and owner must already be captured here.
      context.set(o::Value(selected),o::Key(u"apply"),o::Value(applyTwo));
      factory.creatorAccess(nextAccess);
      return std::vector<std::any>{};
    });
    expect(firstResult!=nullptr);
    expect(firstApply==1);
    expect(factory.create("kind")!=nullptr);
    expect(firstApply==1);
    expect(secondApply==1);
  };

  "factory_falsy_lookup_keeps_null_vs_undefined_apply_errors_and_skips_arguments"_test = [] {
    o::Context context;
    auto runtimeLease=std::make_shared<std::atomic_bool>(true);
    auto bindingOwner=std::make_shared<int>(7);
    const o::ObjectRef node=context.object();
    context.set(o::Value(node),o::Key(u"kind"),o::Value(false));
    context.set(o::Value(node),o::Key(u"fallback"),o::Value(o::Null{}));
    using Access=t::ContextFactoryCreatorAccess<Product>;
    o::Value lastApplyReceiver=o::Undefined{};
    auto access=std::make_shared<Access>(node,bindingOwner,runtimeLease,
      [&](const o::Value& object,const o::Key& key){return context.get(object,key);},
      [&](const o::Value& object,const o::Key& key,o::Value value){context.set(object,key,std::move(value));},
      [&](const o::Value& object,const o::Key& key){context.erase(std::get<o::ObjectRef>(object.data),key);},
      [&](const o::Value& object){return context.ownKeys(object);},
      [&](const o::Value& object){return context.prototype(std::get<o::ObjectRef>(object.data));},
      [&](const o::Value& object,std::optional<o::ObjectRef> prototype){context.setPrototype(std::get<o::ObjectRef>(object.data),prototype);},
      [](const o::Value&){return false;},
      [](const o::Value&){return t::FactoryCreatorRef<Product>{};},
      [](const t::FactoryCreatorRef<Product>&){return o::Value(o::Undefined{});},
      [&](const o::Value& receiver)->o::Value{
        lastApplyReceiver=receiver;
        if(std::holds_alternative<o::Null>(receiver.data))
          throw o::Exception("TypeError",u"Cannot read properties of null (reading 'apply')");
        if(std::holds_alternative<o::Undefined>(receiver.data))
          throw o::Exception("TypeError",u"Cannot read properties of undefined (reading 'apply')");
        return o::Value(o::Undefined{});
      },
      [](const o::Value&,const o::Value&,t::FactoryArgs)->Product*{
        throw o::Exception("TypeError",u"unexpected apply invocation");
      },
      [](const o::Value&,const o::Value&,t::FactoryArgs)->Product*{
        throw o::Exception("TypeError",u"unexpected direct invocation");
      });
    t::AbstractFactory<Product> factory;
    factory.defaultKind=std::u16string(u"fallback");
    factory.creatorAccess(access);
    bool built=false;
    bool nullApply=false;
    try{(void)factory.createWithArguments("kind",[&]{built=true;return std::vector<std::any>{};});}
    catch(const o::Exception& error){nullApply=error.name=="TypeError"&&
      error.message==u"Cannot read properties of null (reading 'apply')";}
    expect(nullApply);
    expect(!built);
    expect(lastApplyReceiver==o::Value(o::Null{}));

    context.set(o::Value(node),o::Key(u"fallback"),o::Value(o::Undefined{}));
    bool undefinedApply=false;
    try{(void)factory.createWithArguments("kind",[&]{built=true;return std::vector<std::any>{};});}
    catch(const o::Exception& error){undefinedApply=error.name=="TypeError"&&
      error.message==u"Cannot read properties of undefined (reading 'apply')";}
    expect(undefinedApply);
    expect(!built);
    expect(lastApplyReceiver==o::Value(o::Undefined{}));
  };

  "replacement_node_access_keeps_weak_owner_without_factory_cycle"_test = [] {
    o::Context context;
    auto runtimeLease=std::make_shared<std::atomic_bool>(true);
    auto factory=std::make_shared<t::AbstractFactory<Product>>();
    const std::weak_ptr<t::AbstractFactory<Product>> weakFactory=factory;
    const o::ObjectRef object=context.object();
    using Access=t::ContextFactoryCreatorAccess<Product>;
    auto access=std::make_shared<Access>(object,std::static_pointer_cast<void>(factory),runtimeLease,
      [&](const o::Value& receiver,const o::Key& key){return context.get(receiver,key);},
      [&](const o::Value& receiver,const o::Key& key,o::Value value){context.set(receiver,key,std::move(value));},
      [&](const o::Value& receiver,const o::Key& key){context.erase(std::get<o::ObjectRef>(receiver.data),key);},
      [&](const o::Value& receiver){return context.ownKeys(receiver);},
      [&](const o::Value& receiver){return context.prototype(std::get<o::ObjectRef>(receiver.data));},
      [&](const o::Value& receiver,std::optional<o::ObjectRef> prototype){context.setPrototype(std::get<o::ObjectRef>(receiver.data),prototype);},
      [](const o::Value&){return false;},
      [](const o::Value&){return t::FactoryCreatorRef<Product>{};},
      [](const t::FactoryCreatorRef<Product>&){return o::Value(o::Undefined{});},
      [](const o::Value&){return o::Value(o::Undefined{});},
      [](const o::Value&,const o::Value&,t::FactoryArgs)->Product*{
        throw o::Exception("TypeError",u"unexpected apply invocation");
      },
      [](const o::Value&,const o::Value&,t::FactoryArgs)->Product*{
        throw o::Exception("TypeError",u"unexpected direct invocation");
      });
    factory->creatorAccess(access);

    factory.reset();
    expect(weakFactory.expired());
    bool rejected=false;
    try{(void)access->lookup("kind");}
    catch(const o::Exception& error){rejected=error.name=="TypeError";}
    expect(rejected);
  };
};
}
