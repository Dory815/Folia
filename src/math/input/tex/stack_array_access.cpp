module folia.math.input.tex.stack_array_access;
import std;
import folia.math.util.js_number;
import folia.math.util.property_host;

namespace folia::math::tex {
using namespace options;
namespace {
void active(const std::weak_ptr<std::atomic_bool>& lease){
    const auto value=lease.lock();
    if(!value||!value->load(std::memory_order_acquire))throw Exception("TypeError",u"Stack Context runtime has ended");
}
std::u16string ascii16(std::string_view text){return std::u16string(text.begin(),text.end());}
Value rawValue(const StackArrayStorageValue& value,const std::function<Value(const StackArrayStorageValue&)>& convert){
    if(const auto* raw=std::get_if<Value>(&value))return *raw;
    return convert(value);
}
Key toKey(Context& context,Value value){
    const Value primitive=context.toPrimitive(std::move(value),PrimitiveHint::stringHint);
    if(const auto* symbol=std::get_if<SymbolRef>(&primitive.data))return Key(*symbol);
    if(const auto* text=std::get_if<std::u16string>(&primitive.data))return Key(*text);
    if(std::holds_alternative<Undefined>(primitive.data))return Key(u"undefined");
    if(std::holds_alternative<Null>(primitive.data))return Key(u"null");
    if(const auto* boolean=std::get_if<bool>(&primitive.data))return Key(*boolean?u"true":u"false");
    if(const auto* number=std::get_if<double>(&primitive.data))return Key(ascii16(js_number::toString(*number)));
    throw Exception("TypeError",u"Cannot convert value to property key");
}
Value scalar(Context* context,const StackArrayScalar& expression,const std::function<Value(const Property&)>& propertyValue){
    if(const auto* value=std::get_if<Value>(&expression))return *value;
    const auto& property=std::get<Property>(expression);
    if(propertyValue)return propertyValue(property);
    if(property.missing())return Value(Undefined{});
    if(std::holds_alternative<PropertyNull>(property.value))return Value(Null{});
    if(const auto* boolean=std::get_if<bool>(&property.value))return Value(*boolean);
    if(const auto* number=std::get_if<double>(&property.value))return Value(*number);
    if(const auto* text=std::get_if<std::string>(&property.value))return Value(property_host::text(*text));
    return Value(Undefined{});
}
double nativeNumber(const StackArrayScalar& expression){
    if(const auto* property=std::get_if<Property>(&expression))return property_host::number(*property);
    const auto& value=std::get<Value>(expression);
    if(std::holds_alternative<Undefined>(value.data))return std::numeric_limits<double>::quiet_NaN();
    if(std::holds_alternative<Null>(value.data))return 0;
    if(const auto* number=std::get_if<double>(&value.data))return *number;
    if(const auto* boolean=std::get_if<bool>(&value.data))return *boolean?1:0;
    if(const auto* text=std::get_if<std::u16string>(&value.data)){
        if(text->empty())return 0;
        const std::string bytes(text->begin(),text->end());char* end=nullptr;const double result=std::strtod(bytes.c_str(),&end);
        return end==bytes.c_str()+bytes.size()?result:std::numeric_limits<double>::quiet_NaN();
    }
    throw Exception("TypeError",u"Cannot convert object to number without a Context");
}
std::optional<std::uint32_t> nativeIndex(const StackArrayScalar& expression){
    const double value=nativeNumber(expression);
    if(!std::isfinite(value)||value<0||std::trunc(value)!=value||value>=4294967295.0)return {};
    return static_cast<std::uint32_t>(value);
}
std::u16string primitiveText(Context& context,Value value){
    value=context.toPrimitive(std::move(value),PrimitiveHint::defaultHint);
    if(const auto* text=std::get_if<std::u16string>(&value.data))return *text;
    if(std::holds_alternative<Undefined>(value.data))return u"undefined";
    if(std::holds_alternative<Null>(value.data))return u"null";
    if(const auto* boolean=std::get_if<bool>(&value.data))return *boolean?u"true":u"false";
    if(const auto* number=std::get_if<double>(&value.data))return ascii16(js_number::toString(*number));
    if(std::holds_alternative<SymbolRef>(value.data))throw Exception("TypeError",u"Cannot convert a Symbol value to a string");
    throw Exception("TypeError",u"Cannot convert value to string");
}
Value nativeStringValue(const StackArrayStorageValue& value,const std::function<std::string(const StackArrayStorageValue&)>& convert){
    if(const auto* raw=std::get_if<Value>(&value)){
        if(const auto* text=std::get_if<std::u16string>(&raw->data))return Value(*text);
        if(std::holds_alternative<Undefined>(raw->data))return Value(u"undefined");
        if(std::holds_alternative<Null>(raw->data))return Value(u"null");
        if(const auto* b=std::get_if<bool>(&raw->data))return Value(*b?u"true":u"false");
        if(const auto* n=std::get_if<double>(&raw->data))return Value(ascii16(js_number::toString(*n)));
        throw Exception("TypeError",u"Cannot convert object to string without a Context");
    }
    return Value(property_host::text(convert(value)));
}
}

std::uint32_t StackArrayStorage::length()const{
    if(entries_.size()>std::numeric_limits<std::uint32_t>::max())throw Exception("RangeError",u"Invalid array length");
    return static_cast<std::uint32_t>(entries_.size());
}
bool StackArrayStorage::has(std::uint32_t i)const noexcept{return i<entries_.size()&&entries_[i].has_value();}
StackArrayStorageValue StackArrayStorage::rawAt(std::uint32_t i)const{return has(i)?entries_[i]->value:StackArrayStorageValue(Value(Undefined{}));}
std::optional<StackArrayStorage::Slot> StackArrayStorage::slotAt(std::uint32_t i)const{return has(i)?entries_[i]:std::nullopt;}
void StackArrayStorage::rawSet(std::uint32_t i,StackArrayStorageValue value){
    if(i>=entries_.size()){if(!lengthWritable_)throw Exception("TypeError",u"Array length is not writable");entries_.resize(static_cast<std::size_t>(i)+1);}
    if(entries_[i]&&!entries_[i]->writable)throw Exception("TypeError",u"Cannot assign to read only property");
    Slot slot=entries_[i].value_or(Slot{});slot.value=std::move(value);slot.accessor=false;slot.getter.reset();slot.setter.reset();entries_[i]=std::move(slot);
}
StackArrayStorageValue StackArrayStorage::popRaw(){if(entries_.empty())return Value(Undefined{});auto v=std::move(entries_.back());entries_.pop_back();return v?v->value:StackArrayStorageValue(Value(Undefined{}));}
void StackArrayStorage::pushRaw(StackArrayStorageValue value){if(!lengthWritable_)throw Exception("TypeError",u"Array length is not writable");Slot slot;slot.value=std::move(value);entries_.push_back(std::move(slot));}
void StackArrayStorage::setLength(std::uint32_t n,bool writable){
    if(!lengthWritable_&&n!=entries_.size())throw Exception("TypeError",u"Cannot assign to read only property");
    if(n<entries_.size())for(std::size_t i=entries_.size();i>n;--i){auto& slot=entries_[i-1];if(slot&&!slot->configurable){entries_.resize(i);if(!writable)lengthWritable_=false;throw Exception("TypeError",u"Cannot delete non-configurable array element");}slot.reset();}
    entries_.resize(n);lengthWritable_=writable;
}
void StackArrayStorage::defineIndex(std::uint32_t i,Descriptor d){
    if(i>=entries_.size()&&!lengthWritable_)throw Exception("TypeError",u"Array length is not writable");
    if(i>=entries_.size())entries_.resize(static_cast<std::size_t>(i)+1);
    Slot s;s.writable=d.writable;s.enumerable=d.enumerable;s.configurable=d.configurable;s.accessor=d.accessor;s.getter=d.getter;s.setter=d.setter;if(!d.accessor)s.value=d.value;entries_[i]=std::move(s);
}
void StackArrayStorage::deleteIndex(std::uint32_t i)noexcept{if(i<entries_.size()&&entries_[i]&&entries_[i]->configurable)entries_[i].reset();}

struct NativeStackArrayAccess::State{StackArrayStorageRef storage;std::function<StackEntry(const Value&)> fromValue;std::function<std::string(const StackArrayStorageValue&)> toString;State(StackArrayStorageRef s,std::function<StackEntry(const Value&)> f,std::function<std::string(const StackArrayStorageValue&)> t):storage(std::move(s)),fromValue(std::move(f)),toString(std::move(t)){if(!storage||!fromValue||!toString)throw Exception("TypeError",u"Native Stack array requires its storage and converters");}};
NativeStackArrayAccess::NativeStackArrayAccess(StackArrayStorageRef s,std::function<StackEntry(const Value&)> f,std::function<std::string(const StackArrayStorageValue&)> t):state_(std::make_shared<State>(std::move(s),std::move(f),std::move(t))){}
NativeStackArrayAccess::~NativeStackArrayAccess()=default;
std::optional<ObjectRef> NativeStackArrayAccess::identity()const noexcept{return {};}
Value NativeStackArrayAccess::sourceValue()const{throw Exception("TypeError",u"Native Stack array has no Context identity before projection");}
Value NativeStackArrayAccess::readLength(){return Value(static_cast<double>(state_->storage->length()));}
StackEntry NativeStackArrayAccess::getIndex(const StackArrayScalar& e){auto i=nativeIndex(e);if(!i)return StackEntry(Property(Missing{}));auto raw=state_->storage->rawAt(*i);if(const auto* entry=std::get_if<StackEntry>(&raw))return *entry;if(const auto* value=std::get_if<Value>(&raw))return state_->fromValue(*value);return Property(Missing{});}
void NativeStackArrayAccess::setIndex(const StackArrayScalar& e,StackArrayStorageValue v){if(auto i=nativeIndex(e))state_->storage->rawSet(*i,std::move(v));}
bool NativeStackArrayAccess::lessThan(const Value& l,const StackArrayScalar& r){if(auto a=std::get_if<std::u16string>(&l.data))if(auto b=std::get_if<Value>(&r))if(auto t=std::get_if<std::u16string>(&b->data))return *a<*t;double a=nativeNumber(StackArrayScalar(l)),b=nativeNumber(r);return !std::isnan(a)&&!std::isnan(b)&&a<b;}
StackArrayScalar NativeStackArrayAccess::subtract(const Value& l,const StackArrayScalar& r){return StackArrayScalar(Value(nativeNumber(StackArrayScalar(l))-nativeNumber(r)));}
StackArrayMethod NativeStackArrayAccess::getMethod(std::u16string_view n){if(n==u"push")return {StackArrayMethodKind::nativePush,std::u16string(n),Undefined{},Undefined{}};if(n==u"pop")return {StackArrayMethodKind::nativePop,std::u16string(n),Undefined{},Undefined{}};if(n==u"join")return {StackArrayMethodKind::nativeJoin,std::u16string(n),Undefined{},Undefined{}};throw Exception("TypeError",u"Native array method is unavailable");}
StackArrayStorageValue NativeStackArrayAccess::callMethod(const StackArrayMethod& m,std::span<const StackArrayStorageValue> args){
    if(m.kind==StackArrayMethodKind::nativePush){for(const auto& v:args)state_->storage->pushRaw(v);return Value(static_cast<double>(state_->storage->length()));}
    if(m.kind==StackArrayMethodKind::nativePop)return state_->storage->popRaw();
    if(m.kind==StackArrayMethodKind::nativeJoin){
        // Array.prototype.join captures length before separator coercion or
        // element reads. Coercion can mutate this array, so keep the bound
        // while each later index lookup remains live.
        const std::uint32_t capturedLength=state_->storage->length();
        std::u16string sep=u",";if(!args.empty())if(auto v=std::get_if<Value>(&args[0])){if(!std::holds_alternative<Undefined>(v->data)){const StackArrayStorageValue rawSeparator(*v);const Value text=nativeStringValue(rawSeparator,state_->toString);sep=std::get<std::u16string>(text.data);}}
        std::u16string out;for(std::uint32_t i=0;i<capturedLength;++i){
            if(i)out+=sep;auto v=state_->storage->rawAt(i);
            if(const auto* raw=std::get_if<Value>(&v)){
                if(std::holds_alternative<Undefined>(raw->data)||std::holds_alternative<Null>(raw->data))continue;
                const Value text=nativeStringValue(v,state_->toString);
                if(const auto* units=std::get_if<std::u16string>(&text.data))out+=*units;
            }else out+=property_host::text(state_->toString(v));
        }return Value(std::move(out));}
    return Value(Undefined{});
}
std::u16string NativeStackArrayAccess::toStringValue(const StackArrayStorageValue& v){return std::get<std::u16string>(nativeStringValue(v,state_->toString).data);}
void NativeStackArrayAccess::pushEntry(const StackEntry& v){state_->storage->pushRaw(StackArrayStorageValue(v));}
StackEntry NativeStackArrayAccess::popEntry(){auto v=state_->storage->popRaw();if(auto e=std::get_if<StackEntry>(&v))return *e;if(auto raw=std::get_if<Value>(&v))return state_->fromValue(*raw);return Property(Missing{});}
StackEntry NativeStackArrayAccess::materialize(const StackEntry& value){if(const auto* raw=std::get_if<Value>(&value))return state_->fromValue(*raw);return value;}
double NativeStackArrayAccess::number(const StackArrayScalar& v){return nativeNumber(v);}
StackEntry NativeStackArrayAccess::readEntry(std::uint32_t i){return getIndex(StackArrayScalar(Value(static_cast<double>(i))));}
const StackArrayStorageRef& NativeStackArrayAccess::storage()const noexcept{return state_->storage;}

struct StackArrayIndexedView::State{StackArrayStorageRef storage;std::function<Value(const StackArrayStorageValue&)> convert;State(StackArrayStorageRef s,std::function<Value(const StackArrayStorageValue&)> c):storage(std::move(s)),convert(std::move(c)){if(!storage||!convert)throw Exception("TypeError",u"Stack Array view requires storage and conversion");}};
StackArrayIndexedView::StackArrayIndexedView(StackArrayStorageRef s,std::function<Value(const StackArrayStorageValue&)> c):state_(std::make_shared<State>(std::move(s),std::move(c))){}
StackArrayIndexedView::~StackArrayIndexedView()=default;
std::uint32_t StackArrayIndexedView::length()const{return state_->storage->length();}
Descriptor StackArrayIndexedView::lengthDescriptor()const{return Descriptor::data(Value(static_cast<double>(length())),state_->storage->lengthWritable(),false,false);}
std::optional<Descriptor> StackArrayIndexedView::indexDescriptor(std::uint32_t i)const{auto slot=state_->storage->slotAt(i);if(!slot)return {};if(slot->accessor)return Descriptor::access(slot->getter,slot->setter,slot->enumerable,slot->configurable);Value value=std::holds_alternative<Value>(slot->value)?std::get<Value>(slot->value):state_->convert(slot->value);return Descriptor::data(std::move(value),slot->writable,slot->enumerable,slot->configurable);}
std::vector<std::uint32_t> StackArrayIndexedView::ownIndexKeys()const{std::vector<std::uint32_t> out;for(std::uint32_t i=0;i<length();++i)if(state_->storage->has(i))out.push_back(i);return out;}
void StackArrayIndexedView::defineIndex(std::uint32_t i,Descriptor d){state_->storage->defineIndex(i,std::move(d));}
void StackArrayIndexedView::defineLength(Descriptor d){if(d.accessor)throw Exception("TypeError",u"Array length cannot be an accessor");auto n=std::get_if<double>(&d.value.data);if(!n||!std::isfinite(*n)||*n<0||*n>4294967295.0||std::trunc(*n)!=*n)throw Exception("RangeError",u"Invalid array length");state_->storage->setLength(static_cast<std::uint32_t>(*n),d.writable);}
void StackArrayIndexedView::deleteIndex(std::uint32_t i){state_->storage->deleteIndex(i);}
const StackArrayStorageRef& StackArrayIndexedView::storage()const noexcept{return state_->storage;}

struct ContextArrayStackAccess::State{Context* context;Value array;std::function<Value(const StackArrayStorageValue&)> toValue;std::function<StackEntry(const Value&)> fromValue;std::function<Value(const Property&)> propertyValue;std::weak_ptr<std::atomic_bool> lease;State(Context& c,Value a,std::function<Value(const StackArrayStorageValue&)> t,std::function<StackEntry(const Value&)> f,std::function<Value(const Property&)> p,std::weak_ptr<std::atomic_bool> l):context(&c),array(std::move(a)),toValue(std::move(t)),fromValue(std::move(f)),propertyValue(std::move(p)),lease(std::move(l)){if(!toValue||!fromValue||!propertyValue)throw Exception("TypeError",u"Stack array converters are required");active(lease);}};
ContextArrayStackAccess::ContextArrayStackAccess(Context& c,Value a,std::function<Value(const StackArrayStorageValue&)> t,std::function<StackEntry(const Value&)> f,std::function<Value(const Property&)> p,std::weak_ptr<std::atomic_bool> l):state_(std::make_shared<State>(c,std::move(a),std::move(t),std::move(f),std::move(p),std::move(l))){}
ContextArrayStackAccess::~ContextArrayStackAccess()=default;
std::optional<ObjectRef> ContextArrayStackAccess::identity()const noexcept{if(auto o=std::get_if<ObjectRef>(&state_->array.data))return *o;return {};}
Value ContextArrayStackAccess::sourceValue()const{return state_->array;}
Value ContextArrayStackAccess::readLength(){active(state_->lease);return state_->context->get(state_->array,Key(u"length"));}
StackEntry ContextArrayStackAccess::getIndex(const StackArrayScalar& e){active(state_->lease);return state_->fromValue(state_->context->get(state_->array,toKey(*state_->context,scalar(state_->context,e,state_->propertyValue))));}
void ContextArrayStackAccess::setIndex(const StackArrayScalar& e,StackArrayStorageValue v){active(state_->lease);const auto k=toKey(*state_->context,scalar(state_->context,e,state_->propertyValue));state_->context->set(state_->array,k,rawValue(v,state_->toValue));}
bool ContextArrayStackAccess::lessThan(const Value& l,const StackArrayScalar& r){active(state_->lease);auto& c=*state_->context;auto a=c.toPrimitive(l,PrimitiveHint::numberHint),b=c.toPrimitive(scalar(&c,r,state_->propertyValue),PrimitiveHint::numberHint);auto x=std::get_if<std::u16string>(&a.data),y=std::get_if<std::u16string>(&b.data);if(x&&y)return *x<*y;double n=c.toNumber(a),m=c.toNumber(b);return !std::isnan(n)&&!std::isnan(m)&&n<m;}
StackArrayScalar ContextArrayStackAccess::subtract(const Value& l,const StackArrayScalar& r){
    active(state_->lease);auto& c=*state_->context;
    const double left=c.toNumber(l);
    const Value rightValue=scalar(&c,r,state_->propertyValue);
    const double right=c.toNumber(rightValue);
    return StackArrayScalar(Value(left-right));
}
StackArrayMethod ContextArrayStackAccess::getMethod(std::u16string_view name){active(state_->lease);Value receiver=state_->array;Value fn=state_->context->get(receiver,Key(std::u16string(name)));return {StackArrayMethodKind::contextCallable,std::u16string(name),std::move(receiver),std::move(fn)};}
StackArrayStorageValue ContextArrayStackAccess::callMethod(const StackArrayMethod& m,std::span<const StackArrayStorageValue> args){active(state_->lease);std::vector<Value> values;values.reserve(args.size());for(auto& a:args)values.push_back(rawValue(a,state_->toValue));return state_->context->callMember(m.callable,m.receiver,values,u"Stack.stack method");}
std::u16string ContextArrayStackAccess::toStringValue(const StackArrayStorageValue& v){active(state_->lease);return primitiveText(*state_->context,rawValue(v,state_->toValue));}
void ContextArrayStackAccess::pushEntry(const StackEntry& e){const auto m=getMethod(u"push");const std::array<StackArrayStorageValue,1> a{e};(void)callMethod(m,a);}
StackEntry ContextArrayStackAccess::popEntry(){const auto m=getMethod(u"pop");auto result=callMethod(m,{});return std::get<Value>(result);}
StackEntry ContextArrayStackAccess::materialize(const StackEntry& value){active(state_->lease);if(const auto* raw=std::get_if<Value>(&value))return state_->fromValue(*raw);return value;}
double ContextArrayStackAccess::number(const StackArrayScalar& e){active(state_->lease);return state_->context->toNumber(scalar(state_->context,e,state_->propertyValue));}
StackEntry ContextArrayStackAccess::readEntry(std::uint32_t i){return getIndex(StackArrayScalar(Value(static_cast<double>(i))));}
}
