// Source class metadata and canonical Context identity for StackItemFactory.
module folia.math.input.tex.parser;
import :stack_item_factory_bindings;
import :runtime;
import folia.math.input.tex.node_factory_host;
import folia.math.util.property_host;
import folia.math.core.mml_tree.mml_node;
import std;

namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view text) { return Key(std::u16string(text)); }
Key indexKey(double index){
    const auto ascii=property_host::string(Property(index));
    return Key(ascii);
}
Value callApply(Context& context,const Value& function,Value thisValue,
                std::span<const Value> arguments){
    const Value apply=context.get(function,key(u"apply"));
    const ObjectRef list=context.array(static_cast<std::uint32_t>(arguments.size()));
    for(std::size_t index=0;index<arguments.size();++index)
        context.set(Value(list),indexKey(static_cast<double>(index)),arguments[index]);
    const std::array<Value,2> applyArguments{std::move(thisValue),Value(list)};
    return context.callMember(apply,function,applyArguments);
}
Value sourcePropertyValue(ParserRuntime& runtime, const Property& property,
                          std::unordered_map<const PropertyStorage*, ObjectRef>& identities);
Value stackPropertyContextValue(ParserRuntime&,const StackProperty&);
StackProperty stackPropertyFromContext(ParserRuntime&,const Value&);
Value propertyValue(ParserRuntime&,const Property&);
Property propertyFromValue(ParserRuntime&,const Value&);
namespace { ObjectRef bindStackPropertyRecord(ParserRuntime&,const std::shared_ptr<StackPropertyStorage>&); }
void requireStackRuntime(const std::weak_ptr<std::atomic_bool>& lease){
    const auto active=lease.lock();
    if(!active||!active->load(std::memory_order_acquire))throw Exception("TypeError",u"Parser runtime has been released");
}
class ContextStackItemSourceAccess final:public StackItemSourceAccess {
    ParserRuntime* runtime_;
    ObjectRef receiver_;
    std::weak_ptr<std::atomic_bool> lease_;
    Context& context()const{requireStackRuntime(lease_);return runtime_->context();}
public:
    ContextStackItemSourceAccess(ParserRuntime& runtime,ObjectRef receiver,std::weak_ptr<std::atomic_bool> lease)
      :runtime_(&runtime),receiver_(receiver),lease_(std::move(lease)){}
    Value receiver()const override{requireStackRuntime(lease_);return Value(receiver_);}
    Value getReceiver(std::u16string_view name)override{return context().get(Value(receiver_),key(name));}
    Property environment()override{return propertyFromValue(*runtime_,getReceiver(u"_env"));}
    void setEnvironment(Property environment)override{
        context().set(Value(receiver_),key(u"_env"),propertyValue(*runtime_,environment));
    }
    Value get(const Value& object,const Key& property)override{return context().get(object,property);}
    Value getIndex(const Value& object,double index)override{return context().get(object,indexKey(index));}
    void setReceiver(std::u16string_view name,Value value)override{context().set(Value(receiver_),key(name),std::move(value));}
    void set(const Value& object,const Key& property,Value value)override{context().set(object,property,std::move(value));}
    void setIndex(const Value& object,double index,Value value)override{context().set(object,indexKey(index),std::move(value));}
    Value callMember(const Value& function,Value receiver,std::span<const Value> args)override{return context().callMember(function,std::move(receiver),args);}
    Value callApply(const Value& function,Value receiver,std::span<const Value> args)override{
        const Value apply=context().get(function,key(u"apply"));const ObjectRef list=context().array(static_cast<std::uint32_t>(args.size()));
        for(std::size_t i=0;i<args.size();++i)context().set(Value(list),indexKey(static_cast<double>(i)),args[i]);
        const std::array<Value,2> applyArgs{std::move(receiver),Value(list)};return context().callMember(apply,function,applyArgs);
    }
    Value readIterable(const Value& value,std::uint32_t count)override{return context().readIterable(value,count);}
    ObjectRef makeArray()override{return bindNodeArray(*runtime_,std::make_shared<tree::ChildArray<mml::MmlNode>>());}
    ObjectRef makeObject()override{return context().object();}
    double toNumber(const Value& value)override{return context().toNumber(value);}
    std::string sourceString(const Value& value)override{
        const Value primitive=context().toPrimitive(value,PrimitiveHint::defaultHint);
        if(const auto* text=std::get_if<std::u16string>(&primitive.data))return property_host::debugText(*text);
        if(std::holds_alternative<Undefined>(primitive.data))return "undefined";
        if(std::holds_alternative<Null>(primitive.data))return "null";
        if(const auto* boolean=std::get_if<bool>(&primitive.data))return *boolean?"true":"false";
        if(const auto* number=std::get_if<double>(&primitive.data))return property_host::debugText(property_host::string(Property(*number)));
        if(std::holds_alternative<SymbolRef>(primitive.data))throw Exception("TypeError",u"Cannot convert a Symbol value to a string");
        throw Exception("TypeError",u"Cannot convert object to primitive value");
    }
    bool truthy(const Value& value)override{requireStackRuntime(lease_);return options::truthy(value);}
    mml::Children resolveNodeArray(const Value& value)override{requireStackRuntime(lease_);const auto* identity=std::get_if<ObjectRef>(&value.data);if(!identity)throw Exception("TypeError",u"StackItem._nodes is not a canonical MML child array at native consumption");return runtime_->nodeArrayIdentities.resolve(*identity);}
    mml::NodeValue decodeNode(const Value& value)override{
        requireStackRuntime(lease_);if(std::holds_alternative<Undefined>(value.data))return Missing{};
        if(std::holds_alternative<Null>(value.data))return PropertyNull{};
        const auto* identity=std::get_if<ObjectRef>(&value.data);if(!identity)throw Exception("TypeError",u"StackItem node-array value is not an MML node");return runtime_->mmlNodeIdentities.resolve(*identity);
    }
    Value encodeNode(mml::NodeValue value)override{
        requireStackRuntime(lease_);if(value.isNode()){const auto identity=runtime_->mmlNodeIdentities.find(value.nodeOrNull());if(!identity)throw Exception("TypeError",u"MML node has no canonical Context identity");return Value(*identity);}
        if(value.kind()==tree::ChildKind::Null)return Value(Null{});return Value(Undefined{});
    }
    Value encodeChildren(mml::Children children)override{requireStackRuntime(lease_);return Value(bindNodeArray(*runtime_,std::move(children)));}
    StackProperty decodeProperty(const Value& value)override{requireStackRuntime(lease_);return stackPropertyFromContext(*runtime_,value);}
    Value encodeProperty(StackProperty value)override{requireStackRuntime(lease_);return stackPropertyContextValue(*runtime_,value);}
    std::vector<Key> ownKeys(const Value& value)override{return context().ownKeys(value);}
    std::optional<Descriptor> descriptor(const Value& value,const Key& property)override{return context().descriptor(value,property);}
};
Value arg(std::span<const Value> args, std::size_t index) {
    return index < args.size() ? args[index] : Value(Undefined{});
}
std::vector<std::any> constructorArgs(ParserRuntime& runtime,std::span<const Value> args,
                                      std::size_t start,bool forwardNodes){
    std::vector<std::any> result;result.reserve(args.size()>start?args.size()-start:0);
    for(std::size_t index=start;index<args.size();++index){
        const Value& value=args[index];
        if(!forwardNodes){result.emplace_back(value);continue;}
        if(const auto* object=std::get_if<ObjectRef>(&value.data)){
            const auto& payload=runtime.context().payload(*object);
            if(const auto* node=std::any_cast<mml::MmlNode*>(&payload)){result.emplace_back(mml::NodeValue(*node));continue;}
            if(const auto* child=std::any_cast<mml::NodeValue>(&payload)){result.emplace_back(*child);continue;}
            throw Exception("Unimplemented",u"BaseItem constructor child has no canonical MML node identity");
        }
        if(std::holds_alternative<Null>(value.data)){result.emplace_back(PropertyNull{});continue;}
        if(std::holds_alternative<Undefined>(value.data)){result.emplace_back(Missing{});continue;}
        throw Exception("Unimplemented",u"BaseItem constructor child is outside the native MML node domain");
    }
    return result;
}
mml::Children nativeChildren(tree::FactoryArgs args){
    auto children=std::make_shared<tree::ChildArray<mml::MmlNode>>();
    for(const auto& value:args){
        if(const auto* child=std::any_cast<mml::NodeValue>(&value))children->push_back(*child);
        else if(const auto* node=std::any_cast<mml::MmlNode*>(&value))children->push_back(*node);
        else if(const auto* missing=std::any_cast<Missing>(&value))children->push_back(*missing);
        else if(const auto* null=std::any_cast<PropertyNull>(&value))children->push_back(*null);
        else property_host::typeError("BaseItem inherited constructor argument is not an MML node");
    }
    return children;
}
BaseItem& sourceItem(ParserRuntime& runtime,Value receiver){
    const auto* object=std::get_if<ObjectRef>(&receiver.data);
    if(!object)throw Exception("TypeError",u"Stack item method receiver is not an object");
    const auto reference=runtime.stackItemIdentities.resolve(*object);
    auto* item=reference?dynamic_cast<BaseItem*>(reference.get()):nullptr;
    if(!item)throw Exception("TypeError",u"Stack item method receiver is not a BaseItem");
    return *item;
}
Value propertyValue(ParserRuntime& runtime,const Property& value){
    // ContextPropertyBindings now returns the canonical Context handle over
    // this exact PropertyStorage.  Do not copy/promote object records while a
    // StackItem is publishing one of its properties.
    return runtime.properties.to(value);
}
Property propertyFromValue(ParserRuntime& runtime,const Value& value){
    return runtime.properties.from(value);
}
class StackNodeClassMapStorage final:public MapStorage{
    ParserRuntime& runtime_;
    std::shared_ptr<StackItemFactory> factory_;
    static std::optional<std::string> keyText(const Value& value){
        const auto* text=std::get_if<std::u16string>(&value.data);
        return text?std::optional<std::string>(property_host::debugText(*text)):std::nullopt;
    }
    Value classValue(const tree::FactoryClassRef<BaseItem>& type)const{
        const auto identity=runtime_.itemClassIdentities.find(type);
        if(!identity)throw Exception("TypeError",u"StackItemFactory nodeMap class has no canonical Context identity");
        return Value(*identity);
    }
public:
    StackNodeClassMapStorage(ParserRuntime& runtime,std::shared_ptr<StackItemFactory> factory)
        :runtime_(runtime),factory_(std::move(factory)){
        if(!factory_)throw Exception("TypeError",u"StackItemFactory nodeMap requires its actual factory owner");
    }
    const void* identity()const noexcept override{return this;}
    std::size_t size()const noexcept override{return factory_->getKinds().size();}
    std::size_t slotCount()const noexcept override{return factory_->getKinds().size();}
    std::optional<std::pair<Value,Value>> entryAt(std::size_t slot)const override{
        const auto kinds=factory_->getKinds();
        if(slot>=kinds.size())return std::nullopt;
        const auto type=factory_->getNodeClass(kinds[slot]);
        if(!type)return std::nullopt;
        return std::pair<Value,Value>{Value(property_host::text(kinds[slot])),classValue(type)};
    }
    Value get(const Value& keyValue)const override{
        const auto name=keyText(keyValue);
        if(!name)return Value(Undefined{});
        const auto type=factory_->getNodeClass(*name);
        return type?classValue(type):Value(Undefined{});
    }
    bool has(const Value& keyValue)const override{
        const auto name=keyText(keyValue);
        return name&&static_cast<bool>(factory_->getNodeClass(*name));
    }
    void set(Value keyValue,Value classValueInput)override{
        const auto name=keyText(keyValue);
        const auto* constructor=std::get_if<ObjectRef>(&classValueInput.data);
        if(!name||!constructor)throw Exception("TypeError",u"StackItemFactory nodeMap keys and values must be stack class strings and constructors");
        factory_->setNodeClass(*name,runtime_.itemClassIdentities.resolve(*constructor));
    }
    bool erase(const Value& keyValue)override{
        const auto name=keyText(keyValue);
        if(!name||!factory_->getNodeClass(*name))return false;
        factory_->deleteNodeClass(*name);return true;
    }
    void clear()noexcept override{
        for(const auto& name:factory_->getKinds())factory_->deleteNodeClass(name);
    }
};
void bindFactoryFields(ParserRuntime& runtime,const std::shared_ptr<StackItemFactory>& factory,
                       ObjectRef identity){
    Context& cx=runtime.context();
    const auto bind=[&](std::u16string_view name,std::function<Value()> read,
                        std::function<void(Value)> write){
        cx.bindData(identity,key(name),std::make_shared<NativeDataField>(factory,
            std::move(read),std::move(write)));
    };
    bind(u"defaultKind",[factory](){return factory->defaultKind;},
        [factory](Value value){factory->defaultKindValue(std::move(value));});
    factory->defaultKindKey([&runtime](const Value& value){
        const Value primitive=runtime.context().toPrimitive(value,PrimitiveHint::stringHint);
        if(const auto* symbol=std::get_if<SymbolRef>(&primitive.data))return Key(*symbol);
        if(const auto* text=std::get_if<std::u16string>(&primitive.data))return Key(*text);
        return Key(property_host::string(runtime.properties.from(primitive)));
    });

    const auto factoryIdentity=runtime.stackItemFactoryIdentities.find(factory.get());
    if(!factoryIdentity)throw Exception("TypeError",u"StackItemFactory has no canonical Context identity");
    const auto mapStorage=std::make_shared<StackNodeClassMapStorage>(runtime,factory);
    const ObjectRef nodeMap=runtime.maps.bind(mapStorage);
    bind(u"nodeMap",[nodeMap](){return Value(nodeMap);},[](Value){
        throw Exception("Unimplemented",u"Replacing StackItemFactory.nodeMap requires its native class-map owner");
    });
    const ObjectRef node=cx.object();
    for(const auto& kind:factory->getKinds()){
        const auto type=factory->getNodeClass(kind);
        const auto constructor=runtime.itemClassIdentities.find(type);
        if(!constructor)throw Exception("TypeError",u"StackItemFactory node creator has no canonical class identity");
        const auto function=cx.function(u"",[factoryObject=*factoryIdentity,classObject=*constructor]
            (Context& context,Value,std::span<const Value> args)->Value{
                std::vector<Value> constructorArgs;constructorArgs.reserve(args.size()+1);
                constructorArgs.emplace_back(factoryObject);
                constructorArgs.insert(constructorArgs.end(),args.begin(),args.end());
                return Value(context.construct(classObject,constructorArgs));
            });
        cx.define(function,key(u"length"),Descriptor::data(Value(0),false,false,true));
        const std::u16string propertyName=property_host::text(kind);
        cx.define(node,key(propertyName),Descriptor::data(Value(function),true,true,true));
    }
    bind(u"node",[node](){return Value(node);},[](Value){
        throw Exception("Unimplemented",u"Replacing StackItemFactory.node requires its native creator owner");
    });
    // Derived field is assigned after AbstractFactory finishes initializing
    // defaultKind, nodeMap, and node. It remains the actual ParseOptions edge.
    bind(u"configuration",[&runtime,factory](){
        const auto* config=std::get_if<StackItemFactoryConfiguration*>(&factory->configuration);
        if(config&&*config){
            auto* parse=dynamic_cast<ParseOptions*>(*config);
            if(parse){const auto ref=parse->shared_from_this();const auto object=runtime.parseOptionsIdentities.find(ref);if(object)return Value(*object);}
        }
        return std::holds_alternative<PropertyNull>(factory->configuration)?Value(Null{}):Value(Undefined{});
    },[&runtime,factory](Value value){
        if(std::holds_alternative<Undefined>(value.data)){factory->configuration=Missing{};return;}
        if(std::holds_alternative<Null>(value.data)){factory->configuration=PropertyNull{};return;}
        const auto* object=std::get_if<ObjectRef>(&value.data);
        if(!object)throw Exception("TypeError",u"StackItemFactory.configuration must be ParseOptions");
        const ParseOptionsRef parse=runtime.parseOptionsIdentities.resolve(*object);
        auto* owner=dynamic_cast<StackItemFactoryConfiguration*>(parse.get());
        if(!owner)throw Exception("TypeError",u"StackItemFactory.configuration must be ParseOptions");
        factory->configuration=owner;
    });
}
void bindSourceField(Context& cx,ObjectRef object,std::shared_ptr<BaseItem> owner,
                     std::u16string_view name,std::function<Value()> read,
                     std::function<void(Value)> write,std::function<void()> remove={}){
    const auto fieldKey=key(name);
    // Source constructors assign their fields on every invocation, including
    // when an ordinary call reuses a supplied receiver.  Re-run that write
    // through Context so writable/accessor descriptors keep their observable
    // behavior; do not treat the existence of our owner binding as success.
    if(cx.hasOwn(Value(object),fieldKey)){
        cx.set(Value(object),fieldKey,read());
        return;
    }
    cx.bindData(object,fieldKey,std::make_shared<NativeDataField>(std::move(owner),std::move(read),std::move(write),std::move(remove)));
}
void assignSourceField(Context& cx,ObjectRef object,const std::shared_ptr<BaseItem>& owner,
                       std::u16string_view name,Value value,
                       std::function<Value()> read,std::function<void(Value)> write,
                       std::function<void()> remove={}){
    cx.assignNativeData(object,key(name),
        std::make_shared<NativeDataField>(std::static_pointer_cast<void>(owner),
                                          std::move(read),std::move(write),std::move(remove)),
        std::move(value));
}
void initializeSourceInstanceFields(ParserRuntime& runtime,ObjectRef object,
        const std::shared_ptr<BaseItem>& owner,const std::optional<StackItemSourceSurface>& surface){
    if(!surface)return;
    Context& cx=runtime.context();
    for(const auto& field:surface->instanceFields){
        if(!field.get||!field.set)
            throw Exception("TypeError",u"StackItem source instance-field metadata requires read and write callbacks");
        const auto get=field.get;
        const auto set=field.set;
        const Value initial=field.initialize?field.initialize(*owner):field.initial;
        assignSourceField(cx,object,owner,field.name,initial,
            [owner,get]{return get(*owner);},
            [owner,set](Value value){set(*owner,std::move(value));});
    }
}
void promoteSourceProperty(ParserRuntime& runtime,Property& property,
                           std::unordered_map<const PropertyStorage*,ObjectRef>& identities){
    (void)identities;
    // The reverse binding is identity preserving; this call only ensures the
    // original storage has its Context view before Source code can observe it.
    (void)runtime.properties.to(property);
}
void promoteBaseProperties(ParserRuntime& runtime,BaseItem& item){
    std::unordered_map<const PropertyStorage*,ObjectRef> identities;
    promoteSourceProperty(runtime,item.global,identities);
    if(item.isOpen()){
        Property environment=item.env();
        promoteSourceProperty(runtime,environment,identities);
        item.setEnv(std::move(environment));
    }
}
void initializeBaseSourceFields(ParserRuntime& runtime,ObjectRef object,
                                const std::shared_ptr<BaseItem>& owner,
                                StackFactoryReference factory,bool sourceAccessAlreadyBound){
    Context& cx=runtime.context();
    // The Source field itself owns the current raw Value. The initial Context
    // array is only a view over the exact native child vector; a later ordinary
    // assignment replaces the Value without validating or copying it.
    const auto initialNodes=owner->sourceNodes();
    const Value initialNodesValue(bindNodeArray(runtime,initialNodes));
    const auto runtimeLease=runtime.executionLifetime();
    owner->setSourceAccess(std::make_shared<ContextStackItemSourceAccess>(runtime,object,runtimeLease));
    cx.set(Value(object),key(u"_nodes"),initialNodesValue);
    // MmlStack's _nodes super-field assignment precedes the BaseItem fields.
    // Each following write mirrors the emitted constructor order.
    assignSourceField(cx,object,owner,u"startStr",Value(std::u16string{}),
        [owner]{return Value(owner->startStr);},[owner](Value value){
            if(const auto* text=std::get_if<std::u16string>(&value.data))owner->startStr=*text;
            else if(std::holds_alternative<Undefined>(value.data))owner->startStr=u"undefined";
            else if(std::holds_alternative<Null>(value.data))owner->startStr=u"null";
            else if(const auto* boolean=std::get_if<bool>(&value.data))owner->startStr=*boolean?u"true":u"false";
            else if(const auto* number=std::get_if<double>(&value.data)){const auto ascii=property_host::string(Property(*number));owner->startStr=std::u16string(ascii.begin(),ascii.end());}
            else throw Exception("Unimplemented",u"StackItem.startStr object conversion has no native binding");
        });
    assignSourceField(cx,object,owner,u"startI",Value(0.0),
        [owner]{return Value(owner->startI);},[&cx,owner](Value value){owner->startI=cx.toNumber(value);});
    assignSourceField(cx,object,owner,u"stopI",Value(0.0),
        [owner]{return Value(owner->stopI);},[&cx,owner](Value value){owner->stopI=cx.toNumber(value);});
    const auto valueOfFactory=[&runtime](const StackFactoryReference& current)->Value{
        if(const auto* service=std::get_if<StackItemFactoryService*>(&current);service&&*service)
            if(auto* actual=dynamic_cast<StackItemFactory*>(*service))
                if(const auto identity=runtime.stackItemFactoryIdentities.find(actual))return Value(*identity);
        return std::holds_alternative<PropertyNull>(current)?Value(Null{}):Value(Undefined{});
    };
    const auto factoryValue=[runtimePtr=&runtime,owner]{
        const auto current=owner->sourceFactory();
        if(const auto* service=std::get_if<StackItemFactoryService*>(&current);service&&*service)
            if(auto* actual=dynamic_cast<StackItemFactory*>(*service))
                if(const auto identity=runtimePtr->stackItemFactoryIdentities.find(actual))return Value(*identity);
        return std::holds_alternative<PropertyNull>(current)?Value(Null{}):Value(Undefined{});
    };
    assignSourceField(cx,object,owner,u"factory",valueOfFactory(factory),factoryValue,
        [&runtime,owner](Value value){
            if(std::holds_alternative<Undefined>(value.data))owner->setSourceFactory(Missing{});
            else if(std::holds_alternative<Null>(value.data))owner->setSourceFactory(PropertyNull{});
            else if(const auto* object=std::get_if<ObjectRef>(&value.data))owner->setSourceFactory(runtime.stackItemFactoryIdentities.resolve(*object));
            else throw Exception("TypeError",u"StackItem.factory must be a StackItemFactory");
        });
    const ObjectRef globalRecord=cx.object();
    assignSourceField(cx,object,owner,u"global",Value(globalRecord),
        [&runtime,owner]{return propertyValue(runtime,owner->global);},
        [&runtime,owner](Value value){owner->global=propertyFromValue(runtime,value);});
    StackPropertyList initialProperties;
    owner->setSourceProperties(initialProperties);
    const Value initialPropertiesValue(bindStackPropertyRecord(runtime,initialProperties.storage()));
    cx.set(Value(object),key(u"_properties"),initialPropertiesValue);
    if(owner->isOpen()){
        const ObjectRef envRecord=cx.object();
        assignSourceField(cx,object,owner,u"_env",Value(envRecord),
            [&runtime,owner]{return propertyValue(runtime,owner->envBacking());},
            [&runtime,owner](Value value){owner->setEnvFromSource(propertyFromValue(runtime,value));},
            [owner]{owner->setEnvFromSource(Missing{});});
    }else if(!sourceAccessAlreadyBound&&!cx.hasOwn(Value(object),key(u"_env"))){
        cx.declareNativeData(object,key(u"_env"),
            std::make_shared<NativeDataField>(std::static_pointer_cast<void>(owner),
                [&runtime,owner]{return propertyValue(runtime,owner->envBacking());},
                [&runtime,owner](Value value){owner->setEnvFromSource(propertyFromValue(runtime,value));},
                [owner]{owner->setEnvFromSource(Missing{});}));
    }
}
void bindPropertyField(ParserRuntime& runtime,ObjectRef object,const std::shared_ptr<BaseItem>& owner,
                       std::u16string_view name,std::function<Property()> read,
                       std::function<void(Property)> write){
    bindSourceField(runtime.context(),object,owner,name,[&runtime,read=std::move(read)]{return propertyValue(runtime,read());},
        [&runtime,write=std::move(write)](Value value){write(propertyFromValue(runtime,value));});
}
void bindDerivedInstanceFields(ParserRuntime& runtime,ObjectRef object,const std::shared_ptr<BaseItem>& owner){
    std::unordered_map<const PropertyStorage*,ObjectRef> identities;
    if(auto array=std::dynamic_pointer_cast<ArrayItem>(owner)){
        promoteSourceProperty(runtime,array->atEnd,identities);
        promoteSourceProperty(runtime,array->templateSubs,identities);
        bindPropertyField(runtime,object,owner,u"atEnd",[array]{return array->atEnd;},[array](Property value){array->atEnd=std::move(value);});
        bindPropertyField(runtime,object,owner,u"templateSubs",[array]{return array->templateSubs;},[array](Property value){array->templateSubs=std::move(value);});
    }
    if(auto equation=std::dynamic_pointer_cast<EqnArrayItem>(owner)){
        promoteSourceProperty(runtime,equation->maxrow,identities);
        bindPropertyField(runtime,object,owner,u"maxrow",[equation]{return equation->maxrow;},[equation](Property value){equation->maxrow=std::move(value);});
    }
    if(auto style=std::dynamic_pointer_cast<MstyleItem>(owner)){
        promoteSourceProperty(runtime,style->attrList,identities);
        bindPropertyField(runtime,object,owner,u"attrList",[style]{return style->attrList;},[style](Property value){style->attrList=std::move(value);});
    }
}
Value stackPropertyValue(ParserRuntime& runtime,const StackProperty& value){
    return stackPropertyContextValue(runtime,value);
}
Value checkTypeValue(ParserRuntime& runtime,const CheckType& value){
    if(std::holds_alternative<Missing>(value))return Undefined{};
    if(std::holds_alternative<PropertyNull>(value))return Null{};
    const auto tuple=std::get<std::shared_ptr<CheckTuple>>(value);
    if(!tuple)return Null{};
    Context& cx=runtime.context();const ObjectRef result=cx.array(2);
    if(const auto* list=std::get_if<CheckList>(&tuple->items);list&&*list){
        const ObjectRef items=cx.array(static_cast<std::uint32_t>((*list)->size()));
        for(std::size_t i=0;i<(*list)->size();++i){
            const auto index=std::to_string(i);Value itemValue(Undefined{});
            const auto& entry=(**list)[i];
            if(const auto* prop=std::get_if<Property>(&entry))itemValue=propertyValue(runtime,*prop);
            else if(const auto* node=std::get_if<mml::NodeValue>(&entry)){
                if(node->isNode())throw Exception("Unimplemented",u"CheckType MML node has no Context identity");
                itemValue=node->kind()==tree::ChildKind::Null?Value(Null{}):Value(Undefined{});
            }else if(const auto* ref=std::get_if<StackItemRef>(&entry)){
                const auto object=runtime.stackItemIdentities.find(*ref);
                if(!object)throw Exception("TypeError",u"CheckType item has no Source identity");
                itemValue=*object;
            }
            cx.set(Value(items),key(std::u16string(index.begin(),index.end())),std::move(itemValue));
        }
        cx.set(Value(result),key(u"0"),Value(items));
    }else if(const auto* first=std::get_if<PropertyNull>(&tuple->items))cx.set(Value(result),key(u"0"),Value(Null{}));
    cx.set(Value(result),key(u"1"),propertyValue(runtime,tuple->success));
    return result;
}
ObjectRef sourceMethod(ParserRuntime& runtime,std::u16string_view name){
    Context& cx=runtime.context();
    const std::u16string member(name);
    auto function=cx.function(u"",[&runtime,member](Context& cx,Value receiver,std::span<const Value> args)->Value{
        BaseItem& item=sourceItem(runtime,receiver);
        auto stringArg=[&](std::size_t index)->std::string{
            const Value value=arg(args,index);
            if(const auto* text=std::get_if<std::u16string>(&value.data))return std::string(text->begin(),text->end());
            if(std::holds_alternative<Undefined>(value.data))return "undefined";
            if(std::holds_alternative<Null>(value.data))return "null";
            throw Exception("Unimplemented",u"StackItem method string conversion is outside the native primitive path");
        };
        auto propertyKeyArg=[&](std::size_t index)->Key{
            const Value primitive=cx.toPrimitive(arg(args,index),PrimitiveHint::stringHint);
            if(const auto* symbol=std::get_if<SymbolRef>(&primitive.data))return Key(*symbol);
            return Key(property_host::string(runtime.properties.from(primitive)));
        };
        if(member==u"isKind")return item.isKind(stringArg(0));
        if(member==u"clearEnv"){item.clearEnv();return Undefined{};}
        if(member==u"toString"){const auto text=item.toString();return property_host::text(text);}
        if(member==u"getName")return stackPropertyValue(runtime,item.getName());
        if(member==u"getErrors"){
            const auto name=stringArg(0);
            const auto errorKey=key(std::u16string(name.begin(),name.end()));
            Context& cx=runtime.context();
            const Value constructor=cx.get(receiver,key(u"constructor"));
            const Value errors=cx.get(constructor,key(u"errors"));
            const Value current=cx.get(errors,errorKey);
            if(truthy(current))return current;
            const auto* module=std::get_if<ObjectRef>(&runtime.stackItemModule.data);
            if(!module)throw Exception("Unimplemented",u"StackItem.BaseItem module is unavailable");
            const Value baseConstructor=cx.get(Value(*module),key(u"BaseItem"));
            const Value baseErrors=cx.get(baseConstructor,key(u"errors"));
            return cx.get(baseErrors,errorKey);
        }
        if(member==u"getProperty"){
            const Value properties=cx.get(receiver,key(u"_properties"));
            const Key propertyKey=propertyKeyArg(0);
            return cx.get(properties,propertyKey);
        }
        if(member==u"setProperty"){
            const Value properties=cx.get(receiver,key(u"_properties"));
            const Key propertyKey=propertyKeyArg(0);
            cx.set(properties,propertyKey,arg(args,1));return receiver;
        }
        if(member==u"setProperties"){
            const Value targetValue=cx.get(receiver,key(u"_properties"));
            const ObjectRef target=cx.toObject(targetValue);
            const Value source=arg(args,0);
            if(std::holds_alternative<Undefined>(source.data)||std::holds_alternative<Null>(source.data))return receiver;
            const Value sourceObject(cx.toObject(source));
            for(const auto& sourceKey:cx.ownKeys(sourceObject)){
                const auto descriptor=cx.descriptor(sourceObject,sourceKey);
                if(descriptor&&descriptor->enumerable)cx.set(Value(target),sourceKey,cx.get(sourceObject,sourceKey));
            }
            return receiver;
        }
        if(member==u"checkItem"){
            const Value value=arg(args,0);const auto* object=std::get_if<ObjectRef>(&value.data);
            if(!object)throw Exception("TypeError",u"StackItem.checkItem requires an object");
            const auto target=runtime.stackItemIdentities.resolve(*object);
            return checkTypeValue(runtime,item.checkItem(dynamic_cast<BaseItem*>(target.get())));
        }
        if(member==u"StartEntry"){auto* array=dynamic_cast<ArrayItem*>(&item);if(!array)throw Exception("TypeError",u"StartEntry receiver is not an ArrayItem");array->StartEntry();return Undefined{};}
        if(member==u"EndEntry"){auto* array=dynamic_cast<ArrayItem*>(&item);if(!array)throw Exception("TypeError",u"EndEntry receiver is not an ArrayItem");array->EndEntry();return Undefined{};}
        if(member==u"EndRow"){auto* array=dynamic_cast<ArrayItem*>(&item);if(!array)throw Exception("TypeError",u"EndRow receiver is not an ArrayItem");array->EndRow();return Undefined{};}
        if(member==u"EndTable"){auto* array=dynamic_cast<ArrayItem*>(&item);if(!array)throw Exception("TypeError",u"EndTable receiver is not an ArrayItem");array->EndTable();return Undefined{};}
        if(member==u"checkLines"){auto* array=dynamic_cast<ArrayItem*>(&item);if(!array)throw Exception("TypeError",u"checkLines receiver is not an ArrayItem");array->checkLines();return Undefined{};}
        if(member==u"addRowSpacing"){
            auto* array=dynamic_cast<ArrayItem*>(&item);if(!array)throw Exception("TypeError",u"addRowSpacing receiver is not an ArrayItem");
            const Value value=arg(args,0);if(const auto* text=std::get_if<std::u16string>(&value.data)){array->addRowSpacing(*text);return Undefined{};}
            throw Exception("TypeError",u"addRowSpacing requires a string");
        }
        if(member==u"addLatexItem")throw Exception("Unimplemented",u"MML node Context identity is not bound");
        if(member==u"createMml"||member==u"handleFrame"||member==u"getEntry"||member==u"extendArray"||member==u"addIndentshift")
            throw Exception("Unimplemented",u"This StackItem member requires the canonical MML/array Context identity bridge");
        throw Exception("TypeError",u"Unknown StackItem method");
    });
    return function;
}
std::function<Property()> currentErrorsBinding(ParserRuntime& runtime,ObjectRef constructor){
    return [&runtime,constructor]{
        const Value current=runtime.context().get(Value(constructor),key(u"errors"));
        return runtime.properties.from(current);
    };
}
std::vector<std::u16string_view> ownMethods(std::string_view name){
    static const std::unordered_map<std::string,std::vector<std::u16string_view>> methods{
        {"StartItem",{u"checkItem"}},{"OpenItem",{u"checkItem"}},{"PrimeItem",{u"checkItem"}},
        {"SubsupItem",{u"checkItem"}},{"OverItem",{u"checkItem",u"toString"}},{"LeftItem",{u"checkItem"}},
        {"BreakItem",{u"checkItem"}},{"BeginItem",{u"checkItem"}},{"StyleItem",{u"checkItem"}},
        {"PositionItem",{u"checkItem"}},{"FnItem",{u"checkItem"}},{"NotItem",{u"checkItem"}},
        {"NonscriptItem",{u"checkItem"}},{"DotsItem",{u"checkItem"}},
        {"ArrayItem",{u"checkItem",u"createMml",u"handleFrame",u"StartEntry",u"getEntry",u"EndEntry",u"EndRow",u"EndTable",u"checkLines",u"addRowSpacing"}},
        {"EqnArrayItem",{u"EndEntry",u"EndRow",u"EndTable",u"extendArray",u"addIndentshift"}},
        {"MstyleItem",{u"checkItem"}},{"EquationItem",{u"checkItem"}}
    };
    const auto found=methods.find(std::string(name));return found==methods.end()?std::vector<std::u16string_view>{}:found->second;
}
double sourceMethodLength(std::u16string_view name){
    if(name==u"checkItem"||name==u"handleFrame"||name==u"addRowSpacing")return 1;
    if(name==u"extendArray")return 2;
    return 0;
}
void functionLength(Context& cx, ObjectRef function, double length) {
    cx.define(function, key(u"length"), Descriptor::data(Value(length), false, false, true));
}

Value sourcePropertyValue(ParserRuntime& runtime,const Property& property,
                          std::unordered_map<const PropertyStorage*,ObjectRef>& identities){
    (void)identities;
    // This is a live view over the exact source record/array storage.  In
    // particular, constructor errors and user-supplied StackItem properties
    // must preserve aliases and subsequent mutations.
    return runtime.properties.to(property);
}

namespace {
class StackPropertyRecordView final:public NativeRecordView,public StackPropertyContextDelegate {
    ParserRuntime& runtime_;
    std::shared_ptr<StackPropertyStorage> storage_;
    ObjectRef identity_{};
    StackPropertyStorage::Attributes* attributes(const Key& key)const{
        const auto found=std::ranges::find_if(storage_->attributes,[&](const auto& item){return item.key==key;});
        return found==storage_->attributes.end()?nullptr:const_cast<StackPropertyStorage::Attributes*>(&*found);
    }
    StackPropertyStorage::Attributes& ensureAttributes(const Key& key){
        if(auto* found=attributes(key))return *found;
        storage_->attributes.push_back({key});return storage_->attributes.back();
    }
public:
    StackPropertyRecordView(ParserRuntime& runtime,std::shared_ptr<StackPropertyStorage> storage)
        :runtime_(runtime),storage_(std::move(storage)){}
    void bindIdentity(ObjectRef identity){identity_=identity;}
    StackProperty read(const Key& key)override{
        if(identity_.context==0)throw Exception("TypeError",u"StackProperty record has no Context identity");
        return stackPropertyFromContext(runtime_,runtime_.context().get(Value(identity_),key));
    }
    bool enumerable(const Key& key)const override{
        if(identity_.context==0)throw Exception("TypeError",u"StackProperty record has no Context identity");
        const auto descriptor=runtime_.context().descriptor(Value(identity_),key);
        return descriptor&&descriptor->enumerable;
    }
    void assign(const Key& key,const StackProperty& value)override{
        if(identity_.context==0)throw Exception("TypeError",u"StackProperty record has no Context identity");
        runtime_.context().set(Value(identity_),key,stackPropertyContextValue(runtime_,value));
    }
    std::optional<Descriptor> ownDescriptor(const Key& key)const override{
        const auto found=std::ranges::find_if(storage_->entries,[&](const auto& item){return item.first==key;});
        if(found==storage_->entries.end())return {};
        if(auto* state=attributes(key);state&&state->accessor)
            return Descriptor::access(state->getter,state->setter,state->enumerable,state->configurable);
        const Value value=stackPropertyContextValue(runtime_,found->second);
        if(auto* state=attributes(key))return Descriptor::data(value,state->writable,state->enumerable,state->configurable);
        return Descriptor::data(value);
    }
    bool ownPropertyEnumerable(const Key& key)const override{
        const auto found=std::ranges::find_if(storage_->entries,[&](const auto& item){return item.first==key;});
        if(found==storage_->entries.end())return false;
        if(const auto* state=attributes(key))return state->enumerable;
        return true;
    }
    std::vector<Key> ownKeys()const override{
        return StackPropertyList::fromStorage(storage_).propertyKeys();
    }
    void defineOwn(const Key& key,Descriptor descriptor)override{
        auto& state=ensureAttributes(key);
        state.writable=descriptor.writable;state.enumerable=descriptor.enumerable;
        state.configurable=descriptor.configurable;state.accessor=descriptor.accessor;
        state.getter=descriptor.getter;state.setter=descriptor.setter;
        if(descriptor.accessor){
            // The descriptor itself owns the getter/setter callable identities;
            // retain a missing value slot so Source reads go through Context.
            StackPropertyList::fromStorage(storage_).setOwn(key,StackProperty(Missing{}));
        }else StackPropertyList::fromStorage(storage_).setOwn(key,stackPropertyFromContext(runtime_,descriptor.value));
    }
    void deleteOwn(const Key& key)override{
        StackPropertyList::fromStorage(storage_).erase(key);
        std::erase_if(storage_->attributes,[&](const auto& item){return item.key==key;});
    }
    std::optional<ObjectRef> prototype()const override{
        // StackPropertyStorage's BuiltinProperty::prototype is the native
        // representation of the ordinary Object.prototype default.  Resolve
        // it through this runtime's actual Context intrinsic; sending the
        // sentinel through generic Property conversion is intentionally invalid.
        if(const auto* property=std::get_if<Property>(&storage_->prototype.value);
           property&&std::holds_alternative<BuiltinProperty>(property->value)&&
           std::get<BuiltinProperty>(property->value)==BuiltinProperty::prototype)
            return runtime_.context().intrinsics().objectPrototype;
        const auto value=stackPropertyContextValue(runtime_,storage_->prototype);
        if(std::holds_alternative<Null>(value.data)||std::holds_alternative<Undefined>(value.data))return {};
        if(const auto* object=std::get_if<ObjectRef>(&value.data))return *object;
        property_host::typeError("StackProperty prototype is not an object or null");
    }
    void setPrototype(std::optional<ObjectRef> prototype)override{
        if(!prototype){storage_->prototype=PropertyNull{};return;}
        storage_->prototype=stackPropertyFromContext(runtime_,Value(*prototype));
    }
};

ObjectRef bindStackPropertyRecord(ParserRuntime& runtime,const std::shared_ptr<StackPropertyStorage>& storage){
    if(!storage)throw Exception("TypeError",u"StackProperty record is null");
    if(const auto known=runtime.stackPropertyIdentities.find(storage.get()))return *known;
    auto view=std::make_shared<StackPropertyRecordView>(runtime,storage);
    const ObjectRef record=runtime.context().nativeRecord(view);
    view->bindIdentity(record);
    const ObjectRef bound=runtime.stackPropertyIdentities.bind(storage.get(),record,std::static_pointer_cast<void>(storage));
    storage->contextDelegate=view;
    return bound;
}
}

Value stackPropertyContextValue(ParserRuntime& runtime,const StackProperty& value){
    if(const auto* property=std::get_if<Property>(&value.value))return propertyValue(runtime,*property);
    if(const auto* node=std::get_if<mml::NodeValue>(&value.value)){
        if(!node->isNode())return node->kind()==tree::ChildKind::Null?Value(Null{}):Value(Undefined{});
        if(const auto identity=runtime.mmlNodeIdentities.find(node->nodeOrNull()))return Value(*identity);
        throw Exception("Unimplemented",u"MML node has no canonical Context identity owner");
    }
    const auto& record=std::get<StackProperty::Record>(value.value);
    return Value(bindStackPropertyRecord(runtime,record));
}

StackProperty stackPropertyFromContext(ParserRuntime& runtime,const Value& value){
    if(std::holds_alternative<Undefined>(value.data))return StackProperty(Missing{});
    if(std::holds_alternative<Null>(value.data))return StackProperty(PropertyNull{});
    if(const auto* item=std::get_if<bool>(&value.data))return StackProperty(*item);
    if(const auto* item=std::get_if<double>(&value.data))return StackProperty(*item);
    if(const auto* item=std::get_if<std::u16string>(&value.data))return StackProperty(property_host::debugText(*item));
    if(const auto* object=std::get_if<ObjectRef>(&value.data)){
        const auto& payload=runtime.context().payload(*object);
        if(const auto* bound=std::any_cast<NativeBindingPayload<StackPropertyStorage*>>(&payload)){
            auto owner=std::shared_ptr<StackPropertyStorage>(bound->lifetime,bound->identity);
            return StackProperty(StackPropertyList::fromStorage(std::move(owner)));
        }
        if(const auto* bound=std::any_cast<NativeBindingPayload<mml::MmlNode*>>(&payload))
            return StackProperty(mml::NodeValue(bound->identity));
        return StackProperty(runtime.properties.from(value));
    }
    throw Exception("Unimplemented",u"StackProperty value requires a canonical Context identity");
}

tree::FactoryClassMap<BaseItem> classesFrom(ParserRuntime& runtime, const Value& value) {
    tree::FactoryClassMap<BaseItem> classes;
    if (std::holds_alternative<Null>(value.data) || std::holds_alternative<Undefined>(value.data))
        return classes;
    if (!std::holds_alternative<ObjectRef>(value.data))
        throw Exception("TypeError", u"StackItemFactory nodes must be an object or null");
    Context& cx = runtime.context();
    for (const auto& name : cx.enumerableStringKeys(value)) {
        const Value constructor = cx.get(value, key(name));
        const auto* function = std::get_if<ObjectRef>(&constructor.data);
        if (!function) throw Exception("TypeError", u"Stack item class is not a constructor");
        classes.emplace_back(property_host::debugText(name), runtime.itemClassIdentities.resolve(*function));
    }
    return classes;
}
}

Property StackItemFactoryBindings::bindErrors(ParserRuntime& runtime,ObjectRef constructor,Property source){
    auto& cx=runtime.context();
    // Source allocates a fresh errors object for each actual class constructor.
    // This shallow Object.assign target becomes the one mutable error table for
    // both the constructor and its native class metadata in this Context.
    const Property contextRecord=runtime.properties.copyOwnEnumerableProperties(source);
    const Value value=runtime.properties.to(contextRecord);
    const auto* object=std::get_if<ObjectRef>(&value.data);
    if(!object)throw Exception("TypeError",u"Source errors must be an object");
    if(const auto parent=cx.prototype(constructor)){
        const Value parentErrors=cx.get(Value(*parent),key(u"errors"));
        if(const auto* parentObject=std::get_if<ObjectRef>(&parentErrors.data))cx.setPrototype(*object,*parentObject);
    }
    cx.define(constructor,key(u"errors"),Descriptor::data(value,true,true,true));
    return runtime.properties.from(Value(*object));
}

void StackItemFactoryBindings::initializeModule(ParserRuntime& runtime) {
    Context& cx = runtime.context();
    if (!std::holds_alternative<Undefined>(runtime.stackItemFactoryClass.data))
        throw Exception("TypeError", u"StackItemFactory module already initialized");

    // StackItem.ts emits MmlStack as a real runtime constructor even though
    // TypeScript marks it abstract.  Its native instance storage is represented
    // by MmlStack in C++; direct Context construction remains an explicit
    // boundary until that abstract native owner can be allocated faithfully.
    const ObjectRef mmlStackConstructor = cx.nativeConstructor(u"MmlStack",
        [](Context&, std::span<const Value>) -> ObjectRef {
            throw Exception("Unimplemented", u"Direct MmlStack construction has no concrete native stack owner");
        });
    functionLength(cx, mmlStackConstructor, 1);
    const ObjectRef mmlStackPrototype = std::get<ObjectRef>(
        cx.get(Value(mmlStackConstructor), key(u"prototype")).data);
    const ObjectRef stackNodes=cx.function(u"get nodes",[](Context& context,Value receiver,std::span<const Value>){
        return context.get(receiver,key(u"_nodes"));
    });
    cx.define(mmlStackPrototype,key(u"nodes"),Descriptor::access(stackNodes,std::nullopt,false,true));
    const auto installStackMethod=[&](std::u16string_view name,double length,auto invoke){
        const auto method=cx.function(u"",[runtimePtr=&runtime,invoke](Context&,Value receiver,std::span<const Value> args){return invoke(sourceItem(*runtimePtr,receiver),args);});
        functionLength(cx,method,length);cx.define(mmlStackPrototype,key(name),Descriptor::data(Value(method),true,true,true));
    };
    installStackMethod(u"Push",0,[](BaseItem& item,std::span<const Value> args){item.sourcePushValues(args);return Value(Undefined{});});
    installStackMethod(u"Pop",0,[](BaseItem& item,std::span<const Value>){return item.sourcePopValue();});
    installStackMethod(u"Peek",1,[](BaseItem& item,std::span<const Value> args){return args.empty()?item.sourcePeekValue():item.sourcePeekValue(arg(args,0));});
    installStackMethod(u"toMml",0,[](BaseItem& item,std::span<const Value> args){return item.sourceToMmlValue(arg(args,0),arg(args,1));});
    installStackMethod(u"create",1,[](BaseItem& item,std::span<const Value> args){return item.sourceCreateValues(args);});
    const ObjectRef firstGetter=cx.function(u"get First",[runtimePtr=&runtime](Context&,Value receiver,std::span<const Value>){
        return sourceItem(*runtimePtr,receiver).sourceFirstValue();
    });
    const ObjectRef firstSetter=cx.function(u"set First",[runtimePtr=&runtime](Context&,Value receiver,std::span<const Value> args){
        sourceItem(*runtimePtr,receiver).sourceSetFirstValue(arg(args,0));return Value(Undefined{});
    });
    cx.define(mmlStackPrototype,key(u"First"),Descriptor::access(firstGetter,firstSetter,false,true));
    const ObjectRef lastGetter=cx.function(u"get Last",[runtimePtr=&runtime](Context&,Value receiver,std::span<const Value>){
        return sourceItem(*runtimePtr,receiver).sourceLastValue();
    });
    const ObjectRef lastSetter=cx.function(u"set Last",[runtimePtr=&runtime](Context&,Value receiver,std::span<const Value> args){
        sourceItem(*runtimePtr,receiver).sourceSetLastValue(arg(args,0));return Value(Undefined{});
    });
    cx.define(mmlStackPrototype,key(u"Last"),Descriptor::access(lastGetter,lastSetter,false,true));
    const ObjectRef stackSize=cx.function(u"",[runtimePtr=&runtime](Context&,Value receiver,std::span<const Value>){
        return sourceItem(*runtimePtr,receiver).sourceSizeValue();
    });
    functionLength(cx,stackSize,0);cx.define(mmlStackPrototype,key(u"Size"),Descriptor::data(Value(stackSize),true,true,true));
    const ObjectRef stackClear=cx.function(u"",[runtimePtr=&runtime](Context&,Value receiver,std::span<const Value>){sourceItem(*runtimePtr,receiver).Clear();return Value(Undefined{});});
    functionLength(cx,stackClear,0);cx.define(mmlStackPrototype,key(u"Clear"),Descriptor::data(Value(stackClear),true,true,true));

    // BaseItem is the actual StackItem.ts class.  Register its genuine factory
    // descriptor and Context constructor before any package subclass is
    // created, so every later class inherits this exact constructor/prototype.
    const auto baseType = registerSubclass(runtime, u"BaseItem", "base", 1,
        typeid(BaseItem), [] { return std::make_unique<BaseItem>(); },
        [](BaseItem&, tree::FactoryArgs) {},
        [](const BaseItem* item) { return dynamic_cast<const BaseItem*>(item) != nullptr; },
        [] { return BaseItem::baseErrors(); }, mmlStackConstructor, true);
    const auto baseConstructor = runtime.itemClassIdentities.find(baseType);
    if (!baseConstructor)
        throw Exception("Unimplemented", u"BaseItem constructor identity was not registered");
    // `BaseItem::errors` is the module-owned native source table.  Each
    // Context gets its own live ordinary-object view on the actual constructor;
    // never replace the canonical table with a view belonging to this Context.
    (void)bindErrors(runtime,*baseConstructor,BaseItem::baseErrors());
    if (auto* info = std::any_cast<std::shared_ptr<StackItemClassInfo>>(&baseType->staticData); info && *info) {
        (*info)->errorsBinding = currentErrorsBinding(runtime,*baseConstructor);
        (*info)->baseErrorsBinding = currentErrorsBinding(runtime,*baseConstructor);
    }
    const ObjectRef stackModule = cx.object();
    cx.define(stackModule, key(u"MmlStack"), Descriptor::data(Value(mmlStackConstructor)));
    cx.define(stackModule, key(u"BaseItem"), Descriptor::data(Value(*baseConstructor)));
    runtime.stackItemModule = Value(stackModule);

    const auto dummyType = StackItemFactory::DefaultStackItems.front().second;
    const ObjectRef dummyConstructor = cx.nativeConstructor(u"DummyItem",
        [&runtime, dummyType](Context&, std::span<const Value> arguments) -> ObjectRef {
            const Value factoryValue = arg(arguments, 0);
            const auto* factoryObject = std::get_if<ObjectRef>(&factoryValue.data);
            if (!factoryObject) throw Exception("TypeError", u"DummyItem requires a StackItemFactory");
            StackItemFactory* const factory = runtime.stackItemFactoryIdentities.resolve(*factoryObject);
            std::vector<std::any> values;
            values.reserve(arguments.size() > 0 ? arguments.size() - 1 : 0);
            for (std::size_t i = 1; i < arguments.size(); ++i) values.emplace_back(arguments[i]);
            BaseItem* const product = dummyType->construct(factory->itemFactory(), values);
            const StackItemRef owner = factory->reference(product);
            const auto constructor = runtime.itemClassIdentities.find(dummyType);
            if (!constructor) throw Exception("Unimplemented", u"DummyItem constructor identity is missing");
            return runtime.stackItemIdentities.instance(owner, *constructor);
        });
    functionLength(cx, dummyConstructor, 1);
    runtime.itemClassIdentities.bind(dummyType, dummyConstructor);

    const ObjectRef constructor = cx.nativeConstructor(u"StackItemFactory",
        [&runtime](Context&, std::span<const Value> arguments) -> ObjectRef {
            auto factory = std::make_shared<StackItemFactory>(classesFrom(runtime, arg(arguments, 0)));
            const auto* classObject = std::get_if<ObjectRef>(&runtime.stackItemFactoryClass.data);
            if (!classObject) throw Exception("Unimplemented", u"StackItemFactory constructor is not initialized");
            const ObjectRef object = runtime.stackItemFactoryIdentities.instance(
                factory.get(), *classObject, std::static_pointer_cast<void>(factory));
            bindFactoryFields(runtime,factory,object);
            return object;
        });
    functionLength(cx, constructor, 0);
    runtime.stackItemFactoryClass = Value(constructor);

    const ObjectRef defaults = cx.object();
    cx.define(defaults, key(u"base"), Descriptor::data(Value(dummyConstructor), true, true, true));
    cx.define(constructor, key(u"DefaultStackItems"), Descriptor::data(Value(defaults), true, true, true));
}

ObjectRef StackItemFactoryBindings::bindActualFactory(
    ParserRuntime& runtime, const std::shared_ptr<StackItemFactory>& factory,
    ObjectRef configuration) {
    if (!factory) throw Exception("TypeError", u"Cannot bind a null StackItemFactory");
    const auto* constructor = std::get_if<ObjectRef>(&runtime.stackItemFactoryClass.data);
    if (!constructor) throw Exception("Unimplemented", u"StackItemFactory metadata is not initialized");
    const auto* config=std::get_if<StackItemFactoryConfiguration*>(&factory->configuration);
    auto* parse=config&&*config?dynamic_cast<ParseOptions*>(*config):nullptr;
    const auto canonical=parse?runtime.parseOptionsIdentities.find(parse->shared_from_this()):std::nullopt;
    if(!canonical||*canonical!=configuration)
        throw Exception("TypeError",u"StackItemFactory must retain the actual ParseOptions Context identity");
    if (const auto existing = runtime.stackItemFactoryIdentities.find(factory.get())) return *existing;
    const ObjectRef identity=runtime.stackItemFactoryIdentities.instance(
        factory.get(), *constructor, std::static_pointer_cast<void>(factory));
    bindFactoryFields(runtime,factory,identity);
    return identity;
}

tree::FactoryClassRef<BaseItem> StackItemFactoryBindings::registerSubclass(
    ParserRuntime& runtime,std::u16string className,std::string kind,
    std::size_t constructorLength,std::type_index nativeType,
    std::function<std::unique_ptr<BaseItem>()> allocate,
    std::function<void(BaseItem&,tree::FactoryArgs)> initializeSource,
    std::function<bool(const BaseItem*)> isInstance,
    std::function<Property()> errorsBinding,
    std::optional<ObjectRef> parentConstructor,bool forwardsBaseNodes,
    std::optional<StackItemSourceSurface> sourceSurface,bool ownsErrors){
    using namespace options;
    if(!allocate||!initializeSource||!isInstance||!errorsBinding)
        throw Exception("TypeError",u"StackItem subclass registration requires complete Source callbacks");
    Context& cx=runtime.context();
    auto type=std::make_shared<tree::FactoryClass<BaseItem>>();
    type->nativeType=nativeType;
    auto info=std::make_shared<StackItemClassInfo>();
    const std::weak_ptr<tree::FactoryClass<BaseItem>> weak(type);
    type->isInstance=std::move(isInstance);
    const ObjectRef constructor=cx.nativeConstructor(className,
        [&runtime,weak,allocate,initializeSource,forwardsBaseNodes,sourceSurface](Context&,std::span<const Value> arguments)->ObjectRef{
            const auto actual=weak.lock();if(!actual)throw Exception("TypeError",u"Stack item class metadata is no longer live");
            StackFactoryReference factory=Missing{};StackItemFactoryService* service=nullptr;
            if(!arguments.empty()){
                const Value supplied=arguments[0];
                if(const auto* object=std::get_if<ObjectRef>(&supplied.data)){
                    service=runtime.stackItemFactoryIdentities.resolve(*object);factory=service;
                }else if(std::holds_alternative<Null>(supplied.data))factory=PropertyNull{};
                else if(!std::holds_alternative<Undefined>(supplied.data))
                    throw Exception("Unimplemented",u"Stack item Source constructor factory has no native identity");
            }
            auto rest=constructorArgs(runtime,arguments,1,forwardsBaseNodes);
            const auto classObject=runtime.itemClassIdentities.find(actual);
            if(!classObject)throw Exception("Unimplemented",u"Stack item constructor identity is not bound");
            if(service){
                BaseItem* const raw=actual->construct(service->itemFactory(),rest);
                const StackItemRef owner=service->reference(raw);
                const auto identity=runtime.stackItemIdentities.find(owner);
                if(!identity)throw Exception("Unimplemented",u"Constructed stack item has no Context identity");
                return *identity;
            }
            auto allocated=allocate();if(!allocated)throw Exception("TypeError",u"Stack item allocator returned null");
            auto owner=std::shared_ptr<BaseItem>(std::move(allocated));
            const auto identity=runtime.stackItemIdentities.instance(std::static_pointer_cast<StackItemContext>(owner),*classObject);
            owner->prepareSourceBaseItem(actual,forwardsBaseNodes?nativeChildren(rest):std::make_shared<tree::ChildArray<mml::MmlNode>>());
            initializeBaseSourceFields(runtime,identity,owner,factory,false);
            initializeSourceInstanceFields(runtime,identity,owner,sourceSurface);
            initializeSource(*owner,rest);
            promoteBaseProperties(runtime,*owner);
            bindDerivedInstanceFields(runtime,identity,owner);
            return identity;
        },
        [&runtime,weak,allocate,initializeSource,isInstance=type->isInstance,forwardsBaseNodes,sourceSurface]
        (Context&,Value receiver,std::span<const Value> arguments)->Value{
            const auto actual=weak.lock();if(!actual)throw Exception("TypeError",u"Stack item class metadata is no longer live");
            const auto* target=std::get_if<ObjectRef>(&receiver.data);
            if(!target)throw Exception("TypeError",u"Stack item constructor receiver is not an object");
            StackFactoryReference factory=Missing{};
            if(!arguments.empty()){
                const Value supplied=arguments[0];
                if(const auto* object=std::get_if<ObjectRef>(&supplied.data))factory=runtime.stackItemFactoryIdentities.resolve(*object);
                else if(std::holds_alternative<Null>(supplied.data))factory=PropertyNull{};
                else if(!std::holds_alternative<Undefined>(supplied.data))
                    throw Exception("Unimplemented",u"Stack item Source constructor factory has no native identity");
            }
            auto rest=constructorArgs(runtime,arguments,1,forwardsBaseNodes);
            if(runtime.context().payload(*target).has_value()){
                StackItemRef reference=runtime.stackItemIdentities.resolve(*target);
                BaseItem* owner=reference?dynamic_cast<BaseItem*>(reference.get()):nullptr;
                if(!owner||!isInstance(owner))throw Exception("Unimplemented",u"Stack item constructor cannot replace a different native receiver class");
                auto baseOwner=std::dynamic_pointer_cast<BaseItem>(reference);
                const bool sourceAccessAlreadyBound=baseOwner->hasSourceNodesOwner();
                baseOwner->prepareSourceBaseItem(actual,forwardsBaseNodes?nativeChildren(rest):std::make_shared<tree::ChildArray<mml::MmlNode>>());
                initializeBaseSourceFields(runtime,*target,baseOwner,factory,sourceAccessAlreadyBound);
                initializeSourceInstanceFields(runtime,*target,baseOwner,sourceSurface);
                initializeSource(*owner,rest);promoteBaseProperties(runtime,*owner);bindDerivedInstanceFields(runtime,*target,std::dynamic_pointer_cast<BaseItem>(reference));return receiver;
            }
            auto allocated=allocate();if(!allocated)throw Exception("TypeError",u"Stack item allocator returned null");
            auto owner=std::shared_ptr<BaseItem>(std::move(allocated));
            const StackItemRef identity=std::static_pointer_cast<StackItemContext>(owner);
            runtime.stackItemIdentities.bind(identity,*target,owner);
            owner->prepareSourceBaseItem(actual,forwardsBaseNodes?nativeChildren(rest):std::make_shared<tree::ChildArray<mml::MmlNode>>());
            initializeBaseSourceFields(runtime,*target,owner,factory,false);
            initializeSourceInstanceFields(runtime,*target,owner,sourceSurface);
            initializeSource(*owner,rest);promoteBaseProperties(runtime,*owner);bindDerivedInstanceFields(runtime,*target,owner);return receiver;
        });
    const ObjectRef prototype=std::get<ObjectRef>(cx.get(Value(constructor),key(u"prototype")).data);
    // Source class inheritance is established before its static fields are
    // initialized.  In particular, an own errors object can inherit from the
    // actual parent constructor.errors object in this Context.
    if(parentConstructor){
        cx.setPrototype(constructor,*parentConstructor);
        const ObjectRef parentPrototype=std::get<ObjectRef>(cx.get(Value(*parentConstructor),key(u"prototype")).data);
        cx.setPrototype(prototype,parentPrototype);
    }
    cx.define(constructor,key(u"length"),Descriptor::data(Value(static_cast<double>(constructorLength)),false,false,true));
    // TypeScript's emitted prototype assignment creates an enumerable own
    // constructor property (the generic native constructor helper defaults
    // this descriptor to non-enumerable).
    cx.define(prototype,key(u"constructor"),Descriptor::data(Value(constructor),true,true,true));
    const bool ownKind=!sourceSurface||sourceSurface->ownKind;
    if(ownKind){
        const ObjectRef kindGetter=cx.function(u"get",[kind](Context&,Value,std::span<const Value>){return Value(std::u16string(kind.begin(),kind.end()));});
        cx.define(prototype,key(u"kind"),Descriptor::access(kindGetter,std::nullopt,false,true));
    }
    const std::string classLabel(className.begin(),className.end());
    std::vector<StackItemMethodSurface> declaredMethods;
    if(sourceSurface){
        declaredMethods=sourceSurface->members;
    }else{
        for(const auto member:ownMethods(classLabel))
            declaredMethods.push_back({std::u16string(member),sourceMethodLength(member)});
    }
    for(const auto& metadata:declaredMethods){
        const auto& member=metadata.name;
        if(metadata.getter){
            if(!metadata.get)throw Exception("TypeError",u"StackItem source getter metadata requires a getter callback");
            const auto getter=metadata.get;
            const auto fn=cx.function(u"get "+member,[&runtime,getter](Context&,Value receiver,std::span<const Value>){
                return getter(sourceItem(runtime,std::move(receiver)));
            });
            cx.define(prototype,key(member),Descriptor::access(fn,std::nullopt,false,true));
        }else{
            const auto fn=sourceMethod(runtime,member);functionLength(cx,fn,metadata.length);
            cx.define(prototype,key(member),Descriptor::data(Value(fn),true,true,true));
        }
    }
    auto installGetter=[&](std::u16string_view member,std::function<Value(BaseItem&)> getter){
        const ObjectRef fn=cx.function(u"get",[&runtime,getter=std::move(getter)](Context&,Value receiver,std::span<const Value>){
            return getter(sourceItem(runtime,std::move(receiver)));
        });
        cx.define(prototype,key(member),Descriptor::access(fn,std::nullopt,false,true));
        return fn;
    };
    if(classLabel=="BaseItem"){
        const ObjectRef envGetter=installGetter(u"env",[&runtime](BaseItem& item){return propertyValue(runtime,item.env());});
        const ObjectRef setEnv=cx.function(u"set",[&runtime](Context&,Value receiver,std::span<const Value> args){
            BaseItem& item=sourceItem(runtime,receiver);const Value raw=arg(args,0);Property value;
            if(std::holds_alternative<Undefined>(raw.data))value=Missing{};
            else if(std::holds_alternative<Null>(raw.data))value=PropertyNull{};
            else if(const auto* p=std::get_if<bool>(&raw.data))value=*p;
            else if(const auto* p=std::get_if<double>(&raw.data))value=*p;
            else if(const auto* p=std::get_if<std::u16string>(&raw.data))value=property_host::debugText(*p);
            else value=runtime.properties.from(raw);
            item.setEnv(std::move(value));return Value(Undefined{});
        });
        installGetter(u"copyEnv",[](BaseItem& item){return Value(item.copyEnv());});
        installGetter(u"isOpen",[](BaseItem& item){return Value(item.isOpen());});
        installGetter(u"isClose",[](BaseItem& item){return Value(item.isClose());});
        installGetter(u"isFinal",[](BaseItem& item){return Value(item.isFinal());});
        cx.define(prototype,key(u"env"),Descriptor::access(envGetter,setEnv,false,true));
        const std::array<std::u16string_view,9> baseMethods{u"isKind",u"clearEnv",u"getName",u"getProperty",u"setProperty",u"setProperties",u"checkItem",u"getErrors",u"toString"};
        for(const auto member:baseMethods){const auto fn=sourceMethod(runtime,member);functionLength(cx,fn,member==u"setProperty"||member==u"setProperties"?2:member==u"checkItem"||member==u"isKind"||member==u"getErrors"||member==u"getProperty"?1:0);cx.define(prototype,key(member),Descriptor::data(Value(fn),true,true,true));}
        const auto addLatex=sourceMethod(runtime,u"addLatexItem");functionLength(cx,addLatex,1);
        cx.define(prototype,key(u"addLatexItem"),Descriptor::data(Value(addLatex),true,true,true));
    }
    const std::array<std::string_view,6> openAccessors{"StartItem","OpenItem","LeftItem","BeginItem","ArrayItem","EquationItem"};
    const std::array<std::string_view,7> closeAccessors{"StopItem","CloseItem","OverItem","Middle","RightItem","EndItem","CellItem"};
    if(std::ranges::find(openAccessors,classLabel)!=openAccessors.end())installGetter(u"isOpen",[](BaseItem& item){return Value(item.isOpen());});
    if(std::ranges::find(closeAccessors,classLabel)!=closeAccessors.end())installGetter(u"isClose",[](BaseItem& item){return Value(item.isClose());});
    if(classLabel=="MmlItem")installGetter(u"isFinal",[](BaseItem& item){return Value(item.isFinal());});
    if(classLabel=="ArrayItem")installGetter(u"copyEnv",[](BaseItem& item){return Value(item.copyEnv());});
    if(ownsErrors){
        (void)bindErrors(runtime,constructor,errorsBinding());
        // A class descriptor owns this Context's error view.  The Source
        // static table passed to bindErrors remains the canonical native
        // module value and must not be overwritten with a Context-bound alias.
    }
    // Source getErrors resolves this.constructor.errors on every call.  Keep
    // that Get live so reassignment and inherited constructor properties use
    // the actual Context identity instead of a copied/native snapshot.
    info->errorsBinding=currentErrorsBinding(runtime,constructor);
    if(const auto* module=std::get_if<ObjectRef>(&runtime.stackItemModule.data)){
        const Value base=cx.get(Value(*module),key(u"BaseItem"));
        if(const auto* baseConstructor=std::get_if<ObjectRef>(&base.data))
            info->baseErrorsBinding=currentErrorsBinding(runtime,*baseConstructor);
    }
    type->staticData=info;
    type->construct=[weak,constructor,&runtime,allocate,initializeSource,forwardsBaseNodes,sourceSurface]
        (tree::FactoryServices<BaseItem>& factory,tree::FactoryArgs args)->BaseItem*{
            const auto actual=weak.lock();if(!actual)property_host::typeError("Stack item class metadata is no longer live");
            auto owner=allocate();if(!owner)property_host::typeError("Stack item allocator returned null");
            auto* service=dynamic_cast<StackItemFactoryService*>(&factory);
            if(!service)property_host::typeError("Native stack factory service is not bound");
            BaseItem* const raw=factory.adopt(std::move(owner));
            const StackItemRef reference=service->reference(raw);
            const ObjectRef identity=runtime.stackItemIdentities.instance(reference,constructor);
            raw->prepareSourceBaseItem(actual,forwardsBaseNodes?nativeChildren(args):std::make_shared<tree::ChildArray<mml::MmlNode>>());
            initializeBaseSourceFields(runtime,identity,std::dynamic_pointer_cast<BaseItem>(reference),service,false);
            initializeSourceInstanceFields(runtime,identity,std::dynamic_pointer_cast<BaseItem>(reference),sourceSurface);
            initializeSource(*raw,args);
            promoteBaseProperties(runtime,*raw);
            bindDerivedInstanceFields(runtime,identity,std::dynamic_pointer_cast<BaseItem>(reference));
            return raw;
        };
    runtime.itemClassIdentities.bind(type,constructor);
    return type;
}
}
