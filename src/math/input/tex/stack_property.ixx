// Typed input/tex/StackItem.ts Prop/PropList binding. Existing MML identities.
export module folia.math.input.tex.stack_property;
export import folia.math.core.mml_tree.mml_node;
export import folia.math.util.property_host;
import std;
export namespace folia::math::tex {
struct StackPropertyStorage;
class StackPropertyList;
struct StackProperty {
    using Record=std::shared_ptr<StackPropertyStorage>;
    std::variant<Property,mml::NodeValue,Record> value=Property(Missing{});
    StackProperty()=default;
    StackProperty(Property p):value(std::move(p)){}
    StackProperty(Missing p):value(Property(p)){}
    StackProperty(PropertyNull p):value(Property(p)){}
    StackProperty(bool p):value(Property(p)){}
    StackProperty(double p):value(Property(p)){}
    StackProperty(int p):value(Property(p)){}
    StackProperty(const char* p):value(Property(p)){}
    StackProperty(std::string p):value(Property(std::move(p))){}
    StackProperty(const PropertyList& p):value(Property(p)){}
    StackProperty(const StackPropertyList&);
    StackProperty(mml::NodeValue p){
        if(p.isNode())value=p;
        else value=p.kind()==tree::ChildKind::Null?Property(PropertyNull{}):Property(Missing{});
    }
    StackProperty(mml::MmlNode* p):StackProperty(mml::NodeValue(p)){}
    bool operator==(const StackProperty&)const=default;
};
struct StackPropertyStorage {
    std::vector<std::pair<std::string,StackProperty>> entries;
    StackProperty prototype=Property(BuiltinProperty::prototype);
};
class StackPropertyList {
    friend struct StackProperty;
    std::shared_ptr<StackPropertyStorage> storage_=std::make_shared<StackPropertyStorage>();
    explicit StackPropertyList(std::shared_ptr<StackPropertyStorage> p):storage_(std::move(p)){}
    static std::optional<std::uint32_t> index(std::string_view key){
        if(key.empty()||(key.size()>1&&key.front()=='0'))return {};
        std::uint64_t n=0;const auto [end,error]=std::from_chars(key.data(),key.data()+key.size(),n);
        if(error!=std::errc{}||end!=key.data()+key.size()||n>=4294967295ULL)return {};
        return static_cast<std::uint32_t>(n);
    }
    // Shared scalar records keep their original PropertyStorage identity.
    // The rich graph is created rich; scalar objects are never promoted/copied.
    static StackProperty next(const Property& p){
        if(auto* storage=std::get_if<std::shared_ptr<PropertyStorage>>(&p.value);storage&&*storage){
            if((*storage)->binding)return (*storage)->binding->prototype();
            if((*storage)->array)property_host::typeError("Native stack property array prototype is unbound");
            if((*storage)->prototype){Property out;out.value=(*storage)->prototype;return out;}
            return (*storage)->objectPrototype?StackProperty(Property(BuiltinProperty::prototype)):StackProperty(PropertyNull{});
        }
        return PropertyNull{};
    }
    static bool ordinarySetter(StackProperty current){
        while(true){
            if(auto* record=std::get_if<StackProperty::Record>(&current.value)){
                for(const auto& [key,value]:(*record)->entries)if(key=="__proto__")return false;
                current=(*record)->prototype;continue;
            }
            if(auto* p=std::get_if<Property>(&current.value)){
                if(auto record=PropertyList::record(*p)){
                    if(record->hasOwn("__proto__"))return false;
                    current=next(*p);continue;
                }
                return *p==Property(BuiltinProperty::prototype);
            }
            property_host::typeError("Native stack node prototype is unbound");
        }
    }
public:
    StackPropertyList()=default;
    StackPropertyList(std::initializer_list<std::pair<std::string,StackProperty>> values){
        for(const auto& [key,value]:values)setOwn(key,value);
    }
    static std::optional<StackPropertyList> record(const StackProperty& value){
        if(auto* p=std::get_if<StackProperty::Record>(&value.value);p&&*p)return StackPropertyList(*p);
        return {};
    }
    static StackPropertyList withPrototype(StackProperty parent){
        StackPropertyList out;out.set("__proto__",std::move(parent));return out;
    }
    bool hasOwn(std::string_view key)const{
        return std::ranges::any_of(storage_->entries,[&](const auto& e){return e.first==key;});
    }
    StackProperty get(std::string_view key)const{
        StackProperty current(*this);
        while(true){
            if(auto* p=std::get_if<StackProperty::Record>(&current.value)){
                for(const auto& [name,value]:(*p)->entries)if(name==key)return value;
                current=(*p)->prototype;continue;
            }
            if(auto* p=std::get_if<Property>(&current.value)){
                if(auto record=PropertyList::record(*p)){
                    if(record->hasOwn(key))return record->getOwn(key);
                    current=next(*p);continue;
                }
                if(*p==Property(BuiltinProperty::prototype))
                    return key=="__proto__"?storage_->prototype:StackProperty(PropertyList{}.get(key));
                return Missing{};
            }
            property_host::typeError("Native stack node prototype is unbound");
        }
    }
    void setOwn(std::string_view key,StackProperty value){
        for(auto& [name,old]:storage_->entries)if(name==key){old=std::move(value);return;}
        storage_->entries.emplace_back(key,std::move(value));
    }
    void set(std::string_view key,StackProperty value){
        if(key=="__proto__"&&ordinarySetter(StackProperty(*this))){
            if(auto* node=std::get_if<mml::NodeValue>(&value.value);node&&node->isNode())
                property_host::typeError("Native stack node prototype is unbound");
            if(std::holds_alternative<StackProperty::Record>(value.value)){
                StackProperty cursor=value;
                while(auto* p=std::get_if<StackProperty::Record>(&cursor.value)){
                    if(*p==storage_)property_host::typeError("Cyclic __proto__ value");
                    cursor=(*p)->prototype;
                }
                storage_->prototype=std::move(value);
            }else if(auto* p=std::get_if<Property>(&value.value);p&&
                (PropertyList::record(*p)||std::holds_alternative<PropertyNull>(p->value)||*p==Property(BuiltinProperty::prototype)))
                storage_->prototype=std::move(value);
            return; // inherited setter ignores primitive assignments
        }
        setOwn(key,std::move(value));
    }
    void erase(std::string_view key){std::erase_if(storage_->entries,[&](const auto& e){return e.first==key;});}
    std::vector<std::string> keys()const{
        std::vector<std::string> out;for(const auto& [key,value]:storage_->entries)out.push_back(key);
        std::stable_sort(out.begin(),out.end(),[](const auto& a,const auto& b){auto i=index(a),j=index(b);return i&&j?*i<*j:bool(i)&&!j;});
        return out;
    }
};
inline StackProperty::StackProperty(const StackPropertyList& p):value(p.storage_){}
namespace stack_property_host {
inline bool truthy(const StackProperty& value){
    if(auto* p=std::get_if<Property>(&value.value))return property_host::truthy(*p);
    return true;
}
inline std::vector<std::string> ownKeys(const StackProperty& value){
    if(auto record=StackPropertyList::record(value))return record->keys();
    if(auto* p=std::get_if<Property>(&value.value)){
        if(property_host::nullish(*p))return {};
        return property_host::ownKeys(*p);
    }
    property_host::typeError("Native stack node assignment source is unbound");
}
inline StackProperty get(const StackProperty& value,std::string_view key){
    if(auto record=StackPropertyList::record(value))return record->get(key);
    if(auto* p=std::get_if<Property>(&value.value))return property_host::get(*p,key);
    property_host::typeError("Native stack node property reflection is unbound");
}
inline void assign(StackPropertyList& target,const StackProperty& source){
    const auto keys=ownKeys(source);
    for(const auto& key:keys)target.set(key,get(source,key));
}
inline Property scalar(const StackProperty& value){
    if(auto* p=std::get_if<Property>(&value.value))return *p;
    property_host::typeError("Native stack scalar property is not a Property");
}
inline mml::NodeValue node(const StackProperty& value){
    if(auto* p=std::get_if<mml::NodeValue>(&value.value))return *p;
    if(auto* p=std::get_if<Property>(&value.value)){
        if(p->missing())return Missing{};
        if(std::holds_alternative<PropertyNull>(p->value))return PropertyNull{};
    }
    property_host::typeError("Native stack node property is not an MML product");
}
}
}
