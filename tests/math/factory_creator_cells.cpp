// Ignored focused witness for the same-owner creator cell projection.
#include "support/folia_test.hpp"
import folia.math.core.tree.factory_types;
import folia.math.core.tree.factory;
import folia.math.util.option_types;

namespace {
using namespace boost::ut;
namespace o=folia::math::options;
namespace t=folia::math::tree;
struct Product{};

class CreatorObjectView final:public o::NativeRecordView {
    t::FactoryCreatorView<Product> owner_;
    std::shared_ptr<void> ownerLease_;
    std::optional<o::ObjectRef> prototype_;
public:
    CreatorObjectView(t::FactoryCreatorView<Product> owner,o::Context& context,
                      std::shared_ptr<void> ownerLease={})
        :owner_(std::move(owner)),ownerLease_(std::move(ownerLease)),
         prototype_(context.intrinsics().objectPrototype){
        if(!owner_)throw o::Exception("TypeError",u"creator view requires its actual object owner");
    }
    std::optional<o::Descriptor> ownDescriptor(const o::Key& key)const override{
        const auto found=std::find_if(owner_->entries.begin(),owner_->entries.end(),
            [&](const auto& entry){return entry.first==key;});
        if(found==owner_->entries.end())return {};
        return found->second.descriptor;
    }
    bool ownPropertyEnumerable(const o::Key& key)const override{
        const auto found=std::find_if(owner_->entries.begin(),owner_->entries.end(),
            [&](const auto& entry){return entry.first==key;});
        return found!=owner_->entries.end()&&found->second.descriptor&&found->second.descriptor->enumerable;
    }
    std::vector<o::Key> ownKeys()const override{
        std::vector<o::Key> keys;
        keys.reserve(owner_->entries.size());
        for(const auto& [key,cell]:owner_->entries){(void)cell;keys.push_back(key);}
        return keys;
    }
    void defineOwn(const o::Key& key,o::Descriptor descriptor)override{
        const auto found=std::find_if(owner_->entries.begin(),owner_->entries.end(),
            [&](const auto& entry){return entry.first==key;});
        if(found==owner_->entries.end())
            owner_->entries.emplace_back(key,t::FactoryCreatorSlot<Product>{ {},std::move(descriptor) });
        else found->second.descriptor=std::move(descriptor);
    }
    void deleteOwn(const o::Key& key)override{
        std::erase_if(owner_->entries,[&](const auto& entry){return entry.first==key;});
    }
    std::optional<o::ObjectRef> prototype()const override{
        return prototype_;
    }
    void setPrototype(std::optional<o::ObjectRef> prototype)override{
        prototype_=prototype;
    }
};

suite creatorCellProjection = [] {
  "creator_context_record_and_retained_alias_share_per_key_cells"_test = [] {
    o::Context context;
    auto owner=std::make_shared<t::FactoryCreatorObject<Product>>();
    const auto retainedAlias=owner;
    auto sharedCreator=std::make_shared<t::FactoryCreator<Product>>();
    owner->entries.emplace_back(o::Key(u"left"),t::FactoryCreatorSlot<Product>{sharedCreator,{}});
    owner->entries.emplace_back(o::Key(u"right"),t::FactoryCreatorSlot<Product>{sharedCreator,{}});
    const auto symbol=context.symbol(u"slot-key");
    const std::u16string loneSurrogate(1,static_cast<char16_t>(0xD800));
    owner->entries.emplace_back(o::Key(symbol),t::FactoryCreatorSlot<Product>{sharedCreator,{}});
    owner->entries.emplace_back(o::Key(loneSurrogate),t::FactoryCreatorSlot<Product>{sharedCreator,{}});
    auto view=std::make_shared<CreatorObjectView>(owner,context);
    const auto object=context.nativeRecord(view);
    const auto sharedFunction=context.function(u"shared");
    context.define(object,o::Key(u"left"),
        o::Descriptor::data(o::Value(sharedFunction),true,true,true));
    context.define(object,o::Key(u"right"),
        o::Descriptor::data(o::Value(sharedFunction),false,false,true));
    context.define(object,o::Key(symbol),
        o::Descriptor::data(o::Value(sharedFunction),true,true,true));
    context.define(object,o::Key(loneSurrogate),
        o::Descriptor::data(o::Value(sharedFunction),true,true,true));

    expect(retainedAlias==owner);
    expect(retainedAlias->entries[0].second.creator==sharedCreator);
    expect(retainedAlias->entries[1].second.creator==sharedCreator);
    expect(retainedAlias->entries[0].second.descriptor->value==o::Value(sharedFunction));
    expect(retainedAlias->entries[1].second.descriptor->value==o::Value(sharedFunction));
    expect(retainedAlias->entries[0].second.descriptor->writable);
    expect(!retainedAlias->entries[1].second.descriptor->writable);
    expect(context.get(o::Value(object),o::Key(u"left"))==o::Value(sharedFunction));
    const auto ownKeys=context.ownKeys(o::Value(object));
    expect(std::find(ownKeys.begin(),ownKeys.end(),o::Key(symbol))!=ownKeys.end());
    expect(std::find(ownKeys.begin(),ownKeys.end(),o::Key(loneSurrogate))!=ownKeys.end());

    const auto replacement=context.function(u"replacement");
    context.set(o::Value(object),o::Key(u"left"),o::Value(replacement));
    expect(retainedAlias->entries[0].second.descriptor->value==o::Value(replacement));
    expect(retainedAlias->entries[1].second.creator==sharedCreator);
    expect(retainedAlias->entries[1].second.descriptor->value==o::Value(sharedFunction));

    // Mutating the retained native alias is visible through the existing
    // Context identity because the NativeRecordView has no shadow dictionary.
    retainedAlias->entries[0].second.descriptor=o::Descriptor::data(
        o::Value(sharedFunction),true,false,true);
    expect(context.get(o::Value(object),o::Key(u"left"))==o::Value(sharedFunction));
    expect(context.descriptor(o::Value(object),o::Key(u"left"))->enumerable==false);

    context.erase(object,o::Key(u"right"));
    expect(retainedAlias->entries.size()==3u);
  };

  "native_factory_dispatch_and_context_share_the_bound_creator_owner"_test = [] {
    o::Context context;
    auto execution=std::make_shared<std::atomic_bool>(true);
    t::AbstractFactory<Product> factory;
    auto access=std::dynamic_pointer_cast<t::FactoryCreatorObjectAccess<Product>>(
        factory.creatorAccess());
    expect(bool(access));
    const auto retainedAlias=access->nativeOwner();
    Product product;
    const auto productObject=context.object();
    context.payload(productObject)=&product;
    int nativeCalls=0;
    auto creator=std::make_shared<t::FactoryCreator<Product>>();
    creator->call=[&](t::FactoryArgs)->Product*{++nativeCalls;return &product;};
    retainedAlias->entries.emplace_back(o::Key(u"chosen"),t::FactoryCreatorSlot<Product>{creator,{}});
    auto recordView=std::make_shared<CreatorObjectView>(retainedAlias,context);
    const auto node=context.nativeRecord(recordView);

    t::FactoryCreatorContextBridge<Product> bridge;
    bridge.node=node;
    const std::shared_ptr<void> retainedFactoryOwner=recordView;
    bridge.ownerLease=retainedFactoryOwner;
    bridge.runtimeLease=execution;
    bridge.lookup=[&](o::ObjectRef receiver,const o::Key& key){
      const o::Value value=context.get(o::Value(receiver),key);
      if(std::holds_alternative<o::Undefined>(value.data))
        return t::FactoryCreatorSelection<Product>{};
      const auto* function=std::get_if<o::ObjectRef>(&value.data);
      t::FactoryCreatorRef<Product> native;
      if(function&&context.kind(*function)==o::ObjectKind::function){
        if(const auto* stored=std::any_cast<t::FactoryCreatorRef<Product>>(&context.payload(*function)))
          native=*stored;
      }
      return t::FactoryCreatorSelection<Product>{t::FactoryCreatorSelectionKind::source_value,std::move(native),value};
    };
    bridge.prepareApply=[&](const auto& selected){
      return t::PreparedFactoryCreatorCall<Product>{selected,context.get(selected.sourceValue,o::Key(u"apply"))};
    };
    bridge.invokePrepared=[&](const auto& prepared,t::FactoryArgs args)->Product*{
      const auto sourceArgs=context.array(static_cast<std::uint32_t>(args.size()));
      const std::array<o::Value,2> applyArgs{o::Value(o::Undefined{}),o::Value(sourceArgs)};
      const auto result=context.callMember(prepared.applyValue,prepared.selection.sourceValue,
          applyArgs,u"node[kind].apply");
      const auto* object=std::get_if<o::ObjectRef>(&result.data);
      if(!object)throw o::Exception("TypeError",u"creator result is not an object");
      return std::any_cast<Product*>(context.payload(*object));
    };
    bridge.invokeDirect=[](const o::Value&,const auto& selected,t::FactoryArgs args)->Product*{
      if(!selected.native||!selected.native->call)throw o::Exception("TypeError",u"not callable");
      return selected.native->call(args);
    };
    bridge.expose=[&](const t::FactoryCreatorRef<Product>& selected){
      const auto function=context.function(u"",[&,selected](o::Context&,o::Value,
          std::span<const o::Value>)->o::Value{
        auto* result=selected->call({});
        if(result!=&product)throw o::Exception("TypeError",u"wrong native product");
        return o::Value(productObject);
      },selected);
      return o::Value(function);
    };
    bridge.ownKeys=[&](o::ObjectRef receiver){return context.ownKeys(o::Value(receiver));};
    bridge.prototype=[&](o::ObjectRef receiver){return context.prototype(receiver);};
    bridge.setPrototype=[&](o::ObjectRef receiver,std::optional<o::ObjectRef> prototype){
      context.setPrototype(receiver,prototype);
    };
    access->bindContextNode(std::move(bridge));

    const auto initialFunction=context.get(o::Value(node),o::Key(u"chosen"));
    expect(retainedAlias==access->nativeOwner());
    expect(retainedAlias->entries[0].second.descriptor->value==initialFunction);
    expect(factory.create("chosen")==&product);
    expect(nativeCalls==1);

    context.set(o::Value(node),o::Key(u"false-value"),o::Value(false));
    context.set(o::Value(node),o::Key(u"zero-value"),o::Value(0));
    context.set(o::Value(node),o::Key(u"empty-value"),o::Value(u""));
    context.set(o::Value(node),o::Key(u"null-value"),o::Value(o::Null{}));
    for(const auto& [name,value]:std::array<std::pair<std::string_view,o::Value>,4>{
          std::pair{"false-value",o::Value(false)},std::pair{"zero-value",o::Value(0)},
          std::pair{"empty-value",o::Value(u"")},std::pair{"null-value",o::Value(o::Null{})}}){
      const auto selection=access->lookup(name);
      expect(selection.kind==t::FactoryCreatorSelectionKind::source_value);
      expect(selection.sourceValue==value);
      expect(!selection.sourceTruthy());
    }

    int replacementCalls=0;
    const auto replacement=context.function(u"replacement",[&](o::Context&,o::Value,
        std::span<const o::Value>)->o::Value{
      ++replacementCalls;
      return o::Value(productObject);
    });
    context.set(o::Value(node),o::Key(u"chosen"),o::Value(replacement));
    expect(retainedAlias->entries[0].second.descriptor->value==o::Value(replacement));
    expect(factory.create("chosen")==&product);
    expect(replacementCalls==1);
    expect(nativeCalls==1);

    retainedAlias->entries[0].second.descriptor=o::Descriptor::data(initialFunction);
    expect(context.get(o::Value(node),o::Key(u"chosen"))==initialFunction);
    expect(factory.create("chosen")==&product);
    expect(nativeCalls==2);
  };

  "creator_bridge_weak_owner_does_not_form_factory_access_cycle"_test = [] {
    o::Context context;
    auto runtimeLease=std::make_shared<std::atomic_bool>(true);
    auto factory=std::make_shared<t::AbstractFactory<Product>>();
    const std::weak_ptr<t::AbstractFactory<Product>> weakFactory=factory;
    auto access=std::dynamic_pointer_cast<t::FactoryCreatorObjectAccess<Product>>(
        factory->creatorAccess());
    expect(bool(access));
    t::FactoryCreatorContextBridge<Product> bridge;
    bridge.node=context.object();
    bridge.ownerLease=std::static_pointer_cast<void>(factory);
    bridge.runtimeLease=runtimeLease;
    bridge.lookup=[](o::ObjectRef,const o::Key&){return t::FactoryCreatorSelection<Product>{};};
    bridge.prepareApply=[](const auto& selection){return t::PreparedFactoryCreatorCall<Product>{selection,{}};};
    bridge.invokePrepared=[](const auto&,t::FactoryArgs)->Product*{return nullptr;};
    bridge.invokeDirect=[](const o::Value&,const auto& selected,t::FactoryArgs args)->Product*{
      if(!selected.native||!selected.native->call)throw o::Exception("TypeError",u"not callable");
      return selected.native->call(args);
    };
    bridge.expose=[](const t::FactoryCreatorRef<Product>&){return o::Value(o::Undefined{});};
    bridge.ownKeys=[](o::ObjectRef){return std::vector<o::Key>{};};
    bridge.prototype=[](o::ObjectRef){return std::optional<o::ObjectRef>{};};
    bridge.setPrototype=[](o::ObjectRef,std::optional<o::ObjectRef>){};
    access->bindContextNode(std::move(bridge));

    factory.reset();
    expect(weakFactory.expired());
    bool rejected=false;
    try{(void)access->lookup("kind");}
    catch(const o::Exception& error){rejected=error.name=="Unimplemented";}
    expect(rejected);
  };

  "context_callable_and_record_view_retain_the_concrete_factory_until_context_release"_test = [] {
    std::weak_ptr<t::AbstractFactory<Product>> weakFactory;
    {
      o::Context context;
      auto factory=std::make_shared<t::AbstractFactory<Product>>();
      weakFactory=factory;
      auto descriptor=std::make_shared<t::FactoryClass<Product>>();
      descriptor->construct=[](t::FactoryServices<Product>& services,t::FactoryArgs args)->Product*{
        (void)args;
        return services.adopt(std::make_unique<Product>());
      };
      descriptor->isInstance=[](const Product*){return true;};
      factory->setNodeClass("owned",descriptor);
      auto access=std::dynamic_pointer_cast<t::FactoryCreatorObjectAccess<Product>>(
          factory->creatorAccess());
      expect(bool(access));
      const auto nativeOwner=access->nativeOwner();
      const auto creator=nativeOwner->entries.front().second.creator;
      const auto function=context.function(u"owned",
          [factory,creator,&context](o::Context&,o::Value,std::span<const o::Value>)->o::Value{
        Product* const product=creator->callWithServices(*factory,{});
        const auto result=context.object();
        context.payload(result)=product;
        return o::Value(result);
      },creator);
      auto recordView=std::make_shared<CreatorObjectView>(nativeOwner,context,
          std::static_pointer_cast<void>(factory));
      const auto node=context.nativeRecord(recordView);

      factory.reset();
      expect(!weakFactory.expired());
      const auto result=context.call(function,o::Value(node));
      const auto* resultObject=std::get_if<o::ObjectRef>(&result.data);
      expect(resultObject!=nullptr);
      const auto* product=std::any_cast<Product*>(&context.payload(*resultObject));
      expect(product!=nullptr);
      expect(weakFactory.lock()->nodeIsKind(*product,"owned"));
    }
    expect(weakFactory.expired());
  };

  "factory_owned_bridge_expose_is_weak_but_returned_source_function_is_strong"_test = [] {
    std::weak_ptr<t::AbstractFactory<Product>> weakFactory;
    {
      o::Context context;
      auto runtimeLease=std::make_shared<std::atomic_bool>(true);
      auto factory=std::make_shared<t::AbstractFactory<Product>>();
      weakFactory=factory;
      auto descriptor=std::make_shared<t::FactoryClass<Product>>();
      descriptor->construct=[](t::FactoryServices<Product>& services,t::FactoryArgs)->Product*{
        return services.adopt(std::make_unique<Product>());
      };
      descriptor->isInstance=[](const Product*){return true;};
      factory->setNodeClass("owned",descriptor);
      auto access=std::dynamic_pointer_cast<t::FactoryCreatorObjectAccess<Product>>(
          factory->creatorAccess());
      expect(bool(access));
      const auto nativeOwner=access->nativeOwner();
      const auto creator=nativeOwner->entries.front().second.creator;
      const std::weak_ptr<t::AbstractFactory<Product>> weakOwner=factory;
      t::FactoryCreatorContextBridge<Product> bridge;
      bridge.node=context.object();
      bridge.ownerLease=std::static_pointer_cast<void>(factory);
      bridge.runtimeLease=runtimeLease;
      bridge.lookup=[](o::ObjectRef,const o::Key&){return t::FactoryCreatorSelection<Product>{};};
      bridge.prepareApply=[](const auto& selected){return t::PreparedFactoryCreatorCall<Product>{selected,{}};};
      bridge.invokePrepared=[](const auto&,t::FactoryArgs)->Product*{return nullptr;};
      bridge.invokeDirect=[](const o::Value&,const auto& selected,t::FactoryArgs args)->Product*{
        if(!selected.native||!selected.native->call)throw o::Exception("TypeError",u"not callable");
        return selected.native->call(args);
      };
      bridge.expose=[weakOwner,&context](const t::FactoryCreatorRef<Product>& selected){
        auto retainedFactory=weakOwner.lock();
        if(!retainedFactory)throw o::Exception("TypeError",u"Factory has expired");
        const auto function=context.function(u"",[retainedFactory,selected](o::Context&,o::Value,
            std::span<const o::Value>)->o::Value{
          Product* const product=selected->callWithServices(*retainedFactory,{});
          return o::Value(product!=nullptr);
        },selected);
        return o::Value(function);
      };
      bridge.ownKeys=[](o::ObjectRef){return std::vector<o::Key>{};};
      bridge.prototype=[](o::ObjectRef){return std::optional<o::ObjectRef>{};};
      bridge.setPrototype=[](o::ObjectRef,std::optional<o::ObjectRef>){};
      access->bindContextNode(std::move(bridge));

      // The factory owns the access and its bridge callback, but that callback
      // holds only a weak factory owner. The Source function made by expose is
      // an external root and captures the actual service strongly.
      factory.reset();
      expect(!weakFactory.expired());
      const auto& cell=nativeOwner->entries.front().second;
      const auto* function=std::get_if<o::ObjectRef>(&cell.descriptor->value.data);
      expect(function!=nullptr);
      expect(context.get(o::Value(*function),o::Key(u"name"))==o::Value(u""));
      expect(context.get(o::Value(*function),o::Key(u"length"))==o::Value(0));
      const auto result=context.call(*function,o::Value(o::Undefined{}));
      expect(result==o::Value(true));
    }
    expect(weakFactory.expired());
  };
};
}
