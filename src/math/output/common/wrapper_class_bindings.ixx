/*************************************************************
 * Canonical Common wrapper class descriptor creation.
 * The actual Source constructor ObjectRef and its one native descriptor are
 * paired once, then consumed by the live SVG defaultNodes resolver.
 *************************************************************/
export module folia.math.output.common:wrapper_class_bindings;
export import :bindings;
export import :wrapper;
import std;

export namespace folia::math::output::common {
template<class Wrapper>
CommonWrapperClassRef registerCommonWrapperClass(
    CommonFactoryContextBindings& bindings,
    options::ObjectRef sourceConstructor,
    CommonWrapperClassData metadata) {
    static_assert(std::is_base_of_v<CommonWrapper,Wrapper>);
    // Keep the exact constructor paired with this descriptor. Callers supply
    // Source style metadata, but the constructor identity is authoritative
    // for prototype-chain instanceof checks and factory selection.
    metadata.sourceConstructor=sourceConstructor;
    auto descriptor=std::make_shared<tree::FactoryClass<CommonWrapper>>();
    descriptor->nativeType=typeid(Wrapper);
    descriptor->staticData=std::move(metadata);
    const std::weak_ptr<tree::FactoryClass<CommonWrapper>> selectedDescriptor=descriptor;
    descriptor->construct=[selectedDescriptor](tree::FactoryServices<CommonWrapper>& services,
                             tree::FactoryArgs args)->CommonWrapper* {
        auto* factory=dynamic_cast<CommonWrapperFactory*>(&services);
        if(!factory)throw options::Exception("TypeError",u"Wrapper constructor requires a CommonWrapperFactory");
        const auto selected=selectedDescriptor.lock();
        if(!selected)throw options::Exception("TypeError",u"Wrapper class descriptor has expired");
        CommonWrapperFactory::ClassConstructionScope classScope(*factory,selected);
        if(args.empty())throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'kind')");
        const auto* node=std::any_cast<mml::MmlNode*>(&args[0]);
        if(!node||!*node)throw options::Exception("TypeError",u"Wrapper constructor argument is not an MmlNode");
        CommonWrapper* parent=nullptr;
        if(args.size()>1){
            const auto* value=std::any_cast<CommonWrapper*>(&args[1]);
            if(!value)throw options::Exception("TypeError",u"Wrapper parent argument is not a CommonWrapper");
            parent=*value;
        }
        return factory->adopt(std::make_unique<Wrapper>(*factory,**node,parent));
    };
    auto* context=&bindings.context();
    descriptor->isInstance=[selectedDescriptor,context](const CommonWrapper* product){
        if(!dynamic_cast<const Wrapper*>(product))return false;
        const auto target=selectedDescriptor.lock();
        if(!target)return false;
        const auto* targetData=std::any_cast<CommonWrapperClassData>(&target->staticData);
        if(!targetData||!targetData->sourceConstructor)return false;
        const auto actualDescriptor=product->selectedClass();
        if(!actualDescriptor)return false;
        const auto* actualData=std::any_cast<CommonWrapperClassData>(&actualDescriptor->staticData);
        if(!actualData||!actualData->sourceConstructor)return false;
        const auto targetPrototypeValue=context->get(options::Value(*targetData->sourceConstructor),
                                                     options::Key(u"prototype"));
        const auto* targetPrototype=std::get_if<options::ObjectRef>(&targetPrototypeValue.data);
        if(!targetPrototype)return false;
        const auto actualPrototypeValue=context->get(options::Value(*actualData->sourceConstructor),
                                                     options::Key(u"prototype"));
        const auto* actualPrototype=std::get_if<options::ObjectRef>(&actualPrototypeValue.data);
        if(!actualPrototype)return false;
        auto current=std::optional<options::ObjectRef>(*actualPrototype);
        std::size_t depth=0;
        while(current&&depth++<1024){
            if(*current==*targetPrototype)return true;
            current=context->prototype(*current);
        }
        return false;
    };
    bindings.bindWrapperClass(descriptor,sourceConstructor);
    return descriptor;
}
}
