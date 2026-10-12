module folia.math.util.context_property_bindings;
import folia.math.util.property_host;
import std;
namespace folia::math {
using namespace options;
struct ContextPropertyBindings::State:std::enable_shared_from_this<State> {
    Context& context;
    OptionFunctionBindings& functions;
    // Weak handles prevent cycles when a record contains itself. The binding
    // retains this owner; Context owns the actual fields and nested records.
    std::vector<std::pair<ObjectRef,std::weak_ptr<PropertyStorage>>> records;
    State(Context& context,OptionFunctionBindings& functions):context(context),functions(functions){}
    struct Record final:PropertyRecordBinding {
        std::shared_ptr<State> owner;
        ObjectRef object;
        Record(std::shared_ptr<State> owner,ObjectRef object):owner(std::move(owner)),object(object){}
        static Key key(std::string_view text){return Key(property_host::text(text));}
        Property get(std::string_view name)override{return owner->from(owner->context.get(object,key(name)));}
        Property getOwn(std::string_view name)override{
            if(!owner->context.hasOwn(object,key(name)))return Missing{};
            return get(name);
        }
        bool has(std::string_view name)const override{return owner->context.has(object,key(name));}
        bool hasOwn(std::string_view name)const override{return owner->context.hasOwn(object,key(name));}
        void set(std::string_view name,Property value)override{owner->context.set(object,key(name),owner->to(value));}
        void setOwn(std::string_view name,Property value)override{
            owner->context.define(object,key(name),Descriptor::data(owner->to(value)));
        }
        void erase(std::string_view name)override{owner->context.erase(object,key(name));}
        std::vector<std::string> keys()const override{
            std::vector<std::string> result;
            for(const auto& text:owner->context.enumerableStringKeys(object))result.push_back(property_host::debugText(text));
            return result;
        }
        Property prototype()const override{
            auto parent=owner->context.prototype(object);
            return owner->from(parent?Value(*parent):Value(Null{}));
        }
        Property primitive(bool)override{
            throw Exception("Unimplemented",u"Context-backed Property ToPrimitive requires the canonical coercion host");
        }
    };
    Property from(Value value){
        if(std::holds_alternative<Undefined>(value.data))return Missing{};
        if(std::holds_alternative<Null>(value.data))return PropertyNull{};
        if(const auto* p=std::get_if<bool>(&value.data))return *p;
        if(const auto* p=std::get_if<double>(&value.data))return *p;
        if(const auto* p=std::get_if<std::u16string>(&value.data))return property_host::debugText(*p);
        if(const auto* object=std::get_if<ObjectRef>(&value.data)){
            const auto kind=context.kind(*object); // Validate Context before caching.
            if(kind==ObjectKind::function){
                auto callable=functions.functionValue(value);
                if(const auto* function=std::get_if<FunctionRef>(&callable))return *function;
                throw Exception("Unimplemented",u"Context function has no canonical typed FunctionRef");
            }
            for(const auto& [known,weak]:records)if(known==*object)if(auto storage=weak.lock()){
                Property result;result.value=std::move(storage);return result;
            }
            const Property result=PropertyList::live(std::make_shared<Record>(shared_from_this(),*object),kind==ObjectKind::array);
            const auto storage=std::get<std::shared_ptr<PropertyStorage>>(result.value);
            records.emplace_back(*object,storage);return result;
        }
        throw Exception("Unimplemented",u"Context Symbol requires a typed Property Symbol binding");
    }
    Value to(const Property& value)const{
        if(value.missing())return Undefined{};
        if(std::holds_alternative<PropertyNull>(value.value))return Null{};
        if(const auto* p=std::get_if<bool>(&value.value))return *p;
        if(const auto* p=std::get_if<double>(&value.value))return *p;
        if(const auto* p=std::get_if<std::string>(&value.value))return property_host::text(*p);
        if(const auto* p=std::get_if<FunctionRef>(&value.value)){
            if(auto object=functions.find(*p))return *object;
            throw Exception("Unimplemented",u"Typed function has no canonical Context callable binding");
        }
        if(const auto* storage=std::get_if<std::shared_ptr<PropertyStorage>>(&value.value);storage&&*storage){
            const auto* bound=dynamic_cast<Record*>((*storage)->binding.get());
            if(bound&&bound->owner.get()==this)return bound->object;
            throw Exception("Unimplemented",u"Native Property record has no live Context record binding");
        }
        throw Exception("Unimplemented",u"Typed intrinsic Property has no canonical Context binding");
    }
};
ContextPropertyBindings::ContextPropertyBindings(Context& context,OptionFunctionBindings& functions){
    if(&functions.context()!=&context)throw Exception("TypeError",u"Property and callable bindings require the same Context");
    state_=std::make_shared<State>(context,functions);
}
ContextPropertyBindings::~ContextPropertyBindings()=default;
Property ContextPropertyBindings::from(Value value){return state_->from(std::move(value));}
Value ContextPropertyBindings::to(const Property& value)const{return state_->to(value);}
}
