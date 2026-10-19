// Candidate Context class binding for pinned MathJax core/Tree/Node.js.
// Generic Tree records live in the original Context object. The same
// TreeNodeAlgorithms drive these prototypes and native AbstractNode<Product>.
export module folia.math.core.tree.node_context_classes;
export import folia.math.core.tree.node_algorithms;
export import folia.math.core.tree.node_context_bindings;
import folia.math.util.property_host;
import std;

export namespace folia::math::tree {
class TreeOwnedObjectHandoff {
    bool claimed_=false;
public:
    void claim()noexcept{claimed_=true;}
    bool claimed()const noexcept{return claimed_;}
};
using TreeOwnedObjectCreated=std::function<void(
    options::ObjectRef,options::Key,options::ObjectRef,TreeOwnedObjectHandoff&)>;
options::Key treeContextPropertyKey(options::Context&,ContextPropertyBindings&,options::Value);
Property treeContextCopyOwnEnumerable(options::Context&,ContextPropertyBindings&,options::Value);
std::string treeContextString(options::Context&,ContextPropertyBindings&,options::Value);

template<class Product> class TreeNodeContextClasses {
    using Value=options::Value;
    using Key=options::Key;
    using ObjectRef=options::ObjectRef;
    using Child=ChildValue<Product>;
    using Receiver=TreeNodeReceiver<Product>;
    using Algorithms=TreeNodeAlgorithms<Product>;
    using Empty=TreeEmptyNodeAlgorithms<Product>;
    struct State:std::enable_shared_from_this<State> {
        options::Context& context;
        ContextPropertyBindings& properties;
        TreeNodeContextBindings<Product>& values;
        std::weak_ptr<std::atomic_bool> lease;
        TreeOwnedObjectCreated ownedObjectCreated;
        ObjectRef nodeConstructor{},emptyConstructor{},nodePrototype{},emptyPrototype{};
        State(options::Context& cx,ContextPropertyBindings& p,
              TreeNodeContextBindings<Product>& v,std::weak_ptr<std::atomic_bool> lifetime,
              TreeOwnedObjectCreated observer)
            :context(cx),properties(p),values(v),lease(std::move(lifetime)),
             ownedObjectCreated(std::move(observer)) {
            if(&p.context()!=&cx)throw options::Exception("TypeError",u"Tree Context values belong to another Context");
        }
        void active()const {
            const auto live=lease.lock();
            if(!live||!live->load(std::memory_order_acquire))
                throw options::Exception("TypeError",u"Tree Context execution has expired");
        }
        static Key key(std::u16string_view value){return Key(std::u16string(value));}
        Value get(Value receiver,std::u16string_view name) {
            active();return context.get(receiver,key(name));
        }
        void set(Value receiver,std::u16string_view name,Value value) {
            active();context.set(receiver,key(name),std::move(value));
        }
        Value invoke(Value receiver,std::u16string_view name,std::span<const Value> args={}) {
            const auto method=get(receiver,name);
            const auto* function=std::get_if<ObjectRef>(&method.data);
            if(!function)throw options::Exception("TypeError",u"Tree member is not callable");
            return context.call(*function,receiver,args);
        }
        Value arg(std::span<const Value> args,std::size_t index)const {
            return index<args.size()?args[index]:Value(options::Undefined{});
        }
        Value source(const Child& child){return values.sourceValue(child);}
        Child child(Value raw){return values.childValue(std::move(raw));}
        typename Receiver::Children array(Value raw){return values.arrayView(std::move(raw));}
        Receiver receiver(Value identity){
            active();
            Receiver access;
            const auto self=this->shared_from_this();
            access.self=[self,identity]{return self->child(identity);};
            access.kind=[self,identity]{return self->properties.from(self->get(identity,u"kind"));};
            access.kindPrefix=[self,identity]{
                return treeContextString(self->context,self->properties,self->get(identity,u"kind"));
            };
            access.properties=[self,identity]{return self->properties.from(self->get(identity,u"properties"));};
            access.assignProperties=[self,identity](Property value){self->set(identity,u"properties",self->properties.to(value));};
            access.copyProperties=[self,identity]{
                return treeContextCopyOwnEnumerable(self->context,self->properties,self->get(identity,u"properties"));
            };
            access.setProperty=[self,identity](Property name,Property value){
                const auto record=self->get(identity,u"properties");
                const auto property=treeContextPropertyKey(self->context,self->properties,self->properties.to(name));
                self->context.set(record,property,self->properties.to(value));
            };
            access.getProperty=[self,identity](Property name){
                const auto record=self->get(identity,u"properties");
                const auto property=treeContextPropertyKey(self->context,self->properties,self->properties.to(name));
                return self->properties.from(self->context.get(record,property));
            };
            access.getPropertyNames=[self,identity]{
                std::vector<std::string> names;
                for(const auto& name:self->context.enumerableStringKeys(self->get(identity,u"properties")))
                    names.push_back(property_host::debugText(name));
                return names;
            };
            access.deleteProperty=[self,identity](Property name){
                const auto record=self->get(identity,u"properties");
                const auto* object=std::get_if<ObjectRef>(&record.data);
                if(!object)throw options::Exception("TypeError",u"Tree properties receiver is not an object");
                const auto property=treeContextPropertyKey(self->context,self->properties,self->properties.to(name));
                self->context.erase(*object,property);
            };
            access.children=[self,identity]{
                const auto raw=self->get(identity,u"childNodes");
                if(std::holds_alternative<options::Null>(raw.data)||
                   std::holds_alternative<options::Undefined>(raw.data))return typename Receiver::Children{};
                return self->array(raw);
            };
            access.copyChildren=[self,identity]{
                const auto raw=self->get(identity,u"childNodes");
                if(!options::truthy(raw))return typename Receiver::Children{};
                return self->array(raw);
            };
            access.assignChildren=[self,identity](typename Receiver::Children children){
                if(!children){self->set(identity,u"childNodes",options::Null{});return;}
                const auto raw=children->sourceValue();
                if(!raw)throw options::Exception("TypeError",u"Context Tree children require their exact Source array value");
                self->set(identity,u"childNodes",*raw);
            };
            access.newChildren=[self]{return self->array(Value(self->context.array(0)));};
            access.pushChild=[self](typename Receiver::Children children,Child next){
                if(!children)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'push')");
                const auto raw=children->sourceValue();
                if(!raw)throw options::Exception("TypeError",u"Context Tree children lost their Source array value");
                const std::array<Value,1> args{self->source(next)};
                (void)self->invoke(*raw,u"push",args);
            };
            access.indexOfRaw=[self](typename Receiver::Children children,Child target)->Property{
                if(!children)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'indexOf')");
                const auto raw=children->sourceValue();
                if(!raw)throw options::Exception("TypeError",u"Context Tree children lost their Source array value");
                const std::array<Value,1> args{self->source(target)};
                return self->properties.from(self->invoke(*raw,u"indexOf",args));
            };
            access.captureCreator=[self,identity]{
                const auto factory=self->get(identity,u"factory");
                const auto method=self->get(factory,u"create");
                return std::function<Child(Property)>([self,factory,method](Property kind){
                    const std::array<Value,1> args{self->properties.to(kind)};
                    const auto* function=std::get_if<ObjectRef>(&method.data);
                    if(!function)throw options::Exception("TypeError",u"Tree factory create is not callable");
                    return self->child(self->context.call(*function,factory,args));
                });
            };
            access.assignPropertiesOn=[self](Child target,Property value){
                self->set(self->source(target),u"properties",self->properties.to(value));
            };
            access.appendOn=[self](Child target,Child next){
                const std::array<Value,1> args{self->source(next)};
                return self->child(self->invoke(self->source(target),u"appendChild",args));
            };
            access.childIndex=[self,identity](Child target)->Property{
                const std::array<Value,1> args{self->source(target)};
                return self->properties.from(self->invoke(identity,u"childIndex",args));
            };
            access.writeChildAt=[self](typename Receiver::Children children,Property index,Child next){
                if(!children)throw options::Exception("TypeError",u"Cannot set properties of null (setting index)");
                const auto raw=children->sourceValue();
                if(!raw)throw options::Exception("TypeError",u"Context Tree children lost their Source array value");
                const auto key=treeContextPropertyKey(self->context,self->properties,self->properties.to(index));
                self->context.set(*raw,key,self->source(next));
            };
            access.spliceChildAt=[self](typename Receiver::Children children,Property index){
                if(!children)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'splice')");
                const auto raw=children->sourceValue();
                if(!raw)throw options::Exception("TypeError",u"Context Tree children lost their Source array value");
                const std::array<Value,2> args{self->properties.to(index),Value(1)};
                (void)self->invoke(*raw,u"splice",args);
            };
            access.factoryNodeIsKind=[self,identity](Property kind){
                const auto factory=self->get(identity,u"factory");
                const std::array<Value,2> args{identity,self->properties.to(kind)};
                return options::truthy(self->invoke(factory,u"nodeIsKind",args));
            };
            access.isKindOn=[self](Child target,Property kind){
                const std::array<Value,1> args{self->properties.to(kind)};
                return options::truthy(self->invoke(self->source(target),u"isKind",args));
            };
            access.walkOn=[self](Child target,const std::function<void(Child,const std::any&)>& visit,
                                 const std::any& data,const std::any& visitorIdentity){
                (void)self->values.nodeAccess(self->source(target))
                    ->walkTreeRaw(visit,data,visitorIdentity);
            };
            return access;
        }
        void initialize(Value receiver,std::span<const Value> arguments){
            active();
            const Value factory=arg(arguments,0);
            const Value suppliedProperties=arg(arguments,1);
            const Value suppliedChildren=arg(arguments,2);
            const Value inputProperties=std::holds_alternative<options::Undefined>(suppliedProperties.data)
                ?Value(context.object()):suppliedProperties;
            const Value inputChildren=std::holds_alternative<options::Undefined>(suppliedChildren.data)
                ?Value(context.array(0)):suppliedChildren;
            const auto self=this->shared_from_this();
            TreeConstructorReceiver owner;
            owner.writeOwnFields=[self,receiver,factory]{
                self->set(receiver,u"factory",factory);
                self->set(receiver,u"parent",options::Null{});
                const auto properties=self->context.object();
                if(self->ownedObjectCreated){
                    TreeOwnedObjectHandoff handoff;
                    try{self->ownedObjectCreated(
                        std::get<ObjectRef>(receiver.data),key(u"properties"),properties,handoff);}
                    catch(...){
                        // A claimed birth is now owned by the observer's retryable
                        // token. Otherwise Tree remains responsible for the orphan.
                        if(!handoff.claimed())try{self->context.retireObjectRecord(properties);}catch(...){}
                        throw;
                    }
                }
                self->set(receiver,u"properties",Value(properties));
                self->set(receiver,u"childNodes",Value(self->context.array(0)));
            };
            owner.propertyKeys=[self,inputProperties]{return self->context.enumerableStringKeys(inputProperties);};
            owner.propertyValue=[self,inputProperties](std::u16string_view name){
                return self->context.get(inputProperties,key(name));
            };
            owner.callSetProperty=[self,receiver](std::u16string_view name,Value value){
                const std::array<Value,2> args{Value(std::u16string(name)),std::move(value)};
                (void)self->invoke(receiver,u"setProperty",args);
            };
            owner.childLengthTruthy=[self,inputChildren]{
                return options::truthy(self->context.get(inputChildren,key(u"length")));
            };
            owner.callSetChildren=[self,receiver,inputChildren]{
                const std::array<Value,1> args{inputChildren};
                (void)self->invoke(receiver,u"setChildren",args);
            };
            Algorithms::initialize(owner);
        }
        template<class Fn> void method(ObjectRef prototype,std::u16string_view name,
                                      double length,Fn&& body){
            const auto self=this->shared_from_this();
            const auto function=context.function(u"",[self,body=std::forward<Fn>(body)](
                options::Context& cx,Value receiver,std::span<const Value> args)->Value{
                self->active();
                if(&cx!=&self->context)throw options::Exception("TypeError",u"Tree method belongs to another Context");
                return body(*self,receiver,args);
            });
            context.define(function,key(u"length"),options::Descriptor::data(Value(length),false,false,true));
            context.set(Value(prototype),key(name),Value(function));
        }
        void installNodeMethods(){
            auto& cx=context;
            const auto self=this->shared_from_this();
            const auto getter=cx.function(u"get",[self](options::Context&,Value,std::span<const Value>){
                self->active();return Value(u"unknown");
            });
            cx.define(getter,key(u"length"),options::Descriptor::data(Value(0),false,false,true));
            cx.define(nodePrototype,key(u"kind"),options::Descriptor::access(getter,std::nullopt,false,true));
            method(nodePrototype,u"setProperty",2,[](State& s,Value r,std::span<const Value> a){
                Algorithms::setProperty(s.receiver(r),s.properties.from(s.arg(a,0)),s.properties.from(s.arg(a,1)));
                return Value(options::Undefined{});
            });
            method(nodePrototype,u"getProperty",1,[](State& s,Value r,std::span<const Value> a){
                return s.properties.to(Algorithms::getProperty(s.receiver(r),s.properties.from(s.arg(a,0))));
            });
            method(nodePrototype,u"getPropertyNames",0,[](State& s,Value r,std::span<const Value>){
                const auto names=Algorithms::getPropertyNames(s.receiver(r));
                const auto result=s.context.array(static_cast<std::uint32_t>(names.size()));
                for(std::size_t i=0;i<names.size();++i)s.context.set(Value(result),Key(property_host::text(std::to_string(i))),
                    Value(property_host::text(names[i])));
                return Value(result);
            });
            method(nodePrototype,u"getAllProperties",0,[](State& s,Value r,std::span<const Value>){
                return s.properties.to(Algorithms::getAllProperties(s.receiver(r)));
            });
            method(nodePrototype,u"removeProperty",0,[](State& s,Value r,std::span<const Value> a){
                std::vector<Property> names;names.reserve(a.size());
                for(const auto& raw:a)names.push_back(s.properties.from(raw));
                Algorithms::removeProperty(s.receiver(r),std::span<const Property>(names));
                return Value(options::Undefined{});
            });
            method(nodePrototype,u"isKind",1,[](State& s,Value r,std::span<const Value> a){
                return Value(Algorithms::isKind(s.receiver(r),s.properties.from(s.arg(a,0))));
            });
            method(nodePrototype,u"setChildren",1,[](State& s,Value r,std::span<const Value> a){
                Algorithms::setChildren(s.receiver(r),s.array(s.arg(a,0)));return Value(options::Undefined{});
            });
            method(nodePrototype,u"appendChild",1,[](State& s,Value r,std::span<const Value> a){
                return s.source(Algorithms::appendChild(s.receiver(r),s.child(s.arg(a,0))));
            });
            method(nodePrototype,u"replaceChild",2,[](State& s,Value r,std::span<const Value> a){
                return s.source(Algorithms::replaceChild(s.receiver(r),s.child(s.arg(a,0)),s.child(s.arg(a,1))));
            });
            method(nodePrototype,u"removeChild",1,[](State& s,Value r,std::span<const Value> a){
                return s.source(Algorithms::removeChild(s.receiver(r),s.child(s.arg(a,0))));
            });
            method(nodePrototype,u"childIndex",1,[](State& s,Value r,std::span<const Value> a){
                return s.properties.to(Algorithms::childIndex(s.receiver(r),s.child(s.arg(a,0))));
            });
            method(nodePrototype,u"copy",0,[](State& s,Value r,std::span<const Value>){
                return s.source(Algorithms::copy(s.receiver(r)));
            });
            method(nodePrototype,u"findNodes",1,[](State& s,Value r,std::span<const Value> a){
                const auto nodes=Algorithms::findNodes(s.receiver(r),s.properties.from(s.arg(a,0)));
                const auto result=s.context.array(static_cast<std::uint32_t>(nodes.size()));
                for(std::size_t i=0;i<nodes.size();++i)s.context.set(Value(result),Key(property_host::text(std::to_string(i))),
                    s.source(nodes[i]));
                return Value(result);
            });
            method(nodePrototype,u"walkTree",2,[](State& s,Value r,std::span<const Value> a){
                const auto callback=s.arg(a,0),data=s.arg(a,1);
                auto owner=s.shared_from_this();
                Algorithms::walkTree(s.receiver(r),[owner,callback](Child node,const std::any& state){
                    const Value data=std::any_cast<Value>(state);
                    const std::array<Value,2> args{owner->source(node),data};
                    const auto* function=std::get_if<ObjectRef>(&callback.data);
                    if(!function)throw options::Exception("TypeError",u"Tree walk callback is not callable");
                    (void)owner->context.call(*function,options::Undefined{},args);
                },std::any(data),std::any(callback));
                return data;
            });
            method(nodePrototype,u"toString",0,[](State& s,Value r,std::span<const Value>){
                return Value(property_host::text(Algorithms::toString(s.receiver(r))));
            });
        }
        void installEmptyMethods(){
            method(emptyPrototype,u"setChildren",1,[](State& s,Value r,std::span<const Value> a){
                (void)a;Empty::setChildren(s.receiver(r),{});return Value(options::Undefined{});
            });
            method(emptyPrototype,u"appendChild",1,[](State& s,Value r,std::span<const Value> a){
                return s.source(Empty::appendChild(s.receiver(r),s.child(s.arg(a,0))));
            });
            method(emptyPrototype,u"replaceChild",2,[](State& s,Value r,std::span<const Value> a){
                return s.source(Empty::replaceChild(s.receiver(r),Child(Missing{}),s.child(s.arg(a,1))));
            });
            method(emptyPrototype,u"childIndex",1,[](State& s,Value r,std::span<const Value> a){
                (void)a;return s.properties.to(Empty::childIndex(s.receiver(r),Child(Missing{})));
            });
            method(emptyPrototype,u"walkTree",2,[](State& s,Value r,std::span<const Value> a){
                const auto callback=s.arg(a,0),data=s.arg(a,1);
                auto owner=s.shared_from_this();
                Empty::walkTree(s.receiver(r),[owner,callback](Child node,const std::any& state){
                    const Value data=std::any_cast<Value>(state);
                    const std::array<Value,2> args{owner->source(node),data};
                    const auto* function=std::get_if<ObjectRef>(&callback.data);
                    if(!function)throw options::Exception("TypeError",u"Tree walk callback is not callable");
                    (void)owner->context.call(*function,options::Undefined{},args);
                },std::any(data));
                return data;
            });
            method(emptyPrototype,u"toString",0,[](State& s,Value r,std::span<const Value>){
                return s.properties.to(Empty::toString(s.receiver(r)));
            });
        }
        void install(){
            active();
            nodePrototype=context.object();emptyPrototype=context.object();
            const auto self=this->shared_from_this();
            const auto make=[self](bool empty){
                const std::u16string label=empty?u"AbstractEmptyNode":u"AbstractNode";
                return self->context.nativeConstructor(label,
                    [self,empty](options::Context& cx,std::span<const Value> args)->ObjectRef{
                        self->active();
                        const auto result=cx.object();
                        cx.setPrototype(result,empty?self->emptyPrototype:self->nodePrototype);
                        self->initialize(Value(result),args);
                        return result;
                    },
                    [self,empty](options::Context& cx,Value receiver,std::span<const Value> args)->Value{
                        self->active();
                        if(&cx!=&self->context)throw options::Exception("TypeError",u"Tree constructor belongs to another Context");
                        self->initialize(receiver,args);
                        return empty?receiver:Value(options::Undefined{});
                    });
            };
            nodeConstructor=make(false);emptyConstructor=make(true);
            context.set(Value(nodeConstructor),key(u"prototype"),Value(nodePrototype));
            context.set(Value(emptyConstructor),key(u"prototype"),Value(emptyPrototype));
            context.setPrototype(emptyConstructor,nodeConstructor);
            context.setPrototype(emptyPrototype,nodePrototype);
            context.define(nodeConstructor,key(u"length"),options::Descriptor::data(Value(3),false,false,true));
            context.define(emptyConstructor,key(u"length"),options::Descriptor::data(Value(0),false,false,true));
            context.define(nodePrototype,key(u"constructor"),options::Descriptor::data(Value(nodeConstructor),true,false,true));
            context.define(emptyPrototype,key(u"constructor"),options::Descriptor::data(Value(emptyConstructor),true,false,true));
            installNodeMethods();installEmptyMethods();
        }
    };
    std::shared_ptr<State> state_;
public:
    TreeNodeContextClasses(options::Context& context,ContextPropertyBindings& properties,
        TreeNodeContextBindings<Product>& values,std::weak_ptr<std::atomic_bool> lease,
        TreeOwnedObjectCreated observer={})
        :state_(std::make_shared<State>(context,properties,values,std::move(lease),std::move(observer))){state_->install();}
    options::ObjectRef abstractNodeConstructor()const{state_->active();return state_->nodeConstructor;}
    options::ObjectRef abstractEmptyNodeConstructor()const{state_->active();return state_->emptyConstructor;}
    options::ObjectRef abstractNodePrototype()const{state_->active();return state_->nodePrototype;}
    options::ObjectRef abstractEmptyNodePrototype()const{state_->active();return state_->emptyPrototype;}
    // Build an operation-time view over this exact Context receiver. The
    // returned callbacks re-read its current descriptors and methods and do
    // not resolve/adopt a native Product or introduce another field owner.
    TreeNodeReceiver<Product> sourceReceiver(options::Value receiver)const{
        state_->active();return state_->receiver(std::move(receiver));
    }
};
}
