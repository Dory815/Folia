// Root-owned typed Source operations, not a second parser or item graph.
export module folia.math.input.tex.stack_host;
export import folia.math.input.tex.stack;
import folia.math.input.tex.node_util;
import folia.math.util.property_host;
import std;
export namespace folia::math::tex::stack_host {
inline bool truthy(const StackEntry& entry){
    if(auto* p=std::get_if<Property>(&entry))return property_host::truthy(*p);
    if(auto* p=std::get_if<mml::NodeValue>(&entry))return p->isNode();
    return bool(std::get<StackItemRef>(entry));
}
inline BaseItem& item(const StackEntry& entry,std::string_view member){
    if(auto* p=std::get_if<StackItemRef>(&entry);p&&*p){
        if(auto* object=dynamic_cast<BaseItem*>(p->get()))return *object;
    }
    if(auto* p=std::get_if<Property>(&entry);p&&property_host::nullish(*p))
        property_host::typeError(std::string("Cannot read properties of ")+
            (p->missing()?"undefined":"null")+" (reading '"+std::string(member)+"')");
    if(auto* p=std::get_if<mml::NodeValue>(&entry);p&&!p->isNode())
        p->requireNode(member);
    property_host::typeError("Native Stack item reflection is unbound");
}
inline StackItemFactoryService& factory(const StackFactoryReference& ref){
    if(auto* p=std::get_if<StackItemFactoryService*>(&ref);p&&*p)return **p;
    property_host::typeError(std::string("Cannot read properties of ")+
        (std::holds_alternative<Missing>(ref)?"undefined":"null")+" (reading 'create')");
}
inline StackEntry create(const StackFactoryReference& ref,std::string_view kind,const std::any& arg){
    auto& f=factory(ref);auto* raw=f.itemFactory().create(kind,tree::FactoryArgs(&arg,1));
    return f.reference(raw);
}
inline bool isNode(const StackEntry& entry){
    if(auto* p=std::get_if<mml::NodeValue>(&entry))return NodeUtil::instance().isNode(std::any(*p));
    return false;
}
inline std::shared_ptr<CheckTuple> tuple(const CheckType& value){
    if(auto* p=std::get_if<std::shared_ptr<CheckTuple>>(&value);p&&*p)return *p;
    if(std::holds_alternative<Missing>(value))
        property_host::typeError("undefined is not iterable (cannot read property Symbol(Symbol.iterator))");
    property_host::typeError("object null is not iterable (cannot read property Symbol(Symbol.iterator))");
}
inline bool replacement(const std::variant<Missing,PropertyNull,CheckList>& value){
    return std::holds_alternative<CheckList>(value)&&bool(std::get<CheckList>(value));
}
inline std::vector<StackEntry> spread(const std::variant<Missing,PropertyNull,CheckList>& value){
    // Source spread snapshots the ordinary array iterator before recursive Push.
    return *std::get<CheckList>(value);
}
inline void assign(const Property& target,const Property& source){
    if(property_host::nullish(target))property_host::typeError("Cannot convert undefined or null to object");
    if(property_host::nullish(source))return;
    const auto keys=property_host::ownKeys(source);
    if(auto record=PropertyList::record(target)){
        for(const auto& key:keys)record->set(key,property_host::get(source,key));
        return;
    }
    // Enabled parser environments are shared records. Empty scalar boxing is
    // exact; assigning reflected properties to scalar boxes is not a binding.
    if(keys.empty())return;
    property_host::typeError("Native Stack scalar assignment target is unbound");
}
inline void deleteEnv(BaseItem&){
    // All enabled BaseItem descendants inherit the env accessor. JS delete of
    // a missing own property succeeds without invoking its setter/backing field.
}
inline StackEntry index(const std::vector<StackItemRef>& entries,double n){
    if(!std::isfinite(n)||n<0||std::trunc(n)!=n||n>=static_cast<double>(entries.size()))
        return Property(Missing{});
    return entries[static_cast<std::size_t>(n)];
}
inline std::string join(const std::vector<StackItemRef>& entries,std::string_view separator){
    std::string out;const auto length=entries.size();
    for(std::size_t i=0;i<length;++i){
        if(i)out+=separator;
        const StackEntry entry=i<entries.size()?StackEntry(entries[i]):StackEntry(Property(Missing{}));
        if(!truthy(entry))continue;
        out+=item(entry,"toString").toString();
    }
    return out;
}
}
