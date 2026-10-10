// Root-owned native class binding for the source constructor object.
export module folia.math.core.mml_tree.class_binding;
export import folia.math.core.mml_tree.mml_node;
import std;
export namespace folia::math::mml {
template<class T>tree::FactoryClassRef<MmlNode> classBinding(){
    auto result=std::make_shared<tree::FactoryClass<MmlNode>>();
    result->nativeType=typeid(T);
    auto info=std::make_shared<MmlClassInfo>();
    if constexpr(std::is_base_of_v<AbstractMmlNode,T>)info->defaultsBinding=[]()->std::optional<PropertyList>{return T::defaults();};
    if constexpr(requires{T::accents();})info->accentsBinding=[] {return T::accents();};
    if constexpr(requires{T::template staticBinding<T>();})info->staticServices=T::template staticBinding<T>();
    result->staticData=info;
    result->construct=[info](Factory& factory,tree::FactoryArgs args){
        auto owned=std::make_unique<T>(factory);auto* node=owned.get();
        factory.adopt(std::move(owned));
        if constexpr(std::is_base_of_v<AbstractMmlNode,T>){
            node->bindConstructorClass(info);
            node->initializeMml(args);
            node->initializeClassFields();
        }else node->initializeNode(args);
        return node;
    };
    result->isInstance=[](const MmlNode* node){return dynamic_cast<const T*>(node)!=nullptr;};
    return result;
}
}
