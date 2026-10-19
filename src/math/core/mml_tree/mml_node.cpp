/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native implementation of the normal MML node behavior from
 * .reference/MathJax-src/ts/core/MmlTree/MmlNode.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 * TS SHA256: cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656
 * Pinned CJS SHA256: 86c5eb572dcf2652c3c52a60b66c720d231e17f600cc9aeef3ace4aeafc81d45
 *************************************************************/
module folia.math.core.mml_tree.mml_node;
import folia.math.core.mml_tree.mml_node_host;
import folia.math.core.mml_tree.mml_node_initialization;
import folia.math.util.property_host;
import std;

namespace folia::math::mml {
namespace {
const PropertyArray TEXSPACELENGTH = {
    "", "thinmathspace", "mediummathspace", "thickmathspace"
};
const PropertyArray TEXSPACE = {
    PropertyArray{ 0,-1, 2, 3, 0, 0, 0, 1},
    PropertyArray{-1,-1, 0, 3, 0, 0, 0, 1},
    PropertyArray{ 2, 2, 0, 0, 2, 0, 0, 2},
    PropertyArray{ 3, 3, 0, 0, 3, 0, 0, 3},
    PropertyArray{ 0, 0, 0, 0, 0, 0, 0, 0},
    PropertyArray{ 0,-1, 2, 3, 0, 0, 0, 1},
    PropertyArray{ 1, 1, 0, 1, 1, 1, 1, 1},
    PropertyArray{ 1,-1, 2, 3, 1, 0, 1, 1}
};

Property class_info_defaults(const tree::FactoryClassRef<MmlNode>& nodeClass) {
    if (!nodeClass) host::typeError("Missing registered MML node class");
    const auto* info = std::any_cast<std::shared_ptr<MmlClassInfo>>(&nodeClass->staticData);
    if (!info || !*info) host::typeError("MML class has no native static metadata");
    const auto defaults=(*info)->getDefaults();
    return defaults?Property(*defaults):Property(Missing{});
}

MmlNode* child_at(const Children& children, std::size_t index, std::string_view operation) {
    if (!children) host::typeError("Cannot read properties of null (reading '0')");
    const NodeValue child = children->get(index);
    return child.requireNode(operation);
}

bool property_set_contains(const Property& value) {
    return std::any_of(MATHVARIANTS.begin(), MATHVARIANTS.end(),
        [&](const Property& candidate) { return host::sameValueZero(candidate, value); });
}

bool forbidden_inheritance(const Property& origin, MmlNode& node,
                           std::string_view name) {
    const auto from = host::get(AbstractMmlNode::noInherit(),host::debugText(host::string(origin)));
    if (host::nullish(from)) return false;
    const auto to = host::get(from,node.kind());
    return !host::nullish(to) && host::truthy(host::get(to,name));
}

bool stopped_inheritance(std::string_view destination, std::string_view name) {
    const auto to = host::get(AbstractMmlNode::stopInherit(),destination);
    return !host::nullish(to) && host::truthy(host::get(to,name));
}

using NativeDynamic=MmlNativeDynamic<MmlNode>;
using NativeReceiver=MmlNodeReceiver<MmlNode,NativeDynamic>;
using NativeCallable=NativeDynamic::Callable;
using NativeSourceValue=MmlSourceValue<MmlNode>;

NativeDynamic native_property(Property value){return NativeDynamic(std::move(value));}
NativeDynamic native_child(NodeValue value){return NativeDynamic(std::move(value));}
NativeDynamic native_callable(std::function<NativeDynamic(NativeDynamic,std::span<const NativeDynamic>)> body){
    auto callable=std::make_shared<MmlNativeCallable<MmlNode>>();
    callable->invoke=std::move(body);
    return NativeDynamic(std::move(callable));
}
Property native_property_arg(const NativeDynamic& value){
    if(const auto* property=std::get_if<Property>(&value.data()))return *property;
    if(const auto* source=std::get_if<NativeSourceValue>(&value.data()))
        return source->provider->propertyValue(source->value);
    host::typeError("Native MML operation expected a Property value");
}
NodeValue native_child_arg(const NativeDynamic& value){
    if(const auto* child=std::get_if<NodeValue>(&value.data()))return *child;
    if(const auto* source=std::get_if<NativeSourceValue>(&value.data()))return source->provider->childValue(source->value);
    if(const auto* property=std::get_if<Property>(&value.data())){
        if(std::holds_alternative<PropertyNull>(property->value))return PropertyNull{};
        if(std::holds_alternative<Missing>(property->value))return Missing{};
    }
    host::typeError("Native MML operation expected a child value");
}
Children native_children_arg(const NativeDynamic& value){
    if(const auto* children=std::get_if<Children>(&value.data()))return *children;
    if(const auto* source=std::get_if<NativeSourceValue>(&value.data()))
        return source->provider->arrayView(source->value);
    host::typeError("Native MML operation expected a child-array owner");
}
std::string native_key(const NativeDynamic& value){
    if(const auto* source=std::get_if<NativeSourceValue>(&value.data()))
        return source->provider->stringValue(source->value);
    return host::debugText(host::string(native_property_arg(value)));
}
Property native_property_key(const NativeDynamic& value){
    if(const auto* property=std::get_if<Property>(&value.data()))return *property;
    if(const auto* source=std::get_if<NativeSourceValue>(&value.data())){
        return source->provider->toPropertyKey(source->value);
    }
    return Property(native_key(value));
}
options::Value native_options_value(const NativeDynamic& value){
    if(const auto* raw=std::get_if<options::Value>(&value.data()))return *raw;
    if(const auto* raw=std::get_if<NativeSourceValue>(&value.data()))return raw->value;
    if(const auto* child=std::get_if<NodeValue>(&value.data()))return child->sourceValue();
    if(const auto* property=std::get_if<Property>(&value.data())){
        if(std::holds_alternative<Missing>(property->value))return options::Undefined{};
        if(std::holds_alternative<PropertyNull>(property->value))return options::Null{};
        if(const auto* item=std::get_if<bool>(&property->value))return *item;
        if(const auto* item=std::get_if<double>(&property->value))return *item;
        if(const auto* item=std::get_if<std::string>(&property->value))return property_host::text(*item);
        if(const auto* item=std::get_if<options::SymbolRef>(&property->value))return *item;
    }
    host::typeError("Native MML argument has no Context Value representation");
}
options::Value native_options_value(const NativeDynamic& value,const tree::TreeNodeAccessRef<MmlNode>& provider){
    if(std::holds_alternative<NativeCallable>(value.data())){
        if(!provider)host::typeError("Native MML callback has no Context provider");
        const auto callable=std::get<NativeCallable>(value.data());
        return provider->makeCallback([callable,provider](std::span<const options::Value> rawArgs){
            std::vector<NativeDynamic> args;args.reserve(rawArgs.size());
            for(const auto& arg:rawArgs)args.emplace_back(NativeSourceValue{arg,provider});
            const auto result=callable->invoke(native_property(Property(Missing{})),args);
            return native_options_value(result);
        });
    }
    return native_options_value(value);
}
NativeDynamic native_mml_get(NativeDynamic object,std::u16string_view key){
    if(const auto* raw=std::get_if<NativeSourceValue>(&object.data()))
        return NativeDynamic(NativeSourceValue{raw->provider->getValue(raw->value,key),raw->provider});
    if(const auto* child=std::get_if<NodeValue>(&object.data())){
        if(const auto source=child->sourceAccess())
            return NativeDynamic(NativeSourceValue{source->get(key),source});
        MmlNode* node=child->nodeOrNull();
        if(!node)host::typeError(std::string("Cannot read properties of ")+
            (child->kind()==tree::ChildKind::Null?"null":"undefined")+
            " (reading '"+host::debugText(key)+"')");
        if(const auto source=node->sourceTreeAccess())
            return native_mml_get(NativeDynamic(NativeSourceValue{source->sourceValue(),source}),key);
        if(key==u"kind")return native_property(Property(node->kind()));
        if(key==u"factory")return NativeDynamic(&node->factoryOwner());
        if(key==u"parent")return native_child(node->getParent());
        if(key==u"childNodes")return NativeDynamic(node->childArrayOwner());
        if(key==u"properties")
            if(const auto* storage=dynamic_cast<tree::AbstractNode<MmlNode,MmlNode>*>(node))
                if(const auto raw=storage->sourcePropertiesRawField())return native_property(*raw);
        if(key==u"properties")return native_property(Property(node->propertyOwner()));
        if(auto* abstract=dynamic_cast<AbstractMmlNode*>(node)){
            if(key==u"arity")return native_property(Property(abstract->arity()));
            if(key==u"isInferred")return native_property(Property(abstract->isInferred()));
            if(key==u"prevClass")return native_property(abstract->sourcePrevClassField());
            if(key==u"prevLevel")return native_property(abstract->sourcePrevLevelField());
            if(key==u"texclass")return native_property(abstract->sourceTexclassField());
            if(key==u"attributes"){
                if(const auto raw=abstract->sourceAttributesRawField()){
                    if(raw->provider)return NativeDynamic(*raw);
                    return NativeDynamic(raw->value);
                }
                const auto value=abstract->attributes();
                return value?NativeDynamic(value):native_property(Property(PropertyNull{}));
            }
        }
        const auto method=[key](NativeDynamic receiver,std::span<const NativeDynamic> args)->NativeDynamic{
            const NodeValue targetValue=native_child_arg(receiver);
            MmlNode* target=targetValue.requireNode("MML method");
            if(key==u"setChildren"){
                target->setChildren(native_children_arg(args.empty()?NativeDynamic(Property(Missing{})):args[0]));
                return native_property(Property(Missing{}));
            }
            if(key==u"appendChild")return native_child(target->appendChild(native_child_arg(args[0])));
            if(key==u"replaceChild")return native_child(target->replaceChild(native_child_arg(args[0]),native_child_arg(args[1])));
            if(key==u"copy"){
                const bool keep=!args.empty()&&[&]{
                    if(const auto* value=std::get_if<Property>(&args[0].data()))return host::truthy(*value);
                    if(const auto* value=std::get_if<NodeValue>(&args[0].data()))return value->truthy();
                    return false;
                }();
                return native_child(NodeValue(target->copy(keep)));
            }
            if(key==u"getProperty")return native_property(target->propertyOwner().getKey(native_property_key(args[0])));
            if(key==u"setProperty"){
                const Property propertyKey=native_property_key(args[0]);
                const auto source=targetValue.sourceAccess()?targetValue.sourceAccess():target->sourceTreeAccess();
                if(source){
                    const auto receiverValue=source->sourceValue();
                    const auto properties=source->getValue(receiverValue,u"properties");
                    // ToPropertyKey is observable for object keys. It has
                    // already run above; pass the converted primitive on to
                    // Context so a user coercion hook is not invoked twice.
                    const auto rawKey=native_options_value(native_property(propertyKey),source);
                    const auto rawValue=native_options_value(args[1],source);
                    source->setPropertyValue(properties,rawKey,rawValue);
                }else{
                    const Property value=native_property_arg(args[1]);
                    if(std::holds_alternative<options::SymbolRef>(propertyKey.value))target->propertyOwner().setKey(propertyKey,value);
                    else target->setProperty(std::get<std::string>(propertyKey.value),value);
                }
                return native_property(Property(Missing{}));
            }
            if(key==u"getPropertyNames"){
                auto names=target->getPropertyNames();
                PropertyArray array;for(auto& name:names)array.push(Property(std::move(name)));
                return native_property(Property(std::move(array)));
            }
            host::typeError("Unsupported native MML method");
        };
        if(key==u"setChildren"||key==u"appendChild"||key==u"replaceChild"||key==u"copy"||
           key==u"getProperty"||key==u"setProperty"||key==u"getPropertyNames")
            return native_callable(method);
        return native_property(node->getProperty(property_host::debugText(key)));
    }
    if(const auto* factory=std::get_if<tree::FactoryServices<MmlNode>*>(&object.data())){
        if(key==u"create")return native_callable([](NativeDynamic receiver,std::span<const NativeDynamic> args){
            auto* service=std::get<tree::FactoryServices<MmlNode>*>(receiver.data());
            if(!service)host::typeError("MML factory is unavailable");
            const auto name=args.empty()?std::string{}:native_key(args.front());
            return native_child(NodeValue(service->create(name,tree::FactoryArgs{})));
        });
        if(key==u"getNodeClass")return native_callable([](NativeDynamic receiver,std::span<const NativeDynamic> args){
            auto* service=std::get<tree::FactoryServices<MmlNode>*>(receiver.data());
            if(!service)host::typeError("MML factory is unavailable");
            const auto name=args.empty()?std::string{}:native_key(args.front());
            return NativeDynamic(service->getNodeClass(name));
        });
        return native_property(Property(Missing{}));
    }
    if(const auto* nodeClass=std::get_if<tree::FactoryClassRef<MmlNode>>(&object.data())){
        if(key==u"defaults")return native_property(class_info_defaults(*nodeClass));
        return native_property(Property(Missing{}));
    }
    if(const auto* property=std::get_if<Property>(&object.data()))
        if(const auto record=PropertyList::record(*property))return native_property(record->get(property_host::debugText(key)));
    if(const auto* list=std::get_if<PropertyList>(&object.data()))
        return native_property(list->get(property_host::debugText(key)));
    if(const auto* attributes=std::get_if<std::shared_ptr<Attributes>>(&object.data())){
        if(!*attributes)return native_property(Property(PropertyNull{}));
        if(key==u"getAllAttributes")return native_callable([](NativeDynamic receiver,std::span<const NativeDynamic>){
            auto value=std::get<std::shared_ptr<Attributes>>(receiver.data());
            return native_property(Property(value->getAllAttributes()));
        });
        if(key==u"set")return native_callable([](NativeDynamic receiver,std::span<const NativeDynamic> args){
            auto value=std::get<std::shared_ptr<Attributes>>(receiver.data());
            value->set(native_key(args[0]),native_property_arg(args[1]));
            return native_property(Property(Missing{}));
        });
        if(key==u"setList")return native_callable([](NativeDynamic receiver,std::span<const NativeDynamic> args){
            auto value=std::get<std::shared_ptr<Attributes>>(receiver.data());
            if(!value)host::typeError("Cannot read properties of null (reading 'setList')");
            const Property source=args.empty()?Property(Missing{}):native_property_arg(args.front());
            value->setList(host::objectAssign(source));
            return native_property(Property(Missing{}));
        });
    }
    if(const auto* children=std::get_if<Children>(&object.data())){
        if(key==u"length")return native_property(Property(static_cast<double>(*children?(*children)->size():0)));
        if(key==u"push")return native_callable([](NativeDynamic receiver,std::span<const NativeDynamic> args){
            const auto children=std::get<Children>(receiver.data());
            if(!children)host::typeError("Cannot read properties of null (reading 'push')");
            for(const auto& argument:args)(void)children->push_back(native_child_arg(argument));
            return native_property(Property(static_cast<double>(children->size())));
        });
        if(key==u"forEach")return native_callable([](NativeDynamic receiver,std::span<const NativeDynamic> args){
            const auto children=std::get<Children>(receiver.data());
            const auto* callback=args.empty()?nullptr:std::get_if<NativeCallable>(&args[0].data());
            if(!callback||!*callback)host::typeError("MML childNodes.forEach is not callable");
            const auto retainedCallback=*callback;
            const auto callbackToken=args[0];
            if(children)children->forEach([retainedCallback,callbackToken,receiver](NodeValue value,std::size_t index,tree::ChildArray<MmlNode>&){
                const std::array<NativeDynamic,3> arguments{native_child(value),native_property(Property(static_cast<double>(index))),receiver};
                (void)retainedCallback->invoke(callbackToken,arguments);
            });
            return native_property(Property(Missing{}));
        });
    }
    return native_property(Property(Missing{}));
}

NativeReceiver native_mml_receiver(AbstractMmlNode* node){
    NativeReceiver receiver;
    receiver.tree=node->treeReceiver();
    const auto factoryLease=node->factoryOwner().executionLease();
    const std::weak_ptr<std::atomic_bool> emptyLease;
    const bool hasFactoryLease=factoryLease.owner_before(emptyLease)||emptyLease.owner_before(factoryLease);
    receiver.ensureActive=[factoryLease,hasFactoryLease]{
        if(!hasFactoryLease)return;
        const auto active=factoryLease.lock();
        if(!active||!active->load(std::memory_order_acquire))
            throw Error("TypeError",u"Native MML receiver owner has expired");
    };
    receiver.self=[node]{return NodeValue(node);};
    receiver.toDynamic=[](NodeValue value){return native_child(std::move(value));};
    receiver.toChild=[](NativeDynamic value){return native_child_arg(value);};
    receiver.get=[](NativeDynamic object,std::u16string_view key){return native_mml_get(std::move(object),key);};
    receiver.getIndex=[](NativeDynamic object,std::size_t index){
        if(const auto* raw=std::get_if<NativeSourceValue>(&object.data()))
            return NativeDynamic(NativeSourceValue{raw->provider->getValue(raw->value,property_host::text(std::to_string(index))),raw->provider});
        if(const auto* children=std::get_if<Children>(&object.data()))
            return native_child((*children)->get(index));
        if(const auto* property=std::get_if<Property>(&object.data())){
            const auto list=PropertyList::record(*property);
            if(list)return native_property(list->get(std::to_string(index)));
        }
        return native_property(Property(Missing{}));
    };
    receiver.childrenValue=[](Children children){return NativeDynamic(std::move(children));};
    receiver.set=[](NativeDynamic object,std::u16string_view key,NativeDynamic value){
        if(const auto* raw=std::get_if<NativeSourceValue>(&object.data())){
            raw->provider->setValue(raw->value,key,native_options_value(value));
            return;
        }
        if(const auto* child=std::get_if<NodeValue>(&object.data())){
            auto* target=child->nodeOrNull();
            if(!target){
                const auto kind=child->kind()==tree::ChildKind::Null?"null":"undefined";
                host::typeError(std::string("Cannot set properties of ")+kind+
                    " (setting '"+host::debugText(key)+"')");
            }
            if(const auto source=target->sourceTreeAccess()){
                source->setValue(source->sourceValue(),key,native_options_value(value,source));
                return;
            }
            if(key==u"childNodes")target->childArrayOwner(native_children_arg(value));
            else if(key==u"parent")target->setParent(native_child_arg(value));
            else if(key==u"prevClass")dynamic_cast<AbstractMmlNode&>(*target).setSourcePrevClassField(native_property_arg(value));
            else if(key==u"prevLevel")dynamic_cast<AbstractMmlNode&>(*target).setSourcePrevLevelField(native_property_arg(value));
            else if(key==u"texclass")dynamic_cast<AbstractMmlNode&>(*target).setSourceTexclassField(native_property_arg(value));
            else if(key==u"properties"){
                if(const auto* raw=std::get_if<NativeSourceValue>(&value.data())){
                    auto* storage=dynamic_cast<tree::AbstractNode<MmlNode,MmlNode>*>(target);
                    if(!storage)host::typeError("MML node has no canonical properties owner");
                    storage->setSourcePropertiesRawField(raw->provider->propertyValue(raw->value));
                    return;
                }
                if(const auto* raw=std::get_if<options::Value>(&value.data())){
                    host::typeError("Raw Context properties require their operation provider");
                }
                const auto record=PropertyList::record(native_property_arg(value));
                if(!record)host::typeError("MML properties must be a record");
                target->propertyOwner(*record);
            }else if(key==u"attributes"){
                auto* abstract=dynamic_cast<AbstractMmlNode*>(target);
                if(!abstract)host::typeError("MML attributes receiver is not an MML node");
                if(const auto* attributes=std::get_if<std::shared_ptr<Attributes>>(&value.data()))abstract->setAttributes(*attributes);
                else if(const auto* raw=std::get_if<NativeSourceValue>(&value.data()))abstract->setSourceAttributesRawField(raw->value,raw->provider);
                else if(const auto* raw=std::get_if<options::Value>(&value.data()))abstract->setSourceAttributesRawField(*raw);
                else abstract->setAttributes({});
            }else host::typeError("Unsupported native MML field assignment");
            return;
        }
        host::typeError("Native MML assignment receiver is not a node");
    };
    receiver.invoke=[](NativeDynamic callable,NativeDynamic exactThis,std::span<const NativeDynamic> args){
        const options::Value* rawCallable=std::get_if<options::Value>(&callable.data());
        if(const auto* sourceCallable=std::get_if<NativeSourceValue>(&callable.data()))rawCallable=&sourceCallable->value;
        if(rawCallable){
            tree::TreeNodeAccessRef<MmlNode> provider;
            options::Value rawThis;
            if(const auto* sourceThis=std::get_if<NativeSourceValue>(&exactThis.data())){
                provider=sourceThis->provider;rawThis=sourceThis->value;
            }else if(const auto* child=std::get_if<NodeValue>(&exactThis.data())){
                if(const auto source=child->sourceAccess()){
                    provider=source;rawThis=source->sourceValue();
                }else if(auto* node=child->nodeOrNull();node&&node->sourceTreeAccess()){
                    provider=node->sourceTreeAccess();rawThis=provider->sourceValue();
                }
            }
            if(!provider)if(const auto* sourceCallable=std::get_if<NativeSourceValue>(&callable.data()))provider=sourceCallable->provider;
            if(!provider)host::typeError("Context callable lost its exact receiver");
            std::vector<options::Value> values;values.reserve(args.size());
            for(const auto& argument:args)values.push_back(native_options_value(argument,provider));
            return NativeDynamic(NativeSourceValue{provider->invokeWithThis(*rawCallable,rawThis,values),provider});
        }
        const auto* target=std::get_if<NativeCallable>(&callable.data());
        if(!target||!*target)host::typeError("MML method is not callable");
        return (*target)->invoke(std::move(exactThis),args);
    };
    const auto nativeTree=receiver.tree;
    const auto activeGuard=receiver.ensureActive;
    receiver.invokeCoreTreeSuper=[nativeTree,activeGuard](std::u16string_view name,NativeDynamic,
                                                          std::span<const NativeDynamic> args){
        if(activeGuard)activeGuard();
        if(name==u"setChildren"){
            const NativeDynamic raw=args.empty()?NativeDynamic(Property(Missing{})):args.front();
            tree::TreeNodeAlgorithms<MmlNode>::setChildren(nativeTree,native_children_arg(raw));
            return NativeDynamic(Property(Missing{}));
        }
        if(name==u"appendChild"){
            if(args.empty())host::typeError("Core appendChild requires a child argument");
            return native_child(tree::TreeNodeAlgorithms<MmlNode>::appendChild(nativeTree,native_child_arg(args.front())));
        }
        if(name==u"replaceChild"){
            if(args.size()<2)host::typeError("Core replaceChild requires two child arguments");
            return native_child(tree::TreeNodeAlgorithms<MmlNode>::replaceChild(
                nativeTree,native_child_arg(args[0]),native_child_arg(args[1])));
        }
        host::typeError("Unknown Core Tree super method");
    };
    receiver.string=[](std::u16string_view value){return native_property(Property(property_host::debugText(value)));};
    receiver.numberValue=[](double value){return native_property(Property(value));};
    receiver.undefined=[](){return native_property(Property(Missing{}));};
    receiver.nullValue=[](){return native_property(Property(PropertyNull{}));};
    receiver.arrayOfOne=[](NativeDynamic value){
        auto array=std::make_shared<tree::ChildArray<MmlNode>>();
        (void)array->push_back(native_child_arg(value));
        return NativeDynamic(Children(std::move(array)));
    };
    receiver.constructAttributes=[](NativeDynamic defaults,NativeDynamic mathDefaults){
        const auto toList=[](const NativeDynamic& value){
            if(const auto* property=std::get_if<Property>(&value.data())){
                if(property->missing()||std::holds_alternative<PropertyNull>(property->value))return PropertyList{};
                if(const auto record=PropertyList::record(*property))return *record;
            }
            if(const auto* list=std::get_if<PropertyList>(&value.data()))return *list;
            host::typeError("MML class defaults must be property records");
        };
        if(const auto* global=std::get_if<Property>(&mathDefaults.data());global&&global->missing())
            host::typeError("Object prototype may only be an Object or null: undefined");
        return NativeDynamic(std::make_shared<Attributes>(toList(defaults),toList(mathDefaults)));
    };
    receiver.truthy=[](NativeDynamic value){
        if(const auto* property=std::get_if<Property>(&value.data()))return host::truthy(*property);
        if(const auto* raw=std::get_if<options::Value>(&value.data()))return options::truthy(*raw);
        if(const auto* raw=std::get_if<NativeSourceValue>(&value.data()))return raw->provider->truthyValue(raw->value);
        if(const auto* child=std::get_if<NodeValue>(&value.data()))return child->truthy();
        if(const auto* children=std::get_if<Children>(&value.data()))return *children!=nullptr;
        if(const auto* attributes=std::get_if<std::shared_ptr<Attributes>>(&value.data()))return static_cast<bool>(*attributes);
        if(const auto* factory=std::get_if<tree::FactoryServices<MmlNode>*>(&value.data()))return *factory!=nullptr;
        return true;
    };
    receiver.number=[](NativeDynamic value){
        if(const auto* property=std::get_if<Property>(&value.data()))return property_host::number(*property);
        if(const auto* raw=std::get_if<NativeSourceValue>(&value.data()))return raw->provider->toNumber(raw->value);
        if(const auto* raw=std::get_if<options::Value>(&value.data())){
            if(std::holds_alternative<options::Null>(raw->data))return 0.0;
            if(const auto* number=std::get_if<double>(&raw->data))return *number;
            if(const auto* boolean=std::get_if<bool>(&raw->data))return *boolean?1.0:0.0;
            if(std::holds_alternative<options::Undefined>(raw->data))return std::numeric_limits<double>::quiet_NaN();
        }
        host::typeError("Native MML number conversion is unavailable for this dynamic value");
    };
    receiver.strictEqual=[](NativeDynamic left,NativeDynamic right){
        const auto& a=left.data();const auto& b=right.data();
        if(const auto* x=std::get_if<NativeSourceValue>(&a)){
            if(const auto* y=std::get_if<NativeSourceValue>(&b))return x->value==y->value;
            if(std::holds_alternative<Property>(b))return x->value==native_options_value(right);
            return false;
        }
        if(const auto* x=std::get_if<NativeSourceValue>(&b)){
            if(const auto* y=std::get_if<options::Value>(&a))return x->value==*y;
            if(std::holds_alternative<Property>(a))return native_options_value(left)==x->value;
            return false;
        }
        if(const auto* x=std::get_if<Property>(&a)){
            if(const auto* y=std::get_if<Property>(&b))return *x==*y;
            if(const auto* y=std::get_if<options::Value>(&b)){
                if(const auto* value=std::get_if<double>(&x->value))if(const auto* other=std::get_if<double>(&y->data))return *value==*other;
                if(const auto* value=std::get_if<bool>(&x->value))if(const auto* other=std::get_if<bool>(&y->data))return *value==*other;
                if(const auto* value=std::get_if<std::string>(&x->value))if(const auto* other=std::get_if<std::u16string>(&y->data))return property_host::text(*value)==*other;
                if(std::holds_alternative<PropertyNull>(x->value))return std::holds_alternative<options::Null>(y->data);
                if(std::holds_alternative<Missing>(x->value))return std::holds_alternative<options::Undefined>(y->data);
            }
        }
        if(const auto* x=std::get_if<options::Value>(&a))if(const auto* y=std::get_if<options::Value>(&b))return *x==*y;
        if(const auto* x=std::get_if<NodeValue>(&a))if(const auto* y=std::get_if<NodeValue>(&b))return *x==*y;
        if(const auto* x=std::get_if<Children>(&a))if(const auto* y=std::get_if<Children>(&b))return x->get()==y->get();
        if(const auto* x=std::get_if<tree::FactoryServices<MmlNode>*>(&a))if(const auto* y=std::get_if<tree::FactoryServices<MmlNode>*>(&b))return *x==*y;
        return false;
    };
    receiver.childrenOf=[](NativeDynamic value){
        if(const auto* source=std::get_if<NativeSourceValue>(&value.data()))return source->provider->arrayView(source->value);
        return native_children_arg(value);
    };
    receiver.objectAssign=[](NativeDynamic value){
        if(const auto* raw=std::get_if<NativeSourceValue>(&value.data()))
            return NativeDynamic(NativeSourceValue{raw->provider->copyEnumerableValue(raw->value),raw->provider});
        if(const auto* property=std::get_if<Property>(&value.data())){
            if(std::holds_alternative<PropertyNull>(property->value)||std::holds_alternative<Missing>(property->value))
                return native_property(Property(PropertyList{}));
            if(const auto list=PropertyList::record(*property))return native_property(Property(list->copyOwnEnumerableProperties()));
        }
        if(const auto* list=std::get_if<PropertyList>(&value.data()))return native_property(Property(list->copyOwnEnumerableProperties()));
        host::typeError("Native Object.assign source is unsupported");
    };
    receiver.ownEnumerableStringKeys=[](NativeDynamic value){
        if(const auto* raw=std::get_if<NativeSourceValue>(&value.data()))return raw->provider->ownEnumerableStringKeys(raw->value);
        const PropertyList* list=std::get_if<PropertyList>(&value.data());
        if(!list)if(const auto* property=std::get_if<Property>(&value.data())){
            if(const auto record=PropertyList::record(*property))return [&]{std::vector<std::u16string> keys;for(const auto& key:record->keys())keys.push_back(property_host::text(key));return keys;}();
        }
        if(!list)host::typeError("Native Object.keys source is unsupported");
        std::vector<std::u16string> keys;for(const auto& key:list->keys())keys.push_back(property_host::text(key));return keys;
    };
    receiver.forOf=[](NativeDynamic value,const std::function<void(NativeDynamic)>& body){
        if(const auto* raw=std::get_if<NativeSourceValue>(&value.data())){
            const auto provider=raw->provider;const auto source=raw->value;
            provider->forOfValue(source,[&](options::Value item){body(NativeDynamic(NativeSourceValue{std::move(item),provider}));});
            return;
        }
        if(const auto* raw=std::get_if<options::Value>(&value.data())){
            (void)raw;host::typeError("Native raw Context value has no operation provider");
        }
        if(const auto* children=std::get_if<Children>(&value.data())){
            if(!*children)host::typeError(
                "object null is not iterable (cannot read property Symbol(Symbol.iterator))");
            (*children)->forOf([&](NodeValue child){body(native_child(std::move(child)));});
            return;
        }
        if(const auto* property=std::get_if<Property>(&value.data())){
            if(const auto array=PropertyArray::array(*property)){
                for(std::size_t index=0;index<array->size();++index)body(native_property(array->get(index)));
            }
            else host::typeError("Native for-of value is not iterable");
            return;
        }
        host::typeError("Native for-of value is not iterable");
    };
    receiver.callback=[](const std::function<NativeDynamic(std::span<const NativeDynamic>)>& body){
        return native_callable([body](NativeDynamic,std::span<const NativeDynamic> args){return body(args);});
    };
    return receiver;
}
}

MmlNodeReceiver<MmlNode,NativeMmlValue> AbstractMmlNode::nativeOperationView(){
    return native_mml_receiver(this);
}

std::vector<Property> MATHVARIANTS = {
    "normal", "bold", "italic", "bold-italic", "double-struck", "fraktur",
    "bold-fraktur", "script", "bold-script", "sans-serif", "bold-sans-serif",
    "sans-serif-italic", "sans-serif-bold-italic", "monospace", "inital",
    "tailed", "looped", "stretched"
};

std::vector<std::string> indentAttributes = {
    "indentalign", "indentalignfirst", "indentshift", "indentshiftfirst"
};

PropertyList& AbstractMmlNode::noInherit() {
    static PropertyList value{
        {"mstyle", PropertyList{
            {"mpadded", PropertyList{{"width", true}, {"height", true}, {"depth", true},
                                      {"lspace", true}, {"voffset", true}}},
            {"mtable", PropertyList{{"width", true}, {"height", true}, {"depth", true}, {"align", true}}}
        }},
        {"maligngroup", PropertyList{
            {"mrow", PropertyList{{"groupalign", true}}},
            {"mtable", PropertyList{{"groupalign", true}}}
        }},
        {"mtr", PropertyList{
            {"msqrt", PropertyList{{"data-vertical-align", true}}},
            {"mroot", PropertyList{{"data-vertical-align", true}}}
        }},
        {"mlabeledtr", PropertyList{
            {"msqrt", PropertyList{{"data-vertical-align", true}}},
            {"mroot", PropertyList{{"data-vertical-align", true}}}
        }}
    };
    return value;
}

PropertyList& AbstractMmlNode::stopInherit() {
    static PropertyList value{{"mtd", PropertyList{
        {"columnalign", true}, {"rowalign", true}, {"groupalign", true}
    }}};
    return value;
}

PropertyList& AbstractMmlNode::alwaysInherit() {
    static PropertyList value{{"scriptminsize", true}, {"scriptsizemultiplier", true},
                              {"infixlinebreakstyle", true}};
    return value;
}

PropertyList& AbstractMmlNode::verifyDefaults() {
    static PropertyList value{{"checkArity", true}, {"checkAttributes", false},
                              {"checkMathvariants", true}, {"fullErrors", false},
                              {"fixMmultiscripts", true}, {"fixMtables", true}};
    return value;
}

void AbstractMmlNode::initializeMml(tree::FactoryArgs args,
                                    std::function<void(Children)> setChildren,
                                    Factory* constructorFactory) {
    (void)setChildren;
    NativeMmlValue rawAttributes(Property(Missing{}));
    if(!args.empty()&&args[0].has_value()){
        if(const auto* value=std::any_cast<Property>(&args[0]))rawAttributes=NativeMmlValue(*value);
        else if(const auto* value=std::any_cast<PropertyList>(&args[0]))rawAttributes=NativeMmlValue(Property(*value));
        else if(std::any_cast<std::nullptr_t>(&args[0]))rawAttributes=NativeMmlValue(Property(PropertyNull{}));
        else if(std::any_cast<PropertyNull>(&args[0]))rawAttributes=NativeMmlValue(Property(PropertyNull{}));
        else host::typeError("Unsupported native MML constructor properties argument");
    }
    Children initialChildren=std::make_shared<tree::ChildArray<MmlNode>>();
    if(args.size()>1&&args[1].has_value()){
        if(const auto* value=std::any_cast<Children>(&args[1]))initialChildren=*value;
        else if(const auto* value=std::any_cast<Property>(&args[1])){
            if(value->missing()){}
            else if(host::nullish(*value))initialChildren.reset();
            else host::typeError("Unsupported native MML constructor children argument");
        }else if(std::any_cast<std::nullptr_t>(&args[1])||std::any_cast<PropertyNull>(&args[1]))initialChildren.reset();
        else host::typeError("Unsupported native MML constructor children argument");
    }
    if(sourceOperationView()){
        if(!sourceArgumentValues())
            host::typeError("Bound MML receiver has no native-to-Context constructor argument converters");
        auto view=sourceOperationView()();
        auto* lexicalFactory=constructorFactory?constructorFactory:&factoryOwner();
        const auto& conversions=*sourceArgumentValues();
        const auto rawFactory=conversions.factory(lexicalFactory);
        const auto sourceAttributes=conversions.attributes(native_property_arg(rawAttributes));
        const auto sourceChildren=conversions.children(initialChildren);
        initializeMmlNode(view,rawFactory,sourceAttributes,sourceChildren);
        return;
    }
    auto view=nativeOperationView();
    auto* sourceFactory=constructorFactory?constructorFactory:&factoryOwner();
    initializeMmlNode(view,NativeMmlValue(sourceFactory),std::move(rawAttributes),
                      NativeMmlValue(std::move(initialChildren)));
}

PropertyList AbstractMmlNode::constructorDefaults()const {
    if(!constructorInfo_)host::typeError("Missing native constructor-class binding");
    auto defaults=constructorInfo_->getDefaults();
    if(!defaults)host::typeError("Constructor class has no defaults");
    return *defaults;
}

MmlNode* AbstractMmlNode::copy(bool keepIds) {
    if(sourceOperationView()){
        auto view=sourceOperationView()();
        const auto result=MmlNodeAlgorithms<MmlNode,options::Value>::copy(view,keepIds);
        return projectSourceNode(result);
    }
    auto view=native_mml_receiver(this);
    const auto result=MmlNodeAlgorithms<MmlNode,NativeDynamic>::copy(view,keepIds);
    return native_child_arg(result).requireNodeForWrite("copy");
}

Property AbstractMmlNode::texClass() { return texclass_; }
void AbstractMmlNode::setTexClass(Property value) { texclass_ = std::move(value); }
Property AbstractMmlNode::prevClass() { return prevClass_; }
void AbstractMmlNode::setPrevClass(Property value) { prevClass_ = std::move(value); }
Property AbstractMmlNode::prevLevel() const { return prevLevel_; }
void AbstractMmlNode::setPrevLevel(Property value) { prevLevel_ = std::move(value); }
std::shared_ptr<Attributes> AbstractMmlNode::attributes() const {
    if(sourceAttributeReader_)return sourceAttributeReader_();
    if(sourceAttributesRaw_){
        if(std::holds_alternative<options::Null>(sourceAttributesRaw_->value.data)||
           std::holds_alternative<options::Undefined>(sourceAttributesRaw_->value.data))return {};
        host::typeError("MML attributes value is not a native Attributes owner");
    }
    return attributes_;
}
void AbstractMmlNode::setAttributes(std::shared_ptr<Attributes> value) { sourceAttributesRaw_.reset();attributes_ = std::move(value); }

bool AbstractMmlNode::isToken() const { return false; }
bool AbstractMmlNode::isEmbellished() const { return false; }
bool AbstractMmlNode::isSpacelike() const { return false; }
bool AbstractMmlNode::linebreakContainer() const { return false; }
std::string AbstractMmlNode::linebreakAlign() const { return "data-align"; }

bool AbstractMmlNode::isEmpty() const {
    tree::TreeCallGuard<MmlNode> guard;
    const auto children = this->childArrayOwner();
    if (!children) throw Error("TypeError",u"object null is not iterable (cannot read property Symbol(Symbol.iterator))");
    for (std::size_t i = 0; i < children->size(); ++i) {
        const NodeValue child = children->get(i);
        if (MmlNode* const node = child.nodeOrNull(); node && !node->isEmpty()) {
            return false;
        }
    }
    return true;
}

double AbstractMmlNode::arity() const { return std::numeric_limits<double>::infinity(); }
bool AbstractMmlNode::isInferred() const { return false; }
Property AbstractMmlNode::notParent() const { return false; }

NodeValue AbstractMmlNode::Parent() const {
    tree::TreeCallGuard<MmlNode> guard;
    NodeValue parent = getParent();
    while (auto* node = parent.nodeOrNull()) {
        if (!host::truthy(node->notParent())) break;
        parent = node->Parent();
    }
    return parent;
}

void AbstractMmlNode::setChildren(Children children) {
    if(sourceOperationView()){
        auto view=sourceOperationView()();
        (void)MmlNodeAlgorithms<MmlNode,options::Value>::setChildren(
            view,view.childrenValue(std::move(children)));
        return;
    }
    auto view=native_mml_receiver(this);
    (void)MmlNodeAlgorithms<MmlNode,NativeDynamic>::setChildren(view,NativeDynamic(std::move(children)));
}

NodeValue AbstractMmlNode::appendChild(NodeValue child) {
    if(sourceOperationView()){
        auto view=sourceOperationView()();
        return view.toChild(MmlNodeAlgorithms<MmlNode,options::Value>::appendChild(view,std::move(child)));
    }
    auto view=native_mml_receiver(this);
    return native_child_arg(MmlNodeAlgorithms<MmlNode,NativeDynamic>::appendChild(view,std::move(child)));
}

NodeValue AbstractMmlNode::replaceChild(NodeValue newChild, NodeValue oldChild) {
    if(sourceOperationView()){
        auto view=sourceOperationView()();
        return view.toChild(MmlNodeAlgorithms<MmlNode,options::Value>::replaceChild(view,std::move(newChild),std::move(oldChild)));
    }
    auto view=native_mml_receiver(this);
    return native_child_arg(MmlNodeAlgorithms<MmlNode,NativeDynamic>::replaceChild(view,std::move(newChild),std::move(oldChild)));
}

NodeValue AbstractMmlNode::core() { return this; }
NodeValue AbstractMmlNode::coreMO() { return this; }
std::size_t AbstractMmlNode::coreIndex() { return 0; }

std::optional<std::size_t> AbstractMmlNode::childPosition() {
    tree::TreeCallGuard<MmlNode> guard;
    NodeValue child;
    NodeValue parent = getParent();
    while (auto* node = parent.nodeOrNull()) {
        if (!host::truthy(node->notParent())) break;
        child = parent;
        parent = node->getParent();
    }
    if (!child.isNode()) child = NodeValue(this);
    if (auto* node = parent.nodeOrNull()) {
        const auto children = node->childArrayOwner();
        if (!children) host::typeError("object null is not iterable (cannot read property Symbol(Symbol.iterator))");
        for (std::size_t index = 0; index < children->size(); ++index) {
            if (children->get(index) == child) return index;
        }
    }
    return {};
}

NodeValue AbstractMmlNode::setTeXclass(NodeValue previous) {
    tree::TreeCallGuard<MmlNode> guard;
    getPrevClass(previous);
    return !host::nullish(texClass()) ? NodeValue(this) : previous;
}

void AbstractMmlNode::updateTeXclass(NodeValue coreNode) {
    tree::TreeCallGuard<MmlNode> guard;
    auto* core = coreNode.nodeOrNull();
    if (!core) return;
    setPrevClass(core->prevClass());
    setPrevLevel(core->prevLevel());
    core->setPrevLevel(PropertyNull{});
    core->setPrevClass(PropertyNull{});
    setTexClass(core->texClass());
}

void AbstractMmlNode::getPrevClass(NodeValue previous) {
    tree::TreeCallGuard<MmlNode> guard;
    auto* node = previous.nodeOrNull();
    if (!node) return;
    setPrevClass(node->texClass());
    auto previousAttributes = node->attributes();
    if (!previousAttributes) host::typeError("Cannot read properties of null (reading 'get')");
    setPrevLevel(previousAttributes->get("scriptlevel"));
}

Property AbstractMmlNode::texSpacing() {
    const Property firstPrevious = prevClass();
    const Property previous = !host::nullish(firstPrevious) ? prevClass() : Property(TEXCLASS.NONE);
    const Property rawCurrent = texClass();
    const Property current = host::truthy(rawCurrent) ? rawCurrent : Property(TEXCLASS.ORD);
    if (previous == Property(TEXCLASS.NONE) || current == Property(TEXCLASS.NONE)) return std::string{};
    const auto rowKey = host::debugText(host::string(previous));
    const Property row = host::get(TEXSPACE, rowKey);
    const auto columnKey = host::debugText(host::string(current));
    const Property space = host::get(row, columnKey);
    const bool previousScript = host::greaterThanZero(prevLevel());
    bool currentScript = false;
    if (!previousScript) {
        auto attrs = attributes();
        if (!attrs) host::typeError("Cannot read scriptlevel from null attributes");
        currentScript = host::greaterThanZero(attrs->get("scriptlevel"));
    }
    if ((previousScript || currentScript) && host::number(space) >= 0) return std::string{};
    const double magnitude = std::abs(host::number(space));
    const auto lengthKey = host::debugText(host::string(magnitude));
    return host::get(TEXSPACELENGTH, lengthKey);
}

bool AbstractMmlNode::hasSpacingAttributes() {
    tree::TreeCallGuard<MmlNode> guard;
    if (!isEmbellished()) return false;
    return coreMO().requireNode("hasSpacingAttributes")->hasSpacingAttributes();
}

void AbstractMmlNode::inheritAttributes(AttributeList inherited, Property display,
                                              Property level, Property prime) {
    tree::TreeCallGuard<MmlNode> guard;
    host::inheritanceDefaults(inherited, display, level, prime);

    const auto defaultsAttributes = this->attributes();
    if (!defaultsAttributes) host::typeError("Cannot read properties of null (reading 'getAllDefaults')");
    const PropertyList defaults = defaultsAttributes->getAllDefaults();
    const std::vector<std::string> keys = host::ownKeys(inherited);
    for (const auto& key : keys) {
        if (defaults.hasOwn(key) || AbstractMmlNode::alwaysInherit().hasOwn(key)) {
            const Property tuple = host::get(inherited, key);
            const auto [origin, value] = unpackAttribute(tuple);
            if (!forbidden_inheritance(origin, *this, key)) {
                const auto currentAttributes = this->attributes();
                if (!currentAttributes) host::typeError("Cannot read properties of null (reading 'setInherited')");
                currentAttributes->setInherited(key, value);
            }
        }
        if (stopped_inheritance(this->kind(), key)) {
            PropertyList stopped = host::objectAssign(inherited);
            stopped.erase(key);
            inherited = Property(std::move(stopped));
        }
    }

    const auto displayAttributes = this->attributes();
    if (!displayAttributes) host::typeError("Cannot read properties of null (reading 'getExplicit')");
    const Property displaystyle = displayAttributes->getExplicit("displaystyle");
    if (displaystyle.missing()) {
        const auto currentAttributes = this->attributes();
        if (!currentAttributes) host::typeError("Cannot read properties of null (reading 'setInherited')");
        currentAttributes->setInherited("displaystyle", display);
    }

    const auto levelAttributes = this->attributes();
    if (!levelAttributes) host::typeError("Cannot read properties of null (reading 'getExplicit')");
    const Property scriptlevel = levelAttributes->getExplicit("scriptlevel");
    if (scriptlevel.missing()) {
        const auto currentAttributes = this->attributes();
        if (!currentAttributes) host::typeError("Cannot read properties of null (reading 'setInherited')");
        currentAttributes->setInherited("scriptlevel", level);
    }

    if (host::truthy(prime)) this->setProperty("texprimestyle", prime);

    const double expectedArity = this->arity();
    if (expectedArity >= 0 && expectedArity != std::numeric_limits<double>::infinity()) {
        const auto children = this->childArrayOwner();
        if (!children) host::typeError("Cannot read properties of null (reading 'length')");
        const double childLength = static_cast<double>(children->size());
        if ((expectedArity == 1 && childLength == 0) ||
            (expectedArity != 1 && childLength != expectedArity)) {
            if (expectedArity < childLength) {
                this->childArrayOwner(children->slice(0, static_cast<std::int64_t>(expectedArity)));
            } else {
                while (true) {
                    const auto currentChildren = this->childArrayOwner();
                    if (!currentChildren) host::typeError("Cannot read properties of null (reading 'length')");
                    if (!(static_cast<double>(currentChildren->size()) < expectedArity)) break;
                    MmlNode* const row = this->factoryOwner().create("mrow", {});
                    this->appendChild(NodeValue(row));
                }
            }
        }
    }

    if (this->linebreakContainer() && !this->isEmbellished()) {
        const std::string align = this->linebreakAlign();
        if (!align.empty()) {
            const auto currentAttributes = this->attributes();
            if (!currentAttributes) host::typeError("Cannot read properties of null (reading 'get')");
            Property indentalign = currentAttributes->get(align);
            if (!host::truthy(indentalign)) indentalign = "left";
            PropertyList additions{
                {"indentalign", indentalign},
                {"indentshift", "0"},
                {"indentalignfirst", indentalign},
                {"indentshiftfirst", "0"},
                {"indentalignlast", "indentalign"},
                {"indentshiftlast", "indentshift"}
            };
            inherited = this->addInheritedAttributes(std::move(inherited), Property(additions));
        }
    }

    this->setChildInheritedAttributes(std::move(inherited), display, level, prime);
}

void AbstractMmlNode::setChildInheritedAttributes(AttributeList inherited, Property display,
                                                   Property level, Property prime) {
    const Children children = this->childArrayOwner();
    if (!children) host::typeError("object null is not iterable (cannot read property Symbol(Symbol.iterator))");
    for (std::size_t index = 0; index < children->size(); ++index) {
        children->get(index).requireNode("setInheritedAttributes")
            ->setInheritedAttributes(inherited, display, level, prime);
    }
}

AttributeList AbstractMmlNode::addInheritedAttributes(AttributeList current, Property additions) {
    auto updated = host::objectAssign(current);
    for (const auto& name : host::ownKeys(additions)) {
        if (name != "displaystyle" && name != "scriptlevel" && name != "style")
            updated.set(name, attributeTuple(kind(), host::get(additions,name)));
    }
    return updated;
}

void AbstractMmlNode::inheritAttributesFrom(NodeValue node) {
    tree::TreeCallGuard<MmlNode> guard;

    MmlNode* const source = node.requireNode("attributes");
    const auto attributes = source->attributes();
    if (!attributes) host::typeError("Cannot read properties of null (reading 'get')");

    const Property display = attributes->get("displaystyle");
    const Property level = attributes->get("scriptlevel");

    PropertyList defaults;
    if (attributes->isSet("mathsize")) {
        const Property mathsize = attributes->get("mathsize");
        defaults.set("mathsize", attributeTuple("math", mathsize));
    }

    Property prime = source->getProperty("texprimestyle");
    if (!host::truthy(prime)) prime = false;

    setInheritedAttributes(Property(std::move(defaults)), display, level, prime);
}

void AbstractMmlNode::verifyTree(Property options) {
    tree::TreeCallGuard<MmlNode> guard;
    if (options.missing()) options = PropertyNull{};
    if (std::holds_alternative<PropertyNull>(options.value)) return;

    verifyAttributes(options);
    const double nodeArity = arity();
    if (host::truthy(host::get(options, "checkArity")) &&
        nodeArity >= 0 && !(std::isinf(nodeArity) && nodeArity > 0)) {
        bool wrongCount = false;
        if (nodeArity == 1) {
            const Children children = this->childArrayOwner();
            if (!children) host::typeError("Cannot read properties of null (reading 'length')");
            wrongCount = children->size() == 0;
        } else {
            const Children children = this->childArrayOwner();
            if (!children) host::typeError("Cannot read properties of null (reading 'length')");
            wrongCount = static_cast<double>(children->size()) != nodeArity;
        }
        if (wrongCount) {
            mError("Wrong number of children for \"" + kind() + "\" node", options, true);
        }
    }
    verifyChildren(options);
}

void AbstractMmlNode::verifyAttributes(Property options) {
    if (host::truthy(host::get(options,"checkAttributes"))) {
        auto attrs = attributes();
        if (!attrs) host::typeError("Cannot verify null MML attributes");
        std::vector<std::string> bad;
        for (const auto& name : attrs->getExplicitNames()) {
            const bool data = name.size() >= 5 && name.substr(0, 5) == "data-";
            const bool allowed = name == "class" || name == "style" || name == "id" ||
                                 name == "href" || name == "xlink:href";
            if (!data && attrs->getDefault(name).missing() && !allowed) bad.push_back(name);
        }
        if (!bad.empty()) {
            std::string joined;
            for (std::size_t i = 0; i < bad.size(); ++i) {
                if (i) joined += ", ";
                joined += bad[i];
            }
            mError("Unknown attributes for " + kind() + " node: " + joined, options, false);
        }
    }
    if (host::truthy(host::get(options,"checkMathvariants"))) {
        auto attrs = attributes();
        if (!attrs) host::typeError("Cannot verify null MML attributes");
        const Property variant = attrs->getExplicit("mathvariant");
        if (host::truthy(variant) && !property_set_contains(variant) &&
            !host::truthy(getProperty("ignore-variant"))) {
            mError("Invalid mathvariant: " + host::debugText(host::string(variant)), options, true);
        }
    }
}

void AbstractMmlNode::verifyChildren(Property options) {
    tree::TreeCallGuard<MmlNode> guard;
    const Children children = this->childArrayOwner();
    if (!children) host::typeError("object null is not iterable (cannot read property Symbol(Symbol.iterator))");
    for (std::size_t index = 0; index < children->size(); ++index) {
        children->get(index).requireNode("verifyTree")->verifyTree(options);
    }
}

MmlNode* AbstractMmlNode::mError(std::string message, Property options, bool shortError) {
    tree::TreeCallGuard<MmlNode> guard;

    const NodeValue initialParent = getParent();
    if (initialParent.isNode() &&
        getParent().requireNode("isKind")->isKind("merror")) return nullptr;

    MmlNode* const merror = factoryOwner().create("merror", {});
    auto* const initialErrorNode = NodeValue(merror).requireNode("attributes");
    auto initialErrorAttributes = initialErrorNode->attributes();
    if (!initialErrorAttributes) host::typeError("Cannot read properties of null (reading 'set')");
    initialErrorAttributes->set("data-mjx-message", message);

    if (options.missing()) host::typeError("Cannot read properties of undefined (reading 'fullErrors')");
    if (std::holds_alternative<PropertyNull>(options.value))
        host::typeError("Cannot read properties of null (reading 'fullErrors')");
    const Property initialFullErrors = host::get(options, "fullErrors");

    if (host::truthy(initialFullErrors) || shortError) {
        MmlNode* const mtext = factoryOwner().create("mtext", {});
        MmlNode* const text = factoryOwner().create("text", {});
        MmlNode* const textReceiver = NodeValue(text).requireNode("setText");

        if (options.missing()) host::typeError("Cannot read properties of undefined (reading 'fullErrors')");
        if (std::holds_alternative<PropertyNull>(options.value))
            host::typeError("Cannot read properties of null (reading 'fullErrors')");
        const Property textFullErrors = host::get(options, "fullErrors");
        const std::u16string textValue = host::truthy(textFullErrors)
            ? host::text(message)
            : host::text(kind());
        auto* const textNode = dynamic_cast<TextNode*>(textReceiver);
        if (!textNode) host::typeError("text.setText is not a function");
        textNode->setText(textValue);

        NodeValue(mtext).requireNode("appendChild")->appendChild(NodeValue(text));
        merror->appendChild(NodeValue(mtext));

        getParent().requireNode("replaceChild")->replaceChild(NodeValue(merror), NodeValue(this));

        if (options.missing()) host::typeError("Cannot read properties of undefined (reading 'fullErrors')");
        if (std::holds_alternative<PropertyNull>(options.value))
            host::typeError("Cannot read properties of null (reading 'fullErrors')");
        const Property titleFullErrors = host::get(options, "fullErrors");
        if (!host::truthy(titleFullErrors)) {
            MmlNode* const titleNode = NodeValue(merror).requireNode("attributes");
            auto titleAttributes = titleNode->attributes();
            if (!titleAttributes) host::typeError("Cannot read properties of null (reading 'set')");
            titleAttributes->set("title", message);
        }
    } else {
        getParent().requireNode("replaceChild")->replaceChild(NodeValue(merror), NodeValue(this));
        merror->appendChild(NodeValue(this));
    }
    return merror;
}
} // namespace folia::math::mml
