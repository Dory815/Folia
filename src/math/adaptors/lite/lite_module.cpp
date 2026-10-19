module folia.math.adaptors.lite.lite_module;
import folia.math.adaptors.lite.source_text_content;
import folia.math.util.property_host;
import folia.math.util.js_number;
import folia.math.util.options;
import folia.math.util.regexp_binding;
import std;

namespace folia::math::adaptors::lite {
using namespace options;

namespace {
Key key(std::u16string_view name) { return Key(std::u16string(name)); }
std::optional<std::pair<std::u16string,std::u16string>> attributeSelector(std::u16string_view text) {
    if(text.size()<5||text.front()!=u'['||text.back()!=u']')return{};
    const auto equals=text.find(u"=\"");
    if(equals==std::u16string_view::npos||text.size()<equals+4||text[text.size()-2]!=u'\"')return{};
    return std::pair{std::u16string(text.substr(1,equals-1)),std::u16string(text.substr(equals+2,text.size()-equals-4))};
}
std::u16string primitiveString(Context& context, const Value& value) {
    if (const auto* p=std::get_if<std::u16string>(&value.data)) return *p;
    if (std::holds_alternative<Undefined>(value.data)) return u"undefined";
    if (std::holds_alternative<Null>(value.data)) return u"null";
    if (const auto* p=std::get_if<bool>(&value.data)) return *p?u"true":u"false";
    if (const auto* p=std::get_if<double>(&value.data)) return property_host::text(js_number::toString(*p));
    if (std::holds_alternative<SymbolRef>(value.data))
        return context.symbolString(std::get<SymbolRef>(value.data));
    if(const auto* object=std::get_if<ObjectRef>(&value.data)){
        // String(object) uses the ordinary string-hint ToPrimitive order.
        // Get and call are dynamic Context operations, so own accessors and
        // inherited user methods retain their Source receiver and exceptions.
        for(const auto name:{u"toString",u"valueOf"}){
            auto method=context.get(value,Key(name));
            const auto* callable=std::get_if<ObjectRef>(&method.data);
            if(!callable||context.kind(*callable)!=ObjectKind::function)continue;
            auto primitive=context.call(*callable,value);
            if(!std::holds_alternative<ObjectRef>(primitive.data))return primitiveString(context,primitive);
        }
        throw Exception("TypeError",u"Cannot convert object to primitive value");
    }
    throw Exception("TypeError",u"Cannot convert value to string");
}
std::u16string implicitString(Context& context,const Value& value){
    if(std::holds_alternative<SymbolRef>(value.data))
        throw Exception("TypeError",u"Cannot convert a Symbol value to a string");
    if(const auto* object=std::get_if<ObjectRef>(&value.data)){
        for(const auto name:{u"toString",u"valueOf"}){
            const Value method=context.get(value,key(name));
            const auto* callable=std::get_if<ObjectRef>(&method.data);
            if(!callable||context.kind(*callable)!=ObjectKind::function)continue;
            const Value primitive=context.call(*callable,value);
            if(std::holds_alternative<ObjectRef>(primitive.data))continue;
            if(std::holds_alternative<SymbolRef>(primitive.data))
                throw Exception("TypeError",u"Cannot convert a Symbol value to a string");
            return primitiveString(context,primitive);
        }
        (void)object;
        throw Exception("TypeError",u"Cannot convert object to primitive value");
    }
    return primitiveString(context,value);
}
Key propertyKey(Context& context,const Value& value){
    if(const auto* symbol=std::get_if<SymbolRef>(&value.data))return Key(*symbol);
    return key(implicitString(context,value));
}
Value missingToNull(const Property& value) {
    if (value.missing()) return Undefined{};
    if (std::holds_alternative<PropertyNull>(value.value)) return Null{};
    return Undefined{}; // Object-valued cases are converted by the session registry.
}
bool instanceOf(Context& context,const Value& value,ObjectRef constructor){
    return ordinaryInstanceOf(context,value,Value(constructor));
}

class LiteArrayView final : public NativeIndexedView {
    PropertyArray slots_;
    std::function<Value(const Property&)> to_;
    std::function<Property(Value)> from_;
    // Indexed descriptors carry attributes independently of their stored
    // values.  Data values stay solely in PropertyArray; this table is only
    // needed for non-default flags and accessors.
    std::unordered_map<std::uint32_t, Descriptor> descriptors_;
    bool lengthWritable_ = true;
public:
    LiteArrayView(PropertyArray slots,std::function<Value(const Property&)> to,
                  std::function<Property(Value)> from)
        :slots_(std::move(slots)),to_(std::move(to)),from_(std::move(from)){}
    std::uint32_t length()const override{return static_cast<std::uint32_t>(slots_.size());}
    Descriptor lengthDescriptor()const override{return Descriptor::data(Value(static_cast<double>(length())),lengthWritable_,false,false);}
    std::optional<Descriptor> indexDescriptor(std::uint32_t index)const override{
        const auto found=descriptors_.find(index);
        if(found!=descriptors_.end()){
            auto descriptor=found->second;
            if(!descriptor.accessor) descriptor.value=to_(slots_.get(index));
            return descriptor;
        }
        if(index>=slots_.size()||!slots_.has(index))return{};
        return Descriptor::data(to_(slots_.get(index)));
    }
    std::vector<std::uint32_t> ownIndexKeys()const override{
        std::vector<std::uint32_t> keys;
        for(std::uint32_t i=0;i<slots_.size();++i)if(slots_.has(i)||descriptors_.contains(i))keys.push_back(i);
        return keys;
    }
    void defineIndex(std::uint32_t index,Descriptor descriptor)override{
        if(index>=0xFFFFFFFFU)throw Exception("RangeError",u"Invalid array index");
        if(index>=slots_.size())slots_.resize(static_cast<std::size_t>(index)+1);
        if(descriptor.accessor){
            slots_.erase(index);
            descriptors_.insert_or_assign(index,std::move(descriptor));
            return;
        }
        slots_.set(index,from_(std::move(descriptor.value)));
        if(descriptor.writable&&descriptor.enumerable&&descriptor.configurable)descriptors_.erase(index);
        else {
            descriptor.value=Undefined{}; // data value is read from slots_
            descriptors_.insert_or_assign(index,std::move(descriptor));
        }
    }
    void defineLength(Descriptor descriptor)override{
        if(descriptor.accessor||descriptor.enumerable||descriptor.configurable)
            throw Exception("TypeError",u"Invalid array length descriptor");
        const auto requested=std::get<double>(descriptor.value.data);
        if(requested<0||requested>4294967295.0||std::trunc(requested)!=requested)
            throw Exception("RangeError",u"Invalid array length");
        const auto next=static_cast<std::uint32_t>(requested);
        const auto old=length();
        if(next<old){
            for(std::uint32_t index=old;index>next;){
                --index;
                const auto found=descriptors_.find(index);
                if(found!=descriptors_.end()&&!found->second.configurable){
                    slots_.resize(static_cast<std::size_t>(index)+1);
                    for(auto it=descriptors_.begin();it!=descriptors_.end();){
                        if(it->first>index)it=descriptors_.erase(it);else ++it;
                    }
                    if(!descriptor.writable)lengthWritable_=false;
                    const auto digits=std::to_string(index);
                    throw Exception("TypeError",u"Cannot delete property '"+
                        std::u16string(digits.begin(),digits.end())+u"' of [object Array]");
                }
                slots_.erase(index);
                descriptors_.erase(index);
            }
        }
        slots_.resize(next);
        for(auto it=descriptors_.begin();it!=descriptors_.end();){
            if(it->first>=next)it=descriptors_.erase(it);else ++it;
        }
        lengthWritable_=descriptor.writable;
    }
    void deleteIndex(std::uint32_t index)override{slots_.erase(index);descriptors_.erase(index);}
};

class ContextNodeProjection final : public LiteNodeContextProjection {
    std::function<Property(std::string_view)> get_;
    std::function<void(std::string_view,Property)> set_;
    std::function<void(std::string_view,Property)> defineOwn_;
    std::function<bool(std::string_view)> hasProperty_;
    std::function<bool(std::string_view)> has_;
    std::function<std::vector<std::string>()> keys_;
    std::function<void(std::string_view)> erase_;
    std::function<std::shared_ptr<styles::Styles>()> getStyles_;
    std::function<void(std::shared_ptr<styles::Styles>)> setStyles_;
public:
    ContextNodeProjection(
        std::function<Property(std::string_view)> get,
        std::function<void(std::string_view,Property)> set,
        std::function<void(std::string_view,Property)> defineOwn,
        std::function<bool(std::string_view)> hasProperty,
        std::function<bool(std::string_view)> has,
        std::function<std::vector<std::string>()> keys,
        std::function<void(std::string_view)> erase,
        std::function<std::shared_ptr<styles::Styles>()> getStyles,
        std::function<void(std::shared_ptr<styles::Styles>)> setStyles)
        :get_(std::move(get)),set_(std::move(set)),defineOwn_(std::move(defineOwn)),
         hasProperty_(std::move(hasProperty)),has_(std::move(has)),keys_(std::move(keys)),erase_(std::move(erase)),
         getStyles_(std::move(getStyles)),setStyles_(std::move(setStyles)){}
    Property getProperty(std::string_view field)const override{return get_(field);}
    void setProperty(std::string_view field,Property value)override{set_(field,std::move(value));}
    void defineOwnProperty(std::string_view field,Property value)override{defineOwn_(field,std::move(value));}
    bool hasProperty(std::string_view field)const override{return hasProperty_(field);}
    bool hasOwnProperty(std::string_view field)const override{return has_(field);}
    std::vector<std::string> keys()const override{return keys_();}
    void eraseProperty(std::string_view field)override{erase_(field);}
    std::shared_ptr<styles::Styles> getStyles()const override{return getStyles_();}
    void setStyles(std::shared_ptr<styles::Styles> value)override{setStyles_(std::move(value));}
};

}

struct StylesContextBindings::State : std::enable_shared_from_this<State> {
    Context& context;
    OptionFunctionBindings& functions;
    ContextPropertyBindings& properties;
    ObjectRef ctor{},proto{};
    std::vector<std::pair<std::shared_ptr<styles::Styles>,ObjectRef>> entries;
    State(Context& c,OptionFunctionBindings& f,ContextPropertyBindings& p):context(c),functions(f),properties(p){}
    std::shared_ptr<styles::Styles> resolve(ObjectRef object)const{
        for(const auto& [style,known]:entries)if(known==object)return style;
        throw Exception("TypeError",u"Receiver is not a Styles instance");
    }
    ObjectRef bind(const std::shared_ptr<styles::Styles>& style){
        if(!style)throw Exception("TypeError",u"Cannot bind a null Styles owner");
        for(const auto& [known,object]:entries)if(known.get()==style.get())return object;
        const auto object=context.instance(ctor,style);
        entries.emplace_back(style,object);return object;
    }
    Value method(std::u16string name,NativeCall call){
        const auto fn=context.function(std::move(name),std::move(call));return Value(fn);
    }
    void initialize(){
        std::weak_ptr<State> weak=shared_from_this();
        ctor=context.nativeConstructor(u"Styles",[weak](Context& cx,std::span<const Value> args)->ObjectRef{
            auto self=weak.lock();if(!self)throw Exception("TypeError",u"Styles module has expired");
            const auto text=args.empty()||std::holds_alternative<Undefined>(args[0].data)?std::u16string{}:primitiveString(cx,args[0]);
            auto style=std::make_shared<styles::Styles>(text);
            const auto object=cx.instance(self->ctor,style);self->entries.emplace_back(std::move(style),object);return object;
        });
        context.define(ctor,key(u"length"),Descriptor::data(Value(0),false,false,true));
        const auto value=context.get(Value(ctor),key(u"prototype"));proto=std::get<ObjectRef>(value.data);
        auto getter=[weak](std::u16string name,auto read){
            auto self=weak.lock();if(!self)throw Exception("TypeError",u"Styles module has expired");
            auto fn=self->context.function(std::move(name),[weak,read](Context& cx,Value receiver,std::span<const Value>)->Value{
                auto state=weak.lock();if(!state)throw Exception("TypeError",u"Styles module has expired");
                auto* object=std::get_if<ObjectRef>(&receiver.data);if(!object)throw Exception("TypeError",u"Styles accessor receiver is not an object");
                auto style=state->resolve(*object);return read(cx,state->properties,*style);
            });
            return fn;
        };
        auto css=getter(u"get cssText",[](Context&,ContextPropertyBindings&,styles::Styles& style)->Value{return style.cssText();});
        context.define(proto,key(u"cssText"),Descriptor::access(css,std::nullopt,false,true));
        auto list=getter(u"get styleList",[](Context&,ContextPropertyBindings& bindings,styles::Styles& style)->Value{
            return bindings.to(bindings.copyOwnEnumerableProperties(Property(style.styleList())));
        });
        context.define(proto,key(u"styleList"),Descriptor::access(list,std::nullopt,false,true));
        auto methodFn=[weak](std::u16string name,auto body){
            auto self=weak.lock();if(!self)throw Exception("TypeError",u"Styles module has expired");
            return self->context.function(std::move(name),[weak,body](Context& cx,Value receiver,std::span<const Value> args)->Value{
                auto state=weak.lock();if(!state)throw Exception("TypeError",u"Styles module has expired");
                auto* object=std::get_if<ObjectRef>(&receiver.data);if(!object)throw Exception("TypeError",u"Styles method receiver is not an object");
                auto style=state->resolve(*object);return body(cx,*style,args);
            });
        };
        context.define(proto,key(u"set"),Descriptor::data(Value(methodFn(u"set",[](Context& cx,styles::Styles& style,std::span<const Value> args)->Value{
            if(args.size()<2)throw Exception("TypeError",u"Styles.set requires name and value");
            const auto name=primitiveString(cx,args[0]);const auto val=primitiveString(cx,args[1]);style.set(name,styles::StyleValue(val));return Undefined{};
        })),true,true,true));
        context.define(proto,key(u"get"),Descriptor::data(Value(methodFn(u"get",[](Context& cx,styles::Styles& style,std::span<const Value> args)->Value{
            if(args.empty())throw Exception("TypeError",u"Styles.get requires a name");return style.get(primitiveString(cx,args[0]));
        })),true,true,true));
        context.define(proto,key(u"combineParent"),Descriptor::data(Value(methodFn(u"combineParent",[](Context& cx,styles::Styles& style,std::span<const Value> args)->Value{
            if(args.empty())throw Exception("TypeError",u"Styles.combineParent requires a name");style.combineParent(property_host::debugText(primitiveString(cx,args[0])));return Undefined{};
        })),true,true,true));
    }
};

StylesContextBindings::StylesContextBindings(Context& c,OptionFunctionBindings& f,ContextPropertyBindings& p){
    if(&f.context()!=&c)throw Exception("TypeError",u"Styles bindings require one Context");
    state_=std::make_shared<State>(c,f,p);state_->initialize();
}
StylesContextBindings::~StylesContextBindings()=default;
ObjectRef StylesContextBindings::constructor()const{return state_->ctor;}
ObjectRef StylesContextBindings::prototype()const{return state_->proto;}
ObjectRef StylesContextBindings::bind(const std::shared_ptr<styles::Styles>& style){return state_->bind(style);}
std::shared_ptr<styles::Styles> StylesContextBindings::resolve(ObjectRef object)const{return state_->resolve(object);}
std::shared_ptr<styles::Styles> StylesContextBindings::construct(std::span<const Value> args){
    const auto object=state_->context.construct(state_->ctor,args);return state_->resolve(object);
}

struct LiteModule::State : std::enable_shared_from_this<State> {
    struct EventRegistration {
        std::u16string type;
        Value callback;
        std::uint64_t sequence=0;
    };
    struct EventRegistrationOwner final : NativeAttachedOwner {
        std::vector<EventRegistration> registrations;
        void add(std::u16string type,Value callback,std::uint64_t sequence){
            registrations.push_back({std::move(type),std::move(callback),sequence});
        }
        void erase(std::uint64_t sequence){
            std::erase_if(registrations,[&](const EventRegistration& item){return item.sequence==sequence;});
        }
        void eraseSince(std::uint64_t first){
            std::erase_if(registrations,[&](const EventRegistration& item){return item.sequence>=first;});
        }
        bool empty()const noexcept{return registrations.empty();}
        bool visitContextEdges(const ContextEdgeVisitor& visit)const override{
            for(const auto& registration:registrations)visit(registration.callback);
            return true;
        }
    };
    Context& context;
    OptionFunctionBindings& functions;
    ContextPropertyBindings& properties;
    std::shared_ptr<LiteNodeStore> store;
    std::shared_ptr<entities::EntityModule> entities;
    StylesContextBindings styles;
    std::shared_ptr<regexp_host::RegExpBindings> regexp;
    std::array<ObjectRef,6> classes{}; // Element, Text, Comment, Document, List, Window.
    ObjectRef parserClass{};
    ObjectRef liteBaseClass{};
    ObjectRef nodeAdaptorClass{};
    ObjectRef adaptorClass{};
    std::vector<std::pair<LiteNodeRef,ObjectRef>> nodes;
    struct ArrayProjection { NodeId ownerNode=0; ObjectRef identity{}; };
    // Exact native array projections created while binding a particular Lite
    // node. The node creation id, rather than projection time, determines its
    // request lifetime (an old node can be projected lazily in a later request).
    std::vector<ArrayProjection> arrayProjections;
    std::unordered_map<NodeId,std::shared_ptr<LiteNodeContextProjection>> contextProjections;
    std::vector<std::pair<std::shared_ptr<LiteParser>,ObjectRef>> parsers;
    std::vector<std::pair<std::shared_ptr<LiteBase>,ObjectRef>> adaptors;
    LiteModule::AppendTarget appendTarget;
    LiteModule::AttributeTarget attributeTarget;
    LiteModule::StyleTarget styleTarget;
    LiteModule::NodeOperationTarget nodeOperationTarget;
    LiteModule::EventMetadataTarget eventMetadataTarget;
    struct EventRegistrationLocation { ObjectRef receiver;std::uint64_t sequence=0; };
    // Request cleanup index only. The Context Record's attached owner is the
    // sole storage for callbacks and event types.
    std::vector<EventRegistrationLocation> eventRegistrationLocations;
    std::uint64_t nextEventSequence=1;
    State(Context& c,OptionFunctionBindings& f,ContextPropertyBindings& p,
          std::shared_ptr<regexp_host::RegExpBindings> rx,std::shared_ptr<LiteNodeStore> s)
        :context(c),functions(f),properties(p),store(std::move(s)),
         entities(std::make_shared<entities::EntityModule>()),styles(c,f,p),regexp(std::move(rx)){
        if(!regexp||&regexp->context()!=&context)
            throw Exception("TypeError",u"LiteModule requires RegExpBindings from its actual Context session");
    }
    ObjectRef classFor(NodeKind k)const{
        switch(k){case NodeKind::Element:return classes[0];case NodeKind::Text:return classes[1];case NodeKind::Comment:return classes[2];case NodeKind::Document:return classes[3];case NodeKind::List:return classes[4];case NodeKind::Window:return classes[5];}
        return classes[0];
    }
    std::optional<LiteNodeRef> nodeFor(ObjectRef object)const{
        for(const auto& [node,known]:nodes)if(known==object)return node;return{};
    }
    void retireEventRegistrationsSince(std::uint64_t first){
        for(const auto& location:eventRegistrationLocations){
            if(location.sequence<first)continue;
            const auto attached=context.attachedOwner(location.receiver);
            if(!attached)continue; // Its receiver record was already retired.
            auto owner=std::dynamic_pointer_cast<EventRegistrationOwner>(attached);
            if(!owner)throw Exception("TypeError",u"Lite host event receiver has a different attached owner");
            owner->erase(location.sequence);
            if(owner->empty())(void)context.clearAttachedOwner(location.receiver,owner.get());
        }
        std::erase_if(eventRegistrationLocations,
            [&](const EventRegistrationLocation& item){return item.sequence>=first;});
    }
    void registerHostEvent(ObjectRef receiver,Value eventType,Value callback){
        (void)context.kind(receiver);
        const auto type=implicitString(context,eventType);
        if(const auto* object=std::get_if<ObjectRef>(&callback.data))(void)context.kind(*object);
        else if(const auto* symbol=std::get_if<SymbolRef>(&callback.data))(void)context.symbolString(*symbol);
        if(nextEventSequence==std::numeric_limits<std::uint64_t>::max())
            throw Exception("RangeError",u"Lite host event registration sequence exhausted");
        const auto sequence=nextEventSequence;
        auto attached=context.attachedOwner(receiver);
        std::shared_ptr<EventRegistrationOwner> owner;
        if(attached){
            owner=std::dynamic_pointer_cast<EventRegistrationOwner>(attached);
            if(!owner)throw Exception("TypeError",u"Context record already has a different attached owner");
        }else{
            owner=std::make_shared<EventRegistrationOwner>();
            context.attachOwner(receiver,owner);
        }
        bool added=false,indexed=false;
        try{
            owner->add(type,callback,sequence);added=true;
            eventRegistrationLocations.push_back({receiver,sequence});indexed=true;
            ++nextEventSequence;
            if(eventMetadataTarget)eventMetadataTarget(context,receiver,type);
        }catch(...){
            if(indexed)std::erase_if(eventRegistrationLocations,
                [&](const EventRegistrationLocation& item){return item.sequence==sequence;});
            if(added)owner->erase(sequence);
            if(owner->empty())
                (void)context.clearAttachedOwner(receiver,owner.get());
            throw;
        }
    }
    ObjectRef parserObject(const std::shared_ptr<LiteParser>& parser){
        if(!parser)throw Exception("TypeError",u"Lite adaptor parser is null");
        for(const auto& [known,object]:parsers)if(known==parser)return object;
        const auto object=context.instance(parserClass,parser);
        std::weak_ptr<State> weak=shared_from_this();
        parser->staticPropertyReader(
            [weak,object](std::u16string_view field)->Property{
                auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
                const auto ctor=s->context.get(Value(object),key(u"constructor"));
                const auto value=s->context.get(ctor,key(field));
                return s->properties.from(value);
            },
            [weak](const Property& receiver,std::u16string_view member)->Property{
                auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
                return s->properties.from(s->context.get(s->properties.to(receiver),key(member)));
            },
            [weak](const Property& receiver,std::u16string_view member)->bool{
                auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
                return s->context.hasOwn(s->properties.to(receiver),key(member));
            });
        parser->contextMethodBridge(
            [weak,object](std::u16string_view name,std::span<const Value> args)->Value{
                auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
                const auto method=s->context.get(Value(object),key(name));
                return s->context.callMember(method,Value(object),args,u"LiteParser method");
            },
            [weak,object](std::u16string_view name)->Value{
                auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
                return s->context.get(Value(object),key(name));
            },
            [weak](Value receiver,std::u16string_view name)->Value{
                auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
                return s->context.get(std::move(receiver),key(name));
            },
            [weak](Value receiver,std::u16string_view name,Value value){
                auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
                s->context.set(std::move(receiver),key(name),std::move(value));
            },
            [weak,object](Value callable,std::span<const Value> args)->Value{
                auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
                const auto* function=std::get_if<ObjectRef>(&callable.data);
                if(!function)throw Exception("TypeError",u"callback is not callable");
                return s->context.call(*function,Value(object),args);
            },
            [weak](AbstractDOMAdaptor& adaptor,std::u16string_view name)->Value{
                auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
                for(const auto& [known,object]:s->adaptors)
                    if(static_cast<AbstractDOMAdaptor*>(known.get())==&adaptor)
                        return s->context.get(Value(object),key(name));
                throw Exception("TypeError",u"Lite parser adaptor is not bound in this module");
            },
            [weak](Value callable,Value receiver,std::span<const Value> args)->Value{
                auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
                return s->context.callMember(std::move(callable),std::move(receiver),args,u"Lite adaptor method");
            },
            [weak](std::function<Value(Value)> body)->Value{
                auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
                const auto callback=s->context.function(u"",[body=std::move(body)](Context&,Value,std::span<const Value> args)->Value{
                    return body(args.empty()?Value(Undefined{}):args[0]);
                });
                s->context.define(callback,key(u"length"),Descriptor::data(Value(1),false,false,true));
                return Value(callback);
            },
            [weak](NodeKind kind,std::span<const Value> args)->Value{
                auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
                std::size_t index=0;
                switch(kind){case NodeKind::Element:index=0;break;case NodeKind::Text:index=1;break;case NodeKind::Comment:index=2;break;case NodeKind::Document:index=3;break;case NodeKind::List:index=4;break;case NodeKind::Window:index=5;break;}
                return Value(s->context.construct(s->classes[index],args));
            },
            [weak](const Property& value)->Value{
                auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
                return s->properties.to(value);
            },
            [weak](const Value& value)->std::u16string{
                auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
                return primitiveString(s->context,value);
            },
            [weak](AbstractDOMAdaptor& adaptor)->Value{
                auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
                for(const auto& [known,contextObject]:s->adaptors)
                    if(static_cast<AbstractDOMAdaptor*>(known.get())==&adaptor)return Value(contextObject);
                throw Exception("TypeError",u"Lite parser adaptor is not bound in this module");
            },
            [weak](LiteNodeRef node)->Value{
                auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
                return node?Value(s->objectFor(node)):Value(Null{});
            },
            [weak](const PropertyArray& array)->Value{
                auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
                return s->properties.to(Property(array));
            },
            [weak](Value value)->PropertyArray{
                auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
                auto array=PropertyArray::array(s->properties.from(std::move(value)));
                if(!array)throw Exception("TypeError",u"Expected an array");
                return *array;
            },
            [weak](Value value)->Property{
                auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
                return s->properties.from(std::move(value));
            },
            [weak](Value value)->LiteNodeRef{
                auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
                const auto property=s->fromContext(std::move(value));
                const auto node=LiteNodeStore::fromProperty(property);
                if(node)return *node;
                if(std::holds_alternative<PropertyNull>(property.value))return {};
                if(property.missing())throw Exception("TypeError",u"Cannot read properties of undefined");
                throw Exception("TypeError",u"Expected a Lite DOM node");
            },
            [weak](Value iterable,const std::function<void(Value)>& body){
                auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
                s->context.forEachIterable(std::move(iterable),body);
            },
            [weak](Value value,NodeKind kind)->bool{
                auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
                const auto* object=std::get_if<ObjectRef>(&value.data);
                if(!object)return false;
                std::size_t index=0;
                switch(kind){case NodeKind::Element:index=0;break;case NodeKind::Text:index=1;break;case NodeKind::Comment:index=2;break;case NodeKind::Document:index=3;break;case NodeKind::List:index=4;break;case NodeKind::Window:index=5;break;}
                const auto constructor=Value(s->classes[index]);
                const auto prototype=s->context.get(constructor,key(u"prototype"));
                const auto* expected=std::get_if<ObjectRef>(&prototype.data);
                if(!expected)throw Exception("TypeError",u"Function has non-object prototype in instanceof check");
                auto current=s->context.prototype(*object);
                while(current){if(*current==*expected)return true;current=s->context.prototype(*current);}
                return false;
            });
        parsers.emplace_back(parser,object);return object;
    }
    std::shared_ptr<LiteParser> parserFor(ObjectRef object)const{
        for(const auto& [parser,known]:parsers)if(known==object)return parser;
        throw Exception("TypeError",u"Object is not a LiteParser instance from this module");
    }
    Value toContext(const Property& value){
        if(auto node=store->fromProperty(value))return Value(objectFor(*node));
        return properties.to(value);
    }
    Property fromContext(Value value){
        if(std::holds_alternative<Undefined>(value.data))return Missing{};
        if(std::holds_alternative<Null>(value.data))return PropertyNull{};
        if(auto* object=std::get_if<ObjectRef>(&value.data))if(auto node=nodeFor(*object))return nodeProperty(*node);
        return properties.from(std::move(value));
    }
    Value toDomValue(const DOMNodeValue& value){
        if(const auto* node=std::get_if<LiteNodeRef>(&value))return Value(objectFor(*node));
        if(std::holds_alternative<Missing>(value))return Undefined{};
        if(std::holds_alternative<PropertyNull>(value))return Null{};
        return properties.to(std::get<Property>(value));
    }
    DOMNodeValue fromDomValue(Value value){return propertyNode(fromContext(std::move(value)));}
    ObjectRef recordObject(const PropertyList& record){
        const auto result=context.object();const auto keys=record.keys();
        for(const auto& name:keys)if(record.hasOwn(name))context.set(Value(result),key(property_host::text(name)),toContext(record.getOwn(name)));
        return result;
    }
    ObjectRef arrayObject(PropertyArray& array,NodeId ownerNode){
        auto view=std::make_shared<LiteArrayView>(array,[weak=weak_from_this()](const Property& p)->Value{auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");return s->toContext(p);},[weak=weak_from_this()](Value v)->Property{auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");return s->fromContext(std::move(v));});
        const ObjectRef identity=context.nativeArray(std::move(view));
        try{arrayProjections.push_back({ownerNode,identity});}
        catch(...){context.retireObjectRecord(identity);throw;}
        return identity;
    }
    void discardArrayProjection(ObjectRef identity){
        const auto found=std::ranges::find_if(arrayProjections,
            [&](const ArrayProjection& entry){return entry.identity==identity;});
        if(found==arrayProjections.end())return;
        context.retireObjectRecord(found->identity);
        arrayProjections.erase(found);
    }
    ObjectRef spreadArray(Value source){
        const auto result=context.array();
        // Array spread uses the iterable protocol, whose array iterator reads
        // the current length on each step and yields holes as undefined.
        context.forEachIterable(std::move(source),[&](Value value){
            const std::array<Value,1> args{value};context.arrayPush(result,args);
        });
        return result;
    }
    ObjectRef objectFor(LiteNodeRef node){
        if(!node||!store->owns(node))throw Exception("TypeError",u"Node belongs to a different Lite module");
        for(const auto& [known,object]:nodes)if(known==node)return object;
        const auto nodeKind=store->record(node).nodeKind;
        const auto ctor=classFor(nodeKind);
        const auto object=context.instance(ctor,node);nodes.emplace_back(node,object);
        // Convert pre-existing native fields to the one live Context record/array
        // before installing fields that return those identities.
        if(nodeKind==NodeKind::Element && store->ownerView(node).status==OwnerViewLookup::Status::none){
            const auto attrs=store->backingField(node,"attributes");
            auto attrsRecord=PropertyList::record(attrs);
            if(!attrsRecord)throw Exception("TypeError",u"Lite node attributes is not an object");
            const auto attrsObject=recordObject(*attrsRecord);
            store->backingSetField(node,"attributes",Property(properties.from(Value(attrsObject))));
            const auto kidsProperty=store->backingField(node,"children");
            auto kidsArray=PropertyArray::array(kidsProperty);
            if(!kidsArray)throw Exception("TypeError",u"Lite node children is not an array");
            const auto kids=arrayObject(*kidsArray,node.id);
            try{store->backingSetField(node,"children",properties.from(Value(kids)));}
            catch(...){discardArrayProjection(kids);throw;}
        } else if(nodeKind==NodeKind::List){
            auto nativeNodes=record(node).listNodes;
            const auto list=arrayObject(nativeNodes,node.id);
            try{record(node).listNodes=PropertyArray::array(properties.from(Value(list))).value();}
            catch(...){discardArrayProjection(list);throw;}
        }
        std::weak_ptr<State> weak=shared_from_this();const auto id=node.id;
        auto owner=std::static_pointer_cast<void>(store);
        auto sourceOwnParent=[&]{
            if(record(node).parentOwn)return true;
            const auto lookup=store->ownerView(node);
            if(lookup.status==OwnerViewLookup::Status::expired)
                throw Exception("TypeError",u"Lite node owner view has expired");
            return lookup.status==OwnerViewLookup::Status::live&&lookup.view->hasOwnProperty("parent");
        };
        auto bind=[&](std::u16string name,auto read,auto write,bool removable=false){
            std::function<void()> remove;
            if(removable){
                const auto field=property_host::debugText(name);
                remove=[weak,id,field]{
                    auto s=weak.lock();
                    if(!s)throw Exception("TypeError",u"Lite module has expired");
                    s->store->backingSetFieldRemovedFromContext(s->store->ref(id),field);
                };
            }
            context.bindData(object,key(name),std::make_shared<NativeDataField>(owner,
                [weak,id,read]()->Value{auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");auto n=s->store->ref(id);return read(*s,n);},
                [weak,id,write](Value value){auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");auto n=s->store->ref(id);write(*s,n,std::move(value));},
                std::move(remove)));
        };
        if(nodeKind==NodeKind::Element){
            bind(u"kind",[](State& s,LiteNodeRef n){return s.toContext(s.store->backingField(n,"kind"));},[](State& s,LiteNodeRef n,Value v){s.store->backingSetFieldFromContext(n,"kind",s.fromContext(std::move(v)));},true);
            bind(u"attributes",[](State& s,LiteNodeRef n){return s.properties.to(s.store->backingField(n,"attributes"));},[](State& s,LiteNodeRef n,Value v){s.store->backingSetFieldFromContext(n,"attributes",s.properties.from(std::move(v)));},true);
            bind(u"children",[](State& s,LiteNodeRef n){return s.properties.to(s.store->backingField(n,"children"));},[](State& s,LiteNodeRef n,Value v){s.store->backingSetFieldFromContext(n,"children",s.properties.from(std::move(v)));},true);
            bind(u"styles",[](State& s,LiteNodeRef n){auto style=s.store->backingNodeStyles(n);return style?Value(s.styles.bind(style)):Value(Null{});},[](State& s,LiteNodeRef n,Value v){if(std::holds_alternative<Null>(v.data)||std::holds_alternative<Undefined>(v.data)){s.store->backingSetNodeStyles(n,{});return;}auto* object=std::get_if<ObjectRef>(&v.data);if(!object)throw Exception("TypeError",u"styles must be a Styles instance or null");s.store->backingSetNodeStyles(n,s.styles.resolve(*object));},true);
            auto parentField=std::make_shared<NativeDataField>(owner,
                [weak,id]()->Value{auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");return s->toContext(s->store->backingField(s->store->ref(id),"parent"));},
                [weak,id](Value value){auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");s->store->backingSetFieldFromContext(s->store->ref(id),"parent",s->fromContext(std::move(value)));},
                [weak,id]{auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");s->store->backingSetFieldRemovedFromContext(s->store->ref(id),"parent");});
            if(sourceOwnParent())context.bindData(object,key(u"parent"),std::move(parentField));
            else context.declareNativeData(object,key(u"parent"),std::move(parentField));
        } else if(nodeKind==NodeKind::Text||nodeKind==NodeKind::Comment){
            bind(u"value",[](State& s,LiteNodeRef n){return s.toContext(s.store->backingField(n,"value"));},[](State& s,LiteNodeRef n,Value v){s.store->backingSetFieldFromContext(n,"value",s.fromContext(std::move(v)));},true);
            auto parentField=std::make_shared<NativeDataField>(owner,
                [weak,id]()->Value{auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");return s->toContext(s->store->backingField(s->store->ref(id),"parent"));},
                [weak,id](Value value){auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");s->store->backingSetFieldFromContext(s->store->ref(id),"parent",s->fromContext(std::move(value)));},
                [weak,id]{auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");s->store->backingSetFieldRemovedFromContext(s->store->ref(id),"parent");});
            if(sourceOwnParent())context.bindData(object,key(u"parent"),std::move(parentField));
            else context.declareNativeData(object,key(u"parent"),std::move(parentField));
        } else if(nodeKind==NodeKind::Document){
            bind(u"defaultView",[](State& s,LiteNodeRef n){return s.toContext(recordConst(n).defaultView);},[](State& s,LiteNodeRef n,Value v){record(n).defaultView=s.fromContext(std::move(v));});
            bind(u"root",[](State& s,LiteNodeRef n){return s.toContext(recordConst(n).root);},[](State& s,LiteNodeRef n,Value v){record(n).root=s.fromContext(std::move(v));});
            bind(u"head",[](State& s,LiteNodeRef n){return s.toContext(recordConst(n).head);},[](State& s,LiteNodeRef n,Value v){record(n).head=s.fromContext(std::move(v));});
            bind(u"body",[](State& s,LiteNodeRef n){return s.toContext(recordConst(n).body);},[](State& s,LiteNodeRef n,Value v){record(n).body=s.fromContext(std::move(v));});
            bind(u"type",[](State& s,LiteNodeRef n){return s.toContext(recordConst(n).documentType);},[](State& s,LiteNodeRef n,Value v){record(n).documentType=s.fromContext(std::move(v));});
        } else if(nodeKind==NodeKind::Window){
            bind(u"document",[](State& s,LiteNodeRef n){return s.toContext(recordConst(n).document);},[](State& s,LiteNodeRef n,Value v){record(n).document=s.fromContext(std::move(v));});
            for(auto [name,ctorValue]:std::array<std::pair<std::u16string,ObjectRef>,6>{{
                {u"DOMParser",parserClass},{u"NodeList",classes[4]},{u"HTMLCollection",classes[4]},
                {u"HTMLElement",classes[0]},{u"DocumentFragment",classes[4]},{u"Document",classes[3]}}})
                context.define(object,key(name),Descriptor::data(Value(ctorValue),true,true,true));
        } else if(nodeKind==NodeKind::List){
            bind(u"nodes",[](State& s,LiteNodeRef n){return s.properties.to(Property(recordConst(n).listNodes));},[](State& s,LiteNodeRef n,Value v){auto p=s.properties.from(std::move(v));auto a=PropertyArray::array(p);if(!a)throw Exception("TypeError",u"nodes must be an array");record(n).listNodes=*a;});
        }
        if(nodeKind==NodeKind::Element||nodeKind==NodeKind::Text||nodeKind==NodeKind::Comment){
        // Extra properties created through the native Lite API before the
        // canonical Context identity existed become own Context properties
        // once. The Context object is the sole owner after this point.
        std::vector<std::pair<std::string,Property>> nativeExtras;
        for(const auto& name:recordConst(node).properties.keys())
            nativeExtras.emplace_back(name,recordConst(node).properties.getOwn(name));
        for(const auto& [name,value]:nativeExtras){
            const auto field=key(property_host::text(name));
            if(!context.hasOwn(Value(object),field))
                context.define(object,field,Descriptor::data(toContext(value)));
        }
        record(node).properties=PropertyList{};
        // Native Lite operations now consult the currently installed Context
        // descriptor on this canonical object. The native data callbacks
        // above go straight to backing storage, so this projection cannot
        // recurse through its own fields. State owns the projection; both the
        // store registration and callback captures are weak toward State.
        auto fieldKey=[](std::string_view field){return key(property_host::text(field));};
        auto getProjection=[weak,object,fieldKey](std::string_view field)->Property{
            auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
            return s->fromContext(s->context.get(Value(object),fieldKey(field)));
        };
        auto setProjection=[weak,object,fieldKey](std::string_view field,Property value){
            auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
            s->context.set(Value(object),fieldKey(field),s->toContext(value));
        };
        auto defineOwnProjection=[weak,object,fieldKey](std::string_view field,Property value){
            auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
            s->context.define(object,fieldKey(field),Descriptor::data(s->toContext(value)));
        };
        auto hasPropertyProjection=[weak,object,fieldKey](std::string_view field)->bool{
            auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
            return s->context.has(Value(object),fieldKey(field));
        };
        auto hasProjection=[weak,object,fieldKey](std::string_view field)->bool{
            auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
            return s->context.hasOwn(Value(object),fieldKey(field));
        };
        auto keysProjection=[weak,object]()->std::vector<std::string>{
            auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
            std::vector<std::string> result;
            for(const auto& name:s->context.enumerableStringKeys(Value(object)))
                result.push_back(property_host::debugText(name));
            return result;
        };
        auto eraseProjection=[weak,object,fieldKey](std::string_view field){
            auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
            s->context.erase(object,fieldKey(field));
        };
        auto getStylesProjection=[weak,object]()->std::shared_ptr<styles::Styles>{
            auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
            const auto value=s->context.get(Value(object),key(u"styles"));
            if(std::holds_alternative<Null>(value.data)||std::holds_alternative<Undefined>(value.data))return {};
            const auto* styleObject=std::get_if<ObjectRef>(&value.data);
            if(!styleObject)throw Exception("TypeError",u"styles must be a Styles instance or null");
            return s->styles.resolve(*styleObject);
        };
        auto setStylesProjection=[weak,object](std::shared_ptr<styles::Styles> value){
            auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
            s->context.set(Value(object),key(u"styles"),value?Value(s->styles.bind(value)):Value(Null{}));
        };
        auto projection=std::make_shared<ContextNodeProjection>(std::move(getProjection),std::move(setProjection),
            std::move(defineOwnProjection),std::move(hasPropertyProjection),std::move(hasProjection),std::move(keysProjection),std::move(eraseProjection),
            std::move(getStylesProjection),std::move(setStylesProjection));
        contextProjections.emplace(node.id,projection);
        store->bindContextProjection(node,projection);
        }
        return object;
    }
    ObjectRef bindBaseAdaptor(const std::shared_ptr<LiteBase>& adaptor,ObjectRef constructor){
        if(!adaptor)throw Exception("TypeError",u"Cannot bind a null LiteAdaptor");
        for(const auto& [known,object]:adaptors)if(known==adaptor)return object;
        const auto base=std::static_pointer_cast<AbstractDOMAdaptor>(adaptor);
        const auto object=context.instance(constructor,base);
        adaptors.emplace_back(adaptor,object);
        // The native LiteBase parser and its Source-visible Context object are
        // one parser instance. Install its canonical constructor lookup before
        // parse/serialize can be called through either surface.
        (void)parserObject(adaptor->parser());
        const auto owner=std::static_pointer_cast<void>(adaptor);
        std::weak_ptr<State> weak=shared_from_this();
        context.bindData(object,key(u"document"),std::make_shared<NativeDataField>(owner,
            [weak,adaptor]{auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");return s->toDomValue(adaptor->document);},
            [weak,adaptor](Value value){auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");adaptor->document=s->fromDomValue(std::move(value));}));
        context.bindData(object,key(u"window"),std::make_shared<NativeDataField>(owner,
            [weak,adaptor]{auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");return Value(s->objectFor(adaptor->window()));},
            [weak,adaptor](Value value){auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");const auto* window=std::get_if<ObjectRef>(&value.data);if(!window)throw Exception("TypeError",u"window must be a LiteWindow");auto node=s->nodeFor(*window);if(recordConst(*node).nodeKind!=NodeKind::Window)throw Exception("TypeError",u"window must be a LiteWindow");adaptor->window(*node);}));
        context.bindData(object,key(u"parser"),std::make_shared<NativeDataField>(owner,
            [weak,adaptor]{auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");return Value(s->parserObject(adaptor->parser()));},
            [weak,adaptor](Value value){auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");const auto* parser=std::get_if<ObjectRef>(&value.data);if(!parser)throw Exception("TypeError",u"parser must be a LiteParser");adaptor->parser(s->parserFor(*parser));}));
        if(const auto nodeAdaptor=std::dynamic_pointer_cast<LiteAdaptor>(adaptor)){
            nodeAdaptor->sourceTextContent([weak,object](LiteNodeRef node)->Property{
                auto self=weak.lock();if(!self)throw Exception("TypeError",u"Lite module has expired");
                const Value receiver(object);
                const Value method=self->context.get(receiver,key(u"textContent"));
                const std::array<Value,1> args{Value(self->objectFor(node))};
                return self->properties.from(self->context.callMember(method,receiver,args,u"NodeMixin textContent"));
            });
            nodeAdaptor->sourceOption([weak,object](std::string_view field)->Property{
                auto self=weak.lock();if(!self)throw Exception("TypeError",u"Lite module has expired");
                const auto opts=self->context.get(Value(object),key(u"options"));
                const auto value=self->context.get(opts,key(property_host::text(field)));
                return self->properties.from(value);
            });
            nodeAdaptor->sourceNumberOption([weak,object](std::string_view field)->double{
                auto self=weak.lock();if(!self)throw Exception("TypeError",u"Lite module has expired");
                const auto opts=self->context.get(Value(object),key(u"options"));
                return self->context.toNumber(self->context.get(opts,key(property_host::text(field))));
            });
            context.bindData(object,key(u"options"),std::make_shared<NativeDataField>(owner,
                [weak,nodeAdaptor]{auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");return s->properties.to(Property(nodeAdaptor->options()));},
                [weak,nodeAdaptor](Value value){auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");auto raw=s->properties.from(std::move(value));auto record=PropertyList::record(raw);if(!record)throw Exception("TypeError",u"options must be an object");nodeAdaptor->options(*record);}));
        }
        context.bindData(object,key(u"canMeasureNodes"),std::make_shared<NativeDataField>(owner,
            [weak,adaptor]{auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");return s->toContext(adaptor->canMeasureNodes);},
            [weak,adaptor](Value value){auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");adaptor->canMeasureNodes=s->fromContext(std::move(value));}));
        return object;
    }
    ObjectRef bindAdaptor(const std::shared_ptr<LiteAdaptor>& adaptor){
        return bindBaseAdaptor(adaptor,adaptorClass);
    }
    std::shared_ptr<LiteBase> resolveBaseAdaptor(ObjectRef object)const{
        for(const auto& [adaptor,known]:adaptors)if(known==object)return adaptor;
        throw Exception("TypeError",u"Object is not a LiteBase instance from this module");
    }
    std::shared_ptr<LiteAdaptor> resolveAdaptor(ObjectRef object)const{
        auto adaptor=std::dynamic_pointer_cast<LiteAdaptor>(resolveBaseAdaptor(object));
        if(adaptor)return adaptor;
        throw Exception("TypeError",u"Object is not a LiteAdaptor instance from this module");
    }
    ObjectRef constructLiteBase(){return bindBaseAdaptor(std::make_shared<LiteBase>(store,entities),liteBaseClass);}
    std::shared_ptr<LiteAdaptor> constructAdaptor(std::span<const Value> args){
        // LiteBase declares no constructor parameters. NodeMixin forwards its
        // first argument to that constructor (ignored), then reads the second
        // argument for its instance options after super() has completed.
        auto adaptor=std::make_shared<LiteAdaptor>(LiteNodeRef{},PropertyList{},store,entities);
        std::weak_ptr<State> weak=shared_from_this();
        adaptor->cjkStripper([weak](std::u16string_view text){
            auto self=weak.lock();if(!self)throw Exception("TypeError",u"Lite module has expired");
            const Value source=self->context.get(Value(self->nodeAdaptorClass),key(u"cjkPattern"));
            if(const auto* pattern=std::get_if<ObjectRef>(&source.data)){
                auto expression=self->regexp->resolve(Value(*pattern));
                return LiteAdaptor::replaceCjk(text,*expression);
            }
            if(std::holds_alternative<Undefined>(source.data)||std::holds_alternative<Null>(source.data))
                throw Exception("TypeError",std::holds_alternative<Undefined>(source.data)?u"Cannot read properties of undefined (reading 'Symbol(Symbol.replace)')":u"Cannot read properties of null (reading 'Symbol(Symbol.replace)')");
            const auto needle=primitiveString(self->context,source);
            std::u16string result(text);
            if(needle.empty())result.insert(0,u"");
            else if(const auto at=result.find(needle);at!=std::u16string::npos)result.erase(at,needle.size());
            return result;
        });
        // Preserve the Source order and graph identity: first create a fresh
        // destination, read the actual constructor's live OPTIONS value, run
        // defaultOptions, then read the second argument and run userOptions.
        const auto destination=context.object();
        const Value classOptions=context.get(Value(adaptorClass),key(u"OPTIONS"));
        const std::array<Value,1> defaults{classOptions};
        Value merged=options::defaultOptions(context,Value(destination),defaults);
        const Value supplied=args.size()>1?args[1]:Value(Undefined{});
        const std::array<Value,1> user{supplied};
        merged=options::userOptions(context,std::move(merged),user);
        const auto converted=properties.from(std::move(merged));
        const auto object=PropertyList::record(converted);
        if(!object)throw Exception("TypeError",u"LiteAdaptor options are not an object");
        adaptor->options(*object);
        return adaptor;
    }
    ObjectRef constructNode(NodeKind kind,std::span<const Value> args){
        LiteNodeRef node;
        if(kind==NodeKind::Element){
            const Property name=args.empty()?Property(Missing{}):fromContext(args[0]);
            node=LiteElement(name,{},PropertyArray{},store).ref();
            const auto object=objectFor(node);
            const Value attrs=args.size()>1?args[1]:Value(Undefined{});
            const Property source=properties.from(attrs);
            const Property copied=properties.copyOwnEnumerableProperties(source);
            context.set(Value(object),key(u"attributes"),properties.to(copied));
            const Value childInput=args.size()>2?args[2]:Value(Undefined{});
            const ObjectRef children=std::holds_alternative<Undefined>(childInput.data)?context.array():spreadArray(childInput);
            context.set(Value(object),key(u"children"),Value(children));
            // Source iterates the newly assigned array, capturing its identity
            // and reading each current slot before assigning the parent.
            context.forEachIterable(Value(children),[&](Value child){context.set(child,key(u"parent"),Value(object));});
            return object;
        } else if(kind==NodeKind::Text||kind==NodeKind::Comment){
            const Property value=args.empty()||std::holds_alternative<Undefined>(args[0].data)?Property(property_host::debugText(u"")):fromContext(args[0]);
            node=LiteText(value,kind==NodeKind::Comment,store).ref();
        } else if(kind==NodeKind::Document){
            LiteNodeValue window=PropertyNull{};if(!args.empty()&&!std::holds_alternative<Undefined>(args[0].data))window=fromContext(args[0]);node=LiteDocument(window,store).ref();
        } else if(kind==NodeKind::List){
            if(args.empty())throw Exception("TypeError",u"children is not iterable");PropertyArray nodes;
            auto iterator=context.getIterator(args[0]);while(true){auto step=context.iteratorNext(iterator);auto object=std::get<ObjectRef>(step.data);if(truthy(context.get(Value(object),key(u"done"))))break;nodes.push(fromContext(context.get(Value(object),key(u"value"))));}
            node=LiteList(std::move(nodes),store).ref();
        } else if(kind==NodeKind::Window){
            node=LiteWindow(store).ref();
        } else throw Exception("TypeError",u"Unknown Lite class constructor");
        return objectFor(node);
    }
    void initialize(){
        std::weak_ptr<State> weak=shared_from_this();
        properties.registerProjection(
            [weak](const Property& value)->std::optional<ObjectRef>{
                const auto* storage=std::get_if<std::shared_ptr<PropertyStorage>>(&value.value);
                if(!storage||!*storage||!(*storage)->binding)return {};
                const auto binding=std::dynamic_pointer_cast<DOMNodePropertyBinding>((*storage)->binding);
                if(!binding)return {};
                const auto node=binding->nodeRef();
                auto self=weak.lock();
                if(!self||!self->store->owns(node))return {};
                return self->objectFor(node);
            },
            [weak](ObjectRef object)->std::optional<Property>{
                auto self=weak.lock();if(!self)return {};
                const auto node=self->nodeFor(object);
                if(!node)return {};
                return nodeProperty(*node);
            });
        auto make=[&](std::u16string name,std::size_t arity,NodeKind kind){
            auto ctor=context.nativeConstructor(std::move(name),[weak,kind](Context&,std::span<const Value> args)->ObjectRef{
                auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");return s->constructNode(kind,args);
            });
            context.define(ctor,key(u"length"),Descriptor::data(Value(static_cast<double>(arity)),false,false,true));
            return ctor;
        };
        classes[0]=make(u"LiteElement",1,NodeKind::Element);
        classes[1]=make(u"LiteText",0,NodeKind::Text);
        classes[2]=make(u"LiteComment",0,NodeKind::Comment);
        classes[3]=make(u"LiteDocument",0,NodeKind::Document);
        classes[4]=make(u"LiteList",1,NodeKind::List);
        classes[5]=make(u"LiteWindow",0,NodeKind::Window);
        liteBaseClass=context.nativeConstructor(u"LiteBase",[weak](Context&,std::span<const Value>)->ObjectRef{
            auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");return s->constructLiteBase();
        });
        context.define(liteBaseClass,key(u"length"),Descriptor::data(Value(0),false,false,true));
        nodeAdaptorClass=context.nativeConstructor(u"NodeAdaptor",[weak](Context&,std::span<const Value> args)->ObjectRef{
            auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");return s->bindBaseAdaptor(s->constructAdaptor(args),s->nodeAdaptorClass);
        });
        context.define(nodeAdaptorClass,key(u"length"),Descriptor::data(Value(0),false,false,true));
        const auto baseProto=std::get<ObjectRef>(context.get(Value(liteBaseClass),key(u"prototype")).data);
        const auto nodeAdaptorProto=std::get<ObjectRef>(context.get(Value(nodeAdaptorClass),key(u"prototype")).data);
        context.setPrototype(nodeAdaptorProto,baseProto);
        context.setPrototype(nodeAdaptorClass,liteBaseClass);
        context.set(Value(nodeAdaptorClass),key(u"cjkPattern"),Value(regexp->create(LiteAdaptor::cjkPatternSource(),u"gu")));
        // The actual Source constructor is retained by the Context. Keep the
        // module's canonical identity tables alive with that constructor, so
        // Context-held parser/node/array objects remain usable after the
        // external C++ LiteModule wrapper is released. State borrows Context;
        // this does not form a Context ownership cycle.
        parserClass=context.nativeConstructor(u"LiteParser",[self=shared_from_this()](Context&,std::span<const Value>)->ObjectRef{
            auto parser=std::make_shared<LiteParser>(self->entities);return self->parserObject(parser);
        });
        context.define(parserClass,key(u"length"),Descriptor::data(Value(0),false,false,true));
        // These are ordinary writable static source fields, not native option
        // defaults.  A later Source assignment remains visible to code which
        // performs a fresh property read from LiteParser.
        auto staticNames=[&](std::initializer_list<std::u16string_view> names){
            const auto object=context.object();
            for(const auto name:names)context.set(Value(object),key(name),Value(true));
            return object;
        };
        context.set(Value(parserClass),key(u"SELF_CLOSING"),Value(staticNames({
            u"area",u"base",u"br",u"col",u"command",u"embed",u"hr",u"img",
            u"input",u"keygen",u"link",u"menuitem",u"meta",u"param",u"source",
            u"track",u"wbr"})));
        context.set(Value(parserClass),key(u"PCDATA"),Value(staticNames({
            u"option",u"textarea",u"fieldset",u"title",u"style",u"script"})));
        const auto namespaces=context.object();
        for(const auto& [name,uri]:std::array<std::pair<std::u16string_view,std::u16string_view>,3>{{
            {u"svg",u"http://www.w3.org/2000/svg"},
            {u"math",u"http://www.w3.org/1998/Math/MathML"},
            {u"html",u"http://www.w3.org/1999/xhtml"}}})
            context.set(Value(namespaces),key(name),Value(std::u16string(uri)));
        context.set(Value(parserClass),key(u"XMLNS"),Value(namespaces));
        const auto parserProto=std::get<ObjectRef>(context.get(Value(parserClass),key(u"prototype")).data);
        auto parserReceiver=[weak](Value receiver)->std::pair<std::shared_ptr<State>,ObjectRef>{
            auto self=weak.lock();if(!self)throw Exception("TypeError",u"Lite module has expired");
            const auto* object=std::get_if<ObjectRef>(&receiver.data);
            if(!object)throw Exception("TypeError",u"LiteParser method receiver is not an object");
            (void)self->parserFor(*object);return {std::move(self),*object};
        };
        auto parserAdaptor=[weak](Context&,const Value& value)->std::shared_ptr<LiteBase>{
            auto self=weak.lock();if(!self)throw Exception("TypeError",u"Lite module has expired");
            const auto* object=std::get_if<ObjectRef>(&value.data);
            if(!object){
                if(std::holds_alternative<Undefined>(value.data))throw Exception("TypeError",u"Cannot read properties of undefined (reading 'createDocument')");
                if(std::holds_alternative<Null>(value.data))throw Exception("TypeError",u"Cannot read properties of null (reading 'createDocument')");
                throw Exception("TypeError",u"adaptor.createDocument is not a function");
            }
            return self->resolveBaseAdaptor(*object);
        };
        auto parserNode=[weak](Context&,const Value& value)->LiteNodeRef{
            auto self=weak.lock();if(!self)throw Exception("TypeError",u"Lite module has expired");
            if(const auto* object=std::get_if<ObjectRef>(&value.data)){
                if(auto node=self->nodeFor(*object))return *node;
            }
            const auto property=self->properties.from(value);
            if(auto node=LiteNodeStore::fromProperty(property))return *node;
            if(std::holds_alternative<Null>(value.data))return {};
            if(std::holds_alternative<Undefined>(value.data))throw Exception("TypeError",u"Cannot read properties of undefined");
            throw Exception("TypeError",u"Expected a Lite DOM node");
        };
        auto defineParserMethod=[&](std::u16string name,std::size_t arity,NativeCall body){
            const auto fn=context.function(name,std::move(body));
            context.define(fn,key(u"length"),Descriptor::data(Value(static_cast<double>(arity)),false,false,true));
            context.define(parserProto,key(std::move(name)),Descriptor::data(Value(fn),true,true,true));
        };
        defineParserMethod(u"parseFromString",3,[parserReceiver,parserAdaptor](Context& cx,Value receiver,std::span<const Value> args)->Value{
            auto [self,object]=parserReceiver(std::move(receiver));
            auto parser=self->parserFor(object);
            const Value text=args.empty()?Value(Undefined{}):args[0];
            const Value adaptorValue=args.size()<3||std::holds_alternative<Undefined>(args[2].data)?Value(Null{}):args[2];
            const auto* textValue=std::get_if<std::u16string>(&text.data);
            if(!textValue){
                if(std::holds_alternative<Undefined>(text.data))throw Exception("TypeError",u"Cannot read properties of undefined (reading 'replace')");
                if(std::holds_alternative<Null>(text.data))throw Exception("TypeError",u"Cannot read properties of null (reading 'replace')");
                throw Exception("TypeError",u"text.replace is not a function");
            }
            auto adaptor=parserAdaptor(cx,adaptorValue);
            const auto result=parser->parseFromString(*textValue,u"text/html",*adaptor);
            return Value(self->objectFor(result));
        });
        auto parserParts=[weak](Context& cx,const Value& value)->PropertyArray{
            auto self=weak.lock();if(!self)throw Exception("TypeError",u"Lite module has expired");
            auto array=PropertyArray::array(self->properties.from(value));
            if(!array)throw Exception("TypeError",u"parts.splice is not a function");
            return *array;
        };
        auto parserNodeResult=[weak](const LiteNodeRef& node)->Value{
            auto self=weak.lock();if(!self)throw Exception("TypeError",u"Lite module has expired");
            return node?Value(self->objectFor(node)):Value(Null{});
        };
        auto parserValue=[&](std::u16string name,std::size_t arity,NativeCall body){
            defineParserMethod(std::move(name),arity,std::move(body));
        };
        parserValue(u"addText",3,[parserReceiver,parserAdaptor,parserNode,parserNodeResult](Context& cx,Value receiver,std::span<const Value> args)->Value{
            auto [self,object]=parserReceiver(std::move(receiver));auto parser=self->parserFor(object);
            auto adaptor=parserAdaptor(cx,args.empty()?Value(Undefined{}):args[0]);
            auto node=parserNode(cx,args.size()<2?Value(Undefined{}):args[1]);
            const auto textValue=args.size()<3?Value(Undefined{}):args[2];
            const auto* text=std::get_if<std::u16string>(&textValue.data);
            if(!text){
                if(std::holds_alternative<Undefined>(textValue.data))throw Exception("TypeError",u"Cannot read properties of undefined (reading 'replace')");
                if(std::holds_alternative<Null>(textValue.data))throw Exception("TypeError",u"Cannot read properties of null (reading 'replace')");
                throw Exception("TypeError",u"text.replace is not a function");
            }
            return parserNodeResult(parser->addText(*adaptor,node,*text));
        });
        parserValue(u"addComment",3,[parserReceiver,parserAdaptor,parserNode,parserNodeResult](Context& cx,Value receiver,std::span<const Value> args)->Value{
            auto [self,object]=parserReceiver(std::move(receiver));auto parser=self->parserFor(object);
            auto adaptor=parserAdaptor(cx,args.empty()?Value(Undefined{}):args[0]);
            auto node=parserNode(cx,args.size()<2?Value(Undefined{}):args[1]);
            const auto text=args.size()<3?Value(Undefined{}):args[2];
            const auto comment=LiteText(self->properties.from(text),true,liteStore(node)).ref();
            return parserNodeResult(adaptor->append(node,comment));
        });
        parserValue(u"closeTag",3,[parserReceiver,parserAdaptor,parserNode,parserNodeResult](Context& cx,Value receiver,std::span<const Value> args)->Value{
            auto [self,object]=parserReceiver(std::move(receiver));auto parser=self->parserFor(object);
            auto adaptor=parserAdaptor(cx,args.empty()?Value(Undefined{}):args[0]);
            auto node=parserNode(cx,args.size()<2?Value(Undefined{}):args[1]);
            const auto tagValue=args.size()<3?Value(Undefined{}):args[2];
            const auto* text=std::get_if<std::u16string>(&tagValue.data);
            if(!text){
                if(std::holds_alternative<Undefined>(tagValue.data))throw Exception("TypeError",u"Cannot read properties of undefined (reading 'slice')");
                if(std::holds_alternative<Null>(tagValue.data))throw Exception("TypeError",u"Cannot read properties of null (reading 'slice')");
                throw Exception("TypeError",u"tag.slice is not a function");
            }
            return parserNodeResult(parser->closeTag(*adaptor,node,*text));
        });
        parserValue(u"openTag",4,[parserReceiver,parserAdaptor,parserNode,parserParts,parserNodeResult](Context& cx,Value receiver,std::span<const Value> args)->Value{
            auto [self,object]=parserReceiver(std::move(receiver));auto parser=self->parserFor(object);
            auto adaptor=parserAdaptor(cx,args.empty()?Value(Undefined{}):args[0]);
            auto node=parserNode(cx,args.size()<2?Value(Undefined{}):args[1]);
            const auto tagValue=args.size()<3?Value(Undefined{}):args[2];
            const auto* tag=std::get_if<std::u16string>(&tagValue.data);
            if(!tag){
                if(std::holds_alternative<Undefined>(tagValue.data))throw Exception("TypeError",u"Cannot read properties of undefined (reading 'match')");
                if(std::holds_alternative<Null>(tagValue.data))throw Exception("TypeError",u"Cannot read properties of null (reading 'match')");
                throw Exception("TypeError",u"tag.match is not a function");
            }
            auto parts=parserParts(cx,args.size()<4?Value(Undefined{}):args[3]);
            return parserNodeResult(parser->openTag(*adaptor,node,*tag,parts));
        });
        parserValue(u"addAttributes",3,[weak](Context& cx,Value,std::span<const Value> args)->Value{
            auto self=weak.lock();if(!self)throw Exception("TypeError",u"Lite module has expired");
            const Value adaptor=args.empty()?Value(Undefined{}):args[0];
            const Value node=args.size()<2?Value(Undefined{}):args[1];
            const Value attributes=args.size()<3?Value(Undefined{}):args[2];
            while(truthy(cx.get(attributes,key(u"length")))){
                const Value splice=cx.get(attributes,key(u"splice"));
                const std::array<Value,2> spliceArgs{Value(0),Value(5)};
                const Value group=cx.callMember(splice,attributes,spliceArgs,u"attributes.splice");
                const Value unpacked=cx.readIterable(group,5);
                const Value name=cx.get(unpacked,key(u"1"));
                const Value v1=cx.get(unpacked,key(u"2"));
                const Value v2=cx.get(unpacked,key(u"3"));
                const Value v3=cx.get(unpacked,key(u"4"));
                const Value raw=truthy(v1)?v1:truthy(v2)?v2:truthy(v3)?v3:Value(u"");
                std::u16string input;
                if(const auto* text=std::get_if<std::u16string>(&raw.data))input=*text;
                else if(std::holds_alternative<Undefined>(raw.data))
                    throw Exception("TypeError",u"Cannot read properties of undefined (reading 'replace')");
                else if(std::holds_alternative<Null>(raw.data))
                    throw Exception("TypeError",u"Cannot read properties of null (reading 'replace')");
                else throw Exception("TypeError",u"text.replace is not a function");
                const Value translated(self->entities->translate(input));
                const Value setAttribute=cx.get(adaptor,key(u"setAttribute"));
                const std::array<Value,3> setArgs{node,name,translated};
                (void)cx.callMember(setAttribute,adaptor,setArgs,u"adaptor.setAttribute");
            }
            return Undefined{};
        });
        parserValue(u"handlePCDATA",4,[](Context& cx,Value,std::span<const Value> args)->Value{
            const Value adaptor=args.empty()?Value(Undefined{}):args[0];
            const Value node=args.size()<2?Value(Undefined{}):args[1];
            const Value kind=args.size()<3?Value(Undefined{}):args[2];
            const Value parts=args.size()<4?Value(Undefined{}):args[3];
            const std::u16string end=u"</"+implicitString(cx,kind)+u">";
            Value ptag=u"";
            const ObjectRef content=cx.array();
            while(truthy(cx.get(parts,key(u"length")))&&ptag!=Value(end)){
                Value push=cx.get(Value(content),key(u"push"));
                const std::array<Value,1> beforeArgs{ptag};
                (void)cx.callMember(push,Value(content),beforeArgs,u"pcdata.push");

                push=cx.get(Value(content),key(u"push"));
                const Value shift=cx.get(parts,key(u"shift"));
                const Value following=cx.callMember(shift,parts,{},u"parts.shift");
                const std::array<Value,1> followingArgs{following};
                (void)cx.callMember(push,Value(content),followingArgs,u"pcdata.push");

                const Value nextShift=cx.get(parts,key(u"shift"));
                ptag=cx.callMember(nextShift,parts,{},u"parts.shift");
            }
            const Value append=cx.get(adaptor,key(u"append"));
            const Value textMethod=cx.get(adaptor,key(u"text"));
            const Value join=cx.get(Value(content),key(u"join"));
            const std::array<Value,1> joinArgs{Value(u"")};
            const Value text=cx.callMember(join,Value(content),joinArgs,u"pcdata.join");
            const std::array<Value,1> textArgs{text};
            const Value textNode=cx.callMember(textMethod,adaptor,textArgs,u"adaptor.text");
            const std::array<Value,2> appendArgs{node,textNode};
            (void)cx.callMember(append,adaptor,appendArgs,u"adaptor.append");
            return Undefined{};
        });
        parserValue(u"checkDocument",2,[weak](Context& cx,Value receiver,std::span<const Value> args)->Value{
            auto self=weak.lock();if(!self)throw Exception("TypeError",u"Lite module has expired");
            const Value adaptor=args.empty()?Value(Undefined{}):args[0];
            const Value root=args.size()<2?Value(Undefined{}):args[1];

            // This mirrors `this.getOnlyChild(adaptor, adaptor.body(root))`:
            // member reads and calls remain live, and the parser method is
            // fetched only after the adaptor body call has completed.
            const Value bodyMethod=cx.get(adaptor,key(u"body"));
            const std::array<Value,1> bodyArgs{root};
            const Value body=cx.callMember(bodyMethod,adaptor,bodyArgs,u"adaptor.body");
            const Value onlyMethod=cx.get(receiver,key(u"getOnlyChild"));
            const std::array<Value,2> onlyArgs{adaptor,body};
            const Value node=cx.callMember(onlyMethod,receiver,onlyArgs,u"this.getOnlyChild");
            if(!truthy(node))return Undefined{};

            const Value childNodes=cx.get(adaptor,key(u"childNodes"));
            const Value loopBodyMethod=cx.get(adaptor,key(u"body"));
            const std::array<Value,1> loopBodyArgs{root};
            const Value loopBody=cx.callMember(loopBodyMethod,adaptor,loopBodyArgs,u"adaptor.body");
            const std::array<Value,1> childrenArgs{loopBody};
            const Value children=cx.callMember(childNodes,adaptor,childrenArgs,u"adaptor.childNodes");
            struct Stop{};
            try{
                cx.forEachIterable(children,[&](Value child){
                    if(child==node)throw Stop{};
                    if(!instanceOf(cx,child,self->classes[2]))return;
                    const Value value=cx.get(child,key(u"value"));
                    if(const auto* text=std::get_if<std::u16string>(&value.data)){
                        if(text->starts_with(u"<!DOCTYPE"))cx.set(root,key(u"type"),value);
                        return;
                    }
                    const Value match=cx.get(value,key(u"match"));
                    const ObjectRef expression=self->regexp->create(u"^<!DOCTYPE",u"");
                    const std::array<Value,1> matchArgs{Value(expression)};
                    (void)cx.callMember(match,value,matchArgs,u"child.value.match");
                });
            }catch(const Stop&){}

            const Value kindMethod=cx.get(adaptor,key(u"kind"));
            const std::array<Value,1> kindArgs{node};
            const Value kind=cx.callMember(kindMethod,adaptor,kindArgs,u"adaptor.kind");
            if(kind==Value(u"html")){
                const Value nodeChildren=cx.get(node,key(u"children"));
                cx.forEachIterable(nodeChildren,[&](Value child){
                    const Value method=cx.get(adaptor,key(u"kind"));
                    const std::array<Value,1> args{child};
                    const Value childKind=cx.callMember(method,adaptor,args,u"adaptor.kind");
                    if(childKind==Value(u"head"))cx.set(root,key(u"head"),child);
                    if(childKind==Value(u"body"))cx.set(root,key(u"body"),child);
                });
                cx.set(root,key(u"root"),node);
                const Value remove=cx.get(adaptor,key(u"remove"));
                const std::array<Value,1> removeArgs{node};
                (void)cx.callMember(remove,adaptor,removeArgs,u"adaptor.remove");

                Value parentMethod=cx.get(adaptor,key(u"parent"));
                Value currentBody=cx.get(root,key(u"body"));
                std::array<Value,1> parentArgs{currentBody};
                Value parent=cx.callMember(parentMethod,adaptor,parentArgs,u"adaptor.parent");
                if(parent!=node){
                    const Value append=cx.get(adaptor,key(u"append"));
                    const std::array<Value,2> args{node,cx.get(root,key(u"body"))};
                    (void)cx.callMember(append,adaptor,args,u"adaptor.append");
                }
                parentMethod=cx.get(adaptor,key(u"parent"));
                const Value currentHead=cx.get(root,key(u"head"));
                parentArgs[0]=currentHead;
                parent=cx.callMember(parentMethod,adaptor,parentArgs,u"adaptor.parent");
                if(parent!=node){
                    const Value insert=cx.get(adaptor,key(u"insert"));
                    const std::array<Value,2> args{cx.get(root,key(u"head")),cx.get(root,key(u"body"))};
                    (void)cx.callMember(insert,adaptor,args,u"adaptor.insert");
                }
            }else if(kind==Value(u"head")){
                const Value replace=cx.get(adaptor,key(u"replace"));
                const std::array<Value,2> args{node,cx.get(root,key(u"head"))};
                const Value replacement=cx.callMember(replace,adaptor,args,u"adaptor.replace");
                cx.set(root,key(u"head"),replacement);
            }else if(kind==Value(u"body")){
                const Value replace=cx.get(adaptor,key(u"replace"));
                const std::array<Value,2> args{node,cx.get(root,key(u"body"))};
                const Value replacement=cx.callMember(replace,adaptor,args,u"adaptor.replace");
                cx.set(root,key(u"body"),replacement);
            }
            return Undefined{};
        });
        parserValue(u"getOnlyChild",2,[weak](Context& cx,Value,std::span<const Value> args)->Value{
            auto self=weak.lock();if(!self)throw Exception("TypeError",u"Lite module has expired");
            const Value adaptor=args.empty()?Value(Undefined{}):args[0];
            const Value body=args.size()<2?Value(Undefined{}):args[1];
            const Value childNodes=cx.get(adaptor,key(u"childNodes"));
            const std::array<Value,1> childArgs{body};
            const Value children=cx.callMember(childNodes,adaptor,childArgs,u"adaptor.childNodes");
            const Value constructor(self->classes[0]);
            Value node=Null{};
            struct Finished{};
            try{
                cx.forEachIterable(children,[&](Value child){
                    if(!instanceOf(cx,child,self->classes[0]))return;
                    if(truthy(node)){node=Null{};throw Finished{};}
                    node=std::move(child);
                });
            }catch(const Finished&){}
            return node;
        });
        parserValue(u"allAttributes",3,[weak](Context& cx,Value receiver,std::span<const Value> args)->Value{
            auto self=weak.lock();if(!self)throw Exception("TypeError",u"Lite module has expired");
            const Value adaptor=args.empty()?Value(Undefined{}):args[0];
            const Value node=args.size()<2?Value(Undefined{}):args[1];
            const Value xml=args.size()<3?Value(Undefined{}):args[2];
            const Value allAttributes=cx.get(adaptor,key(u"allAttributes"));
            const std::array<Value,1> allArgs{node};
            const Value attributes=cx.callMember(allAttributes,adaptor,allArgs,u"adaptor.allAttributes");
            if(!truthy(xml))return attributes;

            const Value kindMethod=cx.get(adaptor,key(u"kind"));
            const std::array<Value,1> kindArgs{node};
            const Value kind=cx.callMember(kindMethod,adaptor,kindArgs,u"adaptor.kind");
            const Value constructor=cx.get(receiver,key(u"constructor"));
            const Value namespaces=cx.get(constructor,key(u"XMLNS"));
            const Key namespaceKey=std::holds_alternative<SymbolRef>(kind.data)
                ?Key(std::get<SymbolRef>(kind.data)):key(primitiveString(cx,kind));
            if(!cx.hasOwn(namespaces,namespaceKey))return attributes;

            struct Found{};
            try{
                cx.forEachIterable(attributes,[&](Value attribute){
                    const Value name=cx.get(attribute,key(u"name"));
                    if(const auto* text=std::get_if<std::u16string>(&name.data);text&&*text==u"xmlns")
                        throw Found{};
                });
            }catch(const Found&){return attributes;}

            // The member call captures push before evaluating the object
            // argument, and the source returns the same array object.
            const Value push=cx.get(attributes,key(u"push"));
            const Value namespaceValue=cx.get(namespaces,namespaceKey);
            const ObjectRef item=cx.object();
            cx.set(Value(item),key(u"name"),Value(u"xmlns"));
            cx.set(Value(item),key(u"value"),namespaceValue);
            const std::array<Value,1> appendArgs{Value(item)};
            (void)cx.callMember(push,attributes,appendArgs,u"attributes.push");
            return attributes;
        });
        defineParserMethod(u"serialize",3,[](Context& cx,Value receiver,std::span<const Value> args)->Value{
            const Value adaptor=args.empty()?Value(Undefined{}):args[0];
            const Value node=args.size()<2?Value(Undefined{}):args[1];
            const Value xml=args.size()<3?Value(Undefined{}):args[2];
            const Value constructor=cx.get(receiver,key(u"constructor"));
            const Value selfClosing=cx.get(constructor,key(u"SELF_CLOSING"));

            const Value kindMethod=cx.get(adaptor,key(u"kind"));
            const std::array<Value,1> kindArgs{node};
            const Value tag=cx.callMember(kindMethod,adaptor,kindArgs,u"adaptor.kind");

            const Value allAttributes=cx.get(receiver,key(u"allAttributes"));
            const std::array<Value,3> attributesArgs{adaptor,node,xml};
            const Value attributes=cx.callMember(allAttributes,receiver,attributesArgs,u"this.allAttributes");
            const Value map=cx.get(attributes,key(u"map"));
            const ObjectRef mapCallback=cx.function(u"",[receiver,xml](Context& context,Value,std::span<const Value> callbackArgs)->Value{
                const Value item=callbackArgs.empty()?Value(Undefined{}):callbackArgs[0];
                const Value name=context.get(item,key(u"name"));
                const auto prefix=implicitString(context,name)+u"=\"";
                const Value protect=context.get(receiver,key(u"protectAttribute"));
                const Value value=context.get(item,key(u"value"));
                const std::array<Value,2> protectArgs{value,xml};
                const Value protectedValue=context.callMember(protect,receiver,protectArgs,u"this.protectAttribute");
                return Value(prefix+implicitString(context,protectedValue)+u"\"");
            });
            cx.define(mapCallback,key(u"length"),Descriptor::data(Value(1),false,false,true));
            const std::array<Value,1> mapArgs{Value(mapCallback)};
            const Value mapped=cx.callMember(map,attributes,mapArgs,u"attributes.map");
            const Value join=cx.get(mapped,key(u"join"));
            const std::array<Value,1> joinArgs{Value(u" ")};
            const Value attributeText=cx.callMember(join,mapped,joinArgs,u"mapped.join");

            const Value serializeInner=cx.get(receiver,key(u"serializeInner"));
            const std::array<Value,3> contentArgs{adaptor,node,xml};
            const Value content=cx.callMember(serializeInner,receiver,contentArgs,u"this.serializeInner");

            // Template interpolation and string addition perform implicit
            // ToString, which rejects Symbols and preserves user conversion
            // callbacks. Keep `tag` raw until each Source interpolation.
            std::u16string result=u"<"+implicitString(cx,tag);
            if(truthy(attributeText))result+=u" "+implicitString(cx,attributeText);
            const bool include=(!truthy(xml)||truthy(content))&&
                !truthy(cx.get(selfClosing,propertyKey(cx,tag)));
            if(include)
                result+=u">"+implicitString(cx,content)+u"</"+implicitString(cx,tag)+u">";
            else if(truthy(xml))result+=u"/>";
            else result+=u">";
            return Value(std::move(result));
        });
        defineParserMethod(u"serializeInner",3,[weak](Context& cx,Value receiver,std::span<const Value> args)->Value{
            auto self=weak.lock();if(!self)throw Exception("TypeError",u"Lite module has expired");
            const Value adaptor=args.empty()?Value(Undefined{}):args[0];
            const Value node=args.size()<2?Value(Undefined{}):args[1];
            const Value xml=args.size()<3?Value(Undefined{}):args[2];
            const Value constructor=cx.get(receiver,key(u"constructor"));
            const Value pcdata=cx.get(constructor,key(u"PCDATA"));
            const Value nodeKind=cx.get(node,key(u"kind"));
            const bool rawText=cx.hasOwn(pcdata,propertyKey(cx,nodeKind));
            const Value childNodes=cx.get(adaptor,key(u"childNodes"));
            const std::array<Value,1> childrenArgs{node};
            const Value children=cx.callMember(childNodes,adaptor,childrenArgs,u"adaptor.childNodes");
            const Value map=cx.get(children,key(u"map"));
            const ObjectRef callback=cx.function(u"",[receiver,adaptor,xml,rawText](Context& context,Value,std::span<const Value> callbackArgs)->Value{
                const Value child=callbackArgs.empty()?Value(Undefined{}):callbackArgs[0];
                if(rawText){
                    const Value value=context.get(adaptor,key(u"value"));
                    const std::array<Value,1> valueArgs{child};
                    return context.callMember(value,adaptor,valueArgs,u"adaptor.value");
                }
                const Value kindMethod=context.get(adaptor,key(u"kind"));
                const std::array<Value,1> kindArgs{child};
                const Value kind=context.callMember(kindMethod,adaptor,kindArgs,u"adaptor.kind");
                if(kind==Value(u"#text")){
                    const Value protect=context.get(receiver,key(u"protectHTML"));
                    const Value value=context.get(adaptor,key(u"value"));
                    const std::array<Value,1> valueArgs{child};
                    const Value text=context.callMember(value,adaptor,valueArgs,u"adaptor.value");
                    const std::array<Value,1> protectArgs{text};
                    return context.callMember(protect,receiver,protectArgs,u"this.protectHTML");
                }
                if(kind==Value(u"#comment"))return context.get(child,key(u"value"));
                const Value serialize=context.get(receiver,key(u"serialize"));
                const std::array<Value,3> serializeArgs{adaptor,child,xml};
                return context.callMember(serialize,receiver,serializeArgs,u"this.serialize");
            });
            cx.define(callback,key(u"length"),Descriptor::data(Value(1),false,false,true));
            const std::array<Value,1> mapArgs{Value(callback)};
            const Value mapped=cx.callMember(map,children,mapArgs,u"childNodes.map");
            const Value join=cx.get(mapped,key(u"join"));
            const std::array<Value,1> joinArgs{Value(u"")};
            return cx.callMember(join,mapped,joinArgs,u"mapped.join");
        });
        const auto protectHTML=context.function(u"protectHTML",[](Context&,Value,std::span<const Value> args)->Value{
            const Value value=args.empty()?Value(Undefined{}):args[0];
            if(const auto* text=std::get_if<std::u16string>(&value.data)){
                LiteParser parser;return Value(parser.protectHTML(*text));
            }
            if(std::holds_alternative<Undefined>(value.data))throw Exception("TypeError",u"Cannot read properties of undefined (reading 'replace')");
            if(std::holds_alternative<Null>(value.data))throw Exception("TypeError",u"Cannot read properties of null (reading 'replace')");
            throw Exception("TypeError",u"text.replace is not a function");
        });
        context.define(protectHTML,key(u"length"),Descriptor::data(Value(1),false,false,true));
        context.define(parserProto,key(u"protectHTML"),Descriptor::data(Value(protectHTML),true,true,true));
        const auto protectAttribute=context.function(u"protectAttribute",[](Context& cx,Value,std::span<const Value> args)->Value{
            const Value value=args.empty()?Value(Undefined{}):args[0];
            const auto text=primitiveString(cx,value);
            const Value xml=args.size()<2?Value(Undefined{}):args[1];
            LiteParser parser;return Value(parser.protectAttribute(text,truthy(xml)));
        });
        context.define(protectAttribute,key(u"length"),Descriptor::data(Value(2),false,false,true));
        context.define(parserProto,key(u"protectAttribute"),Descriptor::data(Value(protectAttribute),true,true,true));
        // Comment extends Text and shares its inherited prototype behavior.
        const auto commentProto=std::get<ObjectRef>(context.get(Value(classes[2]),key(u"prototype")).data);
        const auto textProto=std::get<ObjectRef>(context.get(Value(classes[1]),key(u"prototype")).data);
        context.setPrototype(commentProto,textProto);
        auto defineKind=[&](ObjectRef ctor,std::u16string text){
            const auto proto=std::get<ObjectRef>(context.get(Value(ctor),key(u"prototype")).data);
            auto getter=context.function(u"get kind",[text=std::move(text)](Context&,Value,std::span<const Value>){return Value(text);});
            context.define(proto,key(u"kind"),Descriptor::access(getter,std::nullopt,false,true));
        };
        defineKind(classes[1],u"#text");defineKind(classes[2],u"#comment");defineKind(classes[3],u"#document");
        const auto elementProto=std::get<ObjectRef>(context.get(Value(classes[0]),key(u"prototype")).data);
        const auto addEventListener=context.function(u"addEventListener",
            [weak](Context&,Value receiver,std::span<const Value> args)->Value{
                auto self=weak.lock();
                if(!self)throw Exception("TypeError",u"Lite module has expired");
                const auto* target=std::get_if<ObjectRef>(&receiver.data);
                if(!target)throw Exception("TypeError",u"LiteElement.addEventListener receiver is not an object");
                const auto node=self->nodeFor(*target);
                if(!node||self->store->record(*node).nodeKind!=NodeKind::Element)
                    throw Exception("TypeError",u"LiteElement.addEventListener receiver is not a Lite element");
                const Value typeValue=args.empty()?Value(Undefined{}):args[0];
                const Value callback=args.size()>1?args[1]:Value(Undefined{});
                self->registerHostEvent(*target,typeValue,callback);
                return Undefined{};
            });
        context.define(addEventListener,key(u"length"),Descriptor::data(Value(2),false,false,true));
        if(!context.hasOwn(Value(elementProto),key(u"addEventListener")))
            context.define(elementProto,key(u"addEventListener"),
                Descriptor::data(Value(addEventListener),true,true,true));
        const auto listProto=std::get<ObjectRef>(context.get(Value(classes[4]),key(u"prototype")).data);
        context.define(listProto,key(u"append"),Descriptor::data(Value(context.function(u"append",[weak](Context& cx,Value receiver,std::span<const Value> args)->Value{
            auto self=weak.lock();if(!self)throw Exception("TypeError",u"Lite module has expired");
            if(args.empty())throw Exception("TypeError",u"LiteList.append requires a node");
            auto* object=std::get_if<ObjectRef>(&receiver.data);if(!object)throw Exception("TypeError",u"LiteList.append receiver is not an object");
            auto nodes=cx.get(Value(*object),key(u"nodes"));auto* array=std::get_if<ObjectRef>(&nodes.data);
            if(!array)throw Exception("TypeError",u"this.nodes.push is not a function");
            const std::array<Value,1> value{args[0]};cx.arrayPush(*array,value);return Undefined{};
        })),true,true,true));
        const auto listIterator=context.function(u"[Symbol.iterator]",[weak](Context& cx,Value,std::span<const Value>)->Value{
            auto self=weak.lock();if(!self)throw Exception("TypeError",u"Lite module has expired");
            const auto iterator=cx.object();
            const auto next=cx.function(u"next",[](Context& c,Value receiver,std::span<const Value>)->Value{
                const auto nodes=c.get(receiver,key(u"nodes"));
                (void)c.get(nodes,key(u"length"));
                return Undefined{};
            });
            cx.define(iterator,key(u"next"),Descriptor::data(Value(next),true,true,true));return Value(iterator);
        });
        context.define(listProto,Key(context.intrinsics().symbolIterator),Descriptor::data(Value(listIterator),true,true,true));

        adaptorClass=context.nativeConstructor(u"LiteAdaptor",[weak](Context&,std::span<const Value> args)->ObjectRef{
            auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
            return s->bindAdaptor(s->constructAdaptor(args));
        });
        context.define(adaptorClass,key(u"length"),Descriptor::data(Value(0),false,false,true));
        const auto options=context.object();
        const auto classDefaults=LiteAdaptor::classOptions();
        for(const auto& name:classDefaults.keys())
            context.set(Value(options),key(property_host::text(name)),toContext(classDefaults.get(name)));
        context.define(nodeAdaptorClass,key(u"OPTIONS"),Descriptor::data(Value(options),true,true,true));

        const auto adaptorProto=std::get<ObjectRef>(context.get(Value(adaptorClass),key(u"prototype")).data);
        context.setPrototype(adaptorProto,nodeAdaptorProto);
        context.setPrototype(adaptorClass,nodeAdaptorClass);
        auto installAdaptorMethod=[&](std::u16string name,std::size_t arity,NativeCall body){
            const auto fn=context.function(name,std::move(body));
            context.define(fn,key(u"length"),Descriptor::data(Value(static_cast<double>(arity)),false,false,true));
            const bool mixin=name==u"fontSize"||name==u"fontFamily"||name==u"nodeSize"||name==u"nodeBBox";
            context.define(mixin?nodeAdaptorProto:baseProto,key(std::move(name)),Descriptor::data(Value(fn),true,true,true));
        };
        auto installBaseMeasurement=[&](std::u16string name,std::size_t arity,NativeCall body){
            const auto fn=context.function(name,std::move(body));
            context.define(fn,key(u"length"),Descriptor::data(Value(static_cast<double>(arity)),false,false,true));
            context.define(baseProto,key(std::move(name)),Descriptor::data(Value(fn),true,true,true));
        };
        installBaseMeasurement(u"fontSize",1,[](Context&,Value,std::span<const Value>){return Value(0.0);});
        installBaseMeasurement(u"fontFamily",1,[](Context&,Value,std::span<const Value>){return Value(std::u16string{});});
        installBaseMeasurement(u"nodeSize",3,[](Context& cx,Value,std::span<const Value>){const auto result=cx.array(2);cx.set(Value(result),key(u"0"),Value(0.0));cx.set(Value(result),key(u"1"),Value(0.0));return Value(result);});
        installBaseMeasurement(u"nodeBBox",1,[](Context& cx,Value,std::span<const Value>){const auto result=cx.object();for(const auto name:{u"left",u"right",u"top",u"bottom"})cx.set(Value(result),key(name),Value(0.0));return Value(result);});
        auto adapterForReceiver=[weak](Context&,Value receiver)->std::shared_ptr<LiteBase>{
            auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
            const auto* object=std::get_if<ObjectRef>(&receiver.data);if(!object)throw Exception("TypeError",u"LiteAdaptor method receiver is not an object");
            return s->resolveBaseAdaptor(*object);
        };
        auto requireNode=[weak](Value value)->LiteNodeRef{
            auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
            if(const auto* object=std::get_if<ObjectRef>(&value.data)){
                if(auto node=s->nodeFor(*object))return *node;
                throw Exception("TypeError",u"Expected a Lite DOM node");
            }
            const auto converted=s->properties.from(std::move(value));
            auto node=LiteNodeStore::fromProperty(converted);if(!node)throw Exception("TypeError",u"Expected a Lite DOM node");return *node;
        };
        auto valueAt=[](std::span<const Value> args,std::size_t index)->Value{return index<args.size()?args[index]:Value(Undefined{});};
        auto nodeOperation=[weak](Context& cx,std::u16string_view name,
                                  std::span<const Value> args)->std::optional<Value>{
            auto state=weak.lock();
            if(!state)throw Exception("TypeError",u"Lite module has expired");
            if(state->nodeOperationTarget)return state->nodeOperationTarget(cx,name,args);
            return {};
        };
        auto sourceString=[](Context& cx,const Value& value){return primitiveString(cx,value);};
        auto nodeResult=[weak](LiteNodeRef node)->Value{auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");return Value(s->objectFor(node));};
        auto valueResult=[weak](const DOMNodeValue& value)->Value{auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");return s->toDomValue(value);};
        auto propertyResult=[weak](const Property& value)->Value{auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");return s->properties.to(value);};

        installAdaptorMethod(u"text",1,[adapterForReceiver,sourceString,nodeResult](Context& cx,Value receiver,std::span<const Value> args){auto adaptor=adapterForReceiver(cx,receiver);return nodeResult(adaptor->text(sourceString(cx,args.empty()?Value(Undefined{}):args[0])));});
        installAdaptorMethod(u"comment",1,[adapterForReceiver,sourceString,nodeResult](Context& cx,Value receiver,std::span<const Value> args){auto adaptor=adapterForReceiver(cx,receiver);return nodeResult(adaptor->comment(sourceString(cx,args.empty()?Value(Undefined{}):args[0])));});
        installAdaptorMethod(u"createDocument",0,[adapterForReceiver,nodeResult](Context& cx,Value receiver,std::span<const Value>){auto adaptor=adapterForReceiver(cx,receiver);return nodeResult(adaptor->createDocument());});
        // LiteBase.create and AbstractDOMAdaptor.node/setAttributes retain
        // Source method dispatch, evaluation order and the canonical nodes.
        installAdaptorMethod(u"create",2,[weak,valueAt](Context& cx,Value,std::span<const Value> args){
            const auto state=weak.lock();
            if(!state)throw Exception("TypeError",u"Lite module has expired");
            const std::array<Value,1> constructorArgs{valueAt(args,0)};
            return Value(cx.construct(state->classes[0],constructorArgs));
        });
        installAdaptorMethod(u"node",4,[valueAt](Context& cx,Value receiver,std::span<const Value> args){
            const std::array<Value,2> createArgs{valueAt(args,0),valueAt(args,3)};
            const auto created=cx.callMember(cx.get(receiver,key(u"create")),receiver,createArgs,u"this.create");
            auto definition=valueAt(args,1);
            if(std::holds_alternative<Undefined>(definition.data))definition=Value(cx.object());
            const std::array<Value,2> attributesArgs{created,definition};
            (void)cx.callMember(cx.get(receiver,key(u"setAttributes")),receiver,attributesArgs,u"this.setAttributes");
            auto children=valueAt(args,2);
            if(std::holds_alternative<Undefined>(children.data))children=Value(cx.array());
            cx.forEachIterable(children,[&](Value child){
                const std::array<Value,2> appendArgs{created,std::move(child)};
                (void)cx.callMember(cx.get(receiver,key(u"append")),receiver,appendArgs,u"this.append");
            });
            return created;
        });
        installAdaptorMethod(u"setAttributes",2,[valueAt](Context& cx,Value receiver,std::span<const Value> args){
            const auto node=valueAt(args,0),definition=valueAt(args,1);
            if(truthy(cx.get(definition,key(u"style")))&&
               !std::holds_alternative<std::u16string>(cx.get(definition,key(u"style")).data)){
                const auto names=cx.enumerableStringKeys(cx.get(definition,key(u"style")));
                for(const auto& name:names){
                    std::u16string camel;
                    for(std::size_t index=0;index<name.size();++index){
                        if(name[index]==u'-'&&index+1<name.size()&&name[index+1]>=u'a'&&name[index+1]<=u'z')
                            camel+=static_cast<char16_t>(name[++index]-u'a'+u'A');
                        else camel+=name[index];
                    }
                    const auto method=cx.get(receiver,key(u"setStyle"));
                    const std::array<Value,3> styleArgs{node,Value(camel),
                        cx.get(cx.get(definition,key(u"style")),key(name))};
                    (void)cx.callMember(method,receiver,styleArgs,u"this.setStyle");
                }
            }
            if(truthy(cx.get(definition,key(u"properties")))){
                const auto names=cx.enumerableStringKeys(cx.get(definition,key(u"properties")));
                for(const auto& name:names)
                    cx.set(node,key(name),cx.get(cx.get(definition,key(u"properties")),key(name)));
            }
            for(const auto& name:cx.enumerableStringKeys(definition)){
                if((name!=u"style"||std::holds_alternative<std::u16string>(cx.get(definition,key(u"style")).data))&&
                   name!=u"properties"){
                    const auto method=cx.get(receiver,key(u"setAttribute"));
                    const std::array<Value,3> attributeArgs{node,Value(name),cx.get(definition,key(name))};
                    (void)cx.callMember(method,receiver,attributeArgs,u"this.setAttribute");
                }
            }
            return Value(Undefined{});
        });
        installAdaptorMethod(u"parse",2,[adapterForReceiver,sourceString,valueAt,nodeResult](Context& cx,Value receiver,std::span<const Value> args){
            auto adaptor=adapterForReceiver(cx,receiver);const auto text=sourceString(cx,valueAt(args,0));const auto format=valueAt(args,1);
            return nodeResult(adaptor->parse(text,std::holds_alternative<Undefined>(format.data)?std::u16string_view{}:sourceString(cx,format)));
        });
        installAdaptorMethod(u"clone",2,[adapterForReceiver,requireNode,nodeResult,valueAt](Context& cx,Value receiver,std::span<const Value> args){auto adaptor=adapterForReceiver(cx,receiver);const auto deep=valueAt(args,1);return nodeResult(adaptor->clone(requireNode(valueAt(args,0)),std::holds_alternative<Undefined>(deep.data)?true:truthy(deep)));});
        installAdaptorMethod(u"split",2,[adapterForReceiver,requireNode,nodeResult,valueAt](Context& cx,Value receiver,std::span<const Value> args){auto adaptor=adapterForReceiver(cx,receiver);return nodeResult(adaptor->split(requireNode(valueAt(args,0)),cx.toNumber(valueAt(args,1))));});
        installAdaptorMethod(u"insertRules",2,[adapterForReceiver,requireNode,valueAt,sourceString](Context& cx,Value receiver,std::span<const Value> args){auto adaptor=adapterForReceiver(cx,receiver);std::vector<std::u16string> rules;cx.forEachIterable(valueAt(args,1),[&](Value rule){rules.push_back(sourceString(cx,rule));});adaptor->insertRules(requireNode(valueAt(args,0)),rules);return Value(Undefined{});});
        installAdaptorMethod(u"append",2,[weak,valueAt](Context& cx,Value receiver,std::span<const Value> args){
            // Keep the Source method's observable property and method access
            // order. In particular, a missing parent argument fails while
            // reading `children`, before any native Lite-node conversion.
            const Value node=valueAt(args,0);
            const Value child=valueAt(args,1);
            if(truthy(cx.get(child,key(u"parent")))){
                const Value remove=cx.get(receiver,key(u"remove"));
                const std::array<Value,1> removeArgs{child};
                (void)cx.callMember(remove,receiver,removeArgs,u"this.remove");
            }
            const Value children=cx.get(node,key(u"children"));
            const Value push=cx.get(children,key(u"push"));
            std::optional<Value> intercepted;
            if(auto state=weak.lock();state&&state->appendTarget)
                intercepted=state->appendTarget(cx,receiver,args);
            const std::array<Value,1> pushArgs{child};
            (void)cx.callMember(push,children,pushArgs,u"children.push");
            cx.set(child,key(u"parent"),node);
            return intercepted?std::move(*intercepted):child;
        });
        installAdaptorMethod(u"insert",2,[adapterForReceiver,requireNode,valueAt,nodeOperation](Context& cx,Value receiver,std::span<const Value> args){if(auto result=nodeOperation(cx,u"insert",args))return std::move(*result);auto adaptor=adapterForReceiver(cx,receiver);adaptor->insert(requireNode(valueAt(args,0)),requireNode(valueAt(args,1)));return Value(Undefined{});});
        installAdaptorMethod(u"remove",1,[adapterForReceiver,requireNode,nodeResult,valueAt,nodeOperation](Context& cx,Value receiver,std::span<const Value> args){if(auto result=nodeOperation(cx,u"remove",args))return std::move(*result);auto adaptor=adapterForReceiver(cx,receiver);return nodeResult(adaptor->remove(requireNode(valueAt(args,0))));});
        installAdaptorMethod(u"replace",2,[adapterForReceiver,requireNode,nodeResult,valueAt,nodeOperation](Context& cx,Value receiver,std::span<const Value> args){if(auto result=nodeOperation(cx,u"replace",args))return std::move(*result);auto adaptor=adapterForReceiver(cx,receiver);return nodeResult(adaptor->replace(requireNode(valueAt(args,0)),requireNode(valueAt(args,1))));});
        installAdaptorMethod(u"parent",1,[adapterForReceiver,requireNode,valueResult,valueAt,nodeOperation](Context& cx,Value receiver,std::span<const Value> args){if(auto result=nodeOperation(cx,u"parent",args))return std::move(*result);auto adaptor=adapterForReceiver(cx,receiver);return valueResult(adaptor->parent(requireNode(valueAt(args,0))));});
        installAdaptorMethod(u"next",1,[adapterForReceiver,requireNode,valueResult,valueAt,nodeOperation](Context& cx,Value receiver,std::span<const Value> args){if(auto result=nodeOperation(cx,u"next",args))return std::move(*result);auto adaptor=adapterForReceiver(cx,receiver);return valueResult(adaptor->next(requireNode(valueAt(args,0))));});
        installAdaptorMethod(u"previous",1,[adapterForReceiver,requireNode,valueResult,valueAt,nodeOperation](Context& cx,Value receiver,std::span<const Value> args){if(auto result=nodeOperation(cx,u"previous",args))return std::move(*result);auto adaptor=adapterForReceiver(cx,receiver);return valueResult(adaptor->previous(requireNode(valueAt(args,0))));});
        installAdaptorMethod(u"firstChild",1,[adapterForReceiver,requireNode,valueResult,valueAt,nodeOperation](Context& cx,Value receiver,std::span<const Value> args){if(auto result=nodeOperation(cx,u"firstChild",args))return std::move(*result);auto adaptor=adapterForReceiver(cx,receiver);return valueResult(adaptor->firstChild(requireNode(valueAt(args,0))));});
        installAdaptorMethod(u"lastChild",1,[adapterForReceiver,requireNode,valueResult,valueAt,nodeOperation](Context& cx,Value receiver,std::span<const Value> args){if(auto result=nodeOperation(cx,u"lastChild",args))return std::move(*result);auto adaptor=adapterForReceiver(cx,receiver);return valueResult(adaptor->lastChild(requireNode(valueAt(args,0))));});
        installAdaptorMethod(u"childNode",2,[adapterForReceiver,requireNode,valueResult,valueAt,nodeOperation](Context& cx,Value receiver,std::span<const Value> args){if(auto result=nodeOperation(cx,u"childNode",args))return std::move(*result);auto adaptor=adapterForReceiver(cx,receiver);return valueResult(adaptor->childNode(requireNode(valueAt(args,0)),cx.toNumber(valueAt(args,1))));});
        installAdaptorMethod(u"childNodes",1,[adapterForReceiver,requireNode,weak,valueAt,nodeOperation](Context& cx,Value receiver,std::span<const Value> args){if(auto result=nodeOperation(cx,u"childNodes",args))return std::move(*result);auto adaptor=adapterForReceiver(cx,receiver);auto nodes=adaptor->childNodes(requireNode(valueAt(args,0)));const auto array=cx.array();auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");for(std::size_t i=0;i<nodes.slots.size();++i)cx.arrayPush(array,std::array<Value,1>{s->properties.to(nodes.slots.get(i))});return Value(array);});
        installAdaptorMethod(u"contains",2,[adapterForReceiver,requireNode,valueAt](Context& cx,Value receiver,std::span<const Value> args){auto adaptor=adapterForReceiver(cx,receiver);return Value(adaptor->contains(requireNode(valueAt(args,0)),requireNode(valueAt(args,1))));});
        installAdaptorMethod(u"kind",1,[adapterForReceiver,requireNode,valueAt,nodeOperation](Context& cx,Value receiver,std::span<const Value> args){if(auto result=nodeOperation(cx,u"kind",args))return std::move(*result);auto adaptor=adapterForReceiver(cx,receiver);return Value(adaptor->kind(requireNode(valueAt(args,0))));});
        installAdaptorMethod(u"value",1,[adapterForReceiver,requireNode,valueAt](Context& cx,Value receiver,std::span<const Value> args){auto adaptor=adapterForReceiver(cx,receiver);return Value(adaptor->value(requireNode(valueAt(args,0))));});
        installAdaptorMethod(u"textContent",1,[](Context& cx,Value receiver,std::span<const Value> args){
            const Value node=args.empty()?Value(Undefined{}):args.front();
            return sourceTextContent(cx,std::move(receiver),node);
        });
        installAdaptorMethod(u"innerHTML",1,[adapterForReceiver,requireNode,valueAt](Context& cx,Value receiver,std::span<const Value> args){auto adaptor=adapterForReceiver(cx,receiver);return Value(adaptor->innerHTML(requireNode(valueAt(args,0))));});
        installAdaptorMethod(u"outerHTML",1,[adapterForReceiver,requireNode,valueAt](Context& cx,Value receiver,std::span<const Value> args){auto adaptor=adapterForReceiver(cx,receiver);return Value(adaptor->outerHTML(requireNode(valueAt(args,0))));});
        installAdaptorMethod(u"serializeXML",1,[adapterForReceiver,requireNode,valueAt](Context& cx,Value receiver,std::span<const Value> args){auto adaptor=adapterForReceiver(cx,receiver);return Value(adaptor->serializeXML(requireNode(valueAt(args,0))));});
        installAdaptorMethod(u"setProperty",3,[adapterForReceiver,requireNode,sourceString,valueAt,weak](Context& cx,Value receiver,std::span<const Value> args){auto adaptor=adapterForReceiver(cx,receiver);auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");adaptor->setProperty(requireNode(valueAt(args,0)),property_host::debugText(sourceString(cx,valueAt(args,1))),s->fromContext(valueAt(args,2)));return Value(Undefined{});});
        installAdaptorMethod(u"getProperty",2,[adapterForReceiver,requireNode,sourceString,valueAt,propertyResult](Context& cx,Value receiver,std::span<const Value> args){auto adaptor=adapterForReceiver(cx,receiver);return propertyResult(adaptor->getProperty(requireNode(valueAt(args,0)),property_host::debugText(sourceString(cx,valueAt(args,1)))));});
        installAdaptorMethod(u"setAttribute",4,[adapterForReceiver,requireNode,sourceString,valueAt,weak](Context& cx,Value receiver,std::span<const Value> args){auto adaptor=adapterForReceiver(cx,receiver);auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");if(s->attributeTarget)if(auto intercepted=s->attributeTarget(cx,receiver,args))return std::move(*intercepted);const auto ns=valueAt(args,3);const auto stringValue=Property(property_host::debugText(sourceString(cx,valueAt(args,2))));adaptor->setAttribute(requireNode(valueAt(args,0)),sourceString(cx,valueAt(args,1)),stringValue,std::holds_alternative<Undefined>(ns.data)||std::holds_alternative<Null>(ns.data)?std::u16string_view{}:sourceString(cx,ns));return Value(Undefined{});});
        installAdaptorMethod(u"getAttribute",2,[adapterForReceiver,requireNode,sourceString,valueAt,propertyResult,nodeOperation](Context& cx,Value receiver,std::span<const Value> args){if(auto result=nodeOperation(cx,u"getAttribute",args))return std::move(*result);auto adaptor=adapterForReceiver(cx,receiver);return propertyResult(adaptor->getAttribute(requireNode(valueAt(args,0)),sourceString(cx,valueAt(args,1))));});
        installAdaptorMethod(u"removeAttribute",2,[adapterForReceiver,requireNode,sourceString,valueAt](Context& cx,Value receiver,std::span<const Value> args){auto adaptor=adapterForReceiver(cx,receiver);adaptor->removeAttribute(requireNode(valueAt(args,0)),sourceString(cx,valueAt(args,1)));return Value(Undefined{});});
        installAdaptorMethod(u"hasAttribute",2,[adapterForReceiver,requireNode,sourceString,valueAt](Context& cx,Value receiver,std::span<const Value> args){auto adaptor=adapterForReceiver(cx,receiver);return Value(adaptor->hasAttribute(requireNode(valueAt(args,0)),sourceString(cx,valueAt(args,1))));});
        installAdaptorMethod(u"allAttributes",1,[adapterForReceiver,requireNode,weak,valueAt](Context& cx,Value receiver,std::span<const Value> args){auto adaptor=adapterForReceiver(cx,receiver);auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");const auto result=cx.array();for(const auto& attr:adaptor->allAttributes(requireNode(valueAt(args,0)))){const auto item=cx.object();cx.set(Value(item),key(u"name"),Value(attr.name));cx.set(Value(item),key(u"value"),s->properties.to(attr.value));cx.arrayPush(result,std::array<Value,1>{Value(item)});}return Value(result);});
        installAdaptorMethod(u"addClass",2,[adapterForReceiver,requireNode,sourceString,valueAt](Context& cx,Value receiver,std::span<const Value> args){auto adaptor=adapterForReceiver(cx,receiver);adaptor->addClass(requireNode(valueAt(args,0)),sourceString(cx,valueAt(args,1)));return Value(Undefined{});});
        installAdaptorMethod(u"removeClass",2,[adapterForReceiver,requireNode,sourceString,valueAt](Context& cx,Value receiver,std::span<const Value> args){auto adaptor=adapterForReceiver(cx,receiver);adaptor->removeClass(requireNode(valueAt(args,0)),sourceString(cx,valueAt(args,1)));return Value(Undefined{});});
        installAdaptorMethod(u"hasClass",2,[adapterForReceiver,requireNode,sourceString,valueAt](Context& cx,Value receiver,std::span<const Value> args){auto adaptor=adapterForReceiver(cx,receiver);return Value(adaptor->hasClass(requireNode(valueAt(args,0)),sourceString(cx,valueAt(args,1))));});
        installAdaptorMethod(u"setStyle",3,[adapterForReceiver,requireNode,sourceString,valueAt,weak](Context& cx,Value receiver,std::span<const Value> args){auto adaptor=adapterForReceiver(cx,receiver);auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");if(s->styleTarget)if(auto intercepted=s->styleTarget(cx,receiver,args))return std::move(*intercepted);adaptor->setStyle(requireNode(valueAt(args,0)),sourceString(cx,valueAt(args,1)),sourceString(cx,valueAt(args,2)));return Value(Undefined{});});
        installAdaptorMethod(u"getStyle",2,[adapterForReceiver,requireNode,sourceString,valueAt](Context& cx,Value receiver,std::span<const Value> args){auto adaptor=adapterForReceiver(cx,receiver);return Value(adaptor->getStyle(requireNode(valueAt(args,0)),sourceString(cx,valueAt(args,1))));});
        installAdaptorMethod(u"allStyles",1,[adapterForReceiver,requireNode,valueAt,propertyResult](Context& cx,Value receiver,std::span<const Value> args){auto adaptor=adapterForReceiver(cx,receiver);return propertyResult(adaptor->allStyles(requireNode(valueAt(args,0))));});
        installAdaptorMethod(u"fontSize",1,[adapterForReceiver,requireNode,valueAt](Context& cx,Value receiver,std::span<const Value> args){auto adaptor=adapterForReceiver(cx,receiver);return Value(adaptor->fontSize(requireNode(valueAt(args,0))));});
        installAdaptorMethod(u"fontFamily",1,[adapterForReceiver,weak](Context& cx,Value receiver,std::span<const Value>){auto adaptor=std::dynamic_pointer_cast<LiteAdaptor>(adapterForReceiver(cx,receiver));if(!adaptor)throw Exception("TypeError",u"LiteAdaptor method receiver is not a LiteAdaptor");auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");return s->properties.to(adaptor->fontFamilyProperty());});
        installAdaptorMethod(u"nodeSize",3,[adapterForReceiver,requireNode,valueAt](Context& cx,Value receiver,std::span<const Value> args){auto adaptor=adapterForReceiver(cx,receiver);const auto em=cx.toNumber(valueAt(args,1));const auto local=valueAt(args,2);std::optional<bool> localValue;if(!std::holds_alternative<Undefined>(local.data)&&!std::holds_alternative<Null>(local.data))localValue=truthy(local);const auto [width,height]=adaptor->nodeSize(requireNode(valueAt(args,0)),em,localValue);const auto result=cx.array(2);cx.set(Value(result),key(u"0"),Value(width));cx.set(Value(result),key(u"1"),Value(height));return Value(result);});
        installAdaptorMethod(u"nodeBBox",1,[adapterForReceiver,requireNode,valueAt](Context& cx,Value receiver,std::span<const Value> args){auto adaptor=adapterForReceiver(cx,receiver);const auto box=adaptor->nodeBBox(requireNode(valueAt(args,0)));const auto result=cx.object();cx.set(Value(result),key(u"left"),Value(box.left));cx.set(Value(result),key(u"top"),Value(box.top));cx.set(Value(result),key(u"right"),Value(box.right));cx.set(Value(result),key(u"bottom"),Value(box.bottom));return Value(result);});

        // Remaining AbstractDOMAdaptor/LiteBase methods are bound on the same
        // canonical adaptor instance.  Optional document arguments retain the
        // Source missing/null distinction until the method body reads a field.
        installAdaptorMethod(u"head",1,[adapterForReceiver,valueResult,weak](Context& cx,Value receiver,std::span<const Value> args){
            auto adaptor=adapterForReceiver(cx,receiver);auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
            return valueResult(adaptor->head(args.empty()?LiteNodeValue(Missing{}):s->fromDomValue(args[0])));
        });
        installAdaptorMethod(u"body",1,[adapterForReceiver,valueResult,weak](Context& cx,Value receiver,std::span<const Value> args){
            auto adaptor=adapterForReceiver(cx,receiver);auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
            return valueResult(adaptor->body(args.empty()?LiteNodeValue(Missing{}):s->fromDomValue(args[0])));
        });
        installAdaptorMethod(u"root",1,[adapterForReceiver,valueResult,weak](Context& cx,Value receiver,std::span<const Value> args){
            auto adaptor=adapterForReceiver(cx,receiver);auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
            return valueResult(adaptor->root(args.empty()?LiteNodeValue(Missing{}):s->fromDomValue(args[0])));
        });
        installAdaptorMethod(u"doctype",1,[adapterForReceiver,weak](Context& cx,Value receiver,std::span<const Value> args){
            auto adaptor=adapterForReceiver(cx,receiver);auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
            return Value(adaptor->doctype(args.empty()?LiteNodeValue(Missing{}):s->fromDomValue(args[0])));
        });
        installAdaptorMethod(u"childIndex",1,[adapterForReceiver,requireNode,valueAt](Context& cx,Value receiver,std::span<const Value> args){
            auto adaptor=adapterForReceiver(cx,receiver);return Value(static_cast<double>(adaptor->childIndex(requireNode(valueAt(args,0)))));
        });
        installAdaptorMethod(u"elementById",2,[adapterForReceiver,requireNode,sourceString,valueAt,nodeResult](Context& cx,Value receiver,std::span<const Value> args){
            auto adaptor=adapterForReceiver(cx,receiver);auto node=adaptor->elementById(requireNode(valueAt(args,0)),sourceString(cx,valueAt(args,1)));
            return node?nodeResult(node):Value(Null{});
        });
        installAdaptorMethod(u"elementsByClass",3,[adapterForReceiver,requireNode,sourceString,valueAt,weak](Context& cx,Value receiver,std::span<const Value> args){
            auto adaptor=adapterForReceiver(cx,receiver);auto stop=valueAt(args,2);std::optional<std::size_t> limit;
            if(!std::holds_alternative<Undefined>(stop.data)&&!std::holds_alternative<Null>(stop.data)){const auto n=cx.toNumber(stop);if(std::isfinite(n)&&n>=0)limit=static_cast<std::size_t>(n);}
            const auto nodes=adaptor->elementsByClass(requireNode(valueAt(args,0)),sourceString(cx,valueAt(args,1)),limit);
            auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");const auto result=cx.array();for(auto node:nodes)cx.arrayPush(result,std::array<Value,1>{Value(s->objectFor(node))});return Value(result);
        });
        installAdaptorMethod(u"elementsByAttribute",4,[adapterForReceiver,requireNode,sourceString,valueAt,weak](Context& cx,Value receiver,std::span<const Value> args){
            auto adaptor=adapterForReceiver(cx,receiver);auto stop=valueAt(args,3);std::optional<std::size_t> limit;
            if(!std::holds_alternative<Undefined>(stop.data)&&!std::holds_alternative<Null>(stop.data)){const auto n=cx.toNumber(stop);if(std::isfinite(n)&&n>=0)limit=static_cast<std::size_t>(n);}
            const auto nodes=adaptor->elementsByAttribute(requireNode(valueAt(args,0)),sourceString(cx,valueAt(args,1)),sourceString(cx,valueAt(args,2)),limit);
            auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");const auto result=cx.array();for(auto node:nodes)cx.arrayPush(result,std::array<Value,1>{Value(s->objectFor(node))});return Value(result);
        });
        installAdaptorMethod(u"tags",4,[adapterForReceiver,requireNode,sourceString,valueAt,weak](Context& cx,Value receiver,std::span<const Value> args){
            auto adaptor=adapterForReceiver(cx,receiver);const auto ns=valueAt(args,2),stop=valueAt(args,3);std::optional<std::size_t> limit;
            if(!std::holds_alternative<Undefined>(stop.data)&&!std::holds_alternative<Null>(stop.data)){const auto n=cx.toNumber(stop);if(std::isfinite(n)&&n>=0)limit=static_cast<std::size_t>(n);}
            const auto nodes=adaptor->tags(requireNode(valueAt(args,0)),sourceString(cx,valueAt(args,1)),
                std::holds_alternative<Undefined>(ns.data)||std::holds_alternative<Null>(ns.data)?std::u16string{}:sourceString(cx,ns),limit);
            auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");const auto result=cx.array();for(auto node:nodes)cx.arrayPush(result,std::array<Value,1>{Value(s->objectFor(node))});return Value(result);
        });
        installAdaptorMethod(u"allClasses",1,[adapterForReceiver,requireNode,weak,valueAt](Context& cx,Value receiver,std::span<const Value> args){
            auto adaptor=adapterForReceiver(cx,receiver);auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");const auto result=cx.array();for(const auto& name:adaptor->allClasses(requireNode(valueAt(args,0))))cx.arrayPush(result,std::array<Value,1>{Value(name)});return Value(result);
        });
        installAdaptorMethod(u"cssText",1,[adapterForReceiver,requireNode,valueAt](Context& cx,Value receiver,std::span<const Value> args){
            auto adaptor=adapterForReceiver(cx,receiver);return Value(adaptor->cssText(requireNode(valueAt(args,0))));
        });
        installAdaptorMethod(u"getElement",2,[adapterForReceiver,sourceString,valueAt,weak,valueResult](Context& cx,Value receiver,std::span<const Value> args){
            auto adaptor=adapterForReceiver(cx,receiver);auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
            return valueResult(adaptor->getElement(sourceString(cx,valueAt(args,0)),args.size()<2?LiteNodeValue(Missing{}):s->fromDomValue(args[1])));
        });
        installAdaptorMethod(u"getElements",2,[adapterForReceiver,valueAt,weak](Context& cx,Value receiver,std::span<const Value> args){
            auto adaptor=adapterForReceiver(cx,receiver);auto s=weak.lock();if(!s)throw Exception("TypeError",u"Lite module has expired");
            const auto document=s->fromDomValue(valueAt(args,1));
            const auto body=adaptor->body(document);
            if(!std::holds_alternative<LiteNodeRef>(body))throw Exception("TypeError",u"Cannot read properties of null (reading 'attributes')");
            const auto bodyNode=std::get<LiteNodeRef>(body);
            const auto containers=cx.array();
            ObjectRef result=containers;
            auto push=[&](const Value& item){const auto fn=cx.get(Value(result),key(u"push"));const std::array<Value,1> one{item};cx.callMember(fn,Value(result),one,u"containers.push");};
            auto addConcat=[&](Value item){
                const auto fn=cx.get(Value(result),key(u"concat"));const std::array<Value,1> one{std::move(item)};
                const auto joined=cx.callMember(fn,Value(result),one,u"containers.concat");
                const auto* next=std::get_if<ObjectRef>(&joined.data);if(!next||cx.kind(*next)!=ObjectKind::array)throw Exception("TypeError",u"containers.concat is not an array");result=*next;
            };
            auto isInstance=[&](const Value& candidate,const Value& constructor){
                const auto* object=std::get_if<ObjectRef>(&candidate.data);if(!object)return false;
                const auto prototype=cx.get(constructor,key(u"prototype"));const auto* wanted=std::get_if<ObjectRef>(&prototype.data);if(!wanted)return false;
                auto cursor=cx.prototype(*object);while(cursor){if(*cursor==*wanted)return true;cursor=cx.prototype(*cursor);}return false;
            };
            cx.forEachIterable(valueAt(args,0),[&](Value selector){
                if(const auto* text=std::get_if<std::u16string>(&selector.data)){
                    if(text->empty())return;
                    const auto first=text->front();
                    if(first==u'#'){
                        const auto node=adaptor->elementById(bodyNode,std::u16string_view(*text).substr(1));if(node)push(Value(s->objectFor(node)));
                    }else if(first==u'.'){
                        const auto found=adaptor->elementsByClass(bodyNode,std::u16string_view(*text).substr(1));
                        const auto values=cx.array();for(auto node:found)cx.arrayPush(values,std::array<Value,1>{Value(s->objectFor(node))});addConcat(Value(values));
                    }else {
                        const auto tag=[&]{if(text->empty())return false;auto alpha=[](char16_t c){return(c>=u'a'&&c<=u'z')||(c>=u'A'&&c<=u'Z');};if(!alpha(text->front())&&text->front()!=u'-')return false;for(auto c:*text)if(!alpha(c)&&!(c>=u'0'&&c<=u'9')&&c!=u'-')return false;return true;}();
                        if(tag){const auto found=adaptor->tags(bodyNode,*text);const auto values=cx.array();for(auto node:found)cx.arrayPush(values,std::array<Value,1>{Value(s->objectFor(node))});addConcat(Value(values));}
                        else if(auto attribute=attributeSelector(*text)){
                            const auto found=adaptor->elementsByAttribute(bodyNode,attribute->first,attribute->second);const auto values=cx.array();for(auto node:found)cx.arrayPush(values,std::array<Value,1>{Value(s->objectFor(node))});addConcat(Value(values));
                        }
                    }
                    return;
                }
                if(const auto* object=std::get_if<ObjectRef>(&selector.data)){
                    if(cx.kind(*object)==ObjectKind::array){addConcat(selector);return;}
                    const auto windowValue=cx.get(receiver,key(u"window"));
                    const auto nodeList=cx.get(windowValue,key(u"NodeList"));
                    bool list=isInstance(selector,nodeList);
                    if(!list){const auto windowAgain=cx.get(receiver,key(u"window"));const auto collection=cx.get(windowAgain,key(u"HTMLCollection"));list=isInstance(selector,collection);}
                    if(list){addConcat(cx.get(selector,key(u"nodes")));return;}
                    push(selector);return;
                }
                push(selector);
            });
            return Value(result);
        });
    }
};

LiteModule::LiteModule(Context& c,OptionFunctionBindings& f,ContextPropertyBindings& p,
                       std::shared_ptr<regexp_host::RegExpBindings> regexp,
                       std::shared_ptr<LiteNodeStore> store){
    if(&f.context()!=&c)throw Exception("TypeError",u"Lite bindings require one Context");
    state_=std::make_shared<State>(c,f,p,std::move(regexp),store?std::move(store):LiteNodeStore::create());state_->initialize();
}
LiteModule::~LiteModule()=default;
std::shared_ptr<LiteNodeStore> LiteModule::store()const{return state_->store;}
std::size_t LiteModule::nodeIdentityCountForDiagnostics()const noexcept{return state_->nodes.size();}
std::vector<std::pair<NodeId,NodeKind>> LiteModule::nodeIdentitySnapshotForDiagnostics()const{
    std::vector<std::pair<NodeId,NodeKind>> result;
    result.reserve(state_->nodes.size());
    for(const auto& [node,identity]:state_->nodes){
        (void)identity;
        result.emplace_back(node.id,state_->store->record(node).nodeKind);
    }
    return result;
}
std::vector<std::tuple<NodeId,NodeKind,std::string>> LiteModule::nodeIdentityNamedSnapshotForDiagnostics()const{
    std::vector<std::tuple<NodeId,NodeKind,std::string>> result;
    result.reserve(state_->nodes.size());
    for(const auto& [node,identity]:state_->nodes){
        (void)identity;
        const auto& record=state_->store->record(node);
        const auto* rawName=std::get_if<std::string>(&record.kind.value);
        result.emplace_back(node.id,record.nodeKind,rawName?*rawName:std::string{});
    }
    std::sort(result.begin(),result.end());
    return result;
}
LiteModule::RequestMark LiteModule::requestMark()const noexcept{
    return RequestMark(state_->store->requestMark(),state_->nextEventSequence,state_.get());
}
void LiteModule::retireRequestNodesSince(RequestMark mark){
    const auto current=state_->store->requestMark();
    if(mark.owner!=state_.get()||mark.firstEvent==0||mark.firstEvent>state_->nextEventSequence||
       mark.nodes.owner!=current.owner||mark.nodes.generation!=current.generation||
       mark.nodes.first==0||mark.nodes.first>current.first)
        throw Exception("TypeError",u"Lite request mark does not belong to the current store state");
    // Drop callback/receiver roots while their Context records are still live.
    state_->retireEventRegistrationsSince(mark.firstEvent);
    // These array records are independent Context owners from the Lite node
    // records. Retire only views whose owning node was allocated in this request.
    // Keep each entry until retirement succeeds so a repeated mark can resume.
    for(auto it=state_->arrayProjections.begin();it!=state_->arrayProjections.end();){
        if(it->ownerNode>=mark.nodes.first){
            state_->context.retireObjectRecord(it->identity);
            it=state_->arrayProjections.erase(it);
        }else ++it;
    }
    for(auto it=state_->nodes.begin();it!=state_->nodes.end();){
        if(it->first.id>=mark.nodes.first){
            state_->contextProjections.erase(it->first.id);
            state_->context.retireObjectRecord(it->second);
            it=state_->nodes.erase(it);
        }else ++it;
    }
    state_->store->retireRecordsSince(mark.nodes);
}
void LiteModule::bindOwnerView(LiteNodeRef node,std::weak_ptr<LiteNodeOwnerView> view){state_->store->bindOwnerView(std::move(node),std::move(view));}
Context& LiteModule::context()const{return state_->context;}
std::shared_ptr<entities::EntityModule> LiteModule::entityModule()const{return state_->entities;}
ObjectRef LiteModule::constructor(NodeKind kind)const{return state_->classFor(kind);}
ObjectRef LiteModule::parserConstructor()const{return state_->parserClass;}
ObjectRef LiteModule::adaptorConstructor()const{return state_->adaptorClass;}
ObjectRef LiteModule::windowObject(){return state_->context.construct(state_->classes[5]);}
ObjectRef LiteModule::objectFor(LiteNodeRef node){return state_->objectFor(std::move(node));}
LiteNodeRef LiteModule::nodeFor(ObjectRef object)const{auto node=state_->nodeFor(object);if(!node)throw Exception("TypeError",u"Object is not a Lite node from this module");return *node;}
ObjectRef LiteModule::bindAdaptor(std::shared_ptr<LiteAdaptor> adaptor){return state_->bindAdaptor(adaptor);}
std::shared_ptr<LiteAdaptor> LiteModule::resolveAdaptor(ObjectRef object)const{return state_->resolveAdaptor(object);}
std::shared_ptr<LiteAdaptor> LiteModule::constructAdaptor(std::span<const options::Value> args){return state_->constructAdaptor(args);}
LiteModule::AppendTarget LiteModule::exchangeAppendTarget(AppendTarget target){
    return std::exchange(state_->appendTarget,std::move(target));
}
LiteModule::AttributeTarget LiteModule::exchangeAttributeTarget(AttributeTarget target){
    return std::exchange(state_->attributeTarget,std::move(target));
}
LiteModule::StyleTarget LiteModule::exchangeStyleTarget(StyleTarget target){
    return std::exchange(state_->styleTarget,std::move(target));
}
LiteModule::NodeOperationTarget LiteModule::exchangeNodeOperationTarget(NodeOperationTarget target){
    return std::exchange(state_->nodeOperationTarget,std::move(target));
}
LiteModule::EventMetadataTarget LiteModule::exchangeEventMetadataTarget(EventMetadataTarget target){
    return std::exchange(state_->eventMetadataTarget,std::move(target));
}
void LiteModule::registerHostEvent(ObjectRef receiver,Value eventType,Value callback){
    state_->registerHostEvent(receiver,std::move(eventType),std::move(callback));
}
StylesContextBindings& LiteModule::stylesBindings(){return state_->styles;}
ObjectRef LiteModule::stylesConstructor()const{return state_->styles.constructor();}
std::shared_ptr<styles::Styles> LiteModule::styleFor(ObjectRef object)const{return state_->styles.resolve(object);}
}
