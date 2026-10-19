module folia.math.input.tex.stack_context_bindings;
import std;

namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view text) { return Key(std::u16string(text)); }
Value argument(std::span<const Value> args, std::size_t i) {
    return i < args.size() ? args[i] : Value(Undefined{});
}
void active(const std::weak_ptr<std::atomic_bool>& lease) {
    const auto state=lease.lock();
    if(!state||!state->load(std::memory_order_acquire))
        throw Exception("TypeError",u"Stack Context runtime has ended");
}
void functionLength(Context& cx,ObjectRef function,std::uint32_t length) {
    cx.define(function,key(u"length"),Descriptor::data(Value(static_cast<double>(length)),false,false,true));
}
void sourceField(Context& cx,ObjectRef object,const std::shared_ptr<Stack>& owner,
                 const std::weak_ptr<std::atomic_bool>& lease,
                 std::u16string_view name,Value initial,
                 std::function<Value()> read,std::function<void(Value)> write) {
    auto guardedRead=[lease,read=std::move(read)](){active(lease);return read();};
    auto guardedWrite=[lease,write=std::move(write)](Value value){active(lease);write(std::move(value));};
    cx.assignNativeData(object,key(name),std::make_shared<NativeDataField>(
        std::static_pointer_cast<void>(owner),std::move(guardedRead),std::move(guardedWrite)),std::move(initial));
}
}

std::span<const StackMemberSurface> sourceStackMemberSurface() {
    // Metadata captured from the pinned compiled Source module (not C++ syntax).
    static constexpr std::array members{
        StackMemberSurface{u"constructor",u"Stack",StackMemberKind::constructor,3,false,true,true},
        StackMemberSurface{u"env",u"get",StackMemberKind::getter,0,false,true,false},
        StackMemberSurface{u"env",u"set",StackMemberKind::setter,1,false,true,false},
        StackMemberSurface{u"Push",u"",StackMemberKind::method,0,true,true,true},
        StackMemberSurface{u"Pop",u"",StackMemberKind::method,0,true,true,true},
        StackMemberSurface{u"Top",u"",StackMemberKind::method,1,true,true,true},
        StackMemberSurface{u"Prev",u"",StackMemberKind::method,1,true,true,true},
        StackMemberSurface{u"height",u"get",StackMemberKind::getter,0,false,true,false},
        StackMemberSurface{u"toString",u"",StackMemberKind::method,0,true,true,true},
    };
    return members;
}
std::span<const std::u16string_view> sourceStackOwnFieldOrder() {
    static constexpr std::array<std::u16string_view,4> fields{u"_factory",u"_env",u"global",u"stack"};
    return fields;
}

struct StackContextBindings::State : std::enable_shared_from_this<State> {
    Context* context;
    NativeIdentityBindings<StackRef>* identities;
    StackContextHooks hooks;
    std::weak_ptr<std::atomic_bool> lease;
    ObjectRef constructor{},prototype{};

    State(Context& context,NativeIdentityBindings<StackRef>& identities,StackContextHooks hooks,
          std::weak_ptr<std::atomic_bool> lease)
        :context(&context),identities(&identities),hooks(std::move(hooks)),lease(std::move(lease)){}

    void validate() const {
        active(lease);
        if(!hooks.construct||!hooks.factory||!hooks.setFactory||!hooks.environment||
           !hooks.setEnvironment||!hooks.global||!hooks.setGlobal||!hooks.factoryToValue||
           !hooks.factoryFromValue||!hooks.propertyToValue||!hooks.propertyFromValue||
           !hooks.array||!hooks.replaceArray||!hooks.arrayFromValue||!hooks.entryToValue||
           !hooks.entryFromValue||!hooks.stringToValue)
            throw Exception("TypeError",u"Stack Context bindings are missing a Source owner hook");
    }
    StackRef receiver(Value value) {
        const auto* object=std::get_if<ObjectRef>(&value.data);
        if(!object)throw Exception("TypeError",u"Stack method receiver is not an object");
        return identities->resolve(*object);
    }
    void initializeClass() {
        auto weak=weak_from_this();
        constructor=context->nativeConstructor(u"Stack",[weak](Context&,std::span<const Value> args){
            const auto self=weak.lock();if(!self)throw Exception("TypeError",u"Stack Context runtime has ended");
            self->validate();
            const auto factory=self->hooks.factoryFromValue(argument(args,0));
            const auto env=self->hooks.propertyFromValue(argument(args,1));
            const auto global=self->hooks.propertyFromValue(argument(args,2));
            const auto owner=self->hooks.construct(factory,env,global);
            if(!owner)throw Exception("TypeError",u"Stack constructor returned no native owner");
            return self->bind(owner);
        },[](Context&,Value,std::span<const Value>)->Value{
            throw Exception("TypeError",u"Class constructor Stack cannot be invoked without 'new'");
        });
        functionLength(*context,constructor,3);
        prototype=context->object();
        context->define(constructor,key(u"prototype"),Descriptor::data(Value(prototype),false,false,false));
        context->define(prototype,key(u"constructor"),Descriptor::data(Value(constructor),true,false,true));

        auto method=[&](std::u16string name,std::uint32_t length,
                        std::function<Value(const StackRef&,std::span<const Value>)> call){
            const ObjectRef function=context->function(u"",[weak,call=std::move(call)](
                    Context&,Value thisValue,std::span<const Value> args)->Value{
                const auto self=weak.lock();if(!self)throw Exception("TypeError",u"Stack Context runtime has ended");
                self->validate();return call(self->receiver(std::move(thisValue)),args);
            });
            functionLength(*context,function,length);
            context->define(prototype,key(name),Descriptor::data(Value(function),true,true,true));
            return function;
        };
        const ObjectRef envGetter=context->function(u"get",[weak](Context&,Value thisValue,std::span<const Value>){
            const auto self=weak.lock();if(!self)throw Exception("TypeError",u"Stack Context runtime has ended");
            self->validate();return self->hooks.propertyToValue(self->hooks.environment(self->receiver(std::move(thisValue))));
        });
        functionLength(*context,envGetter,0);
        const ObjectRef envSetter=context->function(u"set",[weak](Context&,Value thisValue,std::span<const Value> args){
            const auto self=weak.lock();if(!self)throw Exception("TypeError",u"Stack Context runtime has ended");
            self->validate();self->hooks.setEnvironment(self->receiver(std::move(thisValue)),
                self->hooks.propertyFromValue(argument(args,0)));return Value(Undefined{});
        });
        functionLength(*context,envSetter,1);
        context->define(prototype,key(u"env"),Descriptor::access(envGetter,envSetter,false,true));
        method(u"Push",0,[weak](const StackRef& stack,std::span<const Value> args){
            const auto self=weak.lock();if(!self)throw Exception("TypeError",u"Stack Context runtime has ended");
            std::vector<StackEntry> entries;entries.reserve(args.size());
            for(const auto& value:args)entries.push_back(StackEntry(value));
            stack->Push(entries);return Value(Undefined{});
        });
        method(u"Pop",0,[weak](const StackRef& stack,std::span<const Value> args){
            const auto self=weak.lock();if(!self)throw Exception("TypeError",u"Stack Context runtime has ended");
            (void)args;return self->hooks.entryToValue(stack->Pop());
        });
        method(u"Top",1,[weak](const StackRef& stack,std::span<const Value> args){
            const auto self=weak.lock();if(!self)throw Exception("TypeError",u"Stack Context runtime has ended");
            return self->hooks.entryToValue(stack->Top(StackArrayScalar(argument(args,0))));
        });
        method(u"Prev",1,[weak](const StackRef& stack,std::span<const Value> args){
            const auto self=weak.lock();if(!self)throw Exception("TypeError",u"Stack Context runtime has ended");
            const Property noPop=options::truthy(argument(args,0));
            return self->hooks.entryToValue(StackEntry(stack->Prev(noPop)));
        });
        const ObjectRef height=context->function(u"get",[weak](Context&,Value thisValue,std::span<const Value>){
            const auto self=weak.lock();if(!self)throw Exception("TypeError",u"Stack Context runtime has ended");
            self->validate();return self->receiver(std::move(thisValue))->height();
        });
        functionLength(*context,height,0);
        context->define(prototype,key(u"height"),Descriptor::access(height,{},false,true));
        method(u"toString",0,[weak](const StackRef& stack,std::span<const Value>){
            const auto self=weak.lock();if(!self)throw Exception("TypeError",u"Stack Context runtime has ended");
            return self->hooks.stringToValue(stack->toString());
        });
    }
    ObjectRef bind(const StackRef& owner) {
        validate();
        const ObjectRef identity=identities->instance(owner,constructor);
        context->setPrototype(identity,prototype);
        const auto getFactory=[self=shared_from_this(),owner]{return self->hooks.factoryToValue(self->hooks.factory(owner));};
        const auto setFactory=[self=shared_from_this(),owner](Value v){self->hooks.setFactory(owner,self->hooks.factoryFromValue(v));};
        sourceField(*context,identity,owner,lease,u"_factory",getFactory(),getFactory,setFactory);
        const auto getEnv=[self=shared_from_this(),owner]{return self->hooks.propertyToValue(self->hooks.environment(owner));};
        const auto setEnv=[self=shared_from_this(),owner](Value v){self->hooks.setEnvironment(owner,self->hooks.propertyFromValue(v));};
        sourceField(*context,identity,owner,lease,u"_env",getEnv(),getEnv,setEnv);
        const auto getGlobal=[self=shared_from_this(),owner]{return self->hooks.propertyToValue(self->hooks.global(owner));};
        const auto setGlobal=[self=shared_from_this(),owner](Value v){self->hooks.setGlobal(owner,self->hooks.propertyFromValue(v));};
        sourceField(*context,identity,owner,lease,u"global",getGlobal(),getGlobal,setGlobal);

        StackArrayAccessRef access=hooks.array(owner);
        if(!access)throw Exception("TypeError",u"Stack has no operational array owner");
        auto arrayIdentity=access->identity();
        if(std::dynamic_pointer_cast<NativeStackArrayAccess>(access)){
            // The first Context Array is the actual Source field value. Its
            // indexed view delegates to the current native storage; install
            // the adapter for this exact identity before publishing the field.
            // No getter can observe an unbound placeholder or a second array.
            const auto native=std::dynamic_pointer_cast<NativeStackArrayAccess>(access);
            auto toValue=[self=shared_from_this()](const StackArrayStorageValue& value){
                if(const auto* raw=std::get_if<Value>(&value))return *raw;
                return self->hooks.entryToValue(std::get<StackEntry>(value));
            };
            auto view=std::make_shared<StackArrayIndexedView>(native->storage(),std::move(toValue));
            arrayIdentity=context->nativeArray(view);
            access=std::make_shared<ContextArrayStackAccess>(*context,*arrayIdentity,
                [self=shared_from_this()](const StackArrayStorageValue& value){
                    if(const auto* raw=std::get_if<Value>(&value))return *raw;
                    return self->hooks.entryToValue(std::get<StackEntry>(value));
                },hooks.entryFromValue,hooks.propertyToValue,lease);
            hooks.replaceArray(owner,access);
        }
        const auto readArray=[self=shared_from_this(),owner]{
            auto actual=self->hooks.array(owner);
            if(!actual)throw Exception("TypeError",u"Stack has no operational array owner");
            return actual->sourceValue();
        };
        const auto writeArray=[self=shared_from_this(),owner](Value value){
            active(self->lease);
            auto actual=self->hooks.arrayFromValue(value);
            if(!actual)throw Exception("TypeError",u"Stack array owner hook returned no assigned value owner");
            self->hooks.replaceArray(owner,std::move(actual));
        };
        sourceField(*context,identity,owner,lease,u"stack",access->sourceValue(),readArray,writeArray);
        return identity;
    }
};

StackContextBindings::StackContextBindings(Context& context,NativeIdentityBindings<StackRef>& identities,
        StackContextHooks hooks,std::weak_ptr<std::atomic_bool> executionLease)
    :state_(std::make_shared<State>(context,identities,std::move(hooks),std::move(executionLease))) {
    state_->validate();state_->initializeClass();
}
StackContextBindings::~StackContextBindings()=default;
ObjectRef StackContextBindings::constructor()const{return state_->constructor;}
ObjectRef StackContextBindings::prototype()const{return state_->prototype;}
ObjectRef StackContextBindings::bind(const StackRef& owner){return state_->bind(owner);}
StackRef StackContextBindings::resolve(ObjectRef identity)const{return state_->identities->resolve(identity);}
Context& StackContextBindings::context()const{return *state_->context;}
}
