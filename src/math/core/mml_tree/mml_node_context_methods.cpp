module folia.math.core.mml_tree.mml_node_context_methods;
import folia.math.util.native_data_fields;
import folia.math.util.property_host;
import std;

namespace folia::math::mml {
using namespace options;

struct MmlNodeContextMethods::State:std::enable_shared_from_this<State>{
    using Dynamic=Value;
    using Receiver=MmlNodeReceiver<MmlNode,Dynamic>;
    using Algorithms=MmlNodeAlgorithms<MmlNode,Dynamic>;
    Context& context;
    NativeIdentityBindings<MmlNode*>& identities;
    ContextPropertyBindings& properties;
    AttributesContextBindings& attributes;
    tree::TreeNodeContextBindings<MmlNode>& treeNodes;
    tree::TreeNodeContextClasses<MmlNode>& treeClasses;
    std::function<Value(tree::FactoryChildren<MmlNode>)> childrenToValue;
    std::function<bool(Value)> isAbstractMmlNode;
    std::function<bool(Value)> isTextNode;
    std::weak_ptr<std::atomic_bool> executionLifetime;
    bool installSourceMethods=true;
    ObjectRef nodePrototype{},mmlNodePrototype{},tokenPrototype{};

    State(Context& context,NativeIdentityBindings<MmlNode*>& identities,
          ContextPropertyBindings& properties,
          AttributesContextBindings& attributes,
          tree::TreeNodeContextBindings<MmlNode>& treeNodes,
          tree::TreeNodeContextClasses<MmlNode>& treeClasses,
          std::function<Value(tree::FactoryChildren<MmlNode>)> childrenToValue,
          std::function<bool(Value)> isAbstractMmlNode,
          std::weak_ptr<std::atomic_bool> executionLifetime,
          std::function<bool(Value)> isTextNode,
          std::optional<MmlNodeContextPrototypes> sharedPrototypes)
        :context(context),identities(identities),properties(properties),attributes(attributes),treeNodes(treeNodes),treeClasses(treeClasses),childrenToValue(std::move(childrenToValue)),isAbstractMmlNode(std::move(isAbstractMmlNode)),isTextNode(std::move(isTextNode)),executionLifetime(std::move(executionLifetime)),installSourceMethods(!sharedPrototypes){
        if(&identities.context()!=&context)
            throw Exception("TypeError",u"MML node methods require the identity registry's Context");
        if(!this->isAbstractMmlNode)
            throw Exception("TypeError",u"MML token traversal requires its Source class identity");
        if(!this->isTextNode)
            throw Exception("TypeError",u"MML token text traversal requires its Source TextNode identity");
        if(sharedPrototypes){
            nodePrototype=sharedPrototypes->node;
            mmlNodePrototype=sharedPrototypes->mmlNode;
            tokenPrototype=sharedPrototypes->token;
        }else{
            nodePrototype=treeClasses.abstractNodePrototype();
            mmlNodePrototype=context.object();
            tokenPrototype=context.object();
            context.setPrototype(mmlNodePrototype,nodePrototype);
            context.setPrototype(tokenPrototype,mmlNodePrototype);
        }
    }
    void requireActive()const{
        const auto lease=executionLifetime.lock();
        if(!lease||!lease->load(std::memory_order_acquire))
            throw Exception("TypeError",u"MML node Context methods execution has expired");
    }
    MmlNode& receiver(Context& callContext,Value receiver)const{
        requireActive();
        if(&callContext!=&context)
            throw Exception("TypeError",u"MML node method called with a foreign Context");
        const auto* object=std::get_if<ObjectRef>(&receiver.data);
        if(!object)throw Exception("TypeError",u"MML node method called with an invalid receiver");
        return *identities.resolve(*object);
    }
    Receiver operationView(Value exactReceiver){
        requireActive();
        const auto self=shared_from_this();
        Receiver view;
        view.tree=treeClasses.sourceReceiver(exactReceiver);
        view.ensureActive=[self]{self->requireActive();};
        view.self=[self,exactReceiver]{return self->treeNodes.childValue(exactReceiver);};
        view.toDynamic=[self](tree::ChildValue<MmlNode> child){return self->treeNodes.sourceValue(child);};
        view.toChild=[self](Value value){return self->treeNodes.childValue(std::move(value));};
        view.get=[self](Value object,std::u16string_view key){
            self->requireActive();return self->context.get(std::move(object),Key(std::u16string(key)));
        };
        view.getIndex=[self](Value object,std::size_t index){
            self->requireActive();return self->context.get(std::move(object),Key(property_host::text(std::to_string(index))));
        };
        view.childrenValue=[self](tree::FactoryChildren<MmlNode> children){
            self->requireActive();
            if(!children)return Value(Null{});
            if(children)if(const auto source=children->sourceValue())return *source;
            if(!self->childrenToValue)throw Exception("TypeError",u"MML child array has no canonical Context identity");
            return self->childrenToValue(std::move(children));
        };
        view.set=[self](Value object,std::u16string_view key,Value value){
            self->requireActive();self->context.set(std::move(object),Key(std::u16string(key)),std::move(value));
        };
        view.invoke=[self](Value callable,Value exactThis,std::span<const Value> args){
            self->requireActive();const auto* function=std::get_if<ObjectRef>(&callable.data);
            if(!function)throw Exception("TypeError",u"MML method is not callable");
            return self->context.call(*function,std::move(exactThis),args);
        };
        view.invokeCoreTreeSuper=[self](std::u16string_view name,Value exactThis,
                                        std::span<const Value> args){
            self->requireActive();
            const Value corePrototype(self->treeClasses.abstractNodePrototype());
            const Value selected=self->context.get(corePrototype,Key(std::u16string(name)));
            const Value call=self->context.get(selected,Key(u"call"));
            const auto* function=std::get_if<ObjectRef>(&call.data);
            if(!function)throw Exception("TypeError",u"Core Tree method .call is not callable");
            std::vector<Value> callArgs;callArgs.reserve(args.size()+1);
            callArgs.push_back(std::move(exactThis));
            callArgs.insert(callArgs.end(),args.begin(),args.end());
            const auto* method=std::get_if<ObjectRef>(&selected.data);
            if(!method)throw Exception("TypeError",u"Core Tree method is not callable");
            return self->context.call(*function,Value(*method),callArgs);
        };
        view.string=[](std::u16string_view text){return Value(std::u16string(text));};
        view.numberValue=[](double number){return Value(number);};
        view.undefined=[](){return Value(Undefined{});};
        view.nullValue=[](){return Value(Null{});};
        view.arrayOfOne=[self](Value value){
            self->requireActive();
            const auto array=self->context.array(1);
            self->context.set(Value(array),Key(u"0"),std::move(value));
            return Value(array);
        };
        view.constructAttributes=[self](Value defaults,Value mathDefaults){
            self->requireActive();
            const std::array<Value,2> args{std::move(defaults),std::move(mathDefaults)};
            return self->context.construct(self->attributes.constructor(),args);
        };
        view.truthy=[](Value value){return options::truthy(value);};
        view.number=[self](Value value){self->requireActive();return self->context.toNumber(value);};
        view.strictEqual=[](Value a,Value b){return a==b;};
        view.childrenOf=[self](Value value){self->requireActive();return self->treeNodes.arrayView(std::move(value));};
        view.objectAssign=[self](Value value){
            self->requireActive();return self->properties.to(tree::treeContextCopyOwnEnumerable(
                self->context,self->properties,std::move(value)));
        };
        view.ownEnumerableStringKeys=[self](Value value){
            self->requireActive();return self->context.enumerableStringKeys(std::move(value));
        };
        view.forOf=[self](Value iterable,const std::function<void(Value)>& body){
            self->requireActive();self->context.forEachIterable(std::move(iterable),body);
        };
        view.callback=[self](const std::function<Value(std::span<const Value>)>& body){
            self->requireActive();
            const auto callback=self->context.function(u"",[self,body](Context&,Value,std::span<const Value> args){
                self->requireActive();return body(args);
            });
            self->context.define(callback,Key(u"length"),Descriptor::data(Value(1),false,false,true));
            return Value(callback);
        };
        return view;
    }
    void initialize(){
        if(!installSourceMethods)return;
        const auto self=shared_from_this();
        // AbstractMmlTokenNode's CJS prototype assignment is also anonymous;
        // getText() has no declared parameters.
        const auto getText=context.function(u"",
            [self](Context& cx,Value thisValue,std::span<const Value>)->Value{
                self->requireActive();
                if(&cx!=&self->context)throw Exception("TypeError",u"MML token getText called with a foreign Context");
                std::u16string text;
                const auto children=self->context.get(thisValue,Key(u"childNodes"));
                self->context.forEachIterable(children,[&](Value child){
                    Value part(Undefined{});
                    if(self->isTextNode&&self->isTextNode(child)){
                        const auto method=self->context.get(child,Key(u"getText"));
                        const auto* function=std::get_if<ObjectRef>(&method.data);
                        if(!function)throw Exception("TypeError",u"TextNode.getText is not callable");
                        part=self->context.call(*function,child);
                    }else{
                        if(!std::holds_alternative<ObjectRef>(child.data)){
                            std::u16string valueText;
                            if(const auto* symbol=std::get_if<SymbolRef>(&child.data))
                                valueText=self->context.symbolString(*symbol);
                            else valueText=property_host::string(self->properties.from(child));
                            throw Exception("TypeError",std::u16string(
                                u"Cannot use 'in' operator to search for 'textContent' in ")+valueText);
                        }
                        if(!self->context.has(child,Key(u"textContent")))return;
                        const auto method=self->context.get(child,Key(u"textContent"));
                        const auto* function=std::get_if<ObjectRef>(&method.data);
                        if(!function||self->context.kind(*function)!=ObjectKind::function)
                            throw Exception("TypeError",u"textContent is not callable");
                        part=self->context.call(*function,child);
                    }
                    const auto primitive=self->context.toPrimitive(part,PrimitiveHint::defaultHint);
                    text+=property_host::string(self->properties.from(primitive));
                });
                return Value(std::move(text));
            });
        context.set(tokenPrototype,Key(u"getText"),Value(getText));
        const auto walkTree=context.function(u"",
            [self](Context& cx,Value thisValue,std::span<const Value> args)->Value{
                self->requireActive();
                if(&cx!=&self->context)throw Exception("TypeError",u"MML token walk called with a foreign Context");
                const Value callback=args.empty()?Value(Undefined{}):args[0];
                const Value data=args.size()<2?Value(Undefined{}):args[1];
                tree::TreeNodeAlgorithms<MmlNode>::walkTreeFiltered(self->treeClasses.sourceReceiver(thisValue),
                    [self,callback](tree::ChildValue<MmlNode> node,const std::any& state){
                        const auto* function=std::get_if<ObjectRef>(&callback.data);
                        if(!function)throw Exception("TypeError",u"MML token walk callback is not callable");
                        const std::array<Value,2> forwarded{self->treeNodes.sourceValue(node),std::any_cast<Value>(state)};
                        (void)self->context.call(*function,Undefined{},forwarded);
                    },std::any(data),std::any(callback),
                    [self](tree::ChildValue<MmlNode> child){
                        self->requireActive();return self->isAbstractMmlNode(self->treeNodes.sourceValue(child));
                    });
                return data;
            });
        context.define(walkTree,Key(u"length"),Descriptor::data(Value(2),false,false,true));
        context.set(tokenPrototype,Key(u"walkTree"),Value(walkTree));

        const auto installMmlGetter=[&](std::u16string_view name,Value inheritedValue){
            const auto getter=context.function(u"get",
                [self,inheritedValue=std::move(inheritedValue)](Context& cx,Value,std::span<const Value>)->Value{
                    self->requireActive();
                    if(&cx!=&self->context)throw Exception("TypeError",u"MML getter called with a foreign Context");
                    return inheritedValue;
                });
            context.define(getter,Key(u"length"),Descriptor::data(Value(0),false,false,true));
            context.define(mmlNodePrototype,Key(std::u16string(name)),
                           Descriptor::access(getter,std::nullopt,false,true));
        };
        installMmlGetter(u"arity",Value(std::numeric_limits<double>::infinity()));
        installMmlGetter(u"isInferred",Value(false));

        const auto bindMmlMethod=[&](std::u16string_view key,double arity,
                                     NativeCall body){
            const auto method=context.function(u"",std::move(body));
            context.define(method,Key(u"length"),Descriptor::data(Value(arity),false,false,true));
            context.set(mmlNodePrototype,Key(std::u16string(key)),Value(method));
        };
        bindMmlMethod(u"setChildren",1,
            [self](Context& cx,Value thisValue,std::span<const Value> args)->Value{
                self->requireActive();
                if(&cx!=&self->context)throw Exception("TypeError",u"MML method called with a foreign Context");
                const Value raw=args.empty()?Value(Undefined{}):args.front();
                auto view=self->operationView(thisValue);
                return Algorithms::setChildren(view,raw);
            });
        bindMmlMethod(u"appendChild",1,
            [self](Context& cx,Value thisValue,std::span<const Value> args)->Value{
                self->requireActive();
                if(&cx!=&self->context)throw Exception("TypeError",u"MML method called with a foreign Context");
                const Value raw=args.empty()?Value(Undefined{}):args.front();
                auto view=self->operationView(thisValue);
                return Algorithms::appendChild(view,view.toChild(raw));
            });
        bindMmlMethod(u"replaceChild",2,
            [self](Context& cx,Value thisValue,std::span<const Value> args)->Value{
                self->requireActive();
                if(&cx!=&self->context)throw Exception("TypeError",u"MML method called with a foreign Context");
                const Value newChild=args.empty()?Value(Undefined{}):args[0];
                const Value oldChild=args.size()<2?Value(Undefined{}):args[1];
                auto view=self->operationView(thisValue);
                return Algorithms::replaceChild(view,view.toChild(newChild),view.toChild(oldChild));
            });
        bindMmlMethod(u"copy",1,
            [self](Context& cx,Value thisValue,std::span<const Value> args)->Value{
                self->requireActive();
                if(&cx!=&self->context)throw Exception("TypeError",u"MML method called with a foreign Context");
                const bool keepIds=!args.empty()&&truthy(args.front());
                auto view=self->operationView(std::move(thisValue));
                return Algorithms::copy(view,keepIds);
            });
    }

};

MmlNodeContextMethods::MmlNodeContextMethods(Context& context,
    NativeIdentityBindings<MmlNode*>& identities,
    ContextPropertyBindings& properties,
    AttributesContextBindings& attributes,
    tree::TreeNodeContextBindings<MmlNode>& treeNodes,
    tree::TreeNodeContextClasses<MmlNode>& treeClasses,
    std::function<Value(tree::FactoryChildren<MmlNode>)> childrenToValue,
    std::function<bool(Value)> isAbstractMmlNode,
    std::weak_ptr<std::atomic_bool> executionLifetime,
    std::function<bool(Value)> isTextNode,
    std::optional<MmlNodeContextPrototypes> sharedPrototypes)
    :state_(std::make_shared<State>(context,identities,properties,attributes,treeNodes,treeClasses,std::move(childrenToValue),std::move(isAbstractMmlNode),std::move(executionLifetime),std::move(isTextNode),std::move(sharedPrototypes))){
    state_->initialize();
}

void MmlNodeContextMethods::bind(ObjectRef identity,MmlNode& node,bool preservePrototype){
    auto& state=*state_;
    state.requireActive();
    if(state.identities.resolve(identity)!=&node)
        throw Exception("Unimplemented",u"MML node Context identity does not match its actual owner");
    node.sourceIdentity(identity);
    const std::weak_ptr<State> weak=state_;
    const Value exactReceiver(identity);
    node.sourceTreeAccess(state.treeNodes.nodeAccess(exactReceiver));
    node.sourceOperationView(
        [weak,exactReceiver]{
            const auto live=weak.lock();
            if(!live)throw Exception("TypeError",u"MML operation Context owner has expired");
            live->requireActive();
            return live->operationView(exactReceiver);
        },
        [weak](Value value)->MmlNode*{
            const auto live=weak.lock();
            if(!live)throw Exception("TypeError",u"MML operation Context owner has expired");
            live->requireActive();
            const auto* object=std::get_if<ObjectRef>(&value.data);
            if(!object)throw Exception("TypeError",u"MML typed operation returned a non-object value");
            return live->identities.resolve(*object);
        });
    if(!preservePrototype)
        state.context.setPrototype(identity,node.isToken()?state.tokenPrototype:state.mmlNodePrototype);
}
ObjectRef MmlNodeContextMethods::nodePrototype()const{return state_->nodePrototype;}
ObjectRef MmlNodeContextMethods::tokenPrototype()const{return state_->tokenPrototype;}
ObjectRef MmlNodeContextMethods::mmlNodePrototype()const{return state_->mmlNodePrototype;}
}

