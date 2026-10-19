// MathJax 4.1.3 input/tex/ParseOptions.ts; Apache-2.0.
// Actual native parser-session and factory owner; no substitute parser graph.
module folia.math.input.tex.parser;
import :runtime;
import :stack_item_factory_bindings;
import folia.math.input.tex.node_util;
import folia.math.input.tex.tex_constants;
import folia.math.util.options;
import folia.math.util.property_host;
import folia.math.util.native_data_fields;
import folia.math.core.math_render_actions;
import folia.math.input.tex.node_factory_host;
import std;

namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view name){return Key(std::u16string(name));}
void requireRuntimeActive(const std::weak_ptr<std::atomic_bool>& lease){
    const auto active=lease.lock();
    if(!active||!active->load(std::memory_order_acquire))
        throw Exception("TypeError",u"Parser runtime has been released");
}
Value argument(std::span<const Value> arguments,std::size_t index){
    return index<arguments.size()?arguments[index]:Value(Undefined{});
}
template<class T>std::shared_ptr<T> shared(const ParserShared<T>& value,std::u16string_view member){
    if(const auto* owner=std::get_if<std::shared_ptr<T>>(&value);owner&&*owner)return *owner;
    throw Exception("TypeError",u"Missing ParseOptions service: "+std::u16string(member));
}
ObjectRef parseOptionsContextIdentity(ParserRuntime& runtime,const ParseOptions& owner){
    const auto identity=runtime.parseOptionsIdentities.find(
        const_cast<ParseOptions&>(owner).shared_from_this());
    if(!identity)throw Exception("TypeError",u"ParseOptions has no canonical Context identity");
    return *identity;
}
void clearParseOptions(Context& context,Value receiver){
    context.set(receiver,key(u"parsers"),Value(context.array()));
    context.set(receiver,key(u"root"),Value(Null{}));
    context.set(receiver,key(u"nodeLists"),Value(context.object()));
    context.set(receiver,key(u"error"),Value(false));
    const Value tags=context.get(receiver,key(u"tags"));
    const Value resetTag=context.get(tags,key(u"resetTag"));
    const auto* method=std::get_if<ObjectRef>(&resetTag.data);
    if(!method||context.kind(*method)!=ObjectKind::function)
        throw Exception("TypeError",u"this.tags.resetTag is not a function");
    (void)context.call(*method,tags,{});
}
std::u16string variant(std::string_view name){
    return property_host::string(property_host::get(property_host::get(TexConstant,"Variant"),name));
}
NodeFactory& nodeFactoryReceiver(ParserRuntime& runtime,const Value& receiver){
    const auto* object=std::get_if<ObjectRef>(&receiver.data);
    if(!object)throw Exception("TypeError",u"NodeFactory method receiver is invalid");
    return *runtime.nodeFactoryIdentities.resolve(*object);
}
std::any nodeFactoryArgument(ParserRuntime& runtime,const Value& value){
    if(const auto* object=std::get_if<ObjectRef>(&value.data)){
        const auto& payload=std::as_const(runtime.context()).payload(*object);
        if(const auto* factory=std::any_cast<NativeBindingPayload<NodeFactory*>>(&payload))
            return runtime.nodeFactoryIdentities.resolve(*object)->shared_from_this();
        if(std::any_cast<NativeBindingPayload<mml::Children>>(&payload))
            return runtime.nodeArrayIdentities.resolve(*object);
        if(std::any_cast<NativeBindingPayload<mml::MmlNode*>>(&payload))
            return &runtime.resolveMmlNode(*object);
        if(runtime.context().kind(*object)==ObjectKind::function)
            return runtime.properties.from(value);
    }
    return runtime.properties.from(value);
}
Value nodeFactorySourceArgument(ParserRuntime& runtime,const std::any& input){
    if(const auto* value=std::any_cast<Value>(&input))return *value;
    if(const auto* property=std::any_cast<Property>(&input))return runtime.properties.to(*property);
    if(const auto* owner=std::any_cast<std::shared_ptr<NodeFactory>>(&input)){
        const auto identity=owner&&*owner?runtime.nodeFactoryIdentities.find(owner->get()):std::optional<ObjectRef>{};
        if(!identity)throw Exception("TypeError",u"NodeFactory argument has no Context identity");
        return Value(*identity);
    }
    if(const auto* children=std::any_cast<mml::Children>(&input)){
        if(!*children)return Value(Null{});
        return Value(bindNodeArray(runtime,*children));
    }
    if(const auto* node=std::any_cast<mml::MmlNode*>(&input)){
        if(!*node)return Value(Null{});
        const auto identity=runtime.mmlNodeIdentities.find(*node);
        if(!identity)throw Exception("TypeError",u"MML argument has no Context identity");
        return Value(*identity);
    }
    if(const auto* node=std::any_cast<mml::NodeValue>(&input)){
        if(node->isNode()){
            const auto identity=runtime.mmlNodeIdentities.find(node->requireNode());
            if(!identity)throw Exception("TypeError",u"MML argument has no Context identity");
            return Value(*identity);
        }
        return node->kind()==tree::ChildKind::Null?Value(Null{}):Value(Undefined{});
    }
    throw Exception("TypeError",u"Unsupported native NodeFactory creator argument");
}
mml::NodeValue invokeContextNodeCreator(ParserRuntime& runtime,
        const std::shared_ptr<NodeFactory>& factory,ObjectRef function,FunctionArgs rest){
    const auto factoryIdentity=runtime.nodeFactoryIdentities.find(factory.get());
    if(!factoryIdentity)throw Exception("TypeError",u"NodeFactory has no Context identity");
    std::vector<Value> arguments;arguments.reserve(std::max(rest.size(),std::size_t(1))+1);
    arguments.emplace_back(*factoryIdentity);
    if(rest.empty())arguments.emplace_back(Undefined{});
    else for(const auto& value:rest)arguments.push_back(nodeFactorySourceArgument(runtime,value));
    const Value result=runtime.context().call(function,Value(Undefined{}),arguments);
    if(std::holds_alternative<Undefined>(result.data))return Missing{};
    if(std::holds_alternative<Null>(result.data))return PropertyNull{};
    const auto* object=std::get_if<ObjectRef>(&result.data);
    if(!object)throw Exception("TypeError",u"NodeFactory creator did not return an MML node");
    return mml::NodeValue(&runtime.resolveMmlNode(*object));
}
void bindNodeFactoryContextExecution(ParserRuntime& runtime,const std::shared_ptr<NodeFactory>& factory){
    const auto lease=runtime.executionLifetime();
    const std::weak_ptr<NodeFactory> weakFactory=factory;
    factory->bindContextExecution(lease,[runtime=&runtime,lease,weakFactory](ObjectRef function,FunctionArgs rest){
        requireRuntimeActive(lease);
        const auto owner=weakFactory.lock();
        if(!owner)throw Exception("TypeError",u"NodeFactory owner has been released");
        return invokeContextNodeCreator(*runtime,owner,function,rest);
    });
}
Value nodeFactoryResult(ParserRuntime& runtime,mml::NodeValue value){
    if(!value.isNode())return value.kind()==tree::ChildKind::Null?Value(Null{}):Value(Undefined{});
    const auto identity=runtime.mmlNodeIdentities.find(value.requireNode());
    if(!identity)throw Exception("TypeError",u"NodeFactory returned an MML node without a Context identity");
    return Value(*identity);
}
void bindDefaultNodeFactoryFunctions(ParserRuntime& runtime){
    const auto lease=runtime.executionLifetime();
    const auto defaults=NodeFactory::defaultFunctions();
    for(std::size_t index=0;index<defaults.size();++index){
        const auto function=defaults[index];
        if(runtime.functions().find(function))continue;
        OptionCallBinding binding;
        binding.receiver=[lease](Context&,Value)->FunctionReceiver{requireRuntimeActive(lease);return Missing{};};
        binding.arguments=[&runtime,lease](Context&,std::span<const Value> arguments){
            requireRuntimeActive(lease);
            std::vector<std::any> converted;converted.reserve(arguments.size());
            for(const auto& value:arguments)converted.push_back(nodeFactoryArgument(runtime,value));
            return converted;
        };
        binding.result=[&runtime,lease](Context&,const std::any& result)->Value{
            requireRuntimeActive(lease);
            if(const auto* node=std::any_cast<mml::NodeValue>(&result))return nodeFactoryResult(runtime,*node);
            if(const auto* property=std::any_cast<Property>(&result))return runtime.properties.to(*property);
            if(const auto* value=std::any_cast<Value>(&result))return *value;
            return Undefined{};
        };
        (void)runtime.functions().bind(function,std::move(binding),u"");
    }
}
struct NodeFactoryLifetime {
    ParseOptionsRef configuration;
    std::shared_ptr<NodeFactory> factory;
};
NativeDataFieldRef nodeFactoryConfigurationField(ParserRuntime& runtime,
        const std::shared_ptr<NodeFactory>& owner,ObjectRef identity){
    const auto executionLease=runtime.executionLifetime();
    return std::make_shared<NativeDataField>(owner,
        [&runtime,owner,executionLease]()->Value{
            requireRuntimeActive(executionLease);
            if(const auto* value=std::get_if<Value>(&owner->configuration))return *value;
            if(std::holds_alternative<Missing>(owner->configuration))return Value(Undefined{});
            if(std::holds_alternative<PropertyNull>(owner->configuration))return Value(Null{});
            const auto* config=std::get_if<NodeFactoryConfiguration*>(&owner->configuration);
            if(!config||!*config)return Value(Undefined{});
            const auto* parse=dynamic_cast<ParseOptions*>(*config);
            if(!parse)throw Exception("TypeError",u"NodeFactory configuration is not ParseOptions");
            const auto configuration=std::const_pointer_cast<ParseOptions>(parse->shared_from_this());
            const auto source=runtime.parseOptionsIdentities.find(configuration);
            if(!source)throw Exception("TypeError",u"NodeFactory configuration has no Context identity");
            return Value(*source);
        },
        [&runtime,owner,identity,executionLease](Value value){
            requireRuntimeActive(executionLease);
            auto payload=std::as_const(runtime.context()).payload(identity);
            auto* binding=std::any_cast<NativeBindingPayload<NodeFactory*>>(&payload);
            if(!binding)throw Exception("TypeError",u"NodeFactory instance payload is invalid");
            if(std::holds_alternative<Undefined>(value.data)){
                owner->configuration=Missing{};binding->lifetime=owner;runtime.context().setPayload(identity,std::move(payload));return;
            }
            if(std::holds_alternative<Null>(value.data)){
                owner->configuration=PropertyNull{};binding->lifetime=owner;runtime.context().setPayload(identity,std::move(payload));return;
            }
            const auto* object=std::get_if<ObjectRef>(&value.data);
            if(object){
                const auto* native=std::any_cast<NativeBindingPayload<ParseOptionsRef>>(
                    &std::as_const(runtime.context()).payload(*object));
                if(native&&native->identity){
                    const auto config=native->identity;
                    const auto canonical=runtime.parseOptionsIdentities.find(config);
                    if(canonical&&*canonical==*object){
                        owner->configuration=config.get();
                        binding->lifetime=std::make_shared<NodeFactoryLifetime>(NodeFactoryLifetime{config,owner});
                        runtime.context().setPayload(identity,std::move(payload));return;
                    }
                }
            }
            // Source declares this as a live, untyped field. Preserve arbitrary
            // values and their Context identity; native parser use still checks
            // that the owner implements ParseOptions' addNode boundary.
            owner->configuration=std::move(value);
            binding->lifetime=owner;
            runtime.context().setPayload(identity,std::move(payload));
        },
        [owner,executionLease]{
            requireRuntimeActive(executionLease);
            owner->configuration=Missing{};
        });
}
Value childValue(ParserRuntime& runtime,const tree::ChildValue<mml::MmlNode>& child){
    if(child.kind()==tree::ChildKind::Undefined)return Value(Undefined{});
    if(child.kind()==tree::ChildKind::Null)return Value(Null{});
    const auto identity=runtime.mmlNodeIdentities.find(child.nodeOrNull());
    if(!identity)throw Exception("TypeError",u"MML child has no canonical Context identity");
    return Value(*identity);
}
std::optional<mml::NodeValue> mmlNodeValue(ParserRuntime& runtime,const Value& value){
    if(std::holds_alternative<Undefined>(value.data))return mml::NodeValue(Missing{});
    if(std::holds_alternative<Null>(value.data))return mml::NodeValue(PropertyNull{});
    if(const auto* object=std::get_if<ObjectRef>(&value.data)){
        if(const auto* binding=std::any_cast<NativeBindingPayload<mml::MmlNode*>>(&std::as_const(runtime.context()).payload(*object));
           binding&&binding->identity)return mml::NodeValue(binding->identity);
    }
    return std::nullopt;
}
Value nodeContextValue(ParserRuntime& runtime,mml::NodeValue node){
    if(!node.isNode())return node.kind()==tree::ChildKind::Null?Value(Null{}):Value(Undefined{});
    const auto identity=runtime.mmlNodeIdentities.find(node.requireNode());
    if(!identity)throw Exception("TypeError",u"MML node has no canonical Context identity");
    return Value(*identity);
}
ObjectRef bindChildren(ParserRuntime& runtime,mml::Children array){
    return runtime.bindMmlChildren(std::move(array));
}
template<class T,class Lookup,class Resolve>
void bindSharedField(Context& cx,ObjectRef identity,const Key& name,
                     const ParseOptionsRef& owner,ParserShared<T> ParseOptions::* member,
                     Lookup lookup,Resolve resolve){
    cx.bindData(identity,name,std::make_shared<NativeDataField>(owner,
        [owner,member,lookup]()->Value{
            const auto& current=owner.get()->*member;
            if(const auto* object=std::get_if<std::shared_ptr<T>>(&current);object&&*object){
                if(const auto result=lookup(*object))return Value(*result);
                throw Exception("TypeError",u"ParseOptions field owner has no canonical Context identity");
            }
            return std::holds_alternative<PropertyNull>(current)?Value(Null{}):Value(Undefined{});
        },
        [owner,member,resolve](Value value){
            if(std::holds_alternative<Undefined>(value.data)){owner.get()->*member=Missing{};return;}
            if(std::holds_alternative<Null>(value.data)){owner.get()->*member=PropertyNull{};return;}
            const auto* object=std::get_if<ObjectRef>(&value.data);
            if(!object)throw Exception("TypeError",u"ParseOptions field requires its canonical object");
            owner.get()->*member=resolve(*object);
        },
        [owner,member](){owner.get()->*member=Missing{};}));
}
}

ObjectRef bindNodeArray(ParserRuntime& runtime,mml::Children array){
    return bindChildren(runtime,std::move(array));
}
ParseOptions::ParseOptions(ParserRuntime& runtime):runtime_(runtime){}
ParserRuntime& ParseOptions::runtime()const{return runtime_;}
ColumnParser& ParseOptions::columnParserOwner()const{
    Context& cx=runtime_.context();
    const ObjectRef self=parseOptionsContextIdentity(runtime_,*this);
    const Value current=cx.get(Value(self),key(u"columnParser"));
    const auto* object=std::get_if<ObjectRef>(&current.data);
    if(!object)throw Exception("TypeError",u"this.columnParser is not a ColumnParser");
    return *runtime_.columnParserIdentities.resolve(*object);
}
NodeFactory& ParseOptions::nodeFactoryOwner()const{
    Context& cx=runtime_.context();
    const ObjectRef self=parseOptionsContextIdentity(runtime_,*this);
    const Value current=cx.get(Value(self),key(u"nodeFactory"));
    const auto* object=std::get_if<ObjectRef>(&current.data);
    if(!object)throw Exception("TypeError",u"this.nodeFactory is not a NodeFactory");
    return *runtime_.nodeFactoryIdentities.resolve(*object);
}
std::shared_ptr<NodeFactory> ParseOptions::nodeFactoryOwnerRef()const{
    return nodeFactoryOwner().shared_from_this();
}
core::MathItemReference ParseOptions::mathItemOwner()const{
    Context& cx=runtime_.context();
    const ObjectRef self=parseOptionsContextIdentity(runtime_,*this);
    const Value value=cx.get(Value(self),key(u"mathItem"));
    if(std::holds_alternative<Undefined>(value.data))return Missing{};
    if(std::holds_alternative<Null>(value.data))return PropertyNull{};
    const auto* object=std::get_if<ObjectRef>(&value.data);
    if(!object)throw Exception("TypeError",u"ParseOptions.mathItem must be a MathItem");
    const auto* native=std::any_cast<NativeObjectRef>(&std::as_const(cx).payload(*object));
    const NativeObjectRef owner=native?*native:NativeObjectRef{};
    auto* item=owner?dynamic_cast<core::MathItem*>(owner.get()):nullptr;
    const auto* source=item?dynamic_cast<const core::MathItemSourceIdentity*>(item):nullptr;
    if(!item||!source||source->sourceIdentity()!=*object||&source->sourceContext()!=&cx)
        throw Exception("TypeError",u"ParseOptions.mathItem must be a canonical MathItem in this Context");
    return item;
}
void ParseOptions::setMathItem(core::MathItem* item){
    const auto* source=item?dynamic_cast<const core::MathItemSourceIdentity*>(item):nullptr;
    if(item&&(!source||&source->sourceContext()!=&runtime_.context()))
        throw Exception("TypeError",u"ParseOptions.mathItem must be a canonical MathItem in this Context");
    mathItem=item?core::MathItemReference(item):core::MathItemReference(PropertyNull{});
    const ObjectRef self=parseOptionsContextIdentity(runtime_,*this);
    runtime_.context().set(Value(self),key(u"mathItem"),item?Value(source->sourceIdentity()):Value(Null{}));
}
void ParseOptions::setTags(TagsRef value){
    tags=std::move(value);
    const ObjectRef self=parseOptionsContextIdentity(runtime_,*this);
    Context& cx=runtime_.context();
    if(cx.hasOwn(Value(self),key(u"tags")))return;
    const auto owner=shared_from_this();
    bindSharedField(cx,self,key(u"tags"),owner,&ParseOptions::tags,
        [&runtime=runtime_](const TagsRef& current){return runtime.tagIdentities.find(current);},
        [&runtime=runtime_](ObjectRef object){return runtime.tagIdentities.resolve(object);});
}
SourceStringMap<Value>& ParseOptions::getVariant(ParserRuntime& runtime){return runtime.variantMap;}

void ParseOptions::initializeModule(ParserRuntime& runtime){
    Context& cx=runtime.context();
    if(std::holds_alternative<Undefined>(runtime.nodeFactoryClass.data)){
        const auto executionLease=runtime.executionLifetime();
        const ObjectRef constructor=cx.nativeConstructor(u"NodeFactory",
            [&runtime,executionLease](Context& context,std::span<const Value> arguments)->ObjectRef{
                requireRuntimeActive(executionLease);
                (void)arguments;
                auto owner=NodeFactory::make();
                const ObjectRef ctor=std::get<ObjectRef>(runtime.nodeFactoryClass.data);
                const ObjectRef identity=runtime.nodeFactoryIdentities.instance(
                    owner.get(),ctor,owner);
                bindNodeFactoryContextExecution(runtime,owner);
                const auto mmlFactoryField=std::make_shared<NativeDataField>(owner,
                    [&runtime,owner,executionLease]()->Value{
                        requireRuntimeActive(executionLease);
                        const auto& current=owner->mmlFactoryReference();
                        if(const auto* native=std::get_if<std::shared_ptr<mml::MmlFactory>>(&current);native&&*native){
                            const auto identity=runtime.mmlFactoryIdentities.find(native->get());
                            if(!identity)throw Exception("TypeError",u"NodeFactory MmlFactory has no canonical Context identity");
                            return Value(*identity);
                        }
                        if(const auto* native=std::get_if<mml::Factory*>(&current);native&&*native){
                            auto* factory=dynamic_cast<mml::MmlFactory*>(*native);
                            if(!factory)throw Exception("TypeError",u"NodeFactory.mmlFactory is not an MmlFactory");
                            const auto identity=runtime.mmlFactoryIdentities.find(factory);
                            if(!identity)throw Exception("TypeError",u"NodeFactory MmlFactory has no canonical Context identity");
                            return Value(*identity);
                        }
                        return std::holds_alternative<PropertyNull>(current)?Value(Null{}):Value(Undefined{});
                    },
                    [&runtime,owner,executionLease](Value value){
                        requireRuntimeActive(executionLease);
                        if(std::holds_alternative<Undefined>(value.data)){owner->setMmlFactory(Missing{});return;}
                        if(std::holds_alternative<Null>(value.data)){owner->setMmlFactory(PropertyNull{});return;}
                        const auto* object=std::get_if<ObjectRef>(&value.data);
                        if(!object)throw Exception("TypeError",u"NodeFactory.mmlFactory must be an MmlFactory");
                        owner->setMmlFactory(runtime.resolveMmlFactory(*object));
                    });
                context.bindData(identity,key(u"mmlFactory"),mmlFactoryField);
                const auto factoryField=std::make_shared<NativeDataField>(owner,
                    [&runtime,owner,executionLease](){requireRuntimeActive(executionLease);return runtime.properties.to(owner->factoryProperty());},
                    [&runtime,owner,executionLease](Value value){requireRuntimeActive(executionLease);owner->setFactoryProperty(runtime.properties.from(std::move(value)));});
                context.bindData(identity,key(u"factory"),factoryField);
                context.declareNativeData(identity,key(u"configuration"),
                    nodeFactoryConfigurationField(runtime,owner,identity));
                return identity;
            });
        cx.define(constructor,key(u"length"),Descriptor::data(Value(0),false,false,true));
        runtime.nodeFactoryClass=Value(constructor);
        const ObjectRef prototype=std::get<ObjectRef>(cx.get(runtime.nodeFactoryClass,key(u"prototype")).data);
        auto method=[&](std::u16string name,double length,
                        std::function<Value(Context&,Value,std::span<const Value>)> call){
            const ObjectRef function=cx.function(u"",std::move(call));
            cx.define(function,key(u"length"),Descriptor::data(Value(length),false,false,true));
            cx.define(prototype,key(name),Descriptor::data(Value(function),true,true,true));
            return function;
        };
        method(u"setMmlFactory",1,[&runtime,executionLease](Context&,Value receiver,std::span<const Value> arguments){
            requireRuntimeActive(executionLease);
            NodeFactory& factory=nodeFactoryReceiver(runtime,receiver);
            const Value value=argument(arguments,0);
            if(std::holds_alternative<Undefined>(value.data))factory.setMmlFactory(Missing{});
            else if(std::holds_alternative<Null>(value.data))factory.setMmlFactory(PropertyNull{});
            else if(const auto* object=std::get_if<ObjectRef>(&value.data))factory.setMmlFactory(runtime.resolveMmlFactory(*object));
            else throw Exception("TypeError",u"NodeFactory.setMmlFactory requires an MmlFactory");
            return Value(Undefined{});
        });
        method(u"set",2,[&runtime,executionLease](Context&,Value receiver,std::span<const Value> arguments){
            requireRuntimeActive(executionLease);
            NodeFactory& factory=nodeFactoryReceiver(runtime,receiver);
            const std::string name=property_host::debugText(property_host::string(runtime.properties.from(argument(arguments,0))));
            const Value value=argument(arguments,1);
            Property creator;
            creator=runtime.properties.from(value);
            factory.set(name,std::move(creator));
            return Value(Undefined{});
        });
        method(u"setCreators",1,[&runtime,executionLease](Context&,Value receiver,std::span<const Value> arguments){
            requireRuntimeActive(executionLease);
            NodeFactory& factory=nodeFactoryReceiver(runtime,receiver);
            factory.setCreators(runtime.context(),runtime.properties,argument(arguments,0));
            return Value(Undefined{});
        });
        method(u"create",1,[&runtime,executionLease](Context&,Value receiver,std::span<const Value> arguments){
            requireRuntimeActive(executionLease);
            if(arguments.empty())throw Exception("TypeError",u"NodeFactory.create requires a kind");
            NodeFactory& factory=nodeFactoryReceiver(runtime,receiver);
            const Property kindValue=runtime.properties.from(arguments[0]);
            std::vector<std::any> converted;converted.reserve(arguments.size()-1);
            for(std::size_t i=1;i<arguments.size();++i)converted.push_back(nodeFactoryArgument(runtime,arguments[i]));
            const auto kindName=node_factory_host::kind(kindValue);
            return nodeFactoryResult(runtime,factory.create(kindName,converted));
        });
        method(u"get",1,[&runtime,executionLease](Context&,Value receiver,std::span<const Value> arguments){
            requireRuntimeActive(executionLease);
            NodeFactory& factory=nodeFactoryReceiver(runtime,receiver);
            const std::string name=property_host::debugText(property_host::string(runtime.properties.from(argument(arguments,0))));
            const Property result=factory.get(name);
            if(const auto* function=std::get_if<FunctionRef>(&result.value);function&&*function){
                if(const auto identity=runtime.functions().find(*function))return Value(*identity);
                throw Exception("Unimplemented",u"NodeFactory function has no Context callable identity");
            }
            return runtime.properties.to(result);
        });
        bindDefaultNodeFactoryFunctions(runtime);
        // The upstream class exposes these exact retained functions on its
        // constructor. Reuse the default factory-table functions so identity,
        // replacement, and callable behavior stay on one owner.
        const std::array<std::u16string,4> staticNames{
            u"createNode",u"createToken",u"createText",u"createError"};
        const std::array<double,4> staticLengths{5,4,2,2};
        const auto staticFunctions=NodeFactory::defaultFunctions();
        for(std::size_t index=0;index<staticFunctions.size();++index){
            const auto identity=runtime.functions().find(staticFunctions[index]);
            if(!identity)throw Exception("TypeError",u"NodeFactory static method has no callable identity");
            cx.define(*identity,key(u"length"),Descriptor::data(Value(staticLengths[index]),false,false,true));
            cx.define(constructor,key(staticNames[index]),Descriptor::data(Value(*identity),true,true,true));
        }
    }
    if(!std::holds_alternative<Undefined>(runtime.parseOptionsClass.data))
        throw Exception("TypeError",u"ParseOptions module already initialized");
    const auto constructor=cx.nativeConstructor(u"ParseOptions",
        [&runtime](Context&,std::span<const Value> arguments)->ObjectRef{
            const auto configValue=argument(arguments,0);
            const auto* object=std::get_if<ObjectRef>(&configValue.data);
            if(!object)throw Exception("TypeError",u"ParseOptions requires its actual ParserConfiguration");
            const auto config=runtime.parserConfigurationIdentities.resolve(*object);
            const auto result=ParseOptions::make(runtime,*config,argument(arguments,1));
            return *runtime.parseOptionsIdentities.find(result);
        });
    cx.define(constructor,key(u"length"),Descriptor::data(Value(2),false,false,true));
    runtime.parseOptionsClass=constructor;
    const ObjectRef prototype=std::get<ObjectRef>(cx.get(Value(constructor),key(u"prototype")).data);
    const ObjectRef pushParser=cx.function(u"",[](Context& context,Value receiver,std::span<const Value> arguments){
        const Value parsers=context.get(receiver,key(u"parsers"));
        const Value unshift=context.get(parsers,key(u"unshift"));
        const auto* method=std::get_if<ObjectRef>(&unshift.data);
        if(!method||context.kind(*method)!=ObjectKind::function)
            throw Exception("TypeError",u"this.parsers.unshift is not a function");
        const std::array<Value,1> values{argument(arguments,0)};
        return context.call(*method,parsers,values);
    });
    cx.define(pushParser,key(u"length"),Descriptor::data(Value(1),false,false,true));
    cx.define(prototype,key(u"pushParser"),Descriptor::data(Value(pushParser),true,true,true));
    const ObjectRef popParser=cx.function(u"",[](Context& context,Value receiver,std::span<const Value>){
        const Value parsers=context.get(receiver,key(u"parsers"));
        const Value shift=context.get(parsers,key(u"shift"));
        const auto* method=std::get_if<ObjectRef>(&shift.data);
        if(!method||context.kind(*method)!=ObjectKind::function)
            throw Exception("TypeError",u"this.parsers.shift is not a function");
        return context.call(*method,parsers,{});
    });
    cx.define(popParser,key(u"length"),Descriptor::data(Value(0),false,false,true));
    cx.define(prototype,key(u"popParser"),Descriptor::data(Value(popParser),true,true,true));
    const ObjectRef clear=cx.function(u"",[](Context& context,Value receiver,std::span<const Value>){
        clearParseOptions(context,std::move(receiver));
        return Value(Undefined{});
    });
    cx.define(clear,key(u"length"),Descriptor::data(Value(0),false,false,true));
    cx.define(prototype,key(u"clear"),Descriptor::data(Value(clear),true,true,true));
    const ObjectRef parserGetter=cx.function(u"get",[](Context& context,Value receiver,std::span<const Value>){
        const Value parsers=context.get(receiver,key(u"parsers"));
        return context.get(parsers,key(u"0"));
    });
    cx.define(parserGetter,key(u"length"),Descriptor::data(Value(0),false,false,true));
    cx.define(prototype,key(u"parser"),Descriptor::access(parserGetter,std::nullopt,false,true));

    // Source callbacks belong to one loaded module, not to each formula.
    for(const auto& name:{u"TeX",u"ISO",u"French",u"upright"}){
        const std::u16string style(name);
        const auto callback=cx.function(u"",[&runtime,style](Context&,Value,std::span<const Value> arguments)->Value{
            if(style==u"ISO")return Value(variant("ITALIC"));
            if(style==u"upright")return Value(variant("NORMAL"));
            if(style==u"TeX"&&!truthy(argument(arguments,1)))return Value(u"");
            const auto input=argument(arguments,0);
            const auto* text=std::get_if<std::u16string>(&input.data);
            if(!text)throw Exception("TypeError",u"Math style requires a string");
            if(style==u"French"){
                const auto normalized=runtime.unicode().nfd(*text);
                return Value(variant(!normalized.empty()&&normalized[0]>=u'a'&&normalized[0]<=u'z'?"ITALIC":"NORMAL"));
            }
            return Value(!text->empty()&&(((*text)[0]>=0x0391&&(*text)[0]<=0x03A9)||(*text)[0]==0x03F4)?variant("NORMAL"):u"");
        });
        cx.define(callback,key(u"length"),Descriptor::data(Value(style==u"TeX"?2:1),false,false,true));
        runtime.variantMap.set(style,callback);
    }
    cx.define(constructor,key(u"getVariant"),
        Descriptor::data(Value(runtime.maps.bind(runtime.variantMap.storage())),true,true,true));
}

ParseOptionsRef ParseOptions::make(ParserRuntime& runtime,ParserConfiguration& configuration,Value definitions){
    const auto* constructor=std::get_if<ObjectRef>(&runtime.parseOptionsClass.data);
    if(!constructor)throw Exception("Unimplemented",u"ParseOptions requires initialized module metadata");
    auto result=std::make_shared<ParseOptions>(runtime);
    runtime.parseOptionsIdentities.instance(result,*constructor);
    result->initializeParseOptions(configuration,std::move(definitions));
    return result;
}

void ParseOptions::initializeParseOptions(ParserConfiguration& configuration,Value definitions){
    Context& cx=runtime_.context();
    options=cx.object();
    const auto actualOwner=shared_from_this();
    const auto sourceIdentity=runtime_.parseOptionsIdentities.find(actualOwner);
    if(!sourceIdentity)throw Exception("TypeError",u"ParseOptions has no canonical Context identity");
    bindNativeValue(cx,*sourceIdentity,key(u"options"),actualOwner,&ParseOptions::options);
    auto columnParserOwner=std::make_shared<ColumnParser>(runtime_);
    columnParserOwner->initializeColumnParser(columnParserOwner);
    const auto columnParserIdentity=columnParserOwner->sourceIdentity();
    columnParser=Value(columnParserIdentity);
    bindNativeValue(cx,*sourceIdentity,key(u"columnParser"),actualOwner,&ParseOptions::columnParser);
    packageData=std::make_shared<SourceStringMap<Value>>();
    const auto packageDataField=std::make_shared<NativeDataField>(actualOwner,
        [this]()->Value{
            if(const auto* owner=std::get_if<std::shared_ptr<SourceStringMap<Value>>>(&packageData);owner&&*owner)
                return Value(runtime_.maps.bind((*owner)->storage()));
            if(std::holds_alternative<PropertyNull>(packageData))return Value(Null{});
            return Value(Undefined{});
        },
        [this](Value value){
            if(std::holds_alternative<Undefined>(value.data)){packageData=Missing{};return;}
            if(std::holds_alternative<Null>(value.data)){packageData=PropertyNull{};return;}
            const auto* object=std::get_if<ObjectRef>(&value.data);
            if(!object)throw Exception("TypeError",u"ParseOptions.packageData must be a Map");
            auto* current=std::get_if<std::shared_ptr<SourceStringMap<Value>>>(&packageData);
            if(!current||!*current)packageData=std::make_shared<SourceStringMap<Value>>();
            current=std::get_if<std::shared_ptr<SourceStringMap<Value>>>(&packageData);
            (*current)->useStorage(runtime_.maps.resolve(*object));
        },
        [this]{packageData=Missing{};});
    cx.bindData(*sourceIdentity,key(u"packageData"),packageDataField);
    parsers=cx.array();
    bindNativeValue(cx,*sourceIdentity,key(u"parsers"),actualOwner,&ParseOptions::parsers);
    const auto rootField=std::make_shared<NativeDataField>(actualOwner,
        [this]()->Value{
            if(root.isNode()){
                const auto identity=runtime_.mmlNodeIdentities.find(root.requireNode());
                if(identity)return Value(*identity);
            }
            return root.kind()==tree::ChildKind::Null?Value(Null{}):Value(Undefined{});
        },
        [this](Value value){
            if(std::holds_alternative<Undefined>(value.data)){root=Missing{};return;}
            if(std::holds_alternative<Null>(value.data)){root=PropertyNull{};return;}
            const auto* object=std::get_if<ObjectRef>(&value.data);
            if(!object)throw Exception("TypeError",u"ParseOptions.root must be an MML node");
            root=&runtime_.resolveMmlNode(*object);
        },[this]{root=Missing{};});
    cx.bindData(*sourceIdentity,key(u"root"),rootField);
    root=PropertyNull{};
    nodeLists=cx.object();
    bindNativeValue(cx,*sourceIdentity,key(u"nodeLists"),actualOwner,&ParseOptions::nodeLists);
    const auto errorField=std::make_shared<NativeDataField>(actualOwner,
        [this](){return Value(error);},
        [this](Value value){error=options::truthy(value);},
        [this]{error=false;});
    cx.bindData(*sourceIdentity,key(u"error"),errorField);
    error=false;
    const Value handlerValue=configuration.sourceField(u"handlers");
    const auto* handlerObject=std::get_if<ObjectRef>(&handlerValue.data);
    if(!handlerObject)throw Exception("TypeError",u"Configuration handlers are not an actual SubHandlers");
    const auto* handlerOwner=std::any_cast<std::shared_ptr<SubHandlers>>(&std::as_const(cx).payload(*handlerObject));
    if(!handlerOwner||!*handlerOwner)throw Exception("Unimplemented",u"Configuration handlers have no native SubHandlers owner");
    handlers=*handlerOwner;
    (void)runtime_.bindSubHandlers(*handlerOwner,*handlerObject);
    const auto handlersField=std::make_shared<NativeDataField>(actualOwner,
        [this](){
            const auto* current=std::get_if<std::shared_ptr<SubHandlers>>(&handlers);
            if(current&&*current){const auto identity=runtime_.findSubHandlers(current->get());if(identity)return Value(*identity);}
            return std::holds_alternative<PropertyNull>(handlers)?Value(Null{}):Value(Undefined{});
        },
        [this](Value value){
            if(std::holds_alternative<Undefined>(value.data)){handlers=Missing{};return;}
            if(std::holds_alternative<Null>(value.data)){handlers=PropertyNull{};return;}
            const auto* object=std::get_if<ObjectRef>(&value.data);
            if(!object)throw Exception("TypeError",u"ParseOptions.handlers must be a SubHandlers object");
            handlers=runtime_.resolveSubHandlers(*object);
        },[this]{handlers=Missing{};});
    cx.bindData(*sourceIdentity,key(u"handlers"),handlersField);
    nodeFactory=Value(cx.construct(std::get<ObjectRef>(runtime_.nodeFactoryClass.data),{}));
    const auto* factoryIdentity=std::get_if<ObjectRef>(&nodeFactory.data);
    if(!factoryIdentity)throw Exception("TypeError",u"NodeFactory construction did not return an instance");
    bindNativeValue(cx,*sourceIdentity,key(u"nodeFactory"),actualOwner,&ParseOptions::nodeFactory);
    const auto* factoryBinding=std::any_cast<NativeBindingPayload<NodeFactory*>>(&std::as_const(cx).payload(*factoryIdentity));
    if(!factoryBinding||!factoryBinding->identity||!factoryBinding->lifetime)
        throw Exception("TypeError",u"NodeFactory instance has no retained native owner");
    const auto factoryOwner=std::shared_ptr<NodeFactory>(factoryBinding->lifetime,factoryBinding->identity);
    cx.assignNativeData(*factoryIdentity,key(u"configuration"),
        nodeFactoryConfigurationField(runtime_,factoryOwner,*factoryIdentity),Value(*sourceIdentity));
    const Value setCreators=cx.get(nodeFactory,key(u"setCreators"));
    const auto* setCreatorsFunction=std::get_if<ObjectRef>(&setCreators.data);
    if(!setCreatorsFunction||cx.kind(*setCreatorsFunction)!=ObjectKind::function)
        throw Exception("TypeError",u"this.nodeFactory.setCreators is not a function");
    const std::array<Value,1> creatorArguments{configuration.sourceField(u"nodes")};
    (void)cx.call(*setCreatorsFunction,nodeFactory,creatorArguments);
    const Value itemDefinitions=configuration.sourceField(u"items");
    tree::FactoryClassMap<BaseItem> classes;
    for(const auto& name:cx.enumerableStringKeys(itemDefinitions)){
        const auto constructorValue=cx.get(itemDefinitions,key(name));
        const auto* constructor=std::get_if<ObjectRef>(&constructorValue.data);
        if(!constructor)throw Exception("TypeError",u"Stack item class is not a constructor");
        classes.emplace_back(property_host::debugText(name),runtime_.itemClassIdentities.resolve(*constructor));
    }
    itemFactory=std::make_shared<StackItemFactory>(std::move(classes));
    shared(itemFactory,u"itemFactory")->configuration=this;
    (void)StackItemFactoryBindings::bindActualFactory(runtime_,shared(itemFactory,u"itemFactory"),*sourceIdentity);
    bindSharedField(cx,*sourceIdentity,key(u"itemFactory"),actualOwner,&ParseOptions::itemFactory,
        [&runtime=runtime_](const std::shared_ptr<StackItemFactory>& factory){return runtime.stackItemFactoryIdentities.find(factory.get());},
        [&runtime=runtime_](ObjectRef object){return runtime.resolveStackItemFactory(object);});
    if(std::holds_alternative<Undefined>(definitions.data))definitions=cx.array();
    // Spread arguments are captured before defaultOptions runs; the options
    // records themselves remain the original live Context identities.
    std::vector<Value> records;
    cx.forEachIterable(definitions,[&](Value value){records.push_back(std::move(value));});
    (void)options::defaultOptions(cx,options,records);
    const std::array<Value,1> configurationDefaults{configuration.sourceField(u"options")};
    (void)options::defaultOptions(cx,options,configurationDefaults);
    const auto style=cx.get(options,key(u"mathStyle"));
    std::optional<Value> selected;
    if(const auto* name=std::get_if<std::u16string>(&style.data))selected=runtime_.variantMap.get(*name);
    if(!selected||!truthy(*selected))selected=runtime_.variantMap.get(u"TeX");
    mathStyle=selected?*selected:Value(Undefined{});
    bindNativeValue(cx,*sourceIdentity,key(u"mathStyle"),actualOwner,&ParseOptions::mathStyle);
}

void ParseOptions::pushParser(TexParserRef parser){
    Context& cx=runtime_.context();
    if(!parser)throw Exception("TypeError",u"ParseOptions.pushParser requires a parser");
    const auto identity=runtime_.parserIdentities.find(parser);
    if(!identity)throw Exception("TypeError",u"ParseOptions parser has no canonical Context identity");
    const ObjectRef self=parseOptionsContextIdentity(runtime_,*this);
    const Value method=cx.get(Value(self),key(u"pushParser"));
    const auto* callable=std::get_if<ObjectRef>(&method.data);
    if(!callable||cx.kind(*callable)!=ObjectKind::function)
        throw Exception("TypeError",u"this.pushParser is not a function");
    const std::array<Value,1> arguments{Value(*identity)};
    (void)cx.call(*callable,Value(self),arguments);
}
void ParseOptions::popParser(){
    Context& cx=runtime_.context();
    const ObjectRef self=parseOptionsContextIdentity(runtime_,*this);
    const Value method=cx.get(Value(self),key(u"popParser"));
    const auto* callable=std::get_if<ObjectRef>(&method.data);
    if(!callable||cx.kind(*callable)!=ObjectKind::function)
        throw Exception("TypeError",u"this.popParser is not a function");
    (void)cx.call(*callable,Value(self),{});
}
ParserShared<TexParser> ParseOptions::parser()const{
    Context& cx=runtime_.context();
    const ObjectRef self=parseOptionsContextIdentity(runtime_,*this);
    const Value value=cx.get(Value(self),key(u"parser"));
    if(std::holds_alternative<Undefined>(value.data))return Missing{};
    if(std::holds_alternative<Null>(value.data))return PropertyNull{};
    const auto* object=std::get_if<ObjectRef>(&value.data);
    if(!object)throw Exception("TypeError",u"this.parsers[0] is not a TexParser");
    return runtime_.parserIdentities.resolve(*object);
}
void ParseOptions::clear(){
    Context& context=runtime_.context();
    const ObjectRef self=parseOptionsContextIdentity(runtime_,*this);
    clearParseOptions(context,Value(self));
}
void ParseOptions::addNode(Property property,mml::NodeValue node){
    Context& cx=runtime_.context();
    const auto name=property_host::string(property);
    Value list=cx.get(nodeLists,key(name));
    if(!truthy(list)){
        list=cx.array();
        cx.set(nodeLists,key(name),list);
    }
    const Value push=cx.get(list,key(u"push"));
    const auto* pushFunction=std::get_if<ObjectRef>(&push.data);
    if(!pushFunction||cx.kind(*pushFunction)!=ObjectKind::function)
        throw Exception("TypeError",u"this.nodeLists[property].push is not a function");
    const std::array<Value,1> pushArguments{nodeContextValue(runtime_,node)};
    (void)cx.call(*pushFunction,list,pushArguments);
    if(Property(node.requireNode("kind")->kind())!=property){
        const auto previous=NodeUtil::instance().getProperty(node,"in-lists");
        std::u16string names;
        if(property_host::truthy(previous)){
            if(!std::holds_alternative<std::string>(previous.value))
                throw Exception("TypeError",u"in-lists.split is not a function");
            names=property_host::string(previous)+u",";
        }
        names+=name;
        NodeUtil::instance().setProperty(node,"in-lists",property_host::debugText(names));
    }
}
mml::Children ParseOptions::getList(std::u16string property){
    Context& cx=runtime_.context();
    Value input=cx.get(nodeLists,key(property));
    if(!truthy(input))input=Value(cx.array());
    auto result=std::make_shared<tree::ChildArray<mml::MmlNode>>();
    cx.forEachIterable(input,[&](Value value){
        const auto node=mmlNodeValue(runtime_,value);
        if(node&&inTree(*node))result->push_back(*node);
    });
    cx.set(nodeLists,key(property),bindChildren(runtime_,result));
    return result;
}
void ParseOptions::removeFromList(std::u16string property,mml::Children removed){
    Context& cx=runtime_.context();
    Value value=cx.get(nodeLists,key(property));
    if(!truthy(value))value=Value(cx.array());
    if(!removed)throw Exception("TypeError",u"Removed node list is null");
    for(std::size_t i=0;i<removed->size();++i){
        const auto node=childValue(runtime_,removed->get(i));
        const Value indexOf=cx.get(value,key(u"indexOf"));
        const auto* indexFunction=std::get_if<ObjectRef>(&indexOf.data);
        if(!indexFunction||cx.kind(*indexFunction)!=ObjectKind::function)
            throw Exception("TypeError",u"list.indexOf is not a function");
        const std::array<Value,1> indexArguments{node};
        const Value index=cx.call(*indexFunction,value,indexArguments);
        if(!(cx.toNumber(index)>=0))continue;
        const Value splice=cx.get(value,key(u"splice"));
        const auto* spliceFunction=std::get_if<ObjectRef>(&splice.data);
        if(!spliceFunction||cx.kind(*spliceFunction)!=ObjectKind::function)
            throw Exception("TypeError",u"list.splice is not a function");
        const std::array<Value,2> spliceArguments{index,Value(1)};
        (void)cx.call(*spliceFunction,value,spliceArguments);
    }
}
bool ParseOptions::inTree(mml::NodeValue node)const{
    while(node.isNode()&&node!=root)node=node.requireNode("parent")->getParent();
    return node.isNode();
}
TeXNodeFactoryReference ParseOptions::nodeFactoryReference()const{
    const ObjectRef self=parseOptionsContextIdentity(runtime_,*this);
    const Value value=runtime_.context().get(Value(self),key(u"nodeFactory"));
    if(std::holds_alternative<Null>(value.data))return PropertyNull{};
    if(std::holds_alternative<Undefined>(value.data))return Missing{};
    const auto* object=std::get_if<ObjectRef>(&value.data);
    if(!object)throw Exception("TypeError",u"this.nodeFactory is not a NodeFactory");
    return runtime_.nodeFactoryIdentities.resolve(*object);
}
}
