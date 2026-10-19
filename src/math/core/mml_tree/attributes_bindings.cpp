module folia.math.core.mml_tree.attributes_bindings;
import folia.math.util.property_host;
import std;

namespace folia::math {
using namespace options;
namespace {
Value arg(std::span<const Value> args,std::size_t index){
    return index<args.size()?args[index]:Value(Undefined{});
}
[[noreturn]] void typeError(std::u16string message){
    throw Exception("TypeError",std::move(message));
}
std::string name(ContextPropertyBindings& properties,Value value){
    return property_host::debugText(property_host::string(properties.from(std::move(value))));
}
Property namesArray(const std::vector<std::string>& names){
    PropertyArray result;
    for(const auto& item:names)result.push(Property(item));
    return Property(result);
}
}

struct AttributesContextBindings::State:std::enable_shared_from_this<State>{
    Context& context;
    OptionFunctionBindings& functions;
    ContextPropertyBindings& properties;
    std::weak_ptr<std::atomic_bool> executionLifetime;
    NativeIdentityBindings<std::shared_ptr<Attributes>> identities;
    ObjectRef ctor{},proto{};

    State(Context& context,OptionFunctionBindings& functions,
          ContextPropertyBindings& properties,std::weak_ptr<std::atomic_bool> executionLifetime)
        :context(context),functions(functions),properties(properties),
         executionLifetime(std::move(executionLifetime)),identities(context){
        if(&functions.context()!=&context||&properties.context()!=&context)
            typeError(u"Attributes bindings require one shared Context");
    }

    void requireActive()const{
        const auto lease=executionLifetime.lock();
        if(!lease||!lease->load())
            throw Exception("TypeError",u"Attributes Context binding execution has expired");
    }

    std::shared_ptr<Attributes> resolve(ObjectRef object)const{
        return identities.resolve(object);
    }

    PropertyList& layer(Attributes& owner,std::string_view field){
        if(field=="attributes")return owner.attributesLayer();
        if(field=="inherited")return owner.inheritedLayer();
        if(field=="defaults")return owner.defaultsLayer();
        return owner.globalLayer();
    }
    void installLayer(const std::shared_ptr<Attributes>& owner,ObjectRef object,
                      std::string field){
        const auto self=shared_from_this();
        auto cell=std::make_shared<NativeDataField>(owner,
            [self,owner,field]{
                self->requireActive();
                if(field=="global")return self->properties.to(owner->globalValue());
                return self->properties.to(Property(self->layer(*owner,field)));
            },
            [self,owner,field](Value value){
                self->requireActive();
                Property property=self->properties.from(std::move(value));
                if(field=="global"){
                    owner->replaceGlobal(std::move(property));
                    return;
                }
                auto replacement=PropertyList::record(property);
                if(!replacement)typeError(u"Attributes."+std::u16string(field.begin(),field.end())+
                                           u" must be assigned an ordinary PropertyList object");
                self->layer(*owner,field)=*replacement;
            });
        context.bindData(object,Key(std::u16string(field.begin(),field.end())),std::move(cell));
    }
    ObjectRef bind(const std::shared_ptr<Attributes>& owner){
        requireActive();
        if(!owner)typeError(u"Attributes owner is null");
        if(auto existing=identities.find(owner))return *existing;
        const auto object=identities.instance(owner,ctor);
        // The Source constructor assigns these own fields in this order.
        installLayer(owner,object,"global");
        installLayer(owner,object,"defaults");
        installLayer(owner,object,"inherited");
        installLayer(owner,object,"attributes");
        return object;
    }

    ObjectRef construct(std::span<const Value> args){
        requireActive();
        const Value rawDefaults=arg(args,0);
        const Value rawGlobal=arg(args,1);

        // Attributes.js first performs Object.create(global), before it reads
        // any property of defaults. Validate the Object.create operand before
        // converting or enumerating the defaults source.
        if(!std::holds_alternative<ObjectRef>(rawGlobal.data)&&
           !std::holds_alternative<Null>(rawGlobal.data))
            typeError(u"Attributes global must be an object or null");

        const Property global=properties.from(rawGlobal);
        // Construct the actual four-layer owner with an empty defaults layer.
        // Its constructor validates global and establishes all prototype links.
        auto owner=std::make_shared<Attributes>(PropertyList{},global);
        const ObjectRef instance=bind(owner);

        // Source assigns defaults only after global/defaults/inherited/
        // attributes have all been installed. Use Context's own-key,
        // descriptor, Get, and Set operations so getters, inherited setters,
        // strings, and nullish/boxed primitive sources follow Object.assign.
        if(!std::holds_alternative<Undefined>(rawDefaults.data)&&
           !std::holds_alternative<Null>(rawDefaults.data)){
            const Value target=context.get(Value(instance),Key(u"defaults"));
            const auto keys=context.ownKeys(rawDefaults);
            for(const auto& key:keys){
                if(!context.ownPropertyEnumerable(rawDefaults,key))continue;
                const Value value=context.get(rawDefaults,key);
                context.set(target,key,value);
            }
        }
        return instance;
    }
    std::shared_ptr<Attributes> receiver(Value receiver){
        auto* object=std::get_if<ObjectRef>(&receiver.data);
        if(!object)typeError(u"Attributes method called on incompatible receiver");
        try{return resolve(*object);}catch(const NativeIdentityResolutionFailure&){
            typeError(u"Attributes method called on incompatible receiver");
        }
    }
    void method(std::u16string methodName,std::uint32_t arity,NativeCall body){
        const auto self=shared_from_this();
        // The pinned compiled Source assigns anonymous function expressions
        // onto Attributes.prototype; this target does not infer their names.
        const auto function=context.function(u"",
            [self,body=std::move(body)](Context& context,Value receiver,std::span<const Value> args){
                self->requireActive();return body(context,std::move(receiver),args);
            });
        context.define(function,Key(u"length"),Descriptor::data(Value(static_cast<double>(arity)),false,false,true));
        // Pinned Attributes.js is emitted as prototype assignments, so these
        // own methods are enumerable and retain Source assignment order.
        context.define(proto,Key(std::move(methodName)),Descriptor::data(Value(function),true,true,true));
    }
    void initialize(){
        const auto self=shared_from_this();
        ctor=context.nativeConstructor(u"Attributes",
            [self](Context&,std::span<const Value> args){self->requireActive();return self->construct(args);});
        context.define(ctor,Key(u"length"),Descriptor::data(Value(2),false,false,true));
        proto=std::get<ObjectRef>(context.get(Value(ctor),Key(u"prototype")).data);
        // Function.prototype objects start with a non-enumerable constructor
        // property in Source; Context::constructor's generic helper is wider.
        context.define(proto,Key(u"constructor"),Descriptor::data(Value(ctor),true,false,true));
        context.define(ctor,Key(u"prototype"),Descriptor::data(Value(proto),true,false,false));

        method(u"set",2,[self](Context&,Value receiver,std::span<const Value> args){
            self->receiver(receiver)->set(name(self->properties,arg(args,0)),self->properties.from(arg(args,1)));
            return Value(Undefined{});
        });
        method(u"setList",1,[self](Context&,Value receiver,std::span<const Value> args){
            const auto value=arg(args,0);
            if(std::holds_alternative<Undefined>(value.data)||std::holds_alternative<Null>(value.data))
                return Value(Undefined{});
            PropertyList source;
            if(std::holds_alternative<std::u16string>(value.data)){
                const auto text=std::get<std::u16string>(value.data);
                for(std::size_t index=0;index<text.size();++index)
                    source.setOwn(std::to_string(index),Property(property_host::debugText(std::u16string_view(text).substr(index,1))));
            }else if(std::holds_alternative<ObjectRef>(value.data)){
                auto record=PropertyList::record(self->properties.from(value));
                if(!record)typeError(u"Attributes.setList source is not a record");
                source=*record;
            }
            self->receiver(receiver)->setList(source);
            return Value(Undefined{});
        });
        method(u"unset",1,[self](Context&,Value receiver,std::span<const Value> args){
            self->receiver(receiver)->unset(name(self->properties,arg(args,0)));
            return Value(Undefined{});
        });
        method(u"get",1,[self](Context&,Value receiver,std::span<const Value> args){
            return self->properties.to(self->receiver(receiver)->get(name(self->properties,arg(args,0))));
        });
        method(u"getExplicit",1,[self](Context&,Value receiver,std::span<const Value> args){
            return self->properties.to(self->receiver(receiver)->getExplicit(name(self->properties,arg(args,0))));
        });
        method(u"hasExplicit",1,[self](Context&,Value receiver,std::span<const Value> args){
            return Value(self->receiver(receiver)->hasExplicit(name(self->properties,arg(args,0))));
        });
        method(u"hasOneOf",1,[self](Context&,Value receiver,std::span<const Value> args){
            std::vector<std::string> names;
            self->context.forEachIterable(arg(args,0),[&](Value value){names.push_back(name(self->properties,std::move(value)));});
            return Value(self->receiver(receiver)->hasOneOf(names));
        });
        method(u"getList",0,[self](Context&,Value receiver,std::span<const Value> args){
            std::vector<std::string> names;names.reserve(args.size());
            for(const auto& value:args)names.push_back(name(self->properties,value));
            return self->properties.to(Property(self->receiver(receiver)->getList(names)));
        });
        method(u"setInherited",2,[self](Context&,Value receiver,std::span<const Value> args){
            self->receiver(receiver)->setInherited(name(self->properties,arg(args,0)),self->properties.from(arg(args,1)));
            return Value(Undefined{});
        });
        method(u"getInherited",1,[self](Context&,Value receiver,std::span<const Value> args){
            return self->properties.to(self->receiver(receiver)->getInherited(name(self->properties,arg(args,0))));
        });
        method(u"getDefault",1,[self](Context&,Value receiver,std::span<const Value> args){
            return self->properties.to(self->receiver(receiver)->getDefault(name(self->properties,arg(args,0))));
        });
        method(u"isSet",1,[self](Context&,Value receiver,std::span<const Value> args){
            return Value(self->receiver(receiver)->isSet(name(self->properties,arg(args,0))));
        });
        method(u"hasDefault",1,[self](Context&,Value receiver,std::span<const Value> args){
            return Value(self->receiver(receiver)->hasDefault(name(self->properties,arg(args,0))));
        });
        method(u"getExplicitNames",0,[self](Context&,Value receiver,std::span<const Value>){
            return self->properties.to(namesArray(self->receiver(receiver)->getExplicitNames()));
        });
        method(u"getInheritedNames",0,[self](Context&,Value receiver,std::span<const Value>){
            return self->properties.to(namesArray(self->receiver(receiver)->getInheritedNames()));
        });
        method(u"getDefaultNames",0,[self](Context&,Value receiver,std::span<const Value>){
            return self->properties.to(namesArray(self->receiver(receiver)->getDefaultNames()));
        });
        method(u"getGlobalNames",0,[self](Context&,Value receiver,std::span<const Value>){
            return self->properties.to(namesArray(self->receiver(receiver)->getGlobalNames()));
        });
        method(u"getAllAttributes",0,[self](Context&,Value receiver,std::span<const Value>){
            return self->properties.to(Property(self->receiver(receiver)->getAllAttributes()));
        });
        method(u"getAllInherited",0,[self](Context&,Value receiver,std::span<const Value>){
            return self->properties.to(Property(self->receiver(receiver)->getAllInherited()));
        });
        method(u"getAllDefaults",0,[self](Context&,Value receiver,std::span<const Value>){
            return self->properties.to(Property(self->receiver(receiver)->getAllDefaults()));
        });
        method(u"getAllGlobals",0,[self](Context&,Value receiver,std::span<const Value>){
            return self->properties.to(self->receiver(receiver)->getAllGlobalsValue());
        });
    }
};

AttributesContextBindings::AttributesContextBindings(Context& context,OptionFunctionBindings& functions,
    ContextPropertyBindings& properties,std::weak_ptr<std::atomic_bool> executionLifetime)
    :state_(std::make_shared<State>(context,functions,properties,std::move(executionLifetime))){
    state_->initialize();
}
AttributesContextBindings::~AttributesContextBindings()=default;
Context& AttributesContextBindings::context()const{return state_->context;}
ObjectRef AttributesContextBindings::constructor()const{return state_->ctor;}
ObjectRef AttributesContextBindings::prototype()const{return state_->proto;}
ObjectRef AttributesContextBindings::bind(const std::shared_ptr<Attributes>& owner){return state_->bind(owner);}
std::shared_ptr<Attributes> AttributesContextBindings::resolve(ObjectRef object)const{return state_->resolve(object);}
std::size_t AttributesContextBindings::canonicalHandleCountForDiagnostics()const noexcept{
    return state_->identities.canonicalHandleCount();
}
Value AttributesContextBindings::toValue(const Property& value)const{state_->requireActive();return state_->properties.to(value);}
Property AttributesContextBindings::fromValue(Value value){state_->requireActive();return state_->properties.from(std::move(value));}
std::function<Value(const Property&)> AttributesContextBindings::toValueConverter()const{
    const auto state=state_;return [state](const Property& value){state->requireActive();return state->properties.to(value);};
}
std::function<Property(Value)> AttributesContextBindings::fromValueConverter()const{
    const auto state=state_;return [state](Value value){state->requireActive();return state->properties.from(std::move(value));};
}
}
