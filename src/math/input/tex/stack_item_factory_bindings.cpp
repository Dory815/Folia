// Root typed ownership/class binding, no substitute creator registry.
module folia.math.input.tex.stack_item_factory;
import folia.math.input.tex.node_factory_host;
import std;
namespace folia::math::tex {
namespace {
class DummyItem:public BaseItem {public:DummyItem()=default;};
tree::FactoryClassRef<BaseItem> dummyClass(){
    auto type=std::make_shared<tree::FactoryClass<BaseItem>>();
    type->nativeType=typeid(DummyItem);
    auto info=std::make_shared<StackItemClassInfo>();info->errorsBinding=[] {return BaseItem::baseErrors();};
    type->staticData=info;
    const std::weak_ptr<tree::FactoryClass<BaseItem>> identity=type;
    type->construct=[identity](tree::FactoryServices<BaseItem>& factory,tree::FactoryArgs args)->BaseItem*{
        auto owned=std::make_unique<DummyItem>();auto* item=owned.get();factory.adopt(std::move(owned));
        auto* service=dynamic_cast<StackItemFactoryService*>(&factory);
        if(!service)property_host::typeError("Native stack factory service is not bound");
        auto nodes=std::make_shared<tree::ChildArray<mml::MmlNode>>();
        for(const auto& arg:args)nodes->push_back(node_factory_host::node(arg));
        item->initializeBaseItem(service,nodes,identity.lock());
        return item;
    };
    type->isInstance=[](const BaseItem* item){return dynamic_cast<const DummyItem*>(item)!=nullptr;};
    return type;
}
}
tree::FactoryClassMap<BaseItem> StackItemFactory::DefaultStackItems{{"base",dummyClass()}};
StackItemFactory::StackItemFactory(std::optional<tree::FactoryClassMap<BaseItem>> nodes,
                                 const tree::FactoryClassMap<BaseItem>* defaults)
    :tree::AbstractFactory<BaseItem>(std::move(nodes),defaults){defaultKind="dummy";}
tree::FactoryServices<BaseItem>& StackItemFactory::itemFactory(){return *this;}
BaseItem* StackItemFactory::adopt(std::unique_ptr<BaseItem> item){
    if(!item)property_host::typeError("Cannot adopt a null factory product");
    auto owner=std::shared_ptr<BaseItem>(std::move(item));auto* raw=owner.get();
    products_.push_back(std::move(owner));return raw;
}
StackItemRef StackItemFactory::reference(BaseItem* item)const{
    for(const auto& owner:products_)if(owner.get()==item)return std::static_pointer_cast<StackItemContext>(owner);
    property_host::typeError("Native stack product does not belong to this session");
}
}
