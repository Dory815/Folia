module folia.math.util.context_property_bindings;
import folia.math.util.property_host;
import std;
namespace folia::math {
using namespace options;
struct ContextPropertyBindings::State:std::enable_shared_from_this<State> {
    Context& context;
    OptionFunctionBindings& functions;
    // Weak handles prevent cycles when a record contains itself. The binding
    // retains this owner; Context owns the actual fields and nested records.
    std::vector<std::pair<ObjectRef,std::weak_ptr<PropertyStorage>>> records;
    std::vector<std::pair<ContextPropertyBindings::PropertyToObject,
                          ContextPropertyBindings::ObjectToProperty>> projections;
    State(Context& context,OptionFunctionBindings& functions):context(context),functions(functions){}
    ObjectRef objectFor(const std::shared_ptr<PropertyStorage>& storage);
    Value toValue(const Property& value);
    Property fromValue(Value value){return from(std::move(value));}
    struct NativeRecord final:NativeRecordView {
        std::shared_ptr<State> owner;
        std::shared_ptr<PropertyStorage> storage;
        NativeRecord(std::shared_ptr<State> owner,std::shared_ptr<PropertyStorage> storage)
            :owner(std::move(owner)),storage(std::move(storage)){}
        static std::string keyText(const Key& key){
            const auto* text=std::get_if<std::u16string>(&key.data);
            if(!text)throw Exception("Unimplemented",u"PropertyStorage does not provide native Symbol-key records");
            return property_host::debugText(*text);
        }
        static std::optional<std::uint32_t> indexOf(std::string_view text){
            if(text.empty()||(text.size()>1&&text.front()=='0'))return {};
            std::uint32_t value{};
            const auto [end,error]=std::from_chars(text.data(),text.data()+text.size(),value);
            if(error!=std::errc{}||end!=text.data()+text.size()||value==std::numeric_limits<std::uint32_t>::max())return {};
            return value;
        }
        std::optional<Descriptor> ownDescriptor(const Key& key)const override{
            if(const auto* symbol=std::get_if<SymbolRef>(&key.data)){
                if(storage->binding){
                    if(!storage->binding->hasOwnSymbol(*symbol))return {};
                    return Descriptor::data(owner->toValue(storage->binding->getOwnSymbol(*symbol)));
                }
                for(const auto& [name,value]:storage->symbolEntries)if(name==*symbol)
                    return Descriptor::data(owner->toValue(value));
                return {};
            }
            if(!std::holds_alternative<std::u16string>(key.data))return {};
            const auto text=keyText(key);
            if(storage->binding){
                if(!storage->binding->hasOwn(text))return {};
                return Descriptor::data(owner->toValue(storage->binding->getOwn(text)));
            }
            for(const auto& [name,value]:storage->entries)if(name==text)
                return Descriptor::data(owner->toValue(value));
            return {};
        }
        bool ownPropertyEnumerable(const Key& key)const override{
            if(const auto* symbol=std::get_if<SymbolRef>(&key.data)){
                if(storage->binding)return storage->binding->ownSymbolEnumerable(*symbol);
                for(const auto& [name,value]:storage->symbolEntries){(void)value;if(name==*symbol)return true;}
                return false;
            }
            if(!std::holds_alternative<std::u16string>(key.data))return false;
            const auto text=keyText(key);
            if(storage->binding)return storage->binding->ownPropertyEnumerable(text);
            for(const auto& [name,value]:storage->entries){(void)value;if(name==text)return true;}
            return false;
        }
        std::vector<Key> ownKeys()const override{
            std::vector<Key> result;
            if(storage->binding){
                for(const auto& name:storage->binding->ownStringKeys())
                    result.emplace_back(property_host::text(name));
                for(const auto& symbol:storage->binding->ownSymbolKeys())result.emplace_back(symbol);
                return result;
            }
            result.reserve(storage->entries.size()+storage->symbolEntries.size());
            std::vector<std::string> indices,names;
            for(const auto& [name,value]:storage->entries){
                (void)value;
                (NativeRecord::indexOf(name)?indices:names).push_back(name);
            }
            std::sort(indices.begin(),indices.end(),[](const auto& left,const auto& right){
                return *NativeRecord::indexOf(left)<*NativeRecord::indexOf(right);
            });
            for(const auto& name:indices)result.emplace_back(property_host::text(name));
            for(const auto& name:names)result.emplace_back(property_host::text(name));
            for(const auto& [symbol,value]:storage->symbolEntries){(void)value;result.emplace_back(symbol);}
            return result;
        }
        void defineOwn(const Key& key,Descriptor descriptor)override{
            if(const auto* symbol=std::get_if<SymbolRef>(&key.data)){
                if(descriptor.accessor)throw Exception("Unimplemented",u"PropertyStorage has no accessor descriptor storage");
                if(!descriptor.writable||!descriptor.enumerable||!descriptor.configurable)
                    throw Exception("Unimplemented",u"PropertyStorage does not retain ordinary descriptor attributes");
                const auto value=owner->fromValue(std::move(descriptor.value));
                if(storage->binding){storage->binding->setOwnSymbol(*symbol,value);return;}
                for(auto& [name,old]:storage->symbolEntries)if(name==*symbol){old=value;return;}
                storage->symbolEntries.emplace_back(*symbol,value);return;
            }
            const auto text=keyText(key);
            if(descriptor.accessor)throw Exception("Unimplemented",u"PropertyStorage has no accessor descriptor storage");
            if(!descriptor.writable||!descriptor.enumerable||!descriptor.configurable)
                throw Exception("Unimplemented",u"PropertyStorage does not retain ordinary descriptor attributes");
            const auto value=owner->fromValue(std::move(descriptor.value));
            if(storage->binding){storage->binding->setOwn(text,value);return;}
            if(storage->array){
                if(text=="length"){
                    const auto* number=std::get_if<double>(&value.value);
                    if(!number||!std::isfinite(*number)||*number<0||*number>4294967295.0||std::trunc(*number)!=*number)
                        throw Exception("RangeError",u"Invalid array length");
                    storage->arrayValues.resize(static_cast<std::size_t>(*number));
                    storage->arrayPresent.resize(static_cast<std::size_t>(*number),false);return;
                }
                if(auto index=indexOf(text)){
                    if(*index>=storage->arrayValues.size()){
                        storage->arrayValues.resize(std::size_t(*index)+1);
                        storage->arrayPresent.resize(std::size_t(*index)+1,false);
                    }
                    storage->arrayValues[*index]=value;storage->arrayPresent[*index]=true;return;
                }
            }
            for(auto& [name,old]:storage->entries)if(name==text){old=value;return;}
            storage->entries.emplace_back(text,value);
        }
        void deleteOwn(const Key& key)override{
            if(const auto* symbol=std::get_if<SymbolRef>(&key.data)){
                if(storage->binding){storage->binding->eraseSymbol(*symbol);return;}
                std::erase_if(storage->symbolEntries,[&](const auto& item){return item.first==*symbol;});
                return;
            }
            const auto text=keyText(key);
            if(storage->binding){storage->binding->erase(text);return;}
            if(storage->array){
                if(text=="length")throw Exception("TypeError",u"Cannot delete array length");
                if(auto index=indexOf(text))if(*index<storage->arrayPresent.size())
                    storage->arrayPresent[*index]=false;
            }
            std::erase_if(storage->entries,[&](const auto& item){return item.first==text;});
        }
        std::optional<ObjectRef> prototype()const override{
            if(storage->prototype)return const_cast<State*>(owner.get())->objectFor(storage->prototype);
            if(storage->objectPrototype)return owner->context.intrinsics().objectPrototype;
            return {};
        }
        void setPrototype(std::optional<ObjectRef> prototype_ref)override{
            if(!prototype_ref){storage->prototype.reset();storage->objectPrototype=false;return;}
            if(*prototype_ref==owner->context.intrinsics().objectPrototype){storage->prototype.reset();storage->objectPrototype=true;return;}
            const auto parent=owner->fromValue(Value(*prototype_ref));
            const auto* value=std::get_if<std::shared_ptr<PropertyStorage>>(&parent.value);
            if(!value||!*value)throw Exception("TypeError",u"Prototype is not a PropertyStorage record");
            storage->prototype=*value;storage->objectPrototype=false;
        }
    };
    struct NativeArray final:NativeIndexedView,NativeRecordView {
        std::shared_ptr<State> owner;
        std::shared_ptr<PropertyStorage> storage;
        NativeArray(std::shared_ptr<State> owner,std::shared_ptr<PropertyStorage> storage)
            :owner(std::move(owner)),storage(std::move(storage)){}
        static std::optional<std::uint32_t> arrayIndex(std::string_view text){
            return NativeRecord::indexOf(text);
        }
        std::uint32_t length()const override{
            if(storage->binding){
                if(!storage->binding->hasOwn("length"))
                    throw Exception("TypeError",u"Bound array has no own length property");
                const auto value=owner->toValue(storage->binding->getOwn("length"));
                const auto* number=std::get_if<double>(&value.data);
                if(!number||!std::isfinite(*number)||*number<0||*number>4294967295.0||std::trunc(*number)!=*number)
                    throw Exception("RangeError",u"Invalid array length");
                return static_cast<std::uint32_t>(*number);
            }
            return static_cast<std::uint32_t>(storage->arrayValues.size());
        }
        Descriptor lengthDescriptor()const override{
            if(storage->binding&&storage->binding->hasOwn("length"))
                return Descriptor::data(owner->toValue(storage->binding->getOwn("length")),true,false,false);
            return Descriptor::data(Value(static_cast<double>(length())),true,false,false);
        }
        std::optional<Descriptor> indexDescriptor(std::uint32_t index)const override{
            if(storage->binding){
                const auto name=std::to_string(index);
                if(!storage->binding->hasOwn(name))return {};
                return Descriptor::data(owner->toValue(storage->binding->getOwn(name)));
            }
            if(index>=storage->arrayPresent.size()||!storage->arrayPresent[index])return {};
            return Descriptor::data(owner->toValue(storage->arrayValues[index]));
        }
        std::vector<std::uint32_t> ownIndexKeys()const override{
            std::vector<std::uint32_t> result;
            if(storage->binding){
                for(const auto& name:storage->binding->keys())if(const auto index=arrayIndex(name);index&&*index<length())result.push_back(*index);
                std::sort(result.begin(),result.end());
                return result;
            }
            for(std::size_t i=0;i<storage->arrayPresent.size();++i)
                if(storage->arrayPresent[i])result.push_back(static_cast<std::uint32_t>(i));
            return result;
        }
        void defineIndex(std::uint32_t index,Descriptor descriptor)override{
            if(descriptor.accessor||!descriptor.writable||!descriptor.enumerable||!descriptor.configurable)
                throw Exception("Unimplemented",u"PropertyArray indices do not retain descriptor attributes");
            if(storage->binding){
                storage->binding->setOwn(std::to_string(index),owner->fromValue(std::move(descriptor.value)));
                return;
            }
            if(index>=storage->arrayValues.size()){
                storage->arrayValues.resize(std::size_t(index)+1);
                storage->arrayPresent.resize(std::size_t(index)+1,false);
            }
            storage->arrayValues[index]=owner->fromValue(std::move(descriptor.value));
            storage->arrayPresent[index]=true;
        }
        void defineLength(Descriptor descriptor)override{
            if(descriptor.accessor)throw Exception("TypeError",u"Array length cannot be an accessor");
            if(!descriptor.writable||descriptor.enumerable||descriptor.configurable)
                throw Exception("Unimplemented",u"PropertyArray length does not retain descriptor attributes");
            if(storage->binding){
                storage->binding->setOwn("length",owner->fromValue(std::move(descriptor.value)));
                return;
            }
            const auto value=owner->fromValue(std::move(descriptor.value));
            const auto* number=std::get_if<double>(&value.value);
            if(!number||!std::isfinite(*number)||*number<0||*number>4294967295.0||std::trunc(*number)!=*number)
                throw Exception("RangeError",u"Invalid array length");
            storage->arrayValues.resize(static_cast<std::size_t>(*number));
            storage->arrayPresent.resize(static_cast<std::size_t>(*number),false);
        }
        void deleteIndex(std::uint32_t index)override{
            if(storage->binding){storage->binding->erase(std::to_string(index));return;}
            if(index<storage->arrayPresent.size())storage->arrayPresent[index]=false;
        }
        std::optional<Descriptor> ownDescriptor(const Key& key)const override{
            if(const auto* symbol=std::get_if<SymbolRef>(&key.data)){
                if(storage->binding){
                    if(!storage->binding->hasOwnSymbol(*symbol))return {};
                    return Descriptor::data(owner->toValue(storage->binding->getOwnSymbol(*symbol)));
                }
                for(const auto& [name,value]:storage->symbolEntries)if(name==*symbol)
                    return Descriptor::data(owner->toValue(value));
                return {};
            }
            if(!std::holds_alternative<std::u16string>(key.data))return {};
            const auto text=NativeRecord::keyText(key);
            if(storage->binding){
                if(!storage->binding->hasOwn(text))return {};
                return Descriptor::data(owner->toValue(storage->binding->getOwn(text)));
            }
            for(const auto& [name,value]:storage->entries)if(name==text)return Descriptor::data(owner->toValue(value));
            return {};
        }
        bool ownPropertyEnumerable(const Key& key)const override{
            if(std::holds_alternative<std::u16string>(key.data)){
                const auto text=NativeRecord::keyText(key);
                if(text=="length")return false;
                if(const auto index=arrayIndex(text)){
                    if(storage->binding)return storage->binding->ownPropertyEnumerable(text);
                    return *index<storage->arrayPresent.size()&&storage->arrayPresent[*index];
                }
                if(storage->binding)return storage->binding->ownPropertyEnumerable(text);
                for(const auto& [own,value]:storage->entries){(void)value;if(own==text)return true;}
                return false;
            }
            if(const auto* symbol=std::get_if<SymbolRef>(&key.data)){
                if(storage->binding)return storage->binding->ownSymbolEnumerable(*symbol);
                for(const auto& [own,value]:storage->symbolEntries){(void)value;if(own==*symbol)return true;}
            }
            return false;
        }
        std::vector<Key> ownKeys()const override{
            std::vector<Key> result;
            if(storage->binding){
                for(const auto& name:storage->binding->ownStringKeys())
                    if(!arrayIndex(name))result.emplace_back(property_host::text(name));
                for(const auto& symbol:storage->binding->ownSymbolKeys())result.emplace_back(symbol);
                return result;
            }
            result.reserve(storage->entries.size()+storage->symbolEntries.size());
            for(const auto& [name,value]:storage->entries){(void)value;result.emplace_back(property_host::text(name));}
            for(const auto& [symbol,value]:storage->symbolEntries){(void)value;result.emplace_back(symbol);}
            return result;
        }
        void defineOwn(const Key& key,Descriptor descriptor)override{
            if(const auto* symbol=std::get_if<SymbolRef>(&key.data)){
                if(descriptor.accessor||!descriptor.writable||!descriptor.enumerable||!descriptor.configurable)
                    throw Exception("Unimplemented",u"PropertyArray Symbol properties do not retain descriptor attributes");
                const auto value=owner->fromValue(std::move(descriptor.value));
                if(storage->binding){storage->binding->setOwnSymbol(*symbol,value);return;}
                for(auto& [name,old]:storage->symbolEntries)if(name==*symbol){old=value;return;}
                storage->symbolEntries.emplace_back(*symbol,value);return;
            }
            const auto text=NativeRecord::keyText(key);
            if(descriptor.accessor||!descriptor.writable||!descriptor.enumerable||!descriptor.configurable)
                throw Exception("Unimplemented",u"PropertyArray named properties do not retain descriptor attributes");
            if(storage->binding){storage->binding->setOwn(text,owner->fromValue(std::move(descriptor.value)));return;}
            const auto value=owner->fromValue(std::move(descriptor.value));
            for(auto& [name,old]:storage->entries)if(name==text){old=value;return;}
            storage->entries.emplace_back(text,value);
        }
        void deleteOwn(const Key& key)override{
            if(const auto* symbol=std::get_if<SymbolRef>(&key.data)){
                if(storage->binding){storage->binding->eraseSymbol(*symbol);return;}
                std::erase_if(storage->symbolEntries,[&](const auto& item){return item.first==*symbol;});
                return;
            }
            const auto text=NativeRecord::keyText(key);
            if(storage->binding){storage->binding->erase(text);return;}
            std::erase_if(storage->entries,[&](const auto& item){return item.first==text;});
        }
        std::optional<ObjectRef> prototype()const override{
            if(storage->binding){
                const auto parent=storage->binding->prototype();
                if(parent.missing()){
                    if(storage->arrayPrototype)return owner->context.intrinsics().arrayPrototype;
                    return {};
                }
                if(std::holds_alternative<PropertyNull>(parent.value))return {};
                if(const auto* builtin=std::get_if<BuiltinProperty>(&parent.value);
                   builtin&&*builtin==BuiltinProperty::arrayPrototype)
                    return owner->context.intrinsics().arrayPrototype;
                const auto value=owner->toValue(parent);
                if(const auto* object=std::get_if<ObjectRef>(&value.data))return *object;
                throw Exception("TypeError",u"Array prototype is not an object or null");
            }
            if(storage->prototype)return const_cast<State*>(owner.get())->objectFor(storage->prototype);
            if(storage->arrayPrototype)return owner->context.intrinsics().arrayPrototype;
            if(storage->objectPrototype)return owner->context.intrinsics().objectPrototype;
            return {};
        }
        void setPrototype(std::optional<ObjectRef> prototype_ref)override{
            if(storage->binding){
                storage->binding->set("__proto__",prototype_ref
                    ?owner->fromValue(Value(*prototype_ref)):Property(PropertyNull{}));
                return;
            }
            if(!prototype_ref){storage->prototype.reset();storage->objectPrototype=false;return;}
            if(*prototype_ref==owner->context.intrinsics().objectPrototype){storage->prototype.reset();storage->objectPrototype=true;return;}
            const auto parent=owner->fromValue(Value(*prototype_ref));
            const auto* value=std::get_if<std::shared_ptr<PropertyStorage>>(&parent.value);
            if(!value||!*value)throw Exception("TypeError",u"Prototype is not a PropertyStorage record");
            storage->prototype=*value;storage->objectPrototype=false;
        }
    };
    struct Record final:PropertyRecordBinding {
        std::shared_ptr<State> owner;
        ObjectRef object;
        Record(std::shared_ptr<State> owner,ObjectRef object):owner(std::move(owner)),object(object){}
        static Key key(std::string_view text){return Key(property_host::text(text));}
        Property get(std::string_view name)override{return owner->from(owner->context.get(object,key(name)));}
        Property getOwn(std::string_view name)override{
            if(!owner->context.hasOwn(object,key(name)))return Missing{};
            return get(name);
        }
        bool has(std::string_view name)const override{return owner->context.has(object,key(name));}
        bool hasOwn(std::string_view name)const override{return owner->context.hasOwn(object,key(name));}
        bool ownPropertyEnumerable(std::string_view name)const override{
            return owner->context.ownPropertyEnumerable(Value(object),key(name));
        }
        void set(std::string_view name,Property value)override{owner->context.set(object,key(name),owner->to(value));}
        void setOwn(std::string_view name,Property value)override{
            owner->context.define(object,key(name),Descriptor::data(owner->to(value)));
        }
        void erase(std::string_view name)override{owner->context.erase(object,key(name));}
        Property getSymbol(SymbolRef symbol)override{
            return owner->from(owner->context.get(Value(object),Key(symbol)));
        }
        Property getOwnSymbol(SymbolRef symbol)override{
            if(!owner->context.hasOwn(object,Key(symbol)))return Missing{};
            return getSymbol(symbol);
        }
        bool hasSymbol(SymbolRef symbol)const override{return owner->context.has(Value(object),Key(symbol));}
        bool hasOwnSymbol(SymbolRef symbol)const override{return owner->context.hasOwn(object,Key(symbol));}
        bool ownSymbolEnumerable(SymbolRef symbol)const override{
            return owner->context.ownPropertyEnumerable(Value(object),Key(symbol));
        }
        void setSymbol(SymbolRef symbol,Property value)override{
            owner->context.set(Value(object),Key(symbol),owner->to(value));
        }
        void setOwnSymbol(SymbolRef symbol,Property value)override{
            owner->context.define(object,Key(symbol),Descriptor::data(owner->to(value)));
        }
        void eraseSymbol(SymbolRef symbol)override{owner->context.erase(object,Key(symbol));}
        std::vector<SymbolRef> symbolKeys()const override{
            std::vector<SymbolRef> result;
            for(const auto& own:owner->context.ownKeys(Value(object))){
                const auto* symbol=std::get_if<SymbolRef>(&own.data);
                if(!symbol)continue;
                if(owner->context.ownPropertyEnumerable(Value(object),own))result.push_back(*symbol);
            }
            return result;
        }
        std::vector<std::string> ownStringKeys()const override{
            std::vector<std::string> result;
            for(const auto& own:owner->context.ownKeys(Value(object)))
                if(const auto* text=std::get_if<std::u16string>(&own.data))
                    result.push_back(property_host::debugText(*text));
            return result;
        }
        std::vector<SymbolRef> ownSymbolKeys()const override{
            std::vector<SymbolRef> result;
            for(const auto& own:owner->context.ownKeys(Value(object)))
                if(const auto* symbol=std::get_if<SymbolRef>(&own.data))result.push_back(*symbol);
            return result;
        }
        std::vector<std::string> keys()const override{
            std::vector<std::string> result;
            for(const auto& text:owner->context.enumerableStringKeys(object))result.push_back(property_host::debugText(text));
            return result;
        }
        Property prototype()const override{
            auto parent=owner->context.prototype(object);
            return owner->from(parent?Value(*parent):Value(Null{}));
        }
        Property primitive(PrimitiveHint hint)override{
            return owner->from(owner->context.toPrimitive(Value(object),hint));
        }
        PropertyList copyEnumerableOwn()override{
            auto& context=owner->context;
            const auto target=context.object();
            const auto keys=context.ownKeys(Value(object));
            for(const auto& key:keys){
                if(!context.ownPropertyEnumerable(Value(object),key))continue;
                const auto value=context.get(Value(object),key);
                context.set(Value(target),key,value);
            }
            const auto copied=owner->from(Value(target));
            auto result=PropertyList::record(copied);
            if(!result)throw Exception("TypeError",u"Copied Context properties are not a record");
            return std::move(*result);
        }
    };
    Property from(Value value){
        if(std::holds_alternative<Undefined>(value.data))return Missing{};
        if(std::holds_alternative<Null>(value.data))return PropertyNull{};
        if(const auto* p=std::get_if<bool>(&value.data))return *p;
        if(const auto* p=std::get_if<double>(&value.data))return *p;
        if(const auto* p=std::get_if<std::u16string>(&value.data))return property_host::debugText(*p);
        if(const auto* symbol=std::get_if<SymbolRef>(&value.data)){
            (void)context.symbolString(*symbol); // Validate the genuine owning Context.
            return Property(*symbol);
        }
        if(const auto* object=std::get_if<ObjectRef>(&value.data)){
            const auto kind=context.kind(*object); // Validate Context before caching.
            if(kind==ObjectKind::function){
                if(const auto function=functions.find(*object))return *function;
                return ContextFunctionIdentity{*object};
            }
            for(auto it=projections.rbegin();it!=projections.rend();++it)
                if(auto projected=it->second(*object))return *projected;
            for(const auto& [known,weak]:records)if(known==*object)if(auto storage=weak.lock()){
                Property result;result.value=std::move(storage);return result;
            }
            const Property result=PropertyList::live(std::make_shared<Record>(shared_from_this(),*object),kind==ObjectKind::array);
            const auto storage=std::get<std::shared_ptr<PropertyStorage>>(result.value);
            records.emplace_back(*object,storage);return result;
        }
        throw Exception("Unimplemented",u"Context value requires its typed Property binding");
    }
    Value to(const Property& value)const{
        if(value.missing())return Undefined{};
        if(std::holds_alternative<PropertyNull>(value.value))return Null{};
        if(const auto* p=std::get_if<bool>(&value.value))return *p;
        if(const auto* p=std::get_if<double>(&value.value))return *p;
        if(const auto* p=std::get_if<std::string>(&value.value))return property_host::text(*p);
        if(const auto* symbol=std::get_if<SymbolRef>(&value.value)){
            (void)context.symbolString(*symbol);
            return *symbol;
        }
        if(const auto* p=std::get_if<FunctionRef>(&value.value)){
            if(auto object=functions.find(*p))return *object;
            throw Exception("Unimplemented",u"Typed function has no canonical Context callable binding");
        }
        if(const auto* function=std::get_if<ContextFunctionIdentity>(&value.value)){
            if(context.kind(function->object)!=ObjectKind::function)
                throw Exception("TypeError",u"Context callable identity no longer names a function");
            return function->object;
        }
        for(auto it=projections.rbegin();it!=projections.rend();++it)
            if(auto projected=it->first(value))return *projected;
        if(const auto* storage=std::get_if<std::shared_ptr<PropertyStorage>>(&value.value);storage&&*storage){
            const auto* bound=dynamic_cast<Record*>((*storage)->binding.get());
            if(bound&&bound->owner.get()==this)return bound->object;
            return const_cast<State*>(this)->objectFor(*storage);
        }
        throw Exception("Unimplemented",u"Typed intrinsic Property has no canonical Context binding");
    }
};
ObjectRef ContextPropertyBindings::State::objectFor(const std::shared_ptr<PropertyStorage>& storage){
    if(!storage)throw Exception("TypeError",u"PropertyStorage is null");
    for(const auto& [object,weak]:records)if(auto current=weak.lock();current&&current==storage)return object;
    ObjectRef object;
    if(storage->array){
        auto view=std::make_shared<NativeArray>(shared_from_this(),storage);
        object=context.nativeArray(view);
        if(storage->prototype||(!storage->objectPrototype&&!storage->arrayPrototype))
            context.setPrototype(object,view->prototype());
    }else{
        auto view=std::make_shared<NativeRecord>(shared_from_this(),storage);
        object=context.nativeRecord(view,view->prototype());
    }
    records.emplace_back(object,storage);
    return object;
}
Value ContextPropertyBindings::State::toValue(const Property& value){
    if(value.missing())return Undefined{};
    if(std::holds_alternative<PropertyNull>(value.value))return Null{};
    if(const auto* p=std::get_if<bool>(&value.value))return *p;
    if(const auto* p=std::get_if<double>(&value.value))return *p;
    if(const auto* p=std::get_if<std::string>(&value.value))return property_host::text(*p);
    if(const auto* p=std::get_if<options::SymbolRef>(&value.value))return *p;
    if(const auto* p=std::get_if<FunctionRef>(&value.value)){
        if(auto object=functions.find(*p))return *object;
        throw Exception("Unimplemented",u"Typed function has no canonical Context callable binding");
    }
    if(const auto* function=std::get_if<ContextFunctionIdentity>(&value.value)){
        if(context.kind(function->object)!=ObjectKind::function)
            throw Exception("TypeError",u"Context callable identity no longer names a function");
        return function->object;
    }
    for(auto it=projections.rbegin();it!=projections.rend();++it)
        if(auto projected=it->first(value))return *projected;
    if(const auto* p=std::get_if<std::shared_ptr<PropertyStorage>>(&value.value);p&&*p)return objectFor(*p);
    throw Exception("Unimplemented",u"Property value has no canonical Context representation");
}
ContextPropertyBindings::ContextPropertyBindings(Context& context,OptionFunctionBindings& functions){
    if(&functions.context()!=&context)throw Exception("TypeError",u"Property and callable bindings require the same Context");
    state_=std::make_shared<State>(context,functions);
}
ContextPropertyBindings::~ContextPropertyBindings()=default;
Context& ContextPropertyBindings::context()const{return state_->context;}
Property ContextPropertyBindings::from(Value value){return state_->from(std::move(value));}
Value ContextPropertyBindings::to(const Property& value)const{return state_->to(value);}
void ContextPropertyBindings::registerProjection(PropertyToObject to,ObjectToProperty from){
    if(!to||!from)throw options::Exception("TypeError",u"Property projection requires both directions");
    state_->projections.emplace_back(std::move(to),std::move(from));
}
Property ContextPropertyBindings::copyOwnEnumerableProperties(const Property& source){
    auto& context=state_->context;
    const auto target=context.object();
    if(source.missing()||std::holds_alternative<PropertyNull>(source.value))
        return state_->from(Value(target));
    if(const auto* storage=std::get_if<std::shared_ptr<PropertyStorage>>(&source.value);
       storage&&*storage&&!(*storage)->binding){
        // Source's native environment is an ordinary PropertyList. Object.assign
        // creates a new target here; entries are read live in key order, while
        // nested Property graphs keep their original storage identity.
        const PropertyList root=PropertyList::record(source).value();
        const auto keys=root.keys();
        for(const auto& key:keys){
            if(!root.hasOwn(key))continue;
            const auto value=root.getOwn(key);
            context.set(Value(target),Key(property_host::text(key)),state_->to(value));
        }
        for(const auto& key:root.symbolKeys())
            context.set(Value(target),Key(key),state_->to(root.getKey(Property(key))));
        return state_->from(Value(target));
    }
    const auto input=state_->to(source);
    if(std::holds_alternative<Null>(input.data)||std::holds_alternative<Undefined>(input.data))
        return state_->from(Value(target));
    // [[OwnPropertyKeys]] is captured before getters run. Descriptor checks
    // and Gets remain live as Object.assign performs them one key at a time.
    const auto keys=context.ownKeys(input);
    for(const auto& key:keys){
        if(!context.ownPropertyEnumerable(input,key))continue;
        const auto value=context.get(input,key);
        context.set(Value(target),key,value);
    }
    return state_->from(Value(target));
}
}
