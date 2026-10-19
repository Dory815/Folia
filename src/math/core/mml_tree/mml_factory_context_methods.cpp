// Context method projection for the actual core/MmlTree/MmlFactory owner.
// MathJax 4.1.3 source is Apache-2.0; see LICENSE-MathJax.txt.
module folia.math.core.mml_tree.mml_factory_context_methods;
import folia.math.core.mml_tree.mml_factory_context_values;
import folia.math.util.property_host;
import std;

namespace folia::math::mml {
using namespace options;

struct MmlFactoryContextMethods::State:std::enable_shared_from_this<State> {
    Context& context;
    ContextPropertyBindings& properties;
    NativeIdentityBindings<MmlNode*>& nodes;
    std::shared_ptr<tree::FactoryContextBaseModule> factoryBase;
    std::weak_ptr<std::atomic_bool> executionLifetime;
    ResolveFactory resolveFactory;
    FindNodeIdentity findNodeIdentity;
    ObjectRef abstractNodeFactoryPrototype;
    ObjectRef abstractNodeFactoryConstructor;
    ObjectRef mmlFactoryPrototype;
    ObjectRef createFunction;

    State(Context& context,ContextPropertyBindings& properties,
          NativeIdentityBindings<MmlNode*>& nodes,
          std::shared_ptr<tree::FactoryContextBaseModule> factoryBase,
          std::weak_ptr<std::atomic_bool> executionLifetime,
          ResolveFactory resolveFactory,FindNodeIdentity findNodeIdentity)
        :context(context),properties(properties),nodes(nodes),factoryBase(std::move(factoryBase)),
         executionLifetime(std::move(executionLifetime)),
         resolveFactory(std::move(resolveFactory)),
         findNodeIdentity(std::move(findNodeIdentity)) {
        if(&properties.context()!=&context||&nodes.context()!=&context||
           !this->factoryBase||&this->factoryBase->context()!=&context)
            throw Exception("TypeError",u"MmlFactory bindings belong to different Contexts");
        if(!this->resolveFactory||!this->findNodeIdentity)
            throw Exception("TypeError",u"MmlFactory methods require canonical owner and node identity resolvers");
        requireActive();
    }

    Value callBaseConstructor(Context& cx,Value receiver,std::span<const Value> args) const {
        const ObjectRef baseConstructor=factoryBase->abstractFactoryConstructor();
        const Value apply=cx.get(Value(baseConstructor),Key(u"apply"));
        const auto argumentList=cx.array(static_cast<std::uint32_t>(args.size()));
        for(std::size_t index=0;index<args.size();++index){
            const auto text=std::to_string(index);
            cx.set(Value(argumentList),Key(std::u16string(text.begin(),text.end())),args[index]);
        }
        const std::array<Value,2> forwarded{std::move(receiver),Value(argumentList)};
        const auto* callable=std::get_if<ObjectRef>(&apply.data);
        if(!callable||cx.kind(*callable)!=ObjectKind::function)
            throw Exception("TypeError",u"AbstractFactory.apply is not a function");
        return cx.call(*callable,Value(baseConstructor),forwarded);
    }

    void install(){
        const auto weak=weak_from_this();
        abstractNodeFactoryConstructor=context.nativeConstructor(u"AbstractNodeFactory",
            [weak](Context& cx,std::span<const Value> args)->ObjectRef{
                const auto self=weak.lock();if(!self)throw Exception("TypeError",u"MmlFactory methods have been released");
                self->requireContext(cx);
                const auto receiver=cx.instance(self->abstractNodeFactoryConstructor);
                const Value result=self->callBaseConstructor(cx,Value(receiver),args);
                if(const auto* returned=std::get_if<ObjectRef>(&result.data))return *returned;
                return receiver;
            },
            [weak](Context& cx,Value receiver,std::span<const Value> args)->Value{
                const auto self=weak.lock();if(!self)throw Exception("TypeError",u"MmlFactory methods have been released");
                self->requireContext(cx);
                const Value result=self->callBaseConstructor(cx,receiver,args);
                return options::truthy(result)?result:receiver;
            });
        context.define(abstractNodeFactoryConstructor,Key(u"name"),Descriptor::data(Value(u"AbstractNodeFactory"),false,false,true));
        context.define(abstractNodeFactoryConstructor,Key(u"length"),Descriptor::data(Value(0),false,false,true));
        context.setPrototype(abstractNodeFactoryConstructor,factoryBase->abstractFactoryConstructor());
        abstractNodeFactoryPrototype=std::get<ObjectRef>(context.get(Value(abstractNodeFactoryConstructor),Key(u"prototype")).data);
        context.setPrototype(abstractNodeFactoryPrototype,factoryBase->abstractFactoryPrototype());
        mmlFactoryPrototype=context.object();
        context.setPrototype(mmlFactoryPrototype,abstractNodeFactoryPrototype);
        initialize();
    }

    void initialize(){
        requireActive();
        auto self=shared_from_this();
        // CJS emits AbstractNodeFactory.prototype.create = function(kind,
        // properties, children), so its inferred name is empty and length 3.
        createFunction=context.function(u"",[self](Context& cx,Value receiver,
                                                   std::span<const Value> args)->Value {
            self->requireContext(cx);
            const Value kindValue=args.empty()?Value(Undefined{}):args[0];
            // AbstractNodeFactory.ts evaluates this.node, ToPropertyKey(kind),
            // this.node[key], then properties/children, and invokes the
            // selected function with this.node as its receiver. Preserve the
            // raw returned Value; it need not be a native MmlNode.
            const Value node=cx.get(receiver,Key(u"node"));
            const Value primitive=cx.toPrimitive(kindValue,PrimitiveHint::stringHint);
            const Key key=std::holds_alternative<SymbolRef>(primitive.data)
                ?Key(std::get<SymbolRef>(primitive.data))
                :Key(property_host::string(self->properties.from(primitive)));
            const Value selected=cx.get(node,key);
            const auto* function=std::get_if<ObjectRef>(&selected.data);
            if(!function||cx.kind(*function)!=ObjectKind::function)
                throw Exception("TypeError",u"this.node[kind] is not a function");
            const Value properties=args.size()>1?args[1]:Value(Undefined{});
            const Value children=args.size()>2?args[2]:Value(Undefined{});
            std::array<Value,2> values{
                std::holds_alternative<Undefined>(properties.data)?Value(cx.object()):properties,
                std::holds_alternative<Undefined>(children.data)?Value(cx.array()):children};
            return cx.call(*function,node,values);
        });
        context.define(createFunction,Key(u"length"),Descriptor::data(Value(3),false,false,true));
        context.define(abstractNodeFactoryPrototype,Key(u"create"),Descriptor::data(Value(createFunction)));
    }

    void requireActive()const{
        const auto lease=executionLifetime.lock();
        if(!lease||!lease->load(std::memory_order_acquire))
            throw Exception("TypeError",u"Parser runtime has been released");
    }
    void requireContext(Context& candidate)const{
        requireActive();
        if(&candidate!=&context)
            throw Exception("TypeError",u"MmlFactory method called with a different Context");
    }
};

MmlFactoryContextMethods::MmlFactoryContextMethods(
    Context& context,ContextPropertyBindings& properties,
    NativeIdentityBindings<MmlNode*>& nodes,
    std::shared_ptr<tree::FactoryContextBaseModule> factoryBase,
    std::weak_ptr<std::atomic_bool> executionLifetime,
    ResolveFactory resolveFactory,FindNodeIdentity findNodeIdentity)
    :state_(std::make_shared<State>(context,properties,nodes,std::move(factoryBase),
        std::move(executionLifetime),std::move(resolveFactory),std::move(findNodeIdentity))) { state_->install(); }

void MmlFactoryContextMethods::bind(ObjectRef identity,const std::shared_ptr<MmlFactory>& owner){
    state_->requireActive();
    if(!owner)throw Exception("TypeError",u"MmlFactory owner is null");
    (void)state_->context.kind(identity);
    const auto resolved=state_->resolveFactory(identity);
    if(!resolved||resolved.get()!=owner.get())
        throw Exception("TypeError",u"MmlFactory Context identity does not retain the supplied owner");
    state_->context.setPrototype(identity,state_->mmlFactoryPrototype);
}

ObjectRef MmlFactoryContextMethods::prototype()const{
    state_->requireActive();
    return state_->mmlFactoryPrototype;
}
ObjectRef MmlFactoryContextMethods::constructor()const{
    state_->requireActive();
    return state_->abstractNodeFactoryConstructor;
}
ObjectRef MmlFactoryContextMethods::basePrototype()const{
    state_->requireActive();
    return state_->factoryBase->abstractFactoryPrototype();
}
}
