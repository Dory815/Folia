// Context field adapters: nodeMap is a live Map over its actual stable
// slot owner; Wrappers is an ordinary record over the existing creator object.
export module folia.math.util.factory_context_field_views;
export import folia.math.core.tree.factory_class_map_owner;
export import folia.math.util.map_bindings;
export import folia.math.util.option_types;
import folia.math.util.property_host;
import std;

export namespace folia::math::tree {
template<class Product>
using FactoryClassMapProjection=options::ProjectedMapStorage<
    options::Value,FactoryClassMapValue<Product>,FactoryMapSameValueZero>;

// Reverse adapter for writable Source `nodeMap` assignment. It retains the
// exact Context Map owner and has no local entries; replacing the factory's
// access handle changes only which actual Map owner its typed methods use.
template<class Product>
class ContextMapOrderedAccess final:public FactoryClassMapAccess<Product> {
public:
    using Mapped=FactoryClassMapValue<Product>;
    using Entry=typename OrderedMapAccess<options::Value,Mapped>::Entry;
    using FromValue=std::function<Mapped(const options::Value&)>;
    using ToValue=std::function<options::Value(const Mapped&)>;
private:
    options::MapStorageRef storage_;
    std::optional<options::Value> sourceValue_;
    FromValue fromValue_;
    ToValue toValue_;
public:
    ContextMapOrderedAccess(options::MapStorageRef storage,FromValue fromValue,ToValue toValue)
        :storage_(std::move(storage)),fromValue_(std::move(fromValue)),toValue_(std::move(toValue)){
        if(!storage_||!fromValue_||!toValue_)
            throw options::Exception("TypeError",u"Context Map adapter requires owner and converters");
    }
    const void* identity()const noexcept override{return storage_->identity();}
    std::size_t size()const noexcept override{return storage_->size();}
    std::size_t slotCount()const noexcept override{return storage_->slotCount();}
    std::optional<options::Value> keyAt(std::size_t slot)const override{
        const auto entry=storage_->entryAt(slot);
        return entry?std::optional<options::Value>(entry->first):std::nullopt;
    }
    std::optional<Entry> entryAt(std::size_t slot)const override{
        auto entry=storage_->entryAt(slot);
        if(!entry)return std::nullopt;
        return Entry{entry->first,Mapped::source(entry->second)};
    }
    std::optional<Mapped> get(const options::Value& key)const override{
        if(!storage_->has(key))return std::nullopt;
        return Mapped::source(storage_->get(key));
    }
    bool has(const options::Value& key)const override{return storage_->has(key);}
    void set(options::Value key,Mapped value)override{
        if(const auto* raw=value.sourceValue())storage_->set(std::move(key),*raw);
        else storage_->set(std::move(key),toValue_(value));
    }
    FactoryClassRef<Product> resolveClass(const options::Value& key)const override{
        const auto value=get(key);
        if(!value)return {};
        return resolveValue(*value);
    }
    FactoryClassRef<Product> resolveValue(const Mapped& value)const override{
        if(const auto* native=value.nativeClass())return *native;
        const auto* raw=value.sourceValue();
        if(!raw)throw options::Exception("TypeError",u"nodeMap entry is not a registered factory class");
        const auto converted=fromValue_(*raw);
        if(const auto* native=converted.nativeClass())return *native;
        throw options::Exception("TypeError",u"nodeMap entry is not a registered factory class");
    }
    bool erase(const options::Value& key)override{return storage_->erase(key);}
    void clear()noexcept override{storage_->clear();}
    const options::MapStorageRef& mapStorage()const noexcept{return storage_;}
    std::optional<options::Value> sourceValue()const override{return sourceValue_;}
    void sourceValue(options::Value value)override{sourceValue_=std::move(value);}
};

template<class Product>
std::shared_ptr<FactoryClassMapAccess<Product>> typedFactoryClassMap(
    options::MapStorageRef storage,
    typename ContextMapOrderedAccess<Product>::FromValue classFromValue,
    typename ContextMapOrderedAccess<Product>::ToValue classToValue){
    return std::make_shared<ContextMapOrderedAccess<Product>>(
        std::move(storage),std::move(classFromValue),std::move(classToValue));
}

// `nodeMap` is keyed by actual Source Values; mapped Values remain raw in the
// one owner, so Source Map writes never resolve constructors prematurely.
template<class Product>
options::MapStorageRef projectFactoryClassMap(
    std::shared_ptr<FactoryClassMapAccess<Product>> owner,
    typename FactoryClassMapProjection<Product>::MappedToValue classToValue){
    return std::make_shared<FactoryClassMapProjection<Product>>(
        std::move(owner),
        [](const options::Value& key){return key;},
        [](const options::Value& key){return key;},
        [](const options::Value& raw){return FactoryClassMapValue<Product>::source(raw);},
        [classToValue=std::move(classToValue)](const FactoryClassMapValue<Product>& value){
            if(const auto* raw=value.sourceValue())return *raw;
            return classToValue(value);
        });
}

enum class FactoryCreatorSelectionKind { absent, creator, source_value, non_callable_inherited };
template<class Product> struct FactoryCreatorSelection {
    FactoryCreatorSelectionKind kind=FactoryCreatorSelectionKind::absent;
    FactoryCreatorRef<Product> native;
    options::Value sourceValue=options::Undefined{};
    // Preserve callability from the exact Source property read so the direct
    // NodeFactory path can defer observable argument conversion until then.
    bool callable=false;
    bool sourceTruthy()const{
        if(kind==FactoryCreatorSelectionKind::creator||kind==FactoryCreatorSelectionKind::non_callable_inherited)return true;
        if(kind==FactoryCreatorSelectionKind::absent)return false;
        if(std::holds_alternative<options::Undefined>(sourceValue.data)||
           std::holds_alternative<options::Null>(sourceValue.data))return false;
        if(const auto* boolean=std::get_if<bool>(&sourceValue.data))return *boolean;
        if(const auto* number=std::get_if<double>(&sourceValue.data))return *number!=0&&!std::isnan(*number);
        if(const auto* text=std::get_if<std::u16string>(&sourceValue.data))return !text->empty();
        return true;
    }
};
template<class Product> struct PreparedFactoryCreatorCall {
    FactoryCreatorSelection<Product> selection;
    options::Value applyValue=options::Undefined{};
};

// First-bind bridge attached to the existing default access object. The node
// record's NativeRecordView below refers to the same FactoryCreatorView; these
// callbacks route Source evaluation through Context while entries remain in
// that concrete native owner.
template<class Product> struct FactoryCreatorContextBridge {
    using Selection=FactoryCreatorSelection<Product>;
    using Prepared=PreparedFactoryCreatorCall<Product>;
    options::ObjectRef node;
    // Weak here avoids a cycle through factory -> access -> bridge. The
    // Context-visible creator callable and NativeRecordView retain the actual
    // concrete factory owner; bridge callbacks lock it during execution.
    std::weak_ptr<void> ownerLease;
    std::weak_ptr<std::atomic_bool> runtimeLease;
    std::function<Selection(options::ObjectRef,const options::Key&)> lookup;
    std::function<Prepared(const Selection&)> prepareApply;
    std::function<Product*(const Prepared&,FactoryArgs)> invokePrepared;
    std::function<Product*(const options::Value&,const Selection&,FactoryArgs)> invokeDirect;
    std::function<options::Value(const FactoryCreatorRef<Product>&)> expose;
    std::function<std::vector<options::Key>(options::ObjectRef)> ownKeys;
    std::function<std::optional<options::ObjectRef>(options::ObjectRef)> prototype;
    std::function<void(options::ObjectRef,std::optional<options::ObjectRef>)> setPrototype;
};

// One replaceable creator owner consumed by native create/wrap and Context
// property bindings. Selection is property lookup only; AbstractFactory owns
// fallback decisions. prepareApply runs before argument building; invocation
// consumes the saved apply value afterwards.
template<class Product>
class FactoryCreatorAccess {
public:
    using Selection=FactoryCreatorSelection<Product>;
    virtual ~FactoryCreatorAccess()=default;
    virtual Selection lookup(std::string_view kind)const=0;
    virtual Selection lookupKey(const options::Key& key)const {
        const auto* text=std::get_if<std::u16string>(&key.data);
        if(!text)return {FactoryCreatorSelectionKind::source_value,{},options::Undefined{}};
        return lookup(property_host::debugText(*text));
    }
    virtual void assign(std::string kind,FactoryCreatorRef<Product> creator)=0;
    virtual void erase(std::string_view kind)=0;
    virtual std::vector<options::Key> ownKeys()const=0;
    virtual std::optional<options::ObjectRef> prototype()const=0;
    virtual void setPrototype(std::optional<options::ObjectRef>)=0;
    virtual PreparedFactoryCreatorCall<Product> prepareApply(const Selection&)const=0;
    virtual Product* invokePrepared(const PreparedFactoryCreatorCall<Product>&,FactoryArgs)const=0;
    virtual bool directCallable(const Selection& selected)const { return selected.callable; }
    virtual Product* invokeDirect(const Selection&,FactoryArgs)const=0;
    virtual FactoryCreatorView<Product> nativeOwner()const=0;
    virtual std::optional<options::ObjectRef> sourceObject()const{return std::nullopt;}
    virtual std::optional<options::Value> sourceValue()const{return std::nullopt;}
};

template<class Product>
class FactoryCreatorObjectAccess final:public FactoryCreatorAccess<Product> {
    FactoryCreatorView<Product> owner_;
    std::optional<FactoryCreatorContextBridge<Product>> bridge_;
    std::shared_ptr<void> liveBridge()const{
        if(!bridge_)return {};
        auto lease=bridge_->runtimeLease.lock();
        if(!lease||!lease->load(std::memory_order_acquire))
            throw options::Exception("Unimplemented",u"Factory Context runtime lease has expired");
        auto owner=bridge_->ownerLease.lock();
        if(!owner)throw options::Exception("Unimplemented",u"Factory Context owner has expired");
        return owner;
    }
    static bool objectPrototypeMember(std::string_view key){
        return key=="constructor"||key=="__defineGetter__"||key=="__defineSetter__"||
            key=="hasOwnProperty"||key=="__lookupGetter__"||key=="__lookupSetter__"||
            key=="isPrototypeOf"||key=="propertyIsEnumerable"||key=="toString"||
            key=="valueOf"||key=="toLocaleString";
    }
    static bool functionPrototypeMember(std::string_view key){
        return key=="prototype"||key=="call"||key=="apply"||key=="bind"||
            key=="arguments"||key=="caller";
    }
    static std::optional<std::uint32_t> arrayIndex(std::string_view key){
        if(key.empty()||(key.size()>1&&key.front()=='0'))return {};
        std::uint64_t value=0;
        const auto [end,error]=std::from_chars(key.data(),key.data()+key.size(),value);
        if(error!=std::errc{}||end!=key.data()+key.size()||value>=std::numeric_limits<std::uint32_t>::max())return {};
        return static_cast<std::uint32_t>(value);
    }
    static bool isStringKey(const options::Key& key,std::string_view utf8){
        const auto* text=std::get_if<std::u16string>(&key.data);
        return text&&*text==property_host::text(utf8);
    }
    static std::optional<std::uint32_t> arrayIndex(const options::Key& key){
        const auto* text=std::get_if<std::u16string>(&key.data);
        if(!text||text->empty()||(text->size()>1&&text->front()==u'0'))return {};
        std::uint64_t value=0;
        for(const char16_t c:*text){
            if(c<u'0'||c>u'9')return {};
            value=value*10+static_cast<unsigned>(c-u'0');
            if(value>=std::numeric_limits<std::uint32_t>::max())return {};
        }
        return static_cast<std::uint32_t>(value);
    }
public:
    explicit FactoryCreatorObjectAccess(FactoryCreatorView<Product> owner):owner_(std::move(owner)){
        if(!owner_)throw options::Exception("TypeError",u"Factory creator owner is null");
    }
    typename FactoryCreatorAccess<Product>::Selection lookup(std::string_view kind)const override{
        return lookupKey(options::Key(property_host::text(kind)));
    }
    typename FactoryCreatorAccess<Product>::Selection lookupKey(const options::Key& key)const override{
        if(bridge_){auto lease=liveBridge();return bridge_->lookup(bridge_->node,key);}
        for(const auto& [ownKey,slot]:owner_->entries)if(ownKey==key){
            const auto value=slot.descriptor?slot.descriptor->value:
                (slot.creator?options::Value(options::ObjectRef{}):options::Value(options::Null{}));
            return {slot.creator?FactoryCreatorSelectionKind::creator:FactoryCreatorSelectionKind::source_value,
                    slot.creator,value,static_cast<bool>(slot.creator&&slot.creator->call)};
        }
        const auto* text=std::get_if<std::u16string>(&key.data);
        if(!text)return {};
        const auto kind=property_host::debugText(*text);
        if(kind=="__proto__"){
            if(owner_->prototypeAssigned)
                return {owner_->prototypeCreator?FactoryCreatorSelectionKind::creator:
                        FactoryCreatorSelectionKind::source_value,owner_->prototypeCreator,
                        owner_->prototypeCreator?options::Value(options::ObjectRef{}):options::Value(options::Null{}),
                        static_cast<bool>(owner_->prototypeCreator&&owner_->prototypeCreator->call)};
            return {FactoryCreatorSelectionKind::non_callable_inherited,{},options::Undefined{}};
        }
        if(owner_->prototypeAssigned&&functionPrototypeMember(kind))
            return {FactoryCreatorSelectionKind::non_callable_inherited,{},options::Undefined{}};
        if(objectPrototypeMember(kind))
            return {FactoryCreatorSelectionKind::non_callable_inherited,{},options::Undefined{}};
        return {};
    }
    void assign(std::string kind,FactoryCreatorRef<Product> creator)override{
        auto lease=liveBridge();
        const options::Key propertyKey(property_host::text(kind));
        auto found=std::find_if(owner_->entries.begin(),owner_->entries.end(),
            [&](const auto& entry){return entry.first==propertyKey;});
        if(kind=="__proto__"&&found==owner_->entries.end()){
            if(bridge_){
                const auto prototype=creator?std::optional<options::ObjectRef>(std::get<options::ObjectRef>(bridge_->expose(creator).data)):
                    std::optional<options::ObjectRef>{};
                bridge_->setPrototype(bridge_->node,prototype);
            }else{owner_->prototypeCreator=std::move(creator);owner_->prototypeAssigned=true;}
        }else if(found==owner_->entries.end())
            owner_->entries.emplace_back(propertyKey,FactoryCreatorSlot<Product>{creator,bridge_?std::optional<options::Descriptor>(options::Descriptor::data(bridge_->expose(creator))):std::nullopt});
        else found->second=FactoryCreatorSlot<Product>{creator,bridge_?std::optional<options::Descriptor>(options::Descriptor::data(bridge_->expose(creator))):std::nullopt};
    }
    void erase(std::string_view kind)override{
        auto lease=liveBridge();
        const auto propertyKey=options::Key(property_host::text(kind));
        const auto found=std::find_if(owner_->entries.begin(),owner_->entries.end(),
            [&](const auto& entry){return entry.first==propertyKey;});
        if(found!=owner_->entries.end())owner_->entries.erase(found);
    }
    std::vector<options::Key> ownKeys()const override{
        if(bridge_){auto lease=liveBridge();return bridge_->ownKeys(bridge_->node);}
        std::vector<const std::pair<options::Key,FactoryCreatorSlot<Product>>*> ordered;
        ordered.reserve(owner_->entries.size());
        for(const auto& entry:owner_->entries)ordered.push_back(&entry);
        std::stable_sort(ordered.begin(),ordered.end(),[](const auto* left,const auto* right){
            const auto a=arrayIndex(left->first),b=arrayIndex(right->first);
            if(a&&b)return *a<*b;
            if(a.has_value()!=b.has_value())return a.has_value();
            const bool leftSymbol=std::holds_alternative<options::SymbolRef>(left->first.data);
            const bool rightSymbol=std::holds_alternative<options::SymbolRef>(right->first.data);
            return !leftSymbol&&rightSymbol;
        });
        std::vector<options::Key> keys;keys.reserve(ordered.size());
        for(const auto* entry:ordered)keys.push_back(entry->first);
        return keys;
    }
    std::optional<options::ObjectRef> prototype()const override{
        if(bridge_){auto lease=liveBridge();return bridge_->prototype(bridge_->node);}return {};
    }
    void setPrototype(std::optional<options::ObjectRef> prototype)override{
        if(bridge_){auto lease=liveBridge();bridge_->setPrototype(bridge_->node,prototype);}
    }
    PreparedFactoryCreatorCall<Product> prepareApply(
        const typename FactoryCreatorAccess<Product>::Selection& selected)const override{
        if(bridge_){auto lease=liveBridge();return bridge_->prepareApply(selected);}
        if(selected.kind==FactoryCreatorSelectionKind::absent)
            throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'apply')");
        if(std::holds_alternative<options::Null>(selected.sourceValue.data))
            throw options::Exception("TypeError",u"Cannot read properties of null (reading 'apply')");
        return {selected,options::Undefined{}};
    }
    Product* invokePrepared(const PreparedFactoryCreatorCall<Product>& prepared,
                   FactoryArgs args)const override{
        if(bridge_){auto lease=liveBridge();return bridge_->invokePrepared(prepared,args);}
        const auto& selected=prepared.selection;
        if(!selected.native||!selected.native->call)
            throw options::Exception("TypeError",u"Factory creator is not callable");
        Product* const result=selected.native->call(args);
        if(!result)throw options::Exception("TypeError",u"Factory constructor returned no product");
        return result;
    }
    Product* invokeDirect(const typename FactoryCreatorAccess<Product>::Selection& selected,
                          FactoryArgs args)const override{
        if(bridge_){
            auto owner=liveBridge();(void)owner;
            if(!bridge_->invokeDirect)
                throw options::Exception("TypeError",u"this.node[kind] is not a function");
            return bridge_->invokeDirect(options::Value(bridge_->node),selected,std::move(args));
        }
        if(selected.kind!=FactoryCreatorSelectionKind::creator||!selected.native||!selected.native->call)
            throw options::Exception("TypeError",u"this.node[kind] is not a function");
        return selected.native->call(std::move(args));
    }
    bool directCallable(const typename FactoryCreatorAccess<Product>::Selection& selected)const override{
        if(bridge_){auto lease=liveBridge();(void)lease;
            // A Context bridge classifies callable Context functions as
            // creator selections even when they have no native creator ref.
            return selected.callable||selected.kind==FactoryCreatorSelectionKind::creator;
        }
        return selected.kind==FactoryCreatorSelectionKind::creator&&selected.native&&
               static_cast<bool>(selected.native->call);
    }
    FactoryCreatorView<Product> nativeOwner()const override{return owner_;}
    std::optional<options::ObjectRef> sourceObject()const override{
        if(bridge_)return bridge_->node;
        return std::nullopt;
    }
    std::optional<options::Value> sourceValue()const override{
        if(bridge_)return options::Value(bridge_->node);
        return std::nullopt;
    }
    void bindContextNode(FactoryCreatorContextBridge<Product> bridge){
        auto owner=bridge.ownerLease.lock();
        if(bridge.node==options::ObjectRef{}||!owner||!bridge.lookup||!bridge.prepareApply||!bridge.invokePrepared||!bridge.invokeDirect||
           !bridge.expose||!bridge.ownKeys||!bridge.prototype||!bridge.setPrototype)
            throw options::Exception("TypeError",u"Factory Context bridge is incomplete");
        auto lease=bridge.runtimeLease.lock();
        if(!lease||!lease->load(std::memory_order_acquire))
            throw options::Exception("Unimplemented",u"Factory Context runtime lease has expired");
        for(auto& entry:owner_->entries)if(!entry.second.descriptor)
            entry.second.descriptor=options::Descriptor::data(bridge.expose(entry.second.creator));
        if(owner_->prototypeAssigned){
            const auto value=owner_->prototypeCreator?bridge.expose(owner_->prototypeCreator):options::Value(options::Null{});
            const auto* object=std::get_if<options::ObjectRef>(&value.data);
            if(object)bridge.setPrototype(bridge.node,*object);
            else if(std::holds_alternative<options::Null>(value.data))bridge.setPrototype(bridge.node,{});
            owner_->prototypeCreator.reset();
            owner_->prototypeAssigned=false;
        }
        bridge_=std::move(bridge);
    }
};

// Context-backed creator owner for a writable Source `node` field. The weak
// lease is checked before every borrowed Context callback. `ReadApply` gets
// the selected function's current `apply` property before argument building.
template<class Product>
class ContextFactoryCreatorAccess final:public FactoryCreatorAccess<Product> {
public:
    using Selection=FactoryCreatorSelection<Product>;
    using Read=std::function<options::Value(const options::Value&,const options::Key&)>;
    using Write=std::function<void(const options::Value&,const options::Key&,options::Value)>;
    using Remove=std::function<void(const options::Value&,const options::Key&)>;
    using OwnKeys=std::function<std::vector<options::Key>(const options::Value&)>;
    using ReadPrototype=std::function<std::optional<options::ObjectRef>(const options::Value&)>;
    using WritePrototype=std::function<void(const options::Value&,std::optional<options::ObjectRef>)>;
    using IsCallable=std::function<bool(const options::Value&)>;
    using Resolve=std::function<FactoryCreatorRef<Product>(const options::Value&)>;
    using Expose=std::function<options::Value(const FactoryCreatorRef<Product>&)>;
    using ReadApply=std::function<options::Value(const options::Value&)>;
    using Apply=std::function<Product*(const options::Value&,const options::Value&,FactoryArgs)>;
    using Direct=std::function<Product*(const options::Value&,const options::Value&,FactoryArgs)>;
private:
    options::Value object_;
    // A factory owns its current access adapter, so retaining the concrete
    // factory owner here would make field-replacement self-cycles. Context
    // bindings retain their own live state; each operation locks this lease.
    std::weak_ptr<void> ownerLease_;
    std::weak_ptr<std::atomic_bool> runtimeLease_;
    Read read_;Write write_;Remove remove_;OwnKeys ownKeys_;
    ReadPrototype readPrototype_;WritePrototype writePrototype_;
    IsCallable isCallable_;Resolve resolve_;Expose expose_;ReadApply readApply_;Apply applySelected_;Direct callDirect_;
    struct AccessLeases {
        std::shared_ptr<void> owner;
        std::shared_ptr<std::atomic_bool> runtime;
    };
    AccessLeases lockRuntime()const{
        auto owner=ownerLease_.lock();
        if(!owner)
            throw options::Exception("TypeError",u"Factory Context owner is no longer available");
        auto lease=runtimeLease_.lock();
        if(!lease||!lease->load(std::memory_order_acquire))
            throw options::Exception("TypeError",u"Factory Context runtime is no longer available");
        return {std::move(owner),std::move(lease)};
    }
    static bool falsy(const options::Value& value){
        if(std::holds_alternative<options::Undefined>(value.data)||
           std::holds_alternative<options::Null>(value.data))return true;
        if(const auto* boolean=std::get_if<bool>(&value.data))return !*boolean;
        if(const auto* number=std::get_if<double>(&value.data))return *number==0||std::isnan(*number);
        if(const auto* text=std::get_if<std::u16string>(&value.data))return text->empty();
        return false;
    }
public:
    ContextFactoryCreatorAccess(options::Value object,std::weak_ptr<void> ownerLease,
        std::weak_ptr<std::atomic_bool> runtimeLease,
        Read read,Write write,Remove remove,OwnKeys ownKeys,ReadPrototype readPrototype,
        WritePrototype writePrototype,IsCallable isCallable,Resolve resolve,Expose expose,
        ReadApply readApply,Apply applySelected,Direct callDirect)
        :object_(object),ownerLease_(std::move(ownerLease)),runtimeLease_(std::move(runtimeLease)),read_(std::move(read)),
         write_(std::move(write)),remove_(std::move(remove)),ownKeys_(std::move(ownKeys)),
         readPrototype_(std::move(readPrototype)),writePrototype_(std::move(writePrototype)),
         isCallable_(std::move(isCallable)),resolve_(std::move(resolve)),expose_(std::move(expose)),readApply_(std::move(readApply)),
         applySelected_(std::move(applySelected)),callDirect_(std::move(callDirect)){
        if(ownerLease_.expired()||!read_||!write_||!remove_||!ownKeys_||!readPrototype_||!writePrototype_||
           !isCallable_||!resolve_||!expose_||!readApply_||!applySelected_||!callDirect_)
            throw options::Exception("TypeError",u"Context creator access requires all live callbacks");
    }
    Selection lookup(std::string_view kind)const override{
        return lookupKey(options::Key(property_host::text(kind)));
    }
    Selection lookupKey(const options::Key& key)const override{
        const auto lease=lockRuntime();(void)lease;
        const options::Value value=read_(object_,key);
        if(std::holds_alternative<options::Undefined>(value.data))return {};
        // Preserve the exact Source Value even when falsey or not a native
        // creator. AbstractFactory applies Source truthiness; prepareApply
        // performs the dynamic property read and invokePrepared applies it.
        const bool callable=isCallable_(value);
        auto native=callable?resolve_(value):FactoryCreatorRef<Product>{};
        return {FactoryCreatorSelectionKind::source_value,std::move(native),value,callable};
    }
    void assign(std::string kind,FactoryCreatorRef<Product> creator)override{
        const auto lease=lockRuntime();(void)lease;
        write_(object_,options::Key(property_host::text(kind)),expose_(creator));
    }
    void erase(std::string_view kind)override{
        const auto lease=lockRuntime();(void)lease;
        remove_(object_,options::Key(property_host::text(kind)));
    }
    std::vector<options::Key> ownKeys()const override{
        const auto lease=lockRuntime();(void)lease;return ownKeys_(object_);
    }
    std::optional<options::ObjectRef> prototype()const override{
        const auto lease=lockRuntime();(void)lease;return readPrototype_(object_);
    }
    void setPrototype(std::optional<options::ObjectRef> value)override{
        const auto lease=lockRuntime();(void)lease;writePrototype_(object_,value);
    }
    PreparedFactoryCreatorCall<Product> prepareApply(const Selection& selected)const override{
        const auto lease=lockRuntime();(void)lease;
        // Source resolves `selected.apply` before evaluating argument builders.
        return {selected,readApply_(selected.sourceValue)};
    }
    Product* invokePrepared(const PreparedFactoryCreatorCall<Product>& prepared,
                            FactoryArgs args)const override{
        const auto lease=lockRuntime();(void)lease;
        return applySelected_(prepared.selection.sourceValue,prepared.applyValue,args);
    }
    Product* invokeDirect(const Selection& selected,FactoryArgs args)const override{
        const auto lease=lockRuntime();(void)lease;
        if(!callDirect_||!selected.callable)
            throw options::Exception("TypeError",u"this.node[kind] is not a function");
        return callDirect_(object_,selected.sourceValue,std::move(args));
    }
    bool directCallable(const Selection& selected)const override{
        const auto lease=lockRuntime();(void)lease;
        return selected.callable;
    }
    FactoryCreatorView<Product> nativeOwner()const override{return {};}
    std::optional<options::ObjectRef> sourceObject()const override{
        if(const auto* object=std::get_if<options::ObjectRef>(&object_.data))return *object;
        return std::nullopt;
    }
    std::optional<options::Value> sourceValue()const override{return object_;}
};

// The creator view is an ordinary Source object, not a Map. Its callbacks
// operate directly on FactoryCreatorObject.entries and prototype fields.
// The owner supplies ownKeys in ECMAScript object-key order, including numeric
// index ordering, and applies descriptor/property semantics to its real fields.
class FactoryCreatorRecordView final:public options::NativeRecordView {
public:
    using ReadOwnKeys=std::function<std::vector<options::Key>()>;
    using ReadDescriptor=std::function<std::optional<options::Descriptor>(const options::Key&)>;
    using ReadEnumerable=std::function<bool(const options::Key&)>;
    using DefineProperty=std::function<void(const options::Key&,options::Descriptor)>;
    using DeleteProperty=std::function<void(const options::Key&)>;
    using ReadPrototype=std::function<std::optional<options::ObjectRef>()>;
    using WritePrototype=std::function<void(std::optional<options::ObjectRef>)>;
private:
    std::shared_ptr<void> ownerLease_;
    ReadOwnKeys readOwnKeys_;
    ReadDescriptor readDescriptor_;
    ReadEnumerable readEnumerable_;
    DefineProperty defineProperty_;
    DeleteProperty deleteProperty_;
    ReadPrototype readPrototype_;
    WritePrototype writePrototype_;
public:
    FactoryCreatorRecordView(std::shared_ptr<void> ownerLease,
        ReadOwnKeys readOwnKeys,ReadDescriptor readDescriptor,
        DefineProperty defineProperty,DeleteProperty deleteProperty,
        ReadPrototype readPrototype,WritePrototype writePrototype,ReadEnumerable readEnumerable)
        :ownerLease_(std::move(ownerLease)),readOwnKeys_(std::move(readOwnKeys)),
         readDescriptor_(std::move(readDescriptor)),
         readEnumerable_(std::move(readEnumerable)),
         defineProperty_(std::move(defineProperty)),deleteProperty_(std::move(deleteProperty)),
         readPrototype_(std::move(readPrototype)),writePrototype_(std::move(writePrototype)){
        if(!ownerLease_||!readOwnKeys_||!readDescriptor_||!defineProperty_||!deleteProperty_||
           !readPrototype_||!writePrototype_||!readEnumerable_)
            throw options::Exception("TypeError",u"Factory creator record requires its actual owner and callbacks");
    }
    std::optional<options::Descriptor> ownDescriptor(const options::Key& key)const override{
        return readDescriptor_(key);
    }
    bool ownPropertyEnumerable(const options::Key& key)const override{
        return readEnumerable_(key);
    }
    std::vector<options::Key> ownKeys()const override{return readOwnKeys_();}
    void defineOwn(const options::Key& key,options::Descriptor descriptor)override{
        defineProperty_(key,std::move(descriptor));
    }
    void deleteOwn(const options::Key& key)override{deleteProperty_(key);}
    std::optional<options::ObjectRef> prototype()const override{return readPrototype_();}
    void setPrototype(std::optional<options::ObjectRef> prototype)override{
        writePrototype_(prototype);
    }
};
}
