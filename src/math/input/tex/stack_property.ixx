// Typed input/tex/StackItem.ts Prop/PropList binding. Existing MML identities.
export module folia.math.input.tex.stack_property;
export import folia.math.core.mml_tree.mml_node;
export import folia.math.util.property_host;
import folia.math.util.option_types;
import std;
export namespace folia::math::tex {
struct StackPropertyStorage;
class StackPropertyList;
struct StackProperty;
using StackPropertyKey=options::Key;
class StackPropertyContextDelegate {
public:
    virtual ~StackPropertyContextDelegate()=default;
    virtual StackProperty read(const StackPropertyKey&)=0;
    virtual bool enumerable(const StackPropertyKey&)const=0;
    virtual void assign(const StackPropertyKey&,const StackProperty&)=0;
};
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
    std::vector<std::pair<StackPropertyKey,StackProperty>> entries;
    struct Attributes {
        StackPropertyKey key{u""};
        bool writable=true,enumerable=true,configurable=true,accessor=false;
        std::optional<options::ObjectRef> getter,setter;
    };
    // Descriptor metadata belongs to this same record; StackProperty remains
    // the sole value owner and is never mirrored into Context.
    std::vector<Attributes> attributes;
    // Bound only while a Context record view is alive. Typed assignment can
    // then perform ordinary descriptor checks, getter reads, and setter writes
    // without adding another property-value store to this owner.
    std::weak_ptr<StackPropertyContextDelegate> contextDelegate;
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
    static std::optional<std::uint32_t> index(const std::u16string& key){
        if(key.empty()||(key.size()>1&&key.front()==u'0'))return {};
        std::uint64_t n=0;
        for(const auto c:key){if(c<u'0'||c>u'9')return {};n=n*10+static_cast<unsigned>(c-u'0');if(n>=4294967295ULL)return {};}
        return static_cast<std::uint32_t>(n);
    }
    static StackPropertyKey key(std::string_view name){return StackPropertyKey(property_host::text(name));}
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
                for(const auto& [key,value]:(*record)->entries)if(const auto* text=std::get_if<std::u16string>(&key.data);text&&*text==u"__proto__")return false;
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
    // The adapter exposes this exact Source record through Context.  It is a
    // shared identity handle, never a serialized or copied entry list.
    const std::shared_ptr<StackPropertyStorage>& storage()const{return storage_;}
    static StackPropertyList fromStorage(std::shared_ptr<StackPropertyStorage> storage){
        if(!storage)throw std::invalid_argument("StackPropertyList requires its actual storage");
        return StackPropertyList(std::move(storage));
    }
    static StackPropertyList withPrototype(StackProperty parent){
        StackPropertyList out;out.set("__proto__",std::move(parent));return out;
    }
    bool hasOwn(std::string_view key)const{
        return hasOwn(StackPropertyList::key(key));
    }
    bool hasOwn(const StackPropertyKey& key)const{
        return std::ranges::any_of(storage_->entries,[&](const auto& e){return e.first==key;});
    }
    bool enumerableOwn(const StackPropertyKey& key)const{
        if(auto delegate=storage_->contextDelegate.lock())return delegate->enumerable(key);
        const auto entry=std::ranges::find_if(storage_->entries,[&](const auto& item){return item.first==key;});
        if(entry==storage_->entries.end())return false;
        const auto state=std::ranges::find_if(storage_->attributes,[&](const auto& item){return item.key==key;});
        return state==storage_->attributes.end()||state->enumerable;
    }
    StackProperty get(std::string_view key)const{
        return get(StackPropertyList::key(key));
    }
    StackProperty get(const StackPropertyKey& key)const{
        if(auto delegate=storage_->contextDelegate.lock())return delegate->read(key);
        StackProperty current(*this);
        while(true){
            if(auto* p=std::get_if<StackProperty::Record>(&current.value)){
                if(auto delegate=(*p)->contextDelegate.lock())return delegate->read(key);
                for(const auto& [name,value]:(*p)->entries)if(name==key){
                    const auto state=std::ranges::find_if((*p)->attributes,[&](const auto& item){return item.key==key;});
                    if(state!=(*p)->attributes.end()&&state->accessor){
                        if(!state->getter)return Missing{};
                        property_host::typeError("StackProperty accessor read requires Context dispatch");
                    }
                    return value;
                }
                current=(*p)->prototype;continue;
            }
            if(auto* p=std::get_if<Property>(&current.value)){
                if(auto record=PropertyList::record(*p)){
                    const auto* text=std::get_if<std::u16string>(&key.data);
                    if(!text)return Missing{};
                    const auto name=property_host::debugText(*text);
                    if(record->hasOwn(name))return record->getOwn(name);
                    current=next(*p);continue;
                }
                if(*p==Property(BuiltinProperty::prototype))
                    if(const auto* text=std::get_if<std::u16string>(&key.data)){
                        const auto name=property_host::debugText(*text);
                        return name=="__proto__"?storage_->prototype:StackProperty(PropertyList{}.get(name));
                    }else return Missing{};
                return Missing{};
            }
            property_host::typeError("Native stack node prototype is unbound");
        }
    }
    void setOwn(std::string_view key,StackProperty value){
        setOwn(StackPropertyList::key(key),std::move(value));
    }
    void setOwn(const StackPropertyKey& key,StackProperty value){
        for(auto& [name,old]:storage_->entries)if(name==key){old=std::move(value);return;}
        storage_->entries.emplace_back(key,std::move(value));
    }
    void set(std::string_view key,StackProperty value){
        set(StackPropertyList::key(key),std::move(value));
    }
    void set(const StackPropertyKey& key,StackProperty value){
        if(auto delegate=storage_->contextDelegate.lock()){delegate->assign(key,value);return;}
        if(const auto* text=std::get_if<std::u16string>(&key.data)){
          if(*text==u"__proto__"&&ordinarySetter(StackProperty(*this))){
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
        }
        const auto checkTarget=[&](const StackPropertyStorage& target)->bool{
            const auto found=std::ranges::find_if(target.entries,[&](const auto& entry){return entry.first==key;});
            if(found==target.entries.end())return false;
            const auto state=std::ranges::find_if(target.attributes,[&](const auto& attribute){return attribute.key==key;});
            if(state!=target.attributes.end()&&state->accessor){
                if(!state->setter)property_host::typeError("Property has no setter");
                property_host::typeError("Symbol accessor assignment requires Context dispatch");
            }
            if(state!=target.attributes.end()&&!state->writable)property_host::typeError("Cannot assign to read only property");
            return true;
        };
        if(checkTarget(*storage_)){setOwn(key,std::move(value));return;}
        StackProperty inherited=storage_->prototype;
        while(const auto* record=std::get_if<StackProperty::Record>(&inherited.value)){
            if(checkTarget(**record)){setOwn(key,std::move(value));return;}
            inherited=(*record)->prototype;
        }
        setOwn(key,std::move(value));
    }
    void erase(std::string_view key){erase(StackPropertyList::key(key));}
    void erase(const StackPropertyKey& key){std::erase_if(storage_->entries,[&](const auto& e){return e.first==key;});}
    std::vector<std::string> keys()const{
        std::vector<std::string> out;for(const auto& [key,value]:storage_->entries)if(const auto* text=std::get_if<std::u16string>(&key.data))out.push_back(property_host::debugText(*text));
        std::stable_sort(out.begin(),out.end(),[](const auto& a,const auto& b){auto i=index(a),j=index(b);return i&&j?*i<*j:bool(i)&&!j;});
        return out;
    }
    std::vector<StackPropertyKey> propertyKeys()const{
        std::vector<std::pair<std::uint32_t,StackPropertyKey>> indices;
        std::vector<StackPropertyKey> strings,symbols;
        for(const auto& [key,value]:storage_->entries){(void)value;
            if(const auto* text=std::get_if<std::u16string>(&key.data)){if(const auto i=index(*text))indices.emplace_back(*i,key);else strings.push_back(key);}
            else symbols.push_back(key);
        }
        std::sort(indices.begin(),indices.end(),[](const auto& a,const auto& b){return a.first<b.first;});
        std::vector<StackPropertyKey> result;result.reserve(indices.size()+strings.size()+symbols.size());
        for(auto& [i,key]:indices){(void)i;result.push_back(std::move(key));}
        result.insert(result.end(),strings.begin(),strings.end());result.insert(result.end(),symbols.begin(),symbols.end());return result;
    }
};
inline StackProperty::StackProperty(const StackPropertyList& p):value(p.storage_){}
namespace stack_property_host {
inline bool truthy(const StackProperty& value){
    if(auto* p=std::get_if<Property>(&value.value))return property_host::truthy(*p);
    return true;
}
inline std::vector<StackPropertyKey> ownKeys(const StackProperty& value){
    if(auto record=StackPropertyList::record(value))return record->propertyKeys();
    if(auto* p=std::get_if<Property>(&value.value)){
        if(property_host::nullish(*p))return {};
        std::vector<StackPropertyKey> result;
        for(const auto& key:property_host::ownKeys(*p))result.emplace_back(property_host::text(key));
        return result;
    }
    property_host::typeError("Native stack node assignment source is unbound");
}
inline StackProperty get(const StackProperty& value,std::string_view key){
    if(auto record=StackPropertyList::record(value))return record->get(key);
    if(auto* p=std::get_if<Property>(&value.value))return property_host::get(*p,key);
    property_host::typeError("Native stack node property reflection is unbound");
}
inline StackProperty get(const StackProperty& value,const StackPropertyKey& key){
    if(auto record=StackPropertyList::record(value))return record->get(key);
    if(const auto* p=std::get_if<Property>(&value.value)){
        if(const auto* text=std::get_if<std::u16string>(&key.data))return property_host::get(*p,property_host::debugText(*text));
        return Missing{};
    }
    property_host::typeError("Native stack node property reflection is unbound");
}
inline void assign(StackPropertyList& target,const StackProperty& source){
    const auto keys=ownKeys(source);
    for(const auto& key:keys){
        if(const auto record=StackPropertyList::record(source);record&&!record->enumerableOwn(key))continue;
        target.set(key,get(source,key));
    }
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
