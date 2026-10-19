module folia.math.core.mml_tree.mml_factory_context_fields;
import folia.math.core.mml_tree.mml_factory_context_values;
import folia.math.core.mml_tree.mml;
import folia.math.core.mml_tree.class_binding;
import folia.math.util.property_host;
import std;

namespace folia::math::mml {
using namespace options;

struct MmlFactoryContextFields::State:std::enable_shared_from_this<State>{
    struct ClassModule {
        struct Entry {
            const tree::FactoryClass<MmlNode>* descriptor=nullptr;
            ObjectRef constructor{};
            std::shared_ptr<MmlClassInfo> info;
        };
        Context& context_;
        OptionFunctionBindings functions_;
        ContextPropertyBindings properties_;
        std::vector<Entry> entries;
        std::vector<std::pair<const PropertyStorage*,ObjectRef>> defaultsObjects;
        std::vector<std::pair<const std::vector<Property>*,ObjectRef>> accentsObjects;
        std::vector<std::pair<std::u16string,tree::FactoryClassRef<MmlNode>>> abstractClasses;
        std::weak_ptr<MmlSourceAllocator> sourceAllocator;

        ClassModule(Context& context,std::weak_ptr<MmlSourceAllocator> allocator)
            :context_(context),functions_(context),properties_(context,functions_),
             sourceAllocator(allocator){}
        Context& context()const{return context_;}
        void requireActive()const{
            if(&properties_.context()!=&context_)
                throw Exception("TypeError",u"MML class Context property owner has been released");
            const auto allocator=sourceAllocator.lock();
            if(!allocator)throw Exception("TypeError",u"MML Context allocator has expired");
            allocator->validate();
        }
        ObjectRef defaultsObject(const PropertyList& defaults){
            requireActive();
            const Property asProperty(defaults);
            const auto* storage=std::get_if<std::shared_ptr<PropertyStorage>>(&asProperty.value);
            if(!storage||!*storage)throw Exception("TypeError",u"MML class defaults template has no property owner");
            for(const auto& [known,object]:defaultsObjects)if(known==storage->get())return object;
            // Static class fields are initialized once from the immutable
            // native template into a new Context-owned ordinary object. The
            // typed template is never registered as a live mutable projection.
            const auto object=context_.object();
            for(const auto& key:defaults.keys()){
                const auto value=properties_.to(defaults.getOwn(key));
                context_.define(object,Key(property_host::text(key)),Descriptor::data(value));
            }
            for(const auto& symbol:defaults.symbolKeys()){
                const auto value=properties_.to(defaults.getKey(Property(symbol)));
                context_.define(object,Key(symbol),Descriptor::data(value));
            }
            defaultsObjects.emplace_back(storage->get(),object);
            return object;
        }
        ObjectRef accentsObject(const std::shared_ptr<std::vector<Property>>& accents){
            requireActive();
            if(!accents)throw Exception("TypeError",u"MML class ACCENTS template is null");
            for(const auto& [known,object]:accentsObjects)if(known==accents.get())return object;
            PropertyArray array;
            for(const auto& value:*accents)array.push(value);
            const auto result=properties_.to(Property(array));
            const auto* object=std::get_if<ObjectRef>(&result.data);
            if(!object)throw Exception("TypeError",u"MML ACCENTS template did not produce an Array");
            accentsObjects.emplace_back(accents.get(),*object);
            return *object;
        }
        Entry* find(const tree::FactoryClass<MmlNode>* descriptor){
            for(auto& entry:entries)if(entry.descriptor==descriptor)return &entry;
            return nullptr;
        }
        const Entry* find(const tree::FactoryClass<MmlNode>* descriptor)const{
            for(const auto& entry:entries)if(entry.descriptor==descriptor)return &entry;
            return nullptr;
        }
        std::optional<ObjectRef> constructorNamed(std::u16string_view name)const{
            for(const auto& entry:entries){
                // Entry retains metadata independently of its raw descriptor pointer.
                if(entry.info&&entry.info->sourceConstructorName==name)return entry.constructor;
            }
            return std::nullopt;
        }
        tree::FactoryClassRef<MmlNode> abstractClass(std::u16string_view name){
            for(const auto& [key,value]:abstractClasses)if(key==name)return value;
            tree::FactoryClassRef<MmlNode> result;
            if(name==u"AbstractMmlNode")
                result=classBinding<AbstractMmlNode>(u"AbstractMmlNode",u"unknown",u"AbstractNode",3);
            else if(name==u"AbstractMmlTokenNode")
                result=classBinding<AbstractMmlTokenNode>(u"AbstractMmlTokenNode",u"unknown",u"AbstractMmlNode",0);
            else if(name==u"AbstractMmlLayoutNode")
                result=classBinding<AbstractMmlLayoutNode>(u"AbstractMmlLayoutNode",u"unknown",u"AbstractMmlNode",0);
            else if(name==u"AbstractMmlBaseNode")
                result=classBinding<AbstractMmlBaseNode>(u"AbstractMmlBaseNode",u"unknown",u"AbstractMmlNode",0);
            else if(name==u"AbstractMmlEmptyNode")
                result=classBinding<AbstractMmlEmptyNode>(u"AbstractMmlEmptyNode",u"unknown",u"AbstractEmptyNode",0);
            if(result){
                const auto* info=std::any_cast<std::shared_ptr<MmlClassInfo>>(&result->staticData);
                if(info&&*info){
                    if(name==u"AbstractMmlNode")(*info)->sourceArity=std::numeric_limits<double>::infinity();
                    else if(name==u"AbstractMmlLayoutNode")(*info)->sourceArity=-1.0;
                    else if(name==u"AbstractMmlEmptyNode"){
                        (*info)->sourceArity=0.0;
                        (*info)->sourceIsInferred=false;
                    }
                }
                abstractClasses.emplace_back(std::u16string(name),result);
            }
            return result;
        }
    };

    Context& context;
    MapContextBindings& maps;
    ContextPropertyBindings& properties;
    NativeIdentityBindings<MmlNode*>& nodes;
    NativeIdentityBindings<Children>& arrays;
    tree::TreeNodeContextBindings<MmlNode>& treeNodes;
    tree::TreeNodeContextClasses<MmlNode>& treeClasses;
    NativeIdentityBindings<MmlFactory*>& factories;
    std::weak_ptr<std::atomic_bool> lease;
    std::shared_ptr<ClassModule> classModule;
    ResolveFactory resolveFactory;
    FindNodeIdentity findNodeIdentity;
    ArgumentToValue argumentToValue;
    std::shared_ptr<MmlSourceAllocator> sourceAllocator;
    std::vector<std::pair<MmlFactory*,ObjectRef>> nodeObjects;
    std::vector<std::tuple<tree::FactoryCreatorRef<MmlNode>,MmlFactory*,ObjectRef>> creatorFunctions;
    std::vector<std::pair<ObjectRef,std::shared_ptr<const std::uint8_t>>> sourceMapIdentities;
    ObjectRef nodePrototype{},tokenPrototype{},abstractMmlNodePrototype{};
    std::optional<ObjectRef> prototypeWithMmlGetter;

    State(Context& c,MapContextBindings& m,ContextPropertyBindings& p,
          NativeIdentityBindings<MmlNode*>& n,NativeIdentityBindings<Children>& a,
          tree::TreeNodeContextBindings<MmlNode>& tn,
          tree::TreeNodeContextClasses<MmlNode>& tc,
          NativeIdentityBindings<MmlFactory*>& f,
          std::weak_ptr<std::atomic_bool> active,ResolveFactory resolve,
          FindNodeIdentity findNode,ArgumentToValue argumentValue,
          std::shared_ptr<MmlSourceAllocator> allocator)
      :context(c),maps(m),properties(p),nodes(n),arrays(a),treeNodes(tn),treeClasses(tc),factories(f),lease(std::move(active)),
       sourceAllocator(allocator),
       resolveFactory(std::move(resolve)),findNodeIdentity(std::move(findNode)),
       argumentToValue(std::move(argumentValue)){
        if(&maps.context()!=&context||&properties.context()!=&context||
           &nodes.context()!=&context||&factories.context()!=&context||
           !resolveFactory||!findNodeIdentity||!argumentToValue||!sourceAllocator)
            throw Exception("TypeError",u"MmlFactory fields require one Context and canonical identity resolvers");
        requireActive();
        // Constructor objects and the allocator share this composition's
        // lifetime. Clearing/recreating ContextBindings cannot reuse
        // constructors that still point weakly at an older allocator.
        classModule=std::make_shared<ClassModule>(context,sourceAllocator);
    }
    std::shared_ptr<std::atomic_bool> requireActive()const{
        auto live=lease.lock();
        if(!live||!live->load(std::memory_order_acquire))
            throw Exception("TypeError",u"MmlFactory Context runtime has been released");
        return live;
    }
    tree::FactoryClassRef<MmlNode> abstractClass(std::u16string_view name){
        return classModule->abstractClass(name);
    }
    ObjectRef treeBaseConstructor(std::u16string_view name){
        if(name==u"AbstractNode")return treeClasses.abstractNodeConstructor();
        if(name==u"AbstractEmptyNode")return treeClasses.abstractEmptyNodeConstructor();
        throw Exception("Unimplemented",u"Unknown Source tree base constructor");
    }
    ObjectRef abstractPrototype(std::u16string_view name){
        if(name==u"AbstractNode")return treeClasses.abstractNodePrototype();
        if(name==u"AbstractEmptyNode")return treeClasses.abstractEmptyNodePrototype();
        const auto actual=abstractClass(name);
        if(!actual)throw Exception("Unimplemented",u"Unknown Source MML abstract descriptor");
        return std::get<ObjectRef>(context.get(Value(classConstructor(actual)),Key(u"prototype")).data);
    }
    std::optional<ObjectRef> existingBasePrototype(std::u16string_view name)const{
        if(name==u"AbstractNode")return treeClasses.abstractNodePrototype();
        if(name==u"AbstractMmlNode")return abstractMmlNodePrototype;
        if(name==u"AbstractMmlTokenNode")return tokenPrototype;
        return std::nullopt;
    }
    ObjectRef classConstructor(const tree::FactoryClassRef<MmlNode>& cls){
        requireActive();
        if(!cls||!cls->construct)
            throw Exception("TypeError",u"MML class descriptor has no constructor capability");
        if(const auto* existing=classModule->find(cls.get()))return existing->constructor;
        const auto* info=std::any_cast<std::shared_ptr<MmlClassInfo>>(&cls->staticData);
        if(!info||!*info||(*info)->sourceConstructorName.empty()||(*info)->sourceKind.empty()||
           (*info)->sourcePrototypeParent.empty())
            throw Exception("Unimplemented",u"MML descriptor lacks pinned Source class metadata");
        const auto self=shared_from_this();
        const auto retained=cls;
        const std::weak_ptr<ClassModule> weakClassModule=classModule;
        const auto name=(*info)->sourceConstructorName;
        const auto kind=(*info)->sourceKind;
        const auto prototypeOverride=existingBasePrototype(name);
        using InitializeReceiver=std::function<Value(Context&,Value,std::span<const Value>)>;
        InitializeReceiver initializeReceiver=[retained,weakClassModule](
                Context& cx,Value receiver,std::span<const Value> args)->Value{
                const auto classModule=weakClassModule.lock();
                if(!classModule)throw Exception("TypeError",u"MML class runtime owner has expired");
                classModule->requireActive();
                if(&cx!=&classModule->context())throw Exception("TypeError",u"MML constructor called in another Context");
                const auto* receiverObject=std::get_if<ObjectRef>(&receiver.data);
                if(!receiverObject)throw Exception("TypeError",u"MML constructor receiver is not an object");
                const Value factoryValue=args.empty()?Value(Undefined{}):args[0];
                const Value rawAttributes=args.size()>1&&
                    !std::holds_alternative<Undefined>(args[1].data)
                    ?args[1]:Value(cx.object());
                const Value rawChildren=args.size()>2&&
                    !std::holds_alternative<Undefined>(args[2].data)
                    ?args[2]:Value(cx.array(0));
                const auto allocator=classModule->sourceAllocator.lock();
                if(!allocator)throw Exception("TypeError",u"MML Context allocator has expired");
                (void)allocator->initializeReceiver(*receiverObject,retained,
                    factoryValue,rawAttributes,rawChildren);
                return receiver;
            };
        auto constructorIdentity=std::make_shared<std::optional<ObjectRef>>();
        const auto constructor=context.nativeConstructor(name,
            [initializeReceiver,constructorIdentity](Context& cx,std::span<const Value> args)->ObjectRef{
                if(!constructorIdentity||!constructorIdentity->has_value())
                    throw Exception("TypeError",u"MML constructor identity is unavailable");
                const Value currentPrototype=cx.get(Value(**constructorIdentity),Key(u"prototype"));
                const auto* objectPrototype=std::get_if<ObjectRef>(&currentPrototype.data);
                const ObjectRef prototype=objectPrototype?*objectPrototype:cx.intrinsics().objectPrototype;
                const auto receiver=cx.objectWithPrototype(prototype);
                (void)initializeReceiver(cx,Value(receiver),args);
                return receiver;
            },
            [initializeReceiver](Context& cx,Value receiver,std::span<const Value> args)->Value{
                return initializeReceiver(cx,std::move(receiver),args);
            },retained);
        if(prototypeOverride){
            context.set(Value(constructor),Key(u"prototype"),Value(*prototypeOverride));
            context.define(*prototypeOverride,Key(u"constructor"),
                Descriptor::data(Value(constructor),true,false,true));
        }
        context.define(constructor,Key(u"length"),Descriptor::data(
            Value(static_cast<double>((*info)->sourceConstructorArity)),false,false,true));
        const auto proto=prototypeOverride?*prototypeOverride:
            std::get<ObjectRef>(context.get(Value(constructor),Key(u"prototype")).data);
        *constructorIdentity=constructor;
        // Class fields are own constructor properties only where the pinned
        // Source class declares them. Values are initialized once per runtime
        // module from immutable typed templates, then that Context object is
        // the single mutable owner observed by both Source and typed nodes.
        if((*info)->sourceOwnDefaults){
            if(auto defaults=(*info)->getDefaults()){
                const auto value=classModule->defaultsObject(*defaults);
                context.define(constructor,Key(u"defaults"),Descriptor::data(Value(value)));
            }
        }
        if((*info)->sourceOwnAccents&&(*info)->accentsBinding){
            const auto value=classModule->accentsObject((*info)->accentsBinding());
            context.define(constructor,Key(u"ACCENTS"),Descriptor::data(Value(value)));
        }
        if(kind!=u"unknown"){
            const auto kindGetter=context.function(u"get",[kind](Context&,Value,std::span<const Value>){
                return Value(kind);
            });
            context.define(kindGetter,Key(u"length"),Descriptor::data(Value(0),false,false,true));
            context.define(proto,Key(u"kind"),Descriptor::access(kindGetter,std::nullopt,false,true));
        }
        if((*info)->sourceArity){
            const auto value=*(*info)->sourceArity;
            const auto getter=context.function(u"get",[value](Context&,Value,std::span<const Value>){return Value(value);});
            context.define(getter,Key(u"length"),Descriptor::data(Value(0),false,false,true));
            context.define(proto,Key(u"arity"),Descriptor::access(getter,std::nullopt,false,true));
        }
        if((*info)->sourceIsInferred){
            const auto value=*(*info)->sourceIsInferred;
            const auto getter=context.function(u"get",[value](Context&,Value,std::span<const Value>){return Value(value);});
            context.define(getter,Key(u"length"),Descriptor::data(Value(0),false,false,true));
            context.define(proto,Key(u"isInferred"),Descriptor::access(getter,std::nullopt,false,true));
        }
        std::optional<ObjectRef> parentConstructor;
        ObjectRef parentPrototype{};
        for(const auto& [candidateKey,candidate]:*MML()){
            const auto* candidateInfo=std::any_cast<std::shared_ptr<MmlClassInfo>>(&candidate->staticData);
            if(candidateInfo&&*candidateInfo&&(*candidateInfo)->sourceConstructorName==(*info)->sourcePrototypeParent){
                parentConstructor=classConstructor(candidate);
                parentPrototype=std::get<ObjectRef>(context.get(Value(*parentConstructor),Key(u"prototype")).data);
                break;
            }
        }
        if(parentConstructor){
            context.setPrototype(constructor,*parentConstructor);
            context.setPrototype(proto,parentPrototype);
        }else{
            const auto& parent=(*info)->sourcePrototypeParent;
            if(const auto parentClass=abstractClass(parent)){
                const auto parentCtor=classConstructor(parentClass);
                context.setPrototype(constructor,parentCtor);
                parentPrototype=std::get<ObjectRef>(context.get(Value(parentCtor),Key(u"prototype")).data);
            }else{
                const auto parentCtor=treeBaseConstructor(parent);
                context.setPrototype(constructor,parentCtor);
                parentPrototype=std::get<ObjectRef>(context.get(Value(parentCtor),Key(u"prototype")).data);
            }
            context.setPrototype(proto,parentPrototype);
        }
        if((*info)->sourceConstructorName==u"TextNode"){
            const std::weak_ptr<ClassModule> weakTextModule=classModule;
            const auto bindTextMethod=[&](std::u16string_view key,double arity,NativeCall body){
                const auto method=context.function(u"",[weakTextModule,body=std::move(body)](
                        Context& cx,Value receiver,std::span<const Value> args)->Value{
                    const auto module=weakTextModule.lock();
                    if(!module)throw Exception("TypeError",u"TextNode class module has expired");
                    module->requireActive();
                    if(&cx!=&module->context_)
                        throw Exception("TypeError",u"TextNode method called with a foreign Context");
                    return body(cx,std::move(receiver),args);
                });
                context.define(method,Key(u"length"),Descriptor::data(Value(arity),false,false,true));
                context.set(proto,Key(std::u16string(key)),Value(method));
            };
            bindTextMethod(u"getText",0,[](Context& cx,Value receiver,std::span<const Value>){
                return cx.get(std::move(receiver),Key(u"text"));
            });
            bindTextMethod(u"setText",1,[](Context& cx,Value receiver,std::span<const Value> args){
                const Value text=args.empty()?Value(Undefined{}):args.front();
                cx.set(receiver,Key(u"text"),text);
                return receiver;
            });
            bindTextMethod(u"toString",0,[](Context& cx,Value receiver,std::span<const Value>){
                return cx.get(std::move(receiver),Key(u"text"));
            });
            bindTextMethod(u"copy",0,[](Context& cx,Value receiver,std::span<const Value>)->Value{
                const Value factory=cx.get(receiver,Key(u"factory"));
                const Value create=cx.get(factory,Key(u"create"));
                const Value kind=cx.get(receiver,Key(u"kind"));
                const auto* createFunction=std::get_if<ObjectRef>(&create.data);
                if(!createFunction||cx.kind(*createFunction)!=ObjectKind::function)
                    throw Exception("TypeError",u"this.factory.create is not a function");
                const std::array<Value,1> createArgs{kind};
                const Value copy=cx.call(*createFunction,factory,createArgs);
                const Value setText=cx.get(copy,Key(u"setText"));
                const Value getText=cx.get(receiver,Key(u"getText"));
                const auto* getTextFunction=std::get_if<ObjectRef>(&getText.data);
                if(!getTextFunction||cx.kind(*getTextFunction)!=ObjectKind::function)
                    throw Exception("TypeError",u"this.getText is not a function");
                const Value text=cx.call(*getTextFunction,receiver);
                const auto* setTextFunction=std::get_if<ObjectRef>(&setText.data);
                if(!setTextFunction||cx.kind(*setTextFunction)!=ObjectKind::function)
                    throw Exception("TypeError",u"this.factory.create(...).setText is not a function");
                const std::array<Value,1> setArgs{text};
                return cx.call(*setTextFunction,copy,setArgs);
            });
        }
        auto runtimeInfo=std::make_shared<MmlClassInfo>(**info);
        if((*info)->defaultsBinding||(*info)->defaults){
            const std::weak_ptr<ClassModule> weakModule=classModule;
            runtimeInfo->defaultsBinding=[weakModule,constructor]()->std::optional<PropertyList>{
                const auto module=weakModule.lock();
                if(!module)throw Exception("TypeError",u"MML class defaults owner has expired");
                module->requireActive();
                const Value raw=module->context_.get(Value(constructor),Key(u"defaults"));
                if(std::holds_alternative<Undefined>(raw.data))return std::nullopt;
                if(std::holds_alternative<Null>(raw.data))return PropertyList::withNullPrototype();
                const Property value=module->properties_.from(raw);
                auto result=PropertyList::record(value);
                if(!result)throw Exception("TypeError",u"MML class defaults must be an object or null");
                return *result;
            };
        }
        if((*info)->accentsBinding){
            const std::weak_ptr<ClassModule> weakModule=classModule;
            runtimeInfo->sourceAccentsPropertyBinding=[weakModule,constructor]{
                const auto module=weakModule.lock();
                if(!module)throw Exception("TypeError",u"MML class ACCENTS owner has expired");
                module->requireActive();
                const Value raw=module->context_.get(Value(constructor),Key(u"ACCENTS"));
                const Property value=module->properties_.from(raw);
                const auto* array=std::get_if<std::shared_ptr<PropertyStorage>>(&value.value);
                if(!array||!*array||!(*array)->array)
                    throw Exception("TypeError",u"MML class ACCENTS must be an Array for native typed consumption");
                const auto result=PropertyArray::array(value);
                if(!result)throw Exception("TypeError",u"MML class ACCENTS must be an Array for native typed consumption");
                return *result;
            };
        }
        classModule->entries.push_back(ClassModule::Entry{cls.get(),constructor,std::move(runtimeInfo)});
        return constructor;
    }
    std::shared_ptr<MmlClassInfo> classInfo(const tree::FactoryClassRef<MmlNode>& cls){
        (void)classConstructor(cls);
        const auto* entry=classModule->find(cls.get());
        if(!entry||!entry->info)throw Exception("TypeError",u"MML runtime class metadata is unavailable");
        return entry->info;
    }
    void bindNodePrototype(ObjectRef identity,MmlNode& node){
        requireActive();
        if(nodes.resolve(identity)!=&node)throw Exception("Unimplemented",u"MML node identity is not its actual owner");
        const auto& actual=node.nativeConstructorClass();
        if(!actual)throw Exception("TypeError",u"MML node has no actual constructor descriptor");
        if(!dynamic_cast<MmlFactory*>(&node.factoryOwner()))
            throw Exception("TypeError",u"MML node has no canonical allocation owner");
        const auto constructor=classConstructor(actual);
        const auto prototype=std::get<ObjectRef>(context.get(Value(constructor),Key(u"prototype")).data);
        context.setPrototype(identity,prototype);
    }
    tree::FactoryClassMapValue<MmlNode> classFromValue(const Value& raw){
        if(const auto* object=std::get_if<ObjectRef>(&raw.data)){
            if(context.kind(*object)==ObjectKind::function){
                const auto* cls=std::any_cast<tree::FactoryClassRef<MmlNode>>(&context.payload(*object));
                if(cls&&*cls)return tree::FactoryClassMapValue<MmlNode>::native(*cls);
            }
        }
        return tree::FactoryClassMapValue<MmlNode>::source(raw);
    }
    Value classToValue(const tree::FactoryClassMapValue<MmlNode>& mapped){
        if(const auto* raw=mapped.sourceValue())return *raw;
        return Value(classConstructor(*mapped.nativeClass()));
    }
    Value creatorValue(const tree::FactoryCreatorRef<MmlNode>& creator,
                       const std::shared_ptr<MmlFactory>& owner){
        requireActive();
        if(!creator||!creator->callWithServices)throw Exception("TypeError",u"MML node entry has no creator");
        for(const auto& [key,factory,value]:creatorFunctions)
            if(key==creator&&factory==owner.get())return Value(value);
        const auto self=shared_from_this();
        const auto liveCreator=creator;
        const auto fn=context.function(u"",[self,liveCreator,owner](Context& cx,Value,
                std::span<const Value> args)->Value{
            self->requireActive();
            if(&cx!=&self->context)throw Exception("TypeError",u"MML node creator called in another Context");
            auto native=mmlFactoryContextArguments(self->properties,self->arrays,self->treeNodes,args);
            auto* node=liveCreator->callWithServices(*owner,tree::FactoryArgs(native));
            if(!node)throw Exception("TypeError",u"MML node creator returned no node");
            const auto identity=self->findNodeIdentity(*node);
            if(!identity)throw Exception("Unimplemented",u"MML creator result has no Context identity");
            return Value(*identity);
        },creator);
        context.define(fn,Key(u"length"),Descriptor::data(Value(0),false,false,true));
        creatorFunctions.emplace_back(creator,owner.get(),fn);
        return Value(fn);
    }
    MmlNode* applyCreator(const Value& selected,const Value& apply,
                          tree::FactoryArgs args){
        requireActive();
        const auto forwarded=context.array(static_cast<std::uint32_t>(args.size()));
        for(std::size_t i=0;i<args.size();++i){
            const auto index=std::to_string(i);
            context.set(Value(forwarded),Key(std::u16string(index.begin(),index.end())),
                        argumentToValue(args[i]));
        }
        const std::array<Value,2> applyArgs{Value(Undefined{}),Value(forwarded)};
        const auto result=context.callMember(apply,selected,applyArgs,u"node[kind].apply");
        const auto* object=std::get_if<ObjectRef>(&result.data);
        if(!object)throw Exception("TypeError",u"MmlFactory creator did not return a node object");
        return nodes.resolve(*object);
    }
    Value sourceMapCall(const Value& receiver,std::u16string_view method,
                        std::span<const Value> args={}){
        requireActive();
        const Value callable=context.get(receiver,Key(std::u16string(method)));
        return context.callMember(callable,receiver,args,method);
    }
    std::vector<Value> sourceMapKeys(const Value& receiver){
        const Value iterator=sourceMapCall(receiver,u"keys");
        std::vector<Value> result;
        context.forEachIterable(iterator,[&](Value key){result.push_back(std::move(key));});
        return result;
    }
    std::shared_ptr<tree::FactoryClassMapAccess<MmlNode>> sourceMapAccess(
            Value receiver,const std::shared_ptr<MmlFactory>& allocator){
        const auto self=shared_from_this();
        const std::weak_ptr<State> weakSelf=self;
        (void)allocator; // The Source class constructor uses the Context allocator.
        const auto weakLease=lease;
        return std::make_shared<tree::SourceFactoryClassMapAccess<MmlNode>>(
            std::move(receiver),
            [weakSelf,weakLease](const Value& target,std::u16string_view method,std::span<const Value> args){
                const auto self=weakSelf.lock();
                if(!self)throw Exception("TypeError",u"MmlFactory Context field owner has been released");
                auto live=weakLease.lock();
                if(!live||!live->load(std::memory_order_acquire))
                    throw Exception("TypeError",u"MmlFactory Context runtime has been released");
                return self->sourceMapCall(target,method,args);
            },
            [weakSelf,weakLease](const Value& target){
                const auto self=weakSelf.lock();
                if(!self)throw Exception("TypeError",u"MmlFactory Context field owner has been released");
                auto live=weakLease.lock();
                if(!live||!live->load(std::memory_order_acquire))
                    throw Exception("TypeError",u"MmlFactory Context runtime has been released");
                return self->sourceMapKeys(target);
            },
            [weakSelf,weakLease](const Value& target)->tree::SourceFactoryClassMapAccess<MmlNode>::StableIdentity{
                const auto self=weakSelf.lock();
                if(!self)return {};
                auto live=weakLease.lock();
                if(!live||!live->load(std::memory_order_acquire))return {};
                const auto* object=std::get_if<ObjectRef>(&target.data);
                if(!object)return {};
                try{
                    auto storage=self->maps.resolve(*object);
                    return {storage->identity(),std::static_pointer_cast<const void>(std::move(storage))};
                }catch(...){}
                for(const auto& [known,token]:self->sourceMapIdentities)
                    if(known==*object)return {token.get(),token};
                self->sourceMapIdentities.emplace_back(*object,std::make_shared<const std::uint8_t>(0));
                const auto& token=self->sourceMapIdentities.back().second;
                return {token.get(),token};
            },
            [weakSelf,weakLease]{
                if(weakSelf.expired())
                    throw Exception("TypeError",u"MmlFactory Context field owner has been released");
                const auto live=weakLease.lock();
                if(!live||!live->load(std::memory_order_acquire))
                    throw Exception("TypeError",u"MmlFactory Context runtime has been released");
            },
            [weakSelf](const Value& raw){
                const auto self=weakSelf.lock();
                if(!self)throw Exception("TypeError",u"MmlFactory Context field owner has been released");
                self->requireActive();
                const auto mapped=self->classFromValue(raw);
                if(const auto* native=mapped.nativeClass())return *native;
                return tree::FactoryClassRef<MmlNode>{};
            },
            [weakSelf](const tree::FactoryClassMapValue<MmlNode>& value){
                const auto self=weakSelf.lock();
                if(!self)throw Exception("TypeError",u"MmlFactory Context field owner has been released");
                self->requireActive();
                return self->classToValue(value);
            });
    }
    std::shared_ptr<tree::ContextFactoryCreatorAccess<MmlNode>> contextCreatorAccess(
        const Value& receiver,const std::weak_ptr<MmlFactory>& weakOwner){
        const auto self=shared_from_this();
        const auto weakLease=lease;
        const auto ensure=[weakLease]{
            const auto live=weakLease.lock();
            if(!live||!live->load(std::memory_order_acquire))
                throw Exception("TypeError",u"MmlFactory Context runtime has been released");
        };
        auto read=[self,ensure](const Value& object,const Key& key){ensure();return self->context.get(object,key);};
        auto write=[self,ensure](const Value& object,const Key& key,Value value){ensure();self->context.set(object,key,std::move(value));};
        auto remove=[self,ensure](const Value& object,const Key& key){
            ensure();const auto* ref=std::get_if<ObjectRef>(&object.data);
            if(!ref)throw Exception("TypeError",u"MmlFactory.node must be an object");
            self->context.erase(*ref,key);
        };
        auto ownKeys=[self,ensure](const Value& object){ensure();return self->context.ownKeys(object);};
        auto readPrototype=[self,ensure](const Value& object)->std::optional<ObjectRef>{
            ensure();const auto* ref=std::get_if<ObjectRef>(&object.data);
            if(!ref)throw Exception("TypeError",u"MmlFactory.node must be an object");
            return self->context.prototype(*ref);
        };
        auto writePrototype=[self,ensure](const Value& object,std::optional<ObjectRef> prototype){
            ensure();const auto* ref=std::get_if<ObjectRef>(&object.data);
            if(!ref)throw Exception("TypeError",u"MmlFactory.node must be an object");
            self->context.setPrototype(*ref,prototype);
        };
        auto callable=[self,ensure](const Value& value){
            ensure();const auto* ref=std::get_if<ObjectRef>(&value.data);
            return ref&&self->context.kind(*ref)==ObjectKind::function;
        };
        auto resolve=[self,ensure](const Value& value){
            ensure();const auto* ref=std::get_if<ObjectRef>(&value.data);
            if(!ref||self->context.kind(*ref)!=ObjectKind::function)return tree::FactoryCreatorRef<MmlNode>{};
            try{return std::any_cast<tree::FactoryCreatorRef<MmlNode>>(self->context.payload(*ref));}
            catch(const std::bad_any_cast&){return tree::FactoryCreatorRef<MmlNode>{};}
        };
        auto expose=[self,ensure,weakOwner](const tree::FactoryCreatorRef<MmlNode>& creator){
            ensure();auto owner=weakOwner.lock();
            if(!owner)throw Exception("TypeError",u"MmlFactory owner has been released");
            return self->creatorValue(creator,owner);
        };
        auto readApply=[self,ensure](const Value& selected){ensure();return self->context.get(selected,Key(u"apply"));};
        auto apply=[self,ensure](const Value& selected,const Value& method,tree::FactoryArgs args){
            ensure();return self->applyCreator(selected,method,std::move(args));
        };
        auto direct=[self,ensure](const Value& target,const Value& selected,tree::FactoryArgs args){
            ensure();const auto* function=std::get_if<ObjectRef>(&selected.data);
            if(!function)throw Exception("TypeError",u"this.node[kind] is not a function");
            std::vector<Value> values;values.reserve(args.size());
            for(const auto& argument:args)values.push_back(self->argumentToValue(argument));
            const Value result=self->context.call(*function,target,values);
            const auto* object=std::get_if<ObjectRef>(&result.data);
            if(!object)throw Exception("TypeError",u"MmlFactory creator did not return a node object");
            return self->nodes.resolve(*object);
        };
        return std::make_shared<tree::ContextFactoryCreatorAccess<MmlNode>>(
            receiver,std::weak_ptr<void>(weakOwner),weakLease,std::move(read),std::move(write),
            std::move(remove),std::move(ownKeys),std::move(readPrototype),std::move(writePrototype),
            std::move(callable),std::move(resolve),std::move(expose),std::move(readApply),
            std::move(apply),std::move(direct));
    }
    void installSourceHooks(const std::shared_ptr<MmlFactory>& owner,ObjectRef identity){
        const auto weakSelf=weak_from_this();
        const std::weak_ptr<MmlFactory> weakOwner=owner;
        const auto require=[weakSelf](){
            auto self=weakSelf.lock();
            if(!self)throw Exception("TypeError",u"MmlFactory fields have been released");
            self->requireActive();return self;
        };
        owner->defaultKindKey([require](const Value& value){
            const auto self=require();
            const Value primitive=self->context.toPrimitive(value,PrimitiveHint::stringHint);
            if(const auto* symbol=std::get_if<SymbolRef>(&primitive.data))return Key(*symbol);
            return Key(property_host::string(self->properties.from(primitive)));
        });
        owner->sourceNodeMapReader([require,identity]{
            const auto self=require();
            const Value current=self->context.get(Value(identity),Key(u"nodeMap"));
            auto currentOwner=self->resolveFactory(identity);
            if(!currentOwner)throw Exception("TypeError",u"MmlFactory owner has been released");
            const auto currentAccess=currentOwner->nodeMapAccess();
            if(const auto* object=std::get_if<ObjectRef>(&current.data)){
                try{if(self->maps.resolve(*object)->identity()==currentAccess->identity())return currentAccess;}
                catch(...){}
            }
            return self->currentNodeMapAccess(currentOwner,current);
        });
        owner->sourceCreatorAccessReader([require,identity,weakOwner]{
            const auto self=require();auto currentOwner=weakOwner.lock();
            if(!currentOwner)throw Exception("TypeError",u"MmlFactory owner has been released");
            const Value current=self->context.get(Value(identity),Key(u"node"));
            return self->currentCreatorAccess(currentOwner,current);
        });
        tree::AbstractFactory<MmlNode>::SourceCreateHooks hooks;
        hooks.readNode=[require,identity]{const auto self=require();return self->context.get(Value(identity),Key(u"node"));};
        hooks.readDefaultKind=[require,identity]{const auto self=require();return self->context.get(Value(identity),Key(u"defaultKind"));};
        hooks.toKey=[require](const Value& value){
            const auto self=require();const Value primitive=self->context.toPrimitive(value,PrimitiveHint::stringHint);
            if(const auto* symbol=std::get_if<SymbolRef>(&primitive.data))return Key(*symbol);
            return Key(property_host::string(self->properties.from(primitive)));
        };
        hooks.lookup=[require](const Value& receiver,const Key& key){const auto self=require();return self->context.get(receiver,key);};
        hooks.readApply=[require](const Value& selected){const auto self=require();return self->context.get(selected,Key(u"apply"));};
        hooks.argumentValue=[require](const std::any& argument){return require()->argumentToValue(argument);};
        hooks.invoke=[require](const Value& selected,const Value& apply,std::span<const Value> arguments){
            const auto self=require();
            const auto array=self->context.array(static_cast<std::uint32_t>(arguments.size()));
            for(std::size_t i=0;i<arguments.size();++i){const auto index=std::to_string(i);
                self->context.set(Value(array),Key(std::u16string(index.begin(),index.end())),arguments[i]);}
            const std::array<Value,2> args{Value(Undefined{}),Value(array)};
            return self->context.callMember(apply,selected,args,u"node[kind].apply");
        };
        hooks.resolveProduct=[require](const Value& result){
            const auto self=require();const auto* object=std::get_if<ObjectRef>(&result.data);
            if(!object)throw Exception("TypeError",u"MmlFactory creator did not return a node object");
            return self->nodes.resolve(*object);
        };
        owner->sourceCreateHooks(std::move(hooks));
    }
    tree::FactoryCreatorContextBridge<MmlNode> bridgeFor(
        ObjectRef node,const std::shared_ptr<MmlFactory>& owner){
        const auto self=shared_from_this();
        const std::weak_ptr<MmlFactory> weakOwner=owner;
        tree::FactoryCreatorContextBridge<MmlNode> bridge;
        bridge.node=node;
        bridge.ownerLease=std::weak_ptr<void>(owner);
        bridge.runtimeLease=lease;
        bridge.lookup=[self](ObjectRef receiver,const Key& key){
            self->requireActive();
            const Value selected=self->context.get(Value(receiver),key);
            if(std::holds_alternative<Undefined>(selected.data))return tree::FactoryCreatorSelection<MmlNode>{};
            tree::FactoryCreatorRef<MmlNode> native;
            bool callable=false;
            if(const auto* object=std::get_if<ObjectRef>(&selected.data)){
                if(self->context.kind(*object)==ObjectKind::function){
                    callable=true;
                    if(const auto* value=std::any_cast<tree::FactoryCreatorRef<MmlNode>>(
                           &self->context.payload(*object)))native=*value;
                }
            }
            return tree::FactoryCreatorSelection<MmlNode>{
                tree::FactoryCreatorSelectionKind::source_value,std::move(native),selected,callable};
        };
        bridge.prepareApply=[self](const tree::FactoryCreatorSelection<MmlNode>& selected){
            self->requireActive();
            return tree::PreparedFactoryCreatorCall<MmlNode>{selected,
                self->context.get(selected.sourceValue,Key(u"apply"))};
        };
        bridge.invokePrepared=[self](const tree::PreparedFactoryCreatorCall<MmlNode>& prepared,
                                     tree::FactoryArgs args){
            self->requireActive();
            return self->applyCreator(prepared.selection.sourceValue,prepared.applyValue,args);
        };
        bridge.invokeDirect=[self](const Value& receiver,const tree::FactoryCreatorSelection<MmlNode>& selected,
                                   tree::FactoryArgs args){
            self->requireActive();
            const auto* function=std::get_if<ObjectRef>(&selected.sourceValue.data);
            if(!function)throw Exception("TypeError",u"this.node[kind] is not a function");
            std::vector<Value> values;values.reserve(args.size());
            for(const auto& argument:args)values.push_back(self->argumentToValue(argument));
            const Value result=self->context.call(*function,receiver,values);
            const auto* object=std::get_if<ObjectRef>(&result.data);
            if(!object)throw Exception("TypeError",u"MmlFactory creator did not return a node object");
            return self->nodes.resolve(*object);
        };
        bridge.expose=[self,weakOwner](const tree::FactoryCreatorRef<MmlNode>& creator){
            self->requireActive();
            auto retained=weakOwner.lock();
            if(!retained)throw Exception("TypeError",u"MmlFactory owner has been released");
            return self->creatorValue(creator,retained);
        };
        bridge.ownKeys=[self](ObjectRef receiver){self->requireActive();return self->context.ownKeys(Value(receiver));};
        bridge.prototype=[self](ObjectRef receiver){self->requireActive();return self->context.prototype(receiver);};
        bridge.setPrototype=[self](ObjectRef receiver,std::optional<ObjectRef> prototype){
            self->requireActive();self->context.setPrototype(receiver,prototype);};
        return bridge;
    }
    ObjectRef creatorRecord(const std::shared_ptr<MmlFactory>& owner){
        requireActive();
        for(const auto& [factory,object]:nodeObjects)if(factory==owner.get())return object;
        const auto self=shared_from_this();
        const auto view=owner->creatorView();
        auto prototype=std::make_shared<std::optional<ObjectRef>>(context.intrinsics().objectPrototype);
        auto nativeView=std::make_shared<tree::FactoryCreatorRecordView>(
            std::static_pointer_cast<void>(view),
            [self,view]{
                self->requireActive();
                std::vector<Key> keys;keys.reserve(view->entries.size());
                for(const auto& entry:view->entries)keys.push_back(entry.first);
                return keys;
            },
            [self,view,owner](const Key& key)->std::optional<Descriptor>{
                self->requireActive();
                for(const auto& entry:view->entries)if(entry.first==key){
                    if(entry.second.descriptor)return entry.second.descriptor;
                    return Descriptor::data(self->creatorValue(entry.second.creator,owner));
                }
                return {};
            },
            [self,view](const Key& key,Descriptor descriptor){
                self->requireActive();
                for(auto& entry:view->entries)if(entry.first==key){entry.second.descriptor=std::move(descriptor);return;}
                view->entries.emplace_back(key,tree::FactoryCreatorSlot<MmlNode>{{},std::move(descriptor)});
            },
            [self,view](const Key& key){
                self->requireActive();
                std::erase_if(view->entries,[&](const auto& entry){return entry.first==key;});
            },
            [self,prototype]{self->requireActive();return *prototype;},
            [self,prototype](std::optional<ObjectRef> value){self->requireActive();*prototype=value;},
            [self,view](const Key& key){
                self->requireActive();
                for(const auto& entry:view->entries)if(entry.first==key)
                    return !entry.second.descriptor||entry.second.descriptor->enumerable;
                return false;
            });
        const auto object=context.nativeRecord(nativeView,context.intrinsics().objectPrototype);
        const auto nativeAccess=std::dynamic_pointer_cast<tree::FactoryCreatorObjectAccess<MmlNode>>(
            owner->creatorAccess());
        if(!nativeAccess)throw Exception("TypeError",u"MmlFactory does not own its canonical creator record");
        nativeAccess->bindContextNode(bridgeFor(object,owner));
        nodeObjects.emplace_back(owner.get(),object);
        return object;
    }
    void setNodeOwner(const std::shared_ptr<MmlFactory>& owner,const Value& value){
        for(const auto& [factory,object]:nodeObjects)
            if(factory==owner.get()&&value==Value(object))return;
        owner->creatorAccess(contextCreatorAccess(value,owner));
    }
    void setNodeMapOwner(const std::shared_ptr<MmlFactory>& owner,Value value){
        if(const auto* object=std::get_if<ObjectRef>(&value.data)){
            try{if(maps.resolve(*object)->identity()==owner->nodeMapAccess()->identity())return;}
            catch(...){}
        }
        owner->nodeMapAccess(sourceMapAccess(std::move(value),owner));
    }
    std::shared_ptr<tree::FactoryClassMapAccess<MmlNode>> currentNodeMapAccess(
        const std::shared_ptr<MmlFactory>& owner,const Value& value){
        auto access=owner->nodeMapAccess();
        if(const auto source=access->sourceValue();source&&*source==value)return access;
        auto replacement=sourceMapAccess(value,owner);
        owner->nodeMapAccess(replacement);
        return replacement;
    }
    std::shared_ptr<tree::FactoryCreatorAccess<MmlNode>> currentCreatorAccess(
        const std::shared_ptr<MmlFactory>& owner,const Value& value){
        auto access=owner->creatorAccess();
        if(const auto source=access->sourceValue();source&&*source==value)return access;
        for(const auto& [factory,object]:nodeObjects)
            if(factory==owner.get()&&value==Value(object))return access;
        auto replacement=contextCreatorAccess(value,owner);
        owner->creatorAccess(replacement);
        return replacement;
    }
    void installPrototypeGetter(ObjectRef prototype){
        if(prototypeWithMmlGetter){
            if(*prototypeWithMmlGetter!=prototype)
                throw Exception("TypeError",u"MmlFactory MML getter was rebound to a different prototype");
            return;
        }
        const auto self=shared_from_this();
        const auto getter=context.function(u"get",[self](Context& cx,Value receiver,
                std::span<const Value>)->Value{
            self->requireActive();
            const auto* object=std::get_if<ObjectRef>(&receiver.data);
            if(!object)throw Exception("TypeError",u"MmlFactory.MML receiver is invalid");
            return cx.get(receiver,Key(u"node"));
        });
        context.define(prototype,Key(u"MML"),Descriptor::access(getter,std::nullopt,false,true));
        // Only commit the once-per-prototype marker after define succeeds. A
        // failed first installation can then be retried by completion-aware bind.
        prototypeWithMmlGetter=prototype;
    }
};

MmlFactoryContextFields::MmlFactoryContextFields(Context& context,MapContextBindings& maps,
    ContextPropertyBindings& properties,NativeIdentityBindings<MmlNode*>& nodes,
    NativeIdentityBindings<Children>& arrays,tree::TreeNodeContextBindings<MmlNode>& treeNodes,
    tree::TreeNodeContextClasses<MmlNode>& treeClasses,
    NativeIdentityBindings<MmlFactory*>& factories,std::weak_ptr<std::atomic_bool> lease,
    ResolveFactory resolveFactory,FindNodeIdentity findNodeIdentity,ArgumentToValue argumentToValue,
    std::shared_ptr<MmlSourceAllocator> sourceAllocator)
    :state_(std::make_shared<State>(context,maps,properties,nodes,arrays,treeNodes,treeClasses,factories,std::move(lease),
                                    std::move(resolveFactory),std::move(findNodeIdentity),
                                    std::move(argumentToValue),sourceAllocator)){}

ObjectRef MmlFactoryContextFields::nodeObject(const std::shared_ptr<MmlFactory>& owner){
    if(!owner)throw Exception("TypeError",u"MmlFactory owner is null");
    return state_->creatorRecord(owner);
}
std::shared_ptr<MmlClassInfo> MmlFactoryContextFields::sourceClassInfo(
        const tree::FactoryClassRef<MmlNode>& cls){return state_->classInfo(cls);}
void MmlFactoryContextFields::detach(const std::shared_ptr<MmlFactory>& factory){
    if(factory)factory->clearSourceClassInfoResolver(state_.get());
}
void MmlFactoryContextFields::retireFactoryOwner(const std::shared_ptr<MmlFactory>& factory){
    if(!factory)return;
    auto& state=*state_;state.requireActive();
    for(auto it=state.nodeObjects.begin();it!=state.nodeObjects.end();){
        if(it->first!=factory.get()){++it;continue;}
        state.context.retireObjectRecord(it->second);
        it=state.nodeObjects.erase(it);
    }
    for(auto it=state.creatorFunctions.begin();it!=state.creatorFunctions.end();){
        if(std::get<1>(*it)!=factory.get()){++it;continue;}
        state.context.retireObjectRecord(std::get<2>(*it));
        it=state.creatorFunctions.erase(it);
    }
}

void MmlFactoryContextFields::bind(ObjectRef identity,const std::shared_ptr<MmlFactory>& owner,
                                   ObjectRef factoryPrototype,ObjectRef nodePrototype,
                                   ObjectRef tokenPrototype,ObjectRef abstractMmlNodePrototype){
    auto& s=*state_;s.requireActive();
    if(!owner||s.factories.resolve(identity)!=owner.get())
        throw Exception("TypeError",u"MmlFactory identity does not match actual owner");
    const std::weak_ptr<State> weakState=state_;
    const std::weak_ptr<MmlFactory> weakOwner=owner;
    owner->setSourceClassInfoResolver(&s,[weakState,weakOwner](const tree::FactoryClassRef<MmlNode>& cls){
        const auto self=weakState.lock();
        if(!self)throw Exception("TypeError",u"MmlFactory class metadata owner has expired");
        self->requireActive();
        const auto factory=weakOwner.lock();
        if(!factory)throw Exception("TypeError",u"MmlFactory owner has expired");
        return self->classInfo(cls);
    });
    s.nodePrototype=nodePrototype;s.tokenPrototype=tokenPrototype;
    s.abstractMmlNodePrototype=abstractMmlNodePrototype;
    const auto retained=owner;
    const auto self=state_;
    s.context.bindData(identity,Key(u"defaultKind"),std::make_shared<NativeDataField>(
        std::static_pointer_cast<void>(retained),[retained]{return retained->defaultKind;},
        [self,retained](Value value){self->requireActive();retained->defaultKindValue(std::move(value));},
        [self]{self->requireActive();}));

    s.context.bindData(identity,Key(u"nodeMap"),std::make_shared<NativeDataField>(
        std::static_pointer_cast<void>(retained),[self,retained](){
            self->requireActive();
            if(const auto raw=retained->nodeMapSourceValue())return *raw;
            auto storage=tree::projectFactoryClassMap<MmlNode>(retained->nodeMapAccess(),
                [self](const tree::FactoryClassMapValue<MmlNode>& mapped){
                    return self->classToValue(mapped);
                });
            return Value(self->maps.bind(storage));
        },[self,retained](Value value){
            self->requireActive();
            self->setNodeMapOwner(retained,std::move(value));
        },[self]{self->requireActive();}));

    s.context.bindData(identity,Key(u"node"),std::make_shared<NativeDataField>(
        std::static_pointer_cast<void>(retained),[self,retained]{
            self->requireActive();
            if(const auto source=retained->creatorAccess()->sourceValue())return *source;
            return Value(self->creatorRecord(retained));
        },[self,retained](Value value){self->requireActive();self->setNodeOwner(retained,value);},
        [self]{self->requireActive();}));
    s.installSourceHooks(owner,identity);
    s.installPrototypeGetter(factoryPrototype);
}

void MmlFactoryContextFields::bindNodePrototype(ObjectRef identity,MmlNode& node){
    state_->bindNodePrototype(identity,node);
}
bool MmlFactoryContextFields::isAbstractMmlNode(Value value){
    auto& state=*state_;state.requireActive();
    const auto descriptor=state.classModule->abstractClass(u"AbstractMmlNode");
    const auto* entry=state.classModule->find(descriptor.get());
    if(!entry)throw Exception("TypeError",u"AbstractMmlNode class module has not been initialized");
    return ordinaryInstanceOf(state.context,value,Value(entry->constructor));
}
std::function<bool(Value)> MmlFactoryContextFields::abstractMmlNodePredicate()const{
    const std::weak_ptr<State::ClassModule> weak=state_->classModule;
    return [weak](Value value){
        const auto module=weak.lock();
        if(!module)throw Exception("TypeError",u"MML class module has expired");
        module->requireActive();
        const auto descriptor=module->abstractClass(u"AbstractMmlNode");
        const auto* entry=module->find(descriptor.get());
        if(!entry)throw Exception("TypeError",u"AbstractMmlNode class module has not been initialized");
        return ordinaryInstanceOf(module->context_,value,Value(entry->constructor));
    };
}
std::function<bool(Value)> MmlFactoryContextFields::textNodePredicate()const{
    const std::weak_ptr<State::ClassModule> weak=state_->classModule;
    return [weak](Value value){
        const auto module=weak.lock();
        if(!module)throw Exception("TypeError",u"MML class module has expired");
        module->requireActive();
        const auto constructor=module->constructorNamed(u"TextNode");
        if(!constructor)throw Exception("TypeError",u"TextNode class module has not been initialized");
        return ordinaryInstanceOf(module->context_,value,Value(*constructor));
    };
}
}
