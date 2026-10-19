// Core Tree views over the actual option::Context node/array owners.
export module folia.math.core.tree.node_context_bindings;
export import folia.math.core.tree.node;
export import folia.math.util.native_identities;
export import folia.math.util.context_property_bindings;
export import folia.math.util.native_data_fields;
import folia.math.util.property_host;
import std;

export namespace folia::math::tree {
template<class Product> struct SourceChildArrayOperations {
    using Value=options::Value;
    std::function<void()> ensureActive;
    std::function<Value()> readLength;
    std::function<bool(std::size_t)> hasIndex;
    std::function<Value(std::size_t)> readIndex;
    std::function<void(std::size_t,Value)> writeIndex;
    std::function<void(std::size_t)> deleteIndex;
    std::function<Value(std::u16string_view,std::span<const Value>)> callMethod;
    std::function<ChildValue<Product>(Value)> fromValue;
    std::function<Value(const ChildValue<Product>&)> toValue;
    std::function<FactoryChildren<Product>(Value)> wrapArray;
    std::function<void(const std::function<void(Value)>&)> forOf;
    std::function<void(const std::function<void(Value,std::size_t)>&)> forEach;
    std::function<std::size_t(const Value&)> toLength;
    std::function<Value(const Value&)> toPrimitive;
    std::function<std::string(const Value&)> stringValue;
    std::function<bool(const Value&)> truthy;
};

// A view, not an indexed mirror. Array slots, descriptors, methods, iterator,
// holes and length remain on the original Context value. Every operation
// checks the runtime lease before borrowing Context.
template<class Product> class SourceChildArrayView final:public ChildArray<Product> {
    using Ops=SourceChildArrayOperations<Product>;
    options::Value source_;
    Ops ops_;
    void active()const{
        if(!ops_.ensureActive)throw options::Exception("TypeError",u"Source child array has no execution lease");
        ops_.ensureActive();
    }
public:
    explicit SourceChildArrayView(options::Value source,Ops ops):source_(std::move(source)),ops_(std::move(ops)){
        if(!ops_.readLength||!ops_.hasIndex||!ops_.readIndex||!ops_.writeIndex||!ops_.deleteIndex||!ops_.callMethod||
           !ops_.fromValue||!ops_.toValue||!ops_.wrapArray||!ops_.forOf||!ops_.forEach||!ops_.toLength||
           !ops_.toPrimitive||!ops_.stringValue||!ops_.truthy)
            throw options::Exception("TypeError",u"Source child array view requires complete operations");
        active();
    }
    const void* identity()const noexcept override{return this;}
    std::optional<options::Value> sourceValue()const override{return source_;}
    std::optional<options::Value> rawLength()const override{active();return ops_.readLength();}
    std::string joinedString()const override{
        active();
        const std::array<options::Value,1> args{options::Value(u",")};
        const auto result=ops_.callMethod(u"join",args);
        const auto primitive=ops_.toPrimitive(result);
        return ops_.stringValue(primitive);
    }
    std::size_t size()const override{active();return ops_.toLength(ops_.readLength());}
    bool lengthTruthy()const override{active();return ops_.truthy(ops_.readLength());}
    bool empty()const override{return size()==0;}
    bool hasOwn(std::size_t index)const override{active();return ops_.hasIndex(index);}
    bool has(std::size_t index)const override{return hasOwn(index);}
    ChildValue<Product> get(std::size_t index)const override{
        active();return ops_.fromValue(ops_.readIndex(index));
    }
    void set(std::size_t index,ChildValue<Product> value)override{
        active();ops_.writeIndex(index,ops_.toValue(value));
    }
    void erase(std::size_t index)override{
        active();ops_.deleteIndex(index);
    }
    std::size_t push_back(ChildValue<Product> value)override{
        active();const std::array<options::Value,1> args{ops_.toValue(value)};
        const auto result=ops_.callMethod(u"push",args);
        return ops_.toLength(result);
    }
    typename ChildArray<Product>::Handle slice(std::int64_t start=0,
        std::optional<std::int64_t> end={})const override{
        active();std::vector<options::Value> args{options::Value(static_cast<double>(start))};
        if(end)args.emplace_back(static_cast<double>(*end));
        return ops_.wrapArray(ops_.callMethod(u"slice",args));
    }
    typename ChildArray<Product>::Handle sliceNumber(double start)const override{
        active();const std::array<options::Value,1> args{options::Value(start)};
        return ops_.wrapArray(ops_.callMethod(u"slice",args));
    }
    typename ChildArray<Product>::Handle splice(std::int64_t start,
        std::optional<std::size_t> count={},std::span<const ChildValue<Product>> added={})override{
        active();std::vector<options::Value> args;
        args.emplace_back(static_cast<double>(start));
        if(count||!added.empty())args.emplace_back(count?options::Value(static_cast<double>(*count)):options::Value(options::Undefined{}));
        for(const auto& value:added)args.push_back(ops_.toValue(value));
        const auto removed=ops_.callMethod(u"splice",args);
        return ops_.wrapArray(removed);
    }
    std::optional<std::size_t> indexOf(ChildValue<Product> value,std::int64_t fromIndex=0)const override{
        active();std::vector<options::Value> args{ops_.toValue(value)};
        if(fromIndex!=0)args.emplace_back(static_cast<double>(fromIndex));
        const auto result=ops_.callMethod(u"indexOf",args);
        const auto* index=std::get_if<double>(&result.data);
        if(!index||*index<0||!std::isfinite(*index))return {};
        return static_cast<std::size_t>(*index);
    }
    void forOf(const typename ChildArray<Product>::ForOf& callback)const override{
        active();if(!callback)throw options::Exception("TypeError",u"Tree iterator callback is not callable");
        ops_.forOf([&](options::Value value){active();callback(ops_.fromValue(std::move(value)));});
    }
    void forEach(const typename ChildArray<Product>::Each& callback)override{
        active();if(!callback)throw options::Exception("TypeError",u"Tree forEach callback is not callable");
        const auto source=source_;
        const auto operations=ops_;
        const auto retainedCallback=callback;
        operations.forEach([source,operations,retainedCallback](options::Value value,std::size_t index){
            operations.ensureActive();
            SourceChildArrayView view(source,operations);
            retainedCallback(operations.fromValue(std::move(value)),index,view);
        });
    }
};

// Source node access is the operation-time adapter used by TreeChildValue.
// It retains only the exact Value and callbacks; it owns no node fields.
template<class Product> struct SourceTreeNodeOperations {
    using Value=options::Value;
    std::function<void()> ensureActive;
    std::function<Value(std::u16string_view)> get;
    std::function<void(std::u16string_view,Value)> set;
    std::function<Value(std::u16string_view,std::span<const Value>)> call;
    std::function<Value(Value,std::span<const Value>)> invoke;
    std::function<Value(const Value&,std::u16string_view)> getValue;
    std::function<void(const Value&,std::u16string_view,Value)> setValue;
    std::function<void(const Value&,const Value&,Value)> setPropertyValue;
    std::function<Value(Value,const Value&,std::span<const Value>)> invokeWithThis;
    std::function<FactoryChildren<Product>(Value)> arrayView;
    std::function<void(const Value&,const std::function<void(Value)>&)> forOfValue;
    std::function<double(const Value&)> toNumber;
    std::function<Property(const Value&)> toPropertyKey;
    std::function<bool(const Value&)> truthyValue;
    std::function<bool(const Value&,const Value&)> sameValueValue;
    std::function<std::string(const Value&)> stringValue;
    std::function<Value(const Value&)> copyEnumerableValue;
    std::function<std::vector<std::u16string>(const Value&)> ownEnumerableStringKeys;
    std::function<Value(const std::function<Value(std::span<const Value>)>&)> makeCallback;
    std::function<ChildValue<Product>(Value)> childValue;
    std::function<Property(Value)> propertyValue;
    std::function<FactoryChildren<Product>()> children;
    std::function<Product*()> resolveNative;
    std::function<ChildValue<Product>(Value)> fromValue;
    std::function<Value(const ChildValue<Product>&)> toValue;
    std::function<bool(const Value&,const Value&)> sameValue;
    std::function<bool(const Value&)> truthy;
    std::function<Value(const Value&)> toPrimitive;
    std::function<std::string(const Value&)> toString;
    std::function<std::any(const Value&,const TreeVisitor<Product>&,const std::any&)> walkTree;
    std::function<std::any(const Value&,const TreeRawVisitor<Product>&,const std::any&,const std::any&)> walkTreeRaw;
};
template<class Product> class SourceTreeNodeAccess final:public TreeNodeAccess<Product> {
    options::Value value_;
    SourceTreeNodeOperations<Product> ops_;
    void active()const{if(!ops_.ensureActive)throw options::Exception("TypeError",u"Source Tree node has no execution lease");ops_.ensureActive();}
public:
    SourceTreeNodeAccess(options::Value value,SourceTreeNodeOperations<Product> ops)
        :value_(std::move(value)),ops_(std::move(ops)){
        if(!ops_.get||!ops_.set||!ops_.call||!ops_.invoke||!ops_.getValue||!ops_.setValue||!ops_.setPropertyValue||
           !ops_.invokeWithThis||!ops_.arrayView||!ops_.forOfValue||!ops_.toNumber||!ops_.toPropertyKey||!ops_.truthyValue||!ops_.sameValueValue||
           !ops_.stringValue||!ops_.copyEnumerableValue||!ops_.ownEnumerableStringKeys||!ops_.makeCallback||!ops_.childValue||!ops_.propertyValue||!ops_.children||!ops_.resolveNative||!ops_.fromValue||
           !ops_.toValue||!ops_.sameValue||!ops_.truthy||!ops_.toString||
           !ops_.toPrimitive||!ops_.stringValue||!ops_.walkTree||!ops_.walkTreeRaw)
            throw options::Exception("TypeError",u"Source Tree node access requires complete operations");
        active();
    }
    options::Value sourceValue()const override{return value_;}
    Product* nativeProduct()const noexcept override{
        try{active();return ops_.resolveNative();}catch(...){return nullptr;}
    }
    options::Value get(std::u16string_view key)const override{active();return ops_.get(key);}
    void set(std::u16string_view key,options::Value value)const override{active();ops_.set(key,std::move(value));}
    options::Value invoke(options::Value callable,std::span<const options::Value> args={})const override{
        active();return ops_.invoke(std::move(callable),args);
    }
    options::Value getValue(const options::Value& receiver,std::u16string_view key)const override{
        active();return ops_.getValue(receiver,key);
    }
    void setValue(const options::Value& receiver,std::u16string_view key,options::Value value)const override{
        active();ops_.setValue(receiver,key,std::move(value));
    }
    void setPropertyValue(const options::Value& receiver,const options::Value& key,
                          options::Value value)const override{
        active();ops_.setPropertyValue(receiver,key,std::move(value));
    }
    options::Value invokeWithThis(options::Value callable,const options::Value& receiver,
        std::span<const options::Value> args={})const override{
        active();return ops_.invokeWithThis(std::move(callable),receiver,args);
    }
    std::shared_ptr<ChildArray<Product>> arrayView(options::Value value)const override{
        active();return ops_.arrayView(std::move(value));
    }
    void forOfValue(const options::Value& value,const std::function<void(options::Value)>& callback)const override{
        active();ops_.forOfValue(value,[&](options::Value item){active();callback(std::move(item));});
    }
    double toNumber(const options::Value& value)const override{active();return ops_.toNumber(value);}
    Property toPropertyKey(const options::Value& value)const override{active();return ops_.toPropertyKey(value);}
    bool truthyValue(const options::Value& value)const override{active();return ops_.truthyValue(value);}
    bool sameValue(const options::Value& left,const options::Value& right)const override{active();return ops_.sameValueValue(left,right);}
    std::string stringValue(const options::Value& value)const override{active();return ops_.stringValue(value);}
    options::Value copyEnumerableValue(const options::Value& value)const override{active();return ops_.copyEnumerableValue(value);}
    std::vector<std::u16string> ownEnumerableStringKeys(const options::Value& value)const override{active();return ops_.ownEnumerableStringKeys(value);}
    options::Value makeCallback(const std::function<options::Value(std::span<const options::Value>)>& body)const override{active();return ops_.makeCallback(body);}
    ChildValue<Product> childValue(options::Value value)const override{active();return ops_.childValue(std::move(value));}
    Property propertyValue(options::Value value)const override{active();return ops_.propertyValue(std::move(value));}
    options::Value call(std::u16string_view key,std::span<const options::Value> args={})const override{active();return ops_.call(key,args);}
    std::shared_ptr<ChildArray<Product>> children()const override{active();return ops_.children();}
    bool sameValue(const TreeNodeAccess<Product>& other)const override{
        active();return ops_.sameValue(value_,other.sourceValue());
    }
    bool truthy()const override{active();return ops_.truthy(value_);}
    ChildValue<Product> parent()const override{active();return ops_.fromValue(ops_.get(u"parent"));}
    void parent(ChildValue<Product> value)const override{active();ops_.set(u"parent",ops_.toValue(value));}
    bool isKind(std::string_view kind)const override{
        active();const std::u16string name(kind.begin(),kind.end());const std::array<options::Value,1> args{options::Value(name)};
        const auto result=call(u"isKind",args);return ops_.truthy(result);
    }
    ChildValue<Product> copy()const override{active();return ops_.fromValue(call(u"copy"));}
    std::any walkTree(const TreeVisitor<Product>& visit,const std::any& data={})const override{active();return ops_.walkTree(value_,visit,data);}
    std::any walkTreeRaw(const TreeRawVisitor<Product>& visit,const std::any& data={},
                         const std::any& visitorIdentity={})const override{
        active();return ops_.walkTreeRaw(value_,visit,data,visitorIdentity);
    }
    std::string toString()const override{active();return ops_.toString(value_);}
};

// The shared Context bridge used by native Tree owners and MML field binding.
// Adapters retain only the exact source Values; all property, method, iterator,
// and indexed operations execute against those Values in the original Context.
template<class Product> class TreeNodeContextBindings {
    using Value=options::Value;
    using ChildArrayRef=FactoryChildren<Product>;
    struct State:std::enable_shared_from_this<State>{
        options::Context& context;
        NativeIdentityBindings<Product*>& identities;
        std::weak_ptr<std::atomic_bool> lease;
        std::vector<std::pair<Value,std::weak_ptr<ChildArray<Product>>>> arrays;
        std::function<FactoryChildren<Product>(Value)> makeArrayView;
        State(options::Context& cx,NativeIdentityBindings<Product*>& ids,
              ContextPropertyBindings& propertyBindings,
              std::weak_ptr<std::atomic_bool> executionLease)
            :context(cx),identities(ids),lease(std::move(executionLease)),properties(propertyBindings){
            if(&ids.context()!=&cx)throw options::Exception("TypeError",u"Tree bindings require one Context identity graph");
            if(&propertyBindings.context()!=&cx)throw options::Exception("TypeError",u"Tree properties require one Context identity graph");
        }
        void active()const{
            const auto live=lease.lock();
            if(!live||!live->load(std::memory_order_acquire))
                throw options::Exception("TypeError",u"Tree Context execution has expired");
        }
        Value anyValue(const std::any& value)const{
            if(!value.has_value())return options::Undefined{};
            if(const auto* raw=std::any_cast<Value>(&value))return *raw;
            throw options::Exception("TypeError",u"Tree Context data must be a Source Value");
        }
        ChildValue<Product> from(Value value){
            if(std::holds_alternative<options::Undefined>(value.data))return Missing{};
            if(std::holds_alternative<options::Null>(value.data))return PropertyNull{};
            if(const auto* object=std::get_if<options::ObjectRef>(&value.data)){
                // Resolve only handles this adapter owns. A same-Context
                // identity from another Parser remains a Source receiver here.
                if(identities.ownsHandle(*object))
                    return ChildValue<Product>(identities.resolve(*object));
            }
            return ChildValue<Product>(sourceNode(std::move(value)));
        }
        Value to(const ChildValue<Product>& value)const{
            if(const auto source=value.sourceAccess())return source->sourceValue();
            if(value.kind()==ChildKind::Undefined)return options::Undefined{};
            if(value.kind()==ChildKind::Null)return options::Null{};
            auto* node=value.nodeOrNull();
            if(const auto identity=identities.findPointee(node))return Value(*identity);
            throw options::Exception("TypeError",u"Tree child has no canonical Context identity");
        }
        TreeNodeAccessRef<Product> sourceNode(Value value){
            active();
            const auto weak=this->weak_from_this();
            SourceTreeNodeOperations<Product> ops;
            ops.ensureActive=[weak]{auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();};
            // Receiver-specific callbacks are installed below; keeping Value
            // in each closure avoids any reconstructed or normalized object.
            const Value receiver=value;
            ops.get=[weak,receiver](std::u16string_view key){auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();return self->context.get(receiver,options::Key(std::u16string(key)));};
            ops.set=[weak,receiver](std::u16string_view key,Value v){auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();self->context.set(receiver,options::Key(std::u16string(key)),std::move(v));};
            ops.call=[weak,receiver](std::u16string_view key,std::span<const Value> args){auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();const auto method=self->context.get(receiver,options::Key(std::u16string(key)));const auto* function=std::get_if<options::ObjectRef>(&method.data);if(!function)throw options::Exception("TypeError",u"Tree member is not callable");return self->context.call(*function,receiver,args);};
            ops.invoke=[weak,receiver](Value callable,std::span<const Value> args){auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();const auto* function=std::get_if<options::ObjectRef>(&callable.data);if(!function)throw options::Exception("TypeError",u"Tree member is not callable");return self->context.call(*function,receiver,args);};
            ops.getValue=[weak](const Value& target,std::u16string_view key){auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();return self->context.get(target,options::Key(std::u16string(key)));};
            ops.setValue=[weak](const Value& target,std::u16string_view key,Value value){auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();self->context.set(target,options::Key(std::u16string(key)),std::move(value));};
            ops.setPropertyValue=[weak](const Value& target,const Value& rawKey,Value value){
                auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();
                const Value primitive=self->context.toPrimitive(rawKey,options::PrimitiveHint::stringHint);
                options::Key key=[&]()->options::Key{
                    if(const auto* symbol=std::get_if<options::SymbolRef>(&primitive.data))return options::Key(*symbol);
                    return options::Key(property_host::string(self->properties.from(primitive)));
                }();
                self->context.set(target,key,std::move(value));
            };
            ops.invokeWithThis=[weak](Value callable,const Value& exactThis,std::span<const Value> args){auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();const auto* function=std::get_if<options::ObjectRef>(&callable.data);if(!function)throw options::Exception("TypeError",u"Tree value is not callable");return self->context.call(*function,exactThis,args);};
            ops.arrayView=[weak](Value raw){auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();if(!self->makeArrayView)throw options::Exception("TypeError",u"Tree child-array adapter is unavailable");return self->makeArrayView(std::move(raw));};
            ops.forOfValue=[weak](const Value& raw,const std::function<void(Value)>& body){auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();self->context.forEachIterable(raw,[&](Value value){self->active();body(std::move(value));});};
            ops.toNumber=[weak](const Value& raw){auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();return self->context.toNumber(raw);};
            ops.toPropertyKey=[weak](const Value& raw){
                auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();
                const Value primitive=self->context.toPrimitive(raw,options::PrimitiveHint::stringHint);
                if(const auto* symbol=std::get_if<options::SymbolRef>(&primitive.data))return Property(*symbol);
                return Property(property_host::debugText(property_host::string(self->properties.from(primitive))));
            };
            ops.truthyValue=[weak](const Value& raw){auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();return options::truthy(raw);};
            ops.sameValueValue=[weak](const Value& left,const Value& right){auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();return options::sameValue(left,right);};
            ops.stringValue=[weak](const Value& raw){auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();const auto primitive=self->context.toPrimitive(raw,options::PrimitiveHint::stringHint);return property_host::debugText(property_host::string(self->properties.from(primitive)));};
            ops.copyEnumerableValue=[weak](const Value& raw){auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();return self->properties.to(self->properties.copyOwnEnumerableProperties(self->properties.from(raw)));};
            ops.ownEnumerableStringKeys=[weak](const Value& raw){auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();return self->context.enumerableStringKeys(raw);};
            ops.makeCallback=[weak](const std::function<Value(std::span<const Value>)>& body){auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();if(!body)throw options::Exception("TypeError",u"Tree callback is not callable");return Value(self->context.function(u"",[weak,body](options::Context&,Value,std::span<const Value> args)->Value{auto live=weak.lock();if(!live)throw options::Exception("TypeError",u"Tree Context bindings were released");live->active();return body(args);}));};
            ops.childValue=[weak](Value value){auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();return self->from(std::move(value));};
            ops.propertyValue=[weak](Value value){auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();return self->properties.from(std::move(value));};
            ops.children=[weak,receiver]{auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();if(!self->makeArrayView)throw options::Exception("TypeError",u"Tree child-array adapter is unavailable");return self->makeArrayView(self->context.get(receiver,options::Key(u"childNodes")));};
            ops.resolveNative=[weak,receiver]()->Product*{auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();const auto* object=std::get_if<options::ObjectRef>(&receiver.data);if(!object||!self->identities.ownsHandle(*object))return nullptr;return self->identities.resolve(*object);};
            ops.fromValue=[weak](Value v){auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();return self->from(std::move(v));};
            ops.toValue=[weak](const ChildValue<Product>& v){auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();return self->to(v);};
            ops.sameValue=[weak](const Value& a,const Value& b){auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();return options::sameValue(a,b);};
            ops.truthy=[weak](const Value& v){auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();return options::truthy(v);};
            ops.toPrimitive=[weak](const Value& v){auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();return self->context.toPrimitive(v,options::PrimitiveHint::defaultHint);};
            ops.toString=[weak](const Value& v){auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();const auto primitive=self->context.toPrimitive(v,options::PrimitiveHint::stringHint);return property_host::debugText(property_host::string(self->properties.from(primitive)));};
            ops.walkTree=[weak](const Value& receiver,const TreeVisitor<Product>& visitor,const std::any& data)->std::any{
                auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();
                if(!visitor)throw options::Exception("TypeError",u"Tree walkTree visitor is not callable");
                const auto callback=self->context.function(u"",[weak,visitor,data](options::Context&,Value,std::span<const Value> args)->Value{
                    auto current=weak.lock();if(!current)throw options::Exception("TypeError",u"Tree Context bindings were released");current->active();
                    if(!args.empty())if(const auto* object=std::get_if<options::ObjectRef>(&args.front().data)){
                        try{visitor(current->identities.resolve(*object),data);}
                        catch(const NativeIdentityResolutionFailure&){
                            throw options::Exception("Unimplemented",u"Native Tree visitor cannot represent a foreign Context node value");
                        }
                    }else{
                        throw options::Exception("Unimplemented",u"Native Tree visitor cannot represent a non-object Context node value");
                    }
                    return current->anyValue(data);
                });
                const auto method=self->context.get(receiver,options::Key(u"walkTree"));
                const std::array<Value,2> args{Value(callback),self->anyValue(data)};
                const auto* function=std::get_if<options::ObjectRef>(&method.data);if(!function)throw options::Exception("TypeError",u"Tree walkTree is not callable");(void)self->context.call(*function,receiver,args);
                return data;
            };
            ops.walkTreeRaw=[weak](const Value& receiver,const TreeRawVisitor<Product>& visitor,
                                   const std::any& data,const std::any& visitorIdentity)->std::any{
                auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();
                if(!visitor)throw options::Exception("TypeError",u"Tree walkTree visitor is not callable");
                const auto supplied=std::any_cast<Value>(&visitorIdentity);
                const auto callback=supplied?*supplied:self->context.function(u"",[weak,visitor,data](options::Context&,Value,std::span<const Value> args)->Value{
                    auto current=weak.lock();if(!current)throw options::Exception("TypeError",u"Tree Context bindings were released");current->active();
                    const auto raw=args.empty()?Value(options::Undefined{}):args[0];
                    const auto childData=args.size()>1?args[1]:Value(options::Undefined{});
                    visitor(current->from(raw),std::any(childData));
                    return options::Undefined{};
                });
                if(!supplied)self->context.define(std::get<options::ObjectRef>(callback.data),options::Key(u"length"),options::Descriptor::data(Value(1),false,false,true));
                const auto sourceData=self->anyValue(data);
                const auto method=self->context.get(receiver,options::Key(u"walkTree"));
                const std::array<Value,2> args{callback,sourceData};
                const auto* function=std::get_if<options::ObjectRef>(&method.data);if(!function)throw options::Exception("TypeError",u"Tree walkTree is not callable");(void)self->context.call(*function,receiver,args);
                return data;
            };
            return std::make_shared<SourceTreeNodeAccess<Product>>(std::move(value),std::move(ops));
        }
        ContextPropertyBindings& properties;
    };
    std::shared_ptr<State> state_;
    explicit TreeNodeContextBindings(std::shared_ptr<State> state):state_(std::move(state)){}
public:
    TreeNodeContextBindings(options::Context& context,NativeIdentityBindings<Product*>& identities,
        ContextPropertyBindings& properties,std::weak_ptr<std::atomic_bool> lease)
        :state_(std::make_shared<State>(context,identities,properties,std::move(lease))){
        const std::weak_ptr<State> weak=state_;
        state_->makeArrayView=[weak](Value value)->ChildArrayRef{
            auto state=weak.lock();
            if(!state)throw options::Exception("TypeError",u"Tree Context bindings were released");
            state->active();
            return TreeNodeContextBindings<Product>(std::move(state)).arrayView(std::move(value));
        };
    }
    ChildValue<Product> childValue(Value value){state_->active();return state_->from(std::move(value));}
    Value sourceValue(const ChildValue<Product>& value)const{state_->active();return state_->to(value);}
    // Keep the operation-time receiver abstraction available to the Core Tree
    // constructor/prototype binder. It retains the exact Context Value and
    // resolves descriptors and methods against that receiver on every call.
    TreeNodeAccessRef<Product> nodeAccess(Value value){state_->active();return state_->sourceNode(std::move(value));}
    ChildArrayRef arrayView(Value source){
        auto& s=*state_;s.active();
        for(auto it=s.arrays.begin();it!=s.arrays.end();){
            if(auto existing=it->second.lock()){
                if(it->first==source)return existing;
                ++it;
            }else it=s.arrays.erase(it);
        }
        const auto weak=s.weak_from_this();
        SourceChildArrayOperations<Product> ops;
        ops.ensureActive=[weak]{auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();};
        ops.readLength=[weak,source]{auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();return self->context.get(source,options::Key(u"length"));};
        ops.hasIndex=[weak,source](std::size_t i){auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();return self->context.has(source,options::Key(property_host::text(std::to_string(i))));};
        ops.readIndex=[weak,source](std::size_t i){auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();return self->context.get(source,options::Key(property_host::text(std::to_string(i))));};
        ops.writeIndex=[weak,source](std::size_t i,Value v){auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();self->context.set(source,options::Key(property_host::text(std::to_string(i))),std::move(v));};
        ops.deleteIndex=[weak,source](std::size_t i){auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();const auto* object=std::get_if<options::ObjectRef>(&source.data);if(!object)throw options::Exception("TypeError",u"Tree child array receiver is not an object");self->context.erase(*object,options::Key(property_host::text(std::to_string(i))));};
        ops.callMethod=[weak,source](std::u16string_view name,std::span<const Value> args){auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();const auto method=self->context.get(source,options::Key(std::u16string(name)));const auto* function=std::get_if<options::ObjectRef>(&method.data);if(!function)throw options::Exception("TypeError",u"Tree array method is not callable");return self->context.call(*function,source,args);};
        ops.fromValue=[weak](Value v){auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();return ChildValue<Product>(self->sourceNode(std::move(v)));};
        ops.toValue=[weak](const ChildValue<Product>& v){auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();return self->to(v);};
        ops.wrapArray=[weak](Value v)->ChildArrayRef{auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();return TreeNodeContextBindings<Product>(self).arrayView(std::move(v));};
        ops.forOf=[weak,source](const std::function<void(Value)>& body){auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();self->context.forEachIterable(source,body);};
        ops.forEach=[weak,source](const std::function<void(Value,std::size_t)>& body){
            auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();
            const auto callback=self->context.function(u"treeChildForEach",[body](options::Context&,Value,std::span<const Value> args){
                const Value value=args.empty()?Value(options::Undefined{}):args[0];
                const auto* numeric=args.size()>1?std::get_if<double>(&args[1].data):nullptr;
                const std::size_t index=numeric&&*numeric>=0?static_cast<std::size_t>(*numeric):0;
                body(value,index);
                return Value(options::Undefined{});
            });
            const Value method=self->context.get(source,options::Key(u"forEach"));
            const std::array<Value,1> arguments{Value(callback)};
            const auto* function=std::get_if<options::ObjectRef>(&method.data);if(!function)throw options::Exception("TypeError",u"Tree array forEach is not callable");(void)self->context.call(*function,source,arguments);
        };
        ops.toLength=[weak](const Value& value){auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();const double number=self->context.toNumber(value);if(std::isnan(number)||number<=0)return std::size_t{0};const double limit=std::min(static_cast<double>(std::numeric_limits<std::size_t>::max()),9007199254740991.0);if(!std::isfinite(number)||number>=limit)return static_cast<std::size_t>(limit);return static_cast<std::size_t>(std::trunc(number));};
        ops.toPrimitive=[weak](const Value& v){auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();return self->context.toPrimitive(v,options::PrimitiveHint::defaultHint);};
        ops.stringValue=[weak](const Value& v){auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();return property_host::debugText(property_host::string(self->properties.from(v)));};
        ops.truthy=[weak](const Value& v){auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Tree Context bindings were released");self->active();return options::truthy(v);};
        auto view=std::make_shared<SourceChildArrayView<Product>>(source,std::move(ops));
        s.arrays.emplace_back(std::move(source),view);return view;
    }
};
}
