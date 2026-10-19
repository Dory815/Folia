// Context bindings for the generic Tree Factory and WrapperFactory owners.
// Runtime hooks resolve and initialize the actual product-specific base owner.
export module folia.math.core.tree.factory_context_bindings;
export import folia.math.core.tree.factory;
export import folia.math.core.tree.wrapper_factory;
export import folia.math.util.option_types;
export import folia.math.util.map_bindings;
export import folia.math.util.runtime_modules;
import folia.math.util.js_number;
import std;

export namespace folia::math::tree {
using options::Context;
using options::Key;
using options::ObjectRef;
using options::Value;

// One Source core/Tree/Factory module instance per RuntimeModules/Context.
// Product adapters reuse this constructor and method table; they do not
// instantiate template-specific copies of AbstractFactory.
class FactoryContextBaseModule : public std::enable_shared_from_this<FactoryContextBaseModule> {
    struct State : std::enable_shared_from_this<State> {
        Context* context;
        std::shared_ptr<options::MapContextBindings> maps;
        std::shared_ptr<std::atomic_bool> active=std::make_shared<std::atomic_bool>(true);
        std::optional<ObjectRef> constructor;
        std::optional<ObjectRef> prototype;

        State(Context& cx,std::shared_ptr<options::MapContextBindings> mapBindings)
            :context(&cx),maps(std::move(mapBindings)) {
            if(!maps||&maps->context()!=&cx)
                throw options::Exception("TypeError",u"AbstractFactory module requires one Context Map owner");
        }
        ~State(){active->store(false,std::memory_order_release);}
        void require(Context& candidate)const {
            if(!active->load(std::memory_order_acquire)||&candidate!=context)
                throw options::Exception("TypeError",u"AbstractFactory runtime has been released");
        }
        Value argument(std::span<const Value> args,std::size_t index)const {
            return index<args.size()?args[index]:Value(options::Undefined{});
        }
        Value invoke(Value callable,Value receiver,std::span<const Value> args={})const {
            const auto* object=std::get_if<ObjectRef>(&callable.data);
            if(!object||context->kind(*object)!=options::ObjectKind::function)
                throw options::Exception("TypeError",u"Factory method is not callable");
            return context->call(*object,std::move(receiver),args);
        }
        Key propertyKey(const Value& value)const {
            const Value primitive=context->toPrimitive(value,options::PrimitiveHint::stringHint);
            if(const auto* symbol=std::get_if<options::SymbolRef>(&primitive.data))return Key(*symbol);
            if(const auto* text=std::get_if<std::u16string>(&primitive.data))return Key(*text);
            if(std::holds_alternative<options::Undefined>(primitive.data))return Key(u"undefined");
            if(std::holds_alternative<options::Null>(primitive.data))return Key(u"null");
            if(const auto* boolean=std::get_if<bool>(&primitive.data))return Key(*boolean?u"true":u"false");
            if(const auto* number=std::get_if<double>(&primitive.data)){
                const auto ascii=js_number::toString(*number);
                return Key(std::u16string(ascii.begin(),ascii.end()));
            }
            throw options::Exception("TypeError",u"Cannot convert Factory property key to a primitive string");
        }
        void method(ObjectRef target,std::u16string_view name,double length,options::NativeCall call) {
            const ObjectRef function=context->function(u"",std::move(call));
            context->define(function,Key(u"name"),options::Descriptor::data(Value(u""),false,false,true));
            context->define(function,Key(u"length"),options::Descriptor::data(Value(length),false,false,true));
            context->set(Value(target),Key(std::u16string(name)),Value(function));
        }
        void initialize(Value receiver,std::span<const Value> args) {
            const auto* object=std::get_if<ObjectRef>(&receiver.data);
            if(!object)throw options::Exception("TypeError",u"Factory constructor called on a non-object");
            context->set(receiver,Key(u"defaultKind"),Value(u"unknown"));
            context->set(receiver,Key(u"nodeMap"),Value(maps->create()));
            context->set(receiver,Key(u"node"),Value(context->object()));
            Value nodes=argument(args,0);
            if(std::holds_alternative<options::Undefined>(nodes.data))nodes=options::Null{};
            if(std::holds_alternative<options::Null>(nodes.data)){
                const Value actualConstructor=context->get(receiver,Key(u"constructor"));
                nodes=context->get(actualConstructor,Key(u"defaultNodes"));
            }
            const auto keys=context->enumerableStringKeys(nodes);
            for(const auto& kind:keys){
                const Value setter=context->get(receiver,Key(u"setNodeClass"));
                const Value nodeClass=context->get(nodes,Key(kind));
                const std::array<Value,2> forwarded{Value(kind),nodeClass};
                (void)invoke(setter,receiver,forwarded);
            }
        }
        void install(){
            const auto weak=weak_from_this();
            auto ctor=context->nativeConstructor(u"AbstractFactory",
                [weak](Context& cx,std::span<const Value> args)->ObjectRef{
                    const auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"AbstractFactory module has expired");
                    self->require(cx);const auto receiver=cx.instance(*self->constructor);self->initialize(Value(receiver),args);return receiver;
                },
                [weak](Context& cx,Value receiver,std::span<const Value> args)->Value{
                    const auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"AbstractFactory module has expired");
                    self->require(cx);self->initialize(std::move(receiver),args);return Value(options::Undefined{});
                });
            constructor=ctor;
            context->define(ctor,Key(u"length"),options::Descriptor::data(Value(1),false,false,true));
            context->define(ctor,Key(u"name"),options::Descriptor::data(Value(u"AbstractFactory"),false,false,true));
            prototype=std::get<ObjectRef>(context->get(Value(ctor),Key(u"prototype")).data);
            context->define(ctor,Key(u"defaultNodes"),options::Descriptor::data(Value(context->object()),true,true,true));
            installMethods(*prototype);
        }
        void installMethods(ObjectRef target){
            const std::weak_ptr<State> weak=shared_from_this();
            method(target,u"create",1,[weak](Context& cx,Value receiver,std::span<const Value> args)->Value{
                const auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"AbstractFactory module has expired");self->require(cx);
                const Value node=cx.get(receiver,Key(u"node"));
                const Key kind=self->propertyKey(self->argument(args,0));
                Value creator=cx.get(node,kind);
                if(!options::truthy(creator)){
                    const Value fallbackNode=cx.get(receiver,Key(u"node"));
                    const Value defaultKind=cx.get(receiver,Key(u"defaultKind"));
                    creator=cx.get(fallbackNode,self->propertyKey(defaultKind));
                }
                const Value apply=cx.get(creator,Key(u"apply"));
                const auto count=args.empty()?0:args.size()-1;
                const auto forwarded=cx.array(static_cast<std::uint32_t>(count));
                for(std::size_t index=0;index<count;++index){
                    const auto digits=std::to_string(index);
                    cx.set(Value(forwarded),Key(std::u16string(digits.begin(),digits.end())),args[index+1]);
                }
                const std::array<Value,2> applyArguments{Value(options::Undefined{}),Value(forwarded)};
                return self->invoke(apply,creator,applyArguments);
            });
            method(target,u"setNodeClass",2,[weak](Context& cx,Value receiver,std::span<const Value> args)->Value{
                const auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"AbstractFactory module has expired");self->require(cx);
                const Value kind=self->argument(args,0), nodeClass=self->argument(args,1);
                const Value firstMap=cx.get(receiver,Key(u"nodeMap"));
                const Value set=cx.get(firstMap,Key(u"set"));const std::array<Value,2> setArgs{kind,nodeClass};
                (void)self->invoke(set,firstMap,setArgs);
                const Value secondMap=cx.get(receiver,Key(u"nodeMap"));
                const Value get=cx.get(secondMap,Key(u"get"));const std::array<Value,1> getArgs{kind};
                const Value selected=self->invoke(get,secondMap,getArgs);
                const Value node=cx.get(receiver,Key(u"node"));const Key property=self->propertyKey(kind);
                const auto creator=cx.function(u"",[weak,selected,factory=receiver](Context& inner,Value,std::span<const Value> rest)->Value{
                    const auto state=weak.lock();if(!state)throw options::Exception("TypeError",u"AbstractFactory module has expired");state->require(inner);
                    const Value bind=inner.get(selected,Key(u"bind"));const Value apply=inner.get(bind,Key(u"apply"));
                    const auto values=inner.array(static_cast<std::uint32_t>(rest.size()+2));
                    inner.set(Value(values),Key(u"0"),Value(options::Undefined{}));inner.set(Value(values),Key(u"1"),factory);
                    for(std::size_t i=0;i<rest.size();++i){const auto text=std::to_string(i+2);inner.set(Value(values),Key(std::u16string(text.begin(),text.end())),rest[i]);}
                    const std::array<Value,2> applyArgs{selected,Value(values)};
                    const Value bound=state->invoke(apply,bind,applyArgs);
                    const auto* ctor=std::get_if<ObjectRef>(&bound.data);
                    if(!ctor||!inner.isConstructor(*ctor))throw options::Exception("TypeError",u"Bound factory class is not a constructor");
                    return Value(inner.construct(*ctor));
                });
                cx.define(creator,Key(u"name"),options::Descriptor::data(Value(u""),false,false,true));
                cx.define(creator,Key(u"length"),options::Descriptor::data(Value(0),false,false,true));
                cx.set(node,property,Value(creator));return Value(options::Undefined{});
            });
            method(target,u"getNodeClass",1,[weak](Context& cx,Value receiver,std::span<const Value> args)->Value{
                const auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"AbstractFactory module has expired");self->require(cx);
                const Value map=cx.get(receiver,Key(u"nodeMap"));const Value get=cx.get(map,Key(u"get"));
                const std::array<Value,1> forwarded{self->argument(args,0)};return self->invoke(get,map,forwarded);
            });
            method(target,u"deleteNodeClass",1,[weak](Context& cx,Value receiver,std::span<const Value> args)->Value{
                const auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"AbstractFactory module has expired");self->require(cx);
                const Value map=cx.get(receiver,Key(u"nodeMap"));const Value erase=cx.get(map,Key(u"delete"));
                const Value kind=self->argument(args,0);const std::array<Value,1> forwarded{kind};
                (void)self->invoke(erase,map,forwarded);
                const Value node=cx.get(receiver,Key(u"node"));const auto* object=std::get_if<ObjectRef>(&node.data);
                if(!object)throw options::Exception("TypeError",u"Cannot delete a property from a non-object");
                cx.erase(*object,self->propertyKey(kind));return Value(options::Undefined{});
            });
            method(target,u"nodeIsKind",2,[weak](Context& cx,Value receiver,std::span<const Value> args)->Value{
                const auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"AbstractFactory module has expired");self->require(cx);
                const Value method=cx.get(receiver,Key(u"getNodeClass"));const std::array<Value,1> forwarded{self->argument(args,1)};
                const Value ctor=self->invoke(method,receiver,forwarded);
                return Value(ordinaryInstanceOf(cx,self->argument(args,0),ctor));
            });
            method(target,u"getKinds",0,[weak](Context& cx,Value receiver,std::span<const Value>)->Value{
                const auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"AbstractFactory module has expired");self->require(cx);
                const Value map=cx.get(receiver,Key(u"nodeMap"));const Value keys=cx.get(map,Key(u"keys"));
                const Value iterator=self->invoke(keys,map);const ObjectRef result=cx.array();
                cx.forEachIterable(iterator,[&](Value key){cx.arrayPush(result,std::span<const Value>(&key,1));});return Value(result);
            });
        }
    };
    std::shared_ptr<State> state_;
public:
    explicit FactoryContextBaseModule(util::RuntimeModules& modules)
        :state_(std::make_shared<State>(modules.context(),modules.getOrCreate<options::MapContextBindings>(
            u"ECMAScript/Map",[&]{return std::make_shared<options::MapContextBindings>(modules.context());}))) {
        state_->install();
    }
    Context& context()const{return *state_->context;}
    ObjectRef abstractFactoryConstructor()const{return *state_->constructor;}
    ObjectRef abstractFactoryPrototype()const{return *state_->prototype;}
};

// Bindings for the real generic Factory base owners. Owner lookup/allocation
// and field projection are capabilities supplied by the product runtime: a
// derived Common/MML receiver therefore resolves to its existing base
// subobject instead of receiving a second payload or registry entry.
template<class Product,class Node=void>
class FactoryContextBindings : public std::enable_shared_from_this<FactoryContextBindings<Product,Node>> {
public:
    using Base=AbstractFactory<Product>;
    using BaseOwner=std::shared_ptr<Base>;
    using ResolveBase=std::function<std::optional<BaseOwner>(ObjectRef)>;
    using BindBase=std::function<void(const BaseOwner&,ObjectRef)>;
    using AllocateBase=std::function<BaseOwner(ObjectRef)>;
    using InitializeFields=std::function<void(const BaseOwner&,ObjectRef)>;
    using InstanceOf=std::function<bool(Context&,const Value&,const Value&)>;

    struct Hooks {
        ResolveBase resolveBase;
        BindBase bindBase;
        AllocateBase allocateBase;
        InitializeFields initializeFields;
        InstanceOf instanceOf;
        // The wrapper allocator returns its actual AbstractFactory base
        // subobject; Common/MML owners are never substituted here.
        std::function<BaseOwner(ObjectRef)> allocateWrapper;
        std::weak_ptr<std::atomic_bool> executionLease;
    };

private:
    struct State : std::enable_shared_from_this<State> {
        Context* context;
        std::shared_ptr<FactoryContextBaseModule> sharedBase;
        Hooks hooks;
        std::optional<ObjectRef> factoryConstructor;
        std::optional<ObjectRef> factoryPrototype;
        std::optional<ObjectRef> wrapperConstructor;
        std::optional<ObjectRef> wrapperPrototype;

        State(Context& cx,std::shared_ptr<FactoryContextBaseModule> base,Hooks value)
            :context(&cx),sharedBase(std::move(base)),hooks(std::move(value)) {
            if(!sharedBase||&sharedBase->context()!=&cx)
                throw options::Exception("TypeError",u"Factory bindings require the Context's shared AbstractFactory module");
            if(!hooks.resolveBase||!hooks.bindBase||!hooks.initializeFields||!hooks.instanceOf)
                throw options::Exception("TypeError",u"Factory owner callbacks are incomplete");
            if constexpr(!std::is_void_v<Node>) {
                if(!hooks.allocateWrapper)
                    throw options::Exception("TypeError",u"AbstractWrapperFactory requires its actual owner allocator");
            }
            if(!hooks.allocateBase)
                hooks.allocateBase=[](ObjectRef){return std::make_shared<Base>();};
        }

        std::shared_ptr<std::atomic_bool> requireExecution() const {
            auto lease=hooks.executionLease.lock();
            if(!lease||!lease->load(std::memory_order_acquire))
                throw options::Exception("TypeError",u"Factory Context runtime has been released");
            return lease;
        }
        void requireContext(Context& candidate) const {
            (void)requireExecution();
            if(&candidate!=context)
                throw options::Exception("TypeError",u"Factory method called with a different Context");
        }
        BaseOwner owner(ObjectRef receiver) const {
            (void)requireExecution();
            auto found=hooks.resolveBase(receiver);
            if(!found||!*found)
                throw options::Exception("TypeError",u"Factory method receiver has no compatible native owner");
            return *found;
        }
        BaseOwner ensureOwner(ObjectRef receiver,bool wrapper) {
            (void)requireExecution();
            auto found=hooks.resolveBase(receiver);
            if(found&&*found)return *found;
            BaseOwner value;
            if(wrapper) {
                if constexpr(!std::is_void_v<Node>) {
                    if(!hooks.allocateWrapper)
                        throw options::Exception("Unimplemented",u"AbstractWrapperFactory owner allocator is not registered");
                    value=hooks.allocateWrapper(receiver);
                } else {
                    throw options::Exception("Unimplemented",u"AbstractWrapperFactory requires a wrapped node type");
                }
            } else value=hooks.allocateBase(receiver);
            if(!value)
                throw options::Exception("TypeError",u"Factory owner allocator returned no owner");
            hooks.bindBase(value,receiver);
            return value;
        }
        Value argument(std::span<const Value> args,std::size_t index) const {
            return index<args.size()?args[index]:Value(options::Undefined{});
        }
        void installFields(const BaseOwner& value,ObjectRef receiver) {
            hooks.initializeFields(value,receiver);
        }
        void initializeBase(ObjectRef receiver,std::span<const Value> args,bool wrapper) {
            if(!wrapper) throw options::Exception("TypeError",u"AbstractFactory is installed by the shared Source module");
            const auto value=ensureOwner(receiver,wrapper);
            installFields(value,receiver);
            (void)context->call(*factoryConstructor,Value(receiver),args);
        }
        ObjectRef constructBase(ObjectRef constructor,std::span<const Value> args,bool wrapper) {
            const ObjectRef receiver=context->instance(constructor);
            initializeBase(receiver,args,wrapper);
            return receiver;
        }
        Value callBase(Value receiver,std::span<const Value> args,bool wrapper) {
            const auto* object=std::get_if<ObjectRef>(&receiver.data);
            if(!object)
                throw options::Exception("TypeError",u"Factory constructor called on a non-object");
            initializeBase(*object,args,wrapper);
            return wrapper?Value(*object):Value(options::Undefined{});
        }
        ObjectRef makeBaseConstructor(bool wrapper) {
            const std::u16string name=wrapper?u"AbstractWrapperFactory":u"AbstractFactory";
            auto self=this->shared_from_this();
            auto constructor=context->nativeConstructor(name,
                [self,wrapper](Context& cx,std::span<const Value> args) {
                    self->requireContext(cx);
                    const auto ctor=wrapper?*self->wrapperConstructor:*self->factoryConstructor;
                    return self->constructBase(ctor,args,wrapper);
                },
                [self,wrapper](Context& cx,Value receiver,std::span<const Value> args) {
                    self->requireContext(cx);
                    return self->callBase(std::move(receiver),args,wrapper);
                });
            context->define(constructor,Key(u"length"),options::Descriptor::data(Value(wrapper?0.0:1.0),false,false,true));
            context->define(constructor,Key(u"name"),options::Descriptor::data(Value(name),false,false,true));
            return constructor;
        }
        void defineMethod(ObjectRef prototype,std::u16string_view name,double length,
                          options::NativeCall callback) {
            const auto method=context->function(u"",std::move(callback));
            context->define(method,Key(u"name"),options::Descriptor::data(Value(u""),false,false,true));
            context->define(method,Key(u"length"),options::Descriptor::data(Value(length),false,false,true));
            context->set(Value(prototype),Key(std::u16string(name)),Value(method));
        }
        void installWrapperMethod(ObjectRef prototype) {
            const std::weak_ptr<State> weak=this->shared_from_this();
            defineMethod(prototype,u"wrap",1,[weak](Context& cx,Value receiver,std::span<const Value> args)->Value {
                auto state=weak.lock();if(!state)throw options::Exception("Unimplemented",u"Factory bindings were released");
                state->requireContext(cx);
                const Value node=state->argument(args,0);
                const Value create=cx.get(receiver,Key(u"create"));
                const Value apply=cx.get(create,Key(u"apply"));
                const Value kind=cx.get(node,Key(u"kind"));
                const auto values=cx.array(static_cast<std::uint32_t>(args.size()+1));
                cx.set(Value(values),Key(u"0"),kind);
                cx.set(Value(values),Key(u"1"),node);
                for(std::size_t i=1;i<args.size();++i) {
                    const auto text=std::to_string(i+1);
                    cx.set(Value(values),Key(std::u16string(text.begin(),text.end())),args[i]);
                }
                const std::array<Value,2> forwarded{receiver,Value(values)};
                const auto* callable=std::get_if<ObjectRef>(&apply.data);
                if(!callable||cx.kind(*callable)!=options::ObjectKind::function)
                    throw options::Exception("TypeError",u"this.create.apply is not a function");
                return cx.call(*callable,create,forwarded);
            });
        }
        void install() {
            if(factoryConstructor)return;
            factoryConstructor=sharedBase->abstractFactoryConstructor();
            factoryPrototype=sharedBase->abstractFactoryPrototype();
            if constexpr(!std::is_void_v<Node>) {
                wrapperConstructor=makeBaseConstructor(true);
                wrapperPrototype=std::get<ObjectRef>(context->get(Value(*wrapperConstructor),Key(u"prototype")).data);
                context->setPrototype(*wrapperConstructor,*factoryConstructor);
                context->setPrototype(*wrapperPrototype,*factoryPrototype);
                installWrapperMethod(*wrapperPrototype);
            }
        }
    };
    std::shared_ptr<State> state_;

public:
    FactoryContextBindings(Context& context,std::shared_ptr<FactoryContextBaseModule> sharedBase,Hooks hooks)
        :state_(std::make_shared<State>(context,std::move(sharedBase),std::move(hooks))) { state_->install(); }

    ObjectRef abstractFactoryConstructor()const { state_->requireExecution();return *state_->factoryConstructor; }
    ObjectRef abstractFactoryPrototype()const { state_->requireExecution();return *state_->factoryPrototype; }
    ObjectRef abstractWrapperFactoryConstructor()const {
        state_->requireExecution();
        if(!state_->wrapperConstructor)throw options::Exception("Unimplemented",u"This Factory binding has no wrapper node type");
        return *state_->wrapperConstructor;
    }
    ObjectRef abstractWrapperFactoryPrototype()const {
        state_->requireExecution();
        if(!state_->wrapperPrototype)throw options::Exception("Unimplemented",u"This Factory binding has no wrapper node type");
        return *state_->wrapperPrototype;
    }
    void bindBase(const BaseOwner& owner,ObjectRef identity)const {
        state_->requireExecution();
        if(!owner)throw options::Exception("TypeError",u"Cannot bind a null Factory owner");
        state_->hooks.bindBase(owner,identity);
        state_->installFields(owner,identity);
    }
    std::optional<BaseOwner> resolveBase(ObjectRef identity)const {
        state_->requireExecution();return state_->hooks.resolveBase(identity);
    }
};
}
