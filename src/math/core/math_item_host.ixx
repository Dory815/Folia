// Root typed native engine edges for MathItem, no lifecycle algorithms.
export module folia.math.core.math_item_host;
export import folia.math.core.math_item;
import folia.math.util.property_host;
import std;
export namespace folia::math::core::math_item_host {
template<class Service,class Ref> Service& require(const Ref& ref,std::string_view member){
    if(auto* p=std::get_if<Service*>(&ref);p&&*p)return **p;
    property_host::typeError(std::string("Cannot read properties of ")+
        (std::holds_alternative<Missing>(ref)?"undefined":"null")+" (reading '"+std::string(member)+"')");
}
inline MathRenderActionsReference actions(MathDocumentReference ref){
    return require<MathDocumentService>(ref,"renderActions").renderActionsReference();
}
inline MathOutputJaxReference output(MathDocumentReference ref){
    return require<MathDocumentService>(ref,"outputJax").outputJaxReference();
}
inline Property constant(std::string_view name){return STATE.get(name);}
inline bool less(const Property& a,const Property& b){
    const auto first=property_host::primitive(a,false),second=property_host::primitive(b,false);
    if(auto*x=std::get_if<std::string>(&first.value))if(auto*y=std::get_if<std::string>(&second.value))
        return property_host::text(*x)<property_host::text(*y);
    return property_host::number(first)<property_host::number(second);
}
inline bool greaterEqual(const Property& a,const Property& b){
    // >= has the same left-to-right primitive conversion, with NaN false.
    const auto first=property_host::primitive(a,false),second=property_host::primitive(b,false);
    if(auto*x=std::get_if<std::string>(&first.value))if(auto*y=std::get_if<std::string>(&second.value))
        return property_host::text(*x)>=property_host::text(*y);
    const double x=property_host::number(first),y=property_host::number(second);
    return x>=y;
}
inline bool hasState(std::string_view key){
    Property current=STATE;
    while(auto record=PropertyList::record(current)){
        if(record->hasOwn(key))return true;
        auto storage=std::get<std::shared_ptr<PropertyStorage>>(current.value);
        if(storage->prototype){current.value=storage->prototype;continue;}
        return storage->objectPrototype&&!PropertyList{}.get(key).missing();
    }
    return false;
}
[[noreturn]] inline void duplicateState(std::string_view name){
    throw tree::FactoryError("Error",property_host::text("State "+std::string(name)+" already exists"));
}
}
