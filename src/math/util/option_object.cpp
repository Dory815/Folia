module folia.math.util.option_types;
import folia.math.util.js_number;
import folia.math.util.js_numeric_input;
import std;
#include "option_storage.hpp"

// Copyright (c) 2017-2026 The MathJax Consortium.
// Native object and property semantics follow the Apache-2.0-licensed
// MathJax `ts/util/Options.ts`; see src/math/LICENSE-MathJax.txt.
namespace {
using namespace folia::math::options;

std::atomic<std::uint64_t> next_context_identity{1};
constexpr std::u16string_view kLength = u"length";
constexpr std::u16string_view kConstructor = u"constructor";
constexpr std::u16string_view kProto = u"__proto__";
struct BoxedPrimitive { Value value; };

[[noreturn]] void type_error(std::u16string message = u"Invalid operation") {
    throw Exception("TypeError", std::move(message));
}

[[noreturn]] void range_error(std::u16string message = u"Invalid array length") {
    throw Exception("RangeError", std::move(message));
}

bool is_nullish(const Value& value) {
    return std::holds_alternative<Undefined>(value.data) || std::holds_alternative<Null>(value.data);
}

ObjectRef to_object(Context& context,Value value) {
    if(const auto* object=std::get_if<ObjectRef>(&value.data)) return *object;
    if(is_nullish(value)) type_error(u"Cannot convert undefined or null to object");
    ObjectRef prototype=context.intrinsics().objectPrototype;
    if(std::holds_alternative<bool>(value.data)) prototype=context.intrinsics().booleanPrototype;
    else if(std::holds_alternative<double>(value.data)) prototype=context.intrinsics().numberPrototype;
    else if(std::holds_alternative<std::u16string>(value.data)) prototype=context.intrinsics().stringPrototype;
    else if(const auto* symbol=std::get_if<SymbolRef>(&value.data)) { (void)context.symbolString(*symbol); prototype=context.intrinsics().symbolPrototype; }
    const auto boxed=context.objectWithPrototype(prototype);
    if(const auto* text=std::get_if<std::u16string>(&value.data)) {
        context.define(boxed,Key(u"length"),Descriptor::data(Value(static_cast<double>(text->size())),false,false,false));
        for(std::size_t index=0;index<text->size();++index) {
            const auto key=std::to_string(index);
            context.define(boxed,Key(std::u16string(key.begin(),key.end())),Descriptor::data(Value(std::u16string(1,(*text)[index])),false,true,false));
        }
    }
    context.payload(boxed)=BoxedPrimitive{std::move(value)};
    return boxed;
}

std::optional<std::uint32_t> array_index(std::u16string_view text) {
    if (text.empty() || (text.size() > 1 && text.front() == u'0')) return std::nullopt;
    std::uint64_t value = 0;
    for (const auto c : text) {
        if (c < u'0' || c > u'9') return std::nullopt;
        value = value * 10 + static_cast<unsigned>(c - u'0');
        if (value > 4294967294ULL) return std::nullopt;
    }
    return static_cast<std::uint32_t>(value);
}

bool key_is(const Key& key, std::u16string_view name) {
    const auto* text = std::get_if<std::u16string>(&key.data);
    return text && *text == name;
}

std::u16string index_string(std::uint32_t index) {
    const auto ascii = std::to_string(index);
    return std::u16string(ascii.begin(), ascii.end());
}

void validate_key(const Context& context, const Key& key) {
    if (const auto* symbol = std::get_if<SymbolRef>(&key.data)) {
        (void)context.symbolString(*symbol);
    }
}

void validate_value(const Context& context, const Value& value) {
    if (const auto* object = std::get_if<ObjectRef>(&value.data)) (void)context.kind(*object);
    if (const auto* symbol = std::get_if<SymbolRef>(&value.data)) (void)context.symbolString(*symbol);
}

bool data_descriptor_compatible(const Descriptor& old_value, const Descriptor& next) {
    if (old_value.accessor != next.accessor) return false;
    if (old_value.accessor) {
        return old_value.getter == next.getter && old_value.setter == next.setter;
    }
    return true;
}

bool ecma_whitespace(char16_t c) {
    return c == 0x0009 || c == 0x000B || c == 0x000C || c == 0x0020 ||
           c == 0x00A0 || c == 0x1680 || (c >= 0x2000 && c <= 0x200A) ||
           c == 0x2028 || c == 0x2029 || c == 0x202F || c == 0x205F ||
           c == 0x3000 || c == 0xFEFF || c == 0x000A || c == 0x000D;
}

double primitive_number(const Value& value) {
    if (std::holds_alternative<Undefined>(value.data)) return std::numeric_limits<double>::quiet_NaN();
    if (std::holds_alternative<Null>(value.data)) return 0;
    if (const auto* boolean = std::get_if<bool>(&value.data)) return *boolean ? 1 : 0;
    if (const auto* number = std::get_if<double>(&value.data)) return *number;
    if (std::holds_alternative<SymbolRef>(value.data)) type_error(u"Cannot convert a Symbol value to a number");
    if (std::holds_alternative<ObjectRef>(value.data)) type_error(u"Object-to-primitive conversion has no native binding");

    auto text = std::get<std::u16string>(value.data);
    while (!text.empty() && ecma_whitespace(text.front())) text.erase(text.begin());
    while (!text.empty() && ecma_whitespace(text.back())) text.pop_back();
    if (text.empty()) return 0;
    std::string ascii;
    ascii.reserve(text.size());
    for (const auto c : text) {
        if (c > 0x7f) return std::numeric_limits<double>::quiet_NaN();
        ascii.push_back(static_cast<char>(c));
    }
    if (ascii == "Infinity" || ascii == "+Infinity") return std::numeric_limits<double>::infinity();
    if (ascii == "-Infinity") return -std::numeric_limits<double>::infinity();
    if (ascii == "NaN" || ascii == "+NaN" || ascii == "-NaN") return std::numeric_limits<double>::quiet_NaN();
    if (ascii.size() > 2 && ascii[0] == '0') {
        const int base = ascii[1] == 'x' || ascii[1] == 'X' ? 16 :
                         ascii[1] == 'b' || ascii[1] == 'B' ? 2 :
                         ascii[1] == 'o' || ascii[1] == 'O' ? 8 : 0;
        if (base != 0) {
            std::uint64_t integer = 0;
            const auto parsed = std::from_chars(ascii.data() + 2, ascii.data() + ascii.size(), integer, base);
            if (parsed.ptr != ascii.data() + ascii.size() || parsed.ec == std::errc::invalid_argument) return std::numeric_limits<double>::quiet_NaN();
            if (parsed.ec == std::errc::result_out_of_range) return std::numeric_limits<double>::infinity();
            return static_cast<double>(integer);
        }
    }
    bool negative = false;
    const char* first = ascii.data();
    const char* last = first + ascii.size();
    if (*first == '+') ++first;
    else if (*first == '-') negative = true;
    if (first == last) return std::numeric_limits<double>::quiet_NaN();
    const char* scan = first + (*first == '-' ? 1 : 0);
    bool integer_digits = false;
    while (scan != last && *scan >= '0' && *scan <= '9') { integer_digits = true; ++scan; }
    bool fraction_digits = false;
    if (scan != last && *scan == '.') {
        ++scan;
        while (scan != last && *scan >= '0' && *scan <= '9') { fraction_digits = true; ++scan; }
    }
    if (!integer_digits && !fraction_digits) return std::numeric_limits<double>::quiet_NaN();
    if (scan != last && (*scan == 'e' || *scan == 'E')) {
        ++scan;
        if (scan != last && (*scan == '+' || *scan == '-')) ++scan;
        const auto exponent_start = scan;
        while (scan != last && *scan >= '0' && *scan <= '9') ++scan;
        if (scan == exponent_start) return std::numeric_limits<double>::quiet_NaN();
    }
    if (scan != last) return std::numeric_limits<double>::quiet_NaN();
    double result = 0;
    const auto parsed = std::from_chars(first, last, result, std::chars_format::general);
    if (parsed.ptr != last || parsed.ec == std::errc::invalid_argument) return std::numeric_limits<double>::quiet_NaN();
    if (parsed.ec == std::errc::result_out_of_range) {
        const auto exponent = ascii.find_first_of("eE");
        if (exponent != std::string::npos && exponent + 1 < ascii.size() && ascii[exponent + 1] == '-') return negative ? -0.0 : 0.0;
        return negative ? -std::numeric_limits<double>::infinity() : std::numeric_limits<double>::infinity();
    }
    return result;
}

std::u16string primitive_string(Context& context,Value value) {
    if(std::holds_alternative<Undefined>(value.data))return u"undefined";
    if(std::holds_alternative<Null>(value.data))return u"null";
    value=context.toPrimitive(std::move(value),PrimitiveHint::stringHint);
    if(const auto* text=std::get_if<std::u16string>(&value.data))return *text;
    if(const auto* boolean=std::get_if<bool>(&value.data))return *boolean?u"true":u"false";
    if(const auto* number=std::get_if<double>(&value.data)){
        const auto ascii=folia::math::js_number::toString(*number);
        return std::u16string(ascii.begin(),ascii.end());
    }
    if(const auto* symbol=std::get_if<SymbolRef>(&value.data)){
        (void)context.symbolString(*symbol);
        type_error(u"Cannot convert a Symbol value to a string");
    }
    type_error(u"Cannot convert object to primitive value");
}

std::uint32_t to_uint32(Context& context,const Value& value) {
    const double number=context.toNumber(value);
    if(!std::isfinite(number)||number==0)return 0;
    constexpr double modulus=4294967296.0;
    double integer=std::fmod(std::trunc(number),modulus);
    if(integer<0)integer+=modulus;
    return static_cast<std::uint32_t>(integer);
}

} // namespace

namespace folia::math::options {

bool truthy(const Value& value) {
    if (std::holds_alternative<Undefined>(value.data) || std::holds_alternative<Null>(value.data)) return false;
    if (const auto* boolean = std::get_if<bool>(&value.data)) return *boolean;
    if (const auto* number = std::get_if<double>(&value.data)) return *number != 0 && !std::isnan(*number);
    if (const auto* string = std::get_if<std::u16string>(&value.data)) return !string->empty();
    return true;
}

bool sameValue(const Value& a, const Value& b) {
    if (a.data.index() != b.data.index()) return false;
    if (const auto* left = std::get_if<double>(&a.data)) {
        const auto right = std::get<double>(b.data);
        if (std::isnan(*left) && std::isnan(right)) return true;
        if (*left == 0 && right == 0) return std::signbit(*left) == std::signbit(right);
        return *left == right;
    }
    return a == b;
}

namespace {
struct ObjectRefHash {
    std::size_t operator()(ObjectRef value)const noexcept{
        const auto first=std::hash<std::uint64_t>{}(value.context);
        const auto second=std::hash<std::uint64_t>{}(value.index);
        return first^(second+0x9e3779b97f4a7c15ULL+(first<<6)+(first>>2));
    }
};
struct SymbolRefHash {
    std::size_t operator()(SymbolRef value)const noexcept{
        const auto first=std::hash<std::uint64_t>{}(value.context);
        const auto second=std::hash<std::uint64_t>{}(value.index);
        return first^(second+0x9e3779b97f4a7c15ULL+(first<<6)+(first>>2));
    }
};
bool ownsPart(const Record& value,NativeOwnerPart part){
    switch(part){
    case NativeOwnerPart::payload:return value.payload.has_value();
    case NativeOwnerPart::callable:return static_cast<bool>(value.callable);
    case NativeOwnerPart::construct:return static_cast<bool>(value.construct);
    case NativeOwnerPart::nativeView:return static_cast<bool>(value.nativeRecordView)||static_cast<bool>(value.nativeIndexedView);
    case NativeOwnerPart::nativeDataField:return false;
    case NativeOwnerPart::attachedOwner:return static_cast<bool>(value.attachedOwner);
    }
    return false;
}
}

void Context::registerNativeOwnerTrace(ObjectRef object,NativeOwnerPart part,NativeOwnerTraceRef trace){
    auto& value=record(object);
    if(part==NativeOwnerPart::attachedOwner)
        type_error(u"Attached owners use the Context record owner slot");
    if(!trace)type_error(u"Native owner edge trace is null");
    if(!ownsPart(value,part))type_error(u"Native owner edge trace does not match an opaque owner");
    if(part==NativeOwnerPart::payload&&value.payloadMutableAliasEscaped)
        type_error(u"Cannot trace a payload after exposing a mutable std::any alias");
    for(const auto& [existing,ownerTrace]:value.ownerTraces)
        if(existing==part)type_error(u"Native owner edge trace is already registered");
    value.ownerTraces.emplace_back(part,std::move(trace));
}
void Context::unregisterNativeOwnerTrace(ObjectRef object,NativeOwnerPart part){
    auto& value=record(object);
    std::erase_if(value.ownerTraces,[&](const auto& entry){return entry.first==part;});
}
NativeAttachedOwnerRef Context::attachedOwner(ObjectRef object)const{
    const auto* value=recordIfLive(object);
    return value?value->attachedOwner:NativeAttachedOwnerRef{};
}
void Context::attachOwner(ObjectRef object,NativeAttachedOwnerRef owner){
    if(!owner)type_error(u"Native attached owner is null");
    auto& value=record(object);
    if(value.attachedOwner&&value.attachedOwner!=owner)
        type_error(u"Context record already has a different attached owner");
    value.attachedOwner=std::move(owner);
}
bool Context::clearAttachedOwner(ObjectRef object,const NativeAttachedOwner* expected){
    if(!expected)type_error(u"Native attached owner clear requires an expected owner");
    auto* value=recordIfLive(object);
    if(!value||value->attachedOwner.get()!=expected)return false;
    value->attachedOwner.reset();
    return true;
}
ContextTraceReport Context::traceReachable(std::span<const ObjectRef> roots)const{
    ContextTraceReport result(state_->identity,roots);
    std::vector<ObjectRef> pending;
    pending.reserve(roots.size());
    std::unordered_set<ObjectRef,ObjectRefHash> seenObjects;
    std::unordered_set<SymbolRef,SymbolRefHash> seenSymbols;
    const auto visitValue=[&](const Value& value){
        if(const auto* object=std::get_if<ObjectRef>(&value.data)){
            (void)record(*object);
            if(seenObjects.insert(*object).second)pending.push_back(*object);
        }else if(const auto* symbol=std::get_if<SymbolRef>(&value.data)){
            (void)symbolString(*symbol);
            if(seenSymbols.insert(*symbol).second)result.symbols_.push_back(*symbol);
        }
    };
    const auto addPin=[&](ObjectRef object,NativeOwnerPart part,std::u16string detail){
        result.opaquePins_.push_back({object,part,std::move(detail)});
    };
    for(const auto root:roots)visitValue(Value(root));
    while(!pending.empty()){
        const auto object=pending.back();pending.pop_back();
        result.objects_.push_back(object);
        const auto& value=record(object);
        // NativeRecordView is authoritative; its explicit trace reports the
        // current prototype without invoking a view virtual getter.
        if(!value.nativeRecordView&&value.prototype)visitValue(Value(*value.prototype));
        for(const auto& [key,descriptor]:value.properties){
            if(const auto* symbol=std::get_if<SymbolRef>(&key.data)){
                (void)symbolString(*symbol);
                if(seenSymbols.insert(*symbol).second)result.symbols_.push_back(*symbol);
            }
            if(descriptor.accessor){
                if(descriptor.getter)visitValue(Value(*descriptor.getter));
                if(descriptor.setter)visitValue(Value(*descriptor.setter));
            }else visitValue(descriptor.value);
        }
        const auto traceFor=[&](NativeOwnerPart part)->NativeOwnerTraceRef{
            if(part==NativeOwnerPart::attachedOwner)return value.attachedOwner;
            for(const auto& [registeredPart,trace]:value.ownerTraces)
                if(registeredPart==part)return trace;
            return {};
        };
        const auto traceOrPin=[&](NativeOwnerPart part){
            if(!ownsPart(value,part))return;
            const auto trace=traceFor(part);
            const bool complete=trace&&trace->visitContextEdges(visitValue);
            if(!complete){
                const auto detail=part==NativeOwnerPart::payload?u"payload":
                    part==NativeOwnerPart::callable?u"callable":
                    part==NativeOwnerPart::construct?u"construct":
                    part==NativeOwnerPart::attachedOwner?u"attached owner":u"native view";
                addPin(object,part,detail);
            }
        };
        traceOrPin(NativeOwnerPart::payload);
        traceOrPin(NativeOwnerPart::callable);
        traceOrPin(NativeOwnerPart::construct);
        traceOrPin(NativeOwnerPart::nativeView);
        traceOrPin(NativeOwnerPart::attachedOwner);
        const auto visitFields=[&](const auto& fields){
            for(const auto& [key,field]:fields){
                if(const auto* symbol=std::get_if<SymbolRef>(&key.data)){
                    (void)symbolString(*symbol);
                    if(seenSymbols.insert(*symbol).second)result.symbols_.push_back(*symbol);
                }
                if(!field)continue;
                const bool complete=field->hasEdgeTrace()&&field->visitContextEdges(visitValue);
                if(!complete){
                    const auto* text=std::get_if<std::u16string>(&key.data);
                    addPin(object,NativeOwnerPart::nativeDataField,
                        text?*text:std::u16string(u"<symbol-key native data field>"));
                }
            }
        };
        visitFields(value.nativeFields);
        visitFields(value.declaredNativeFields);
    }
    return result;
}

Record& Context::record(ObjectRef object) const {
    auto* value=recordIfLive(object);
    if(!value)type_error(u"Object handle refers to a retired Context record");
    return *value;
}
Record* Context::recordIfLive(ObjectRef object)const{
    if (object.context != state_->identity || object.index >= state_->objects.size() || !state_->objects[object.index]) {
        type_error(u"Object belongs to a different or invalid Context");
    }
    if(state_->objects[object.index]->retired)return nullptr;
    return state_->objects[object.index].get();
}

ObjectRef Context::allocate(ObjectKind kind, std::optional<ObjectRef> prototype) {
    if (prototype && prototype->context != state_->identity) type_error(u"Prototype belongs to a different Context");
    auto value = std::make_unique<Record>();
    value->kind = kind;
    value->prototype = prototype;
    const auto index = static_cast<std::uint64_t>(state_->objects.size());
    state_->objects.push_back(std::move(value));
    return {state_->identity, index};
}

Context::Context() : state_(std::make_unique<ContextState>()) {
    state_->identity = next_context_identity.fetch_add(1, std::memory_order_relaxed);
    if (state_->identity == 0) state_->identity = next_context_identity.fetch_add(1, std::memory_order_relaxed);
    auto& i = state_->intrinsics;
    i.symbolSpecies = symbol(u"Symbol.species");
    i.symbolIterator = symbol(u"Symbol.iterator");
    i.symbolIsConcatSpreadable = symbol(u"Symbol.isConcatSpreadable");
    i.symbolToPrimitive = symbol(u"Symbol.toPrimitive");
    i.symbolToStringTag = symbol(u"Symbol.toStringTag");
    i.symbolSplit = symbol(u"Symbol.split");
    i.symbolMatch = symbol(u"Symbol.match");

    i.objectPrototype = allocate(ObjectKind::object, std::nullopt);
    i.functionPrototype = allocate(ObjectKind::function, i.objectPrototype);
    i.arrayPrototype = allocate(ObjectKind::array, i.objectPrototype);
    i.expandablePrototype = allocate(ObjectKind::object, i.objectPrototype);
    i.numberPrototype = allocate(ObjectKind::object, i.objectPrototype);
    i.booleanPrototype = allocate(ObjectKind::object, i.objectPrototype);
    i.stringPrototype = allocate(ObjectKind::object, i.objectPrototype);
    i.symbolPrototype = allocate(ObjectKind::object, i.objectPrototype);
    record(i.numberPrototype).payload=BoxedPrimitive{Value(0)};
    record(i.booleanPrototype).payload=BoxedPrimitive{Value(false)};
    record(i.stringPrototype).payload=BoxedPrimitive{Value(std::u16string{})};
    define(i.stringPrototype,Key(u"length"),Descriptor::data(Value(0),false,false,false));

    auto make_intrinsic = [&](std::u16string name, std::uint32_t arity, ObjectRef prototype) {
        const auto fn = allocate(ObjectKind::function, i.functionPrototype);
        auto& r = record(fn);
        r.label = name;
        r.properties.emplace_back(Key(u"length"), Descriptor::data(Value(static_cast<double>(arity)), false, false, true));
        r.properties.emplace_back(Key(u"name"), Descriptor::data(Value(name), false, false, true));
        r.properties.emplace_back(Key(u"prototype"), Descriptor::data(Value(prototype), false, false, false));
        return fn;
    };

    i.objectConstructor = make_intrinsic(u"Object", 1, i.objectPrototype);
    i.functionConstructor = make_intrinsic(u"Function", 1, i.functionPrototype);
    i.arrayConstructor = make_intrinsic(u"Array", 1, i.arrayPrototype);
    i.expandableConstructor = make_intrinsic(u"Expandable", 0, i.expandablePrototype);
    i.numberConstructor = make_intrinsic(u"Number", 1, i.numberPrototype);
    i.booleanConstructor = make_intrinsic(u"Boolean", 1, i.booleanPrototype);
    i.stringConstructor = make_intrinsic(u"String", 1, i.stringPrototype);
    i.symbolConstructor = make_intrinsic(u"Symbol", 0, i.symbolPrototype);

    const auto arraySpeciesGetter = function(u"get [Symbol.species]",
        [](Context&, Value receiver, std::span<const Value>) { return receiver; });
    define(i.arrayConstructor, Key(i.symbolSpecies),
           Descriptor::access(arraySpeciesGetter, std::nullopt, false, true));
    define(i.symbolConstructor, Key(u"isConcatSpreadable"),
           Descriptor::data(Value(i.symbolIsConcatSpreadable), false, false, false));
    define(i.symbolConstructor,Key(u"split"),
           Descriptor::data(Value(i.symbolSplit),false,false,false));
    define(i.symbolConstructor,Key(u"match"),
           Descriptor::data(Value(i.symbolMatch),false,false,false));

    auto link_constructor = [&](ObjectRef prototype, ObjectRef constructor) {
        define(prototype, Key(std::u16string(kConstructor)), Descriptor::data(Value(constructor), true, false, true));
    };
    link_constructor(i.objectPrototype, i.objectConstructor);
    link_constructor(i.functionPrototype, i.functionConstructor);
    link_constructor(i.arrayPrototype, i.arrayConstructor);
    link_constructor(i.expandablePrototype, i.expandableConstructor);
    link_constructor(i.numberPrototype, i.numberConstructor);
    link_constructor(i.booleanPrototype, i.booleanConstructor);
    link_constructor(i.stringPrototype, i.stringConstructor);
    link_constructor(i.symbolPrototype, i.symbolConstructor);
    define(i.symbolPrototype,Key(i.symbolToStringTag),Descriptor::data(Value(u"Symbol"),false,false,true));

    const auto string_trim=function(u"trim",[](Context& context,Value receiver,std::span<const Value>)->Value{
        if(is_nullish(receiver))type_error(u"String.prototype.trim called on null or undefined");
        const Value primitive=context.toPrimitive(receiver,PrimitiveHint::stringHint);
        std::u16string text;
        if(const auto* value=std::get_if<std::u16string>(&primitive.data))text=*value;
        else if(std::holds_alternative<Undefined>(primitive.data))text=u"undefined";
        else if(std::holds_alternative<Null>(primitive.data))text=u"null";
        else if(const auto* value=std::get_if<bool>(&primitive.data))text=*value?u"true":u"false";
        else if(const auto* value=std::get_if<double>(&primitive.data)){
            const auto ascii=folia::math::js_number::toString(*value);text.assign(ascii.begin(),ascii.end());
        }else type_error(u"Cannot convert a Symbol value to a string");
        std::u16string_view trimmed(text);
        while(!trimmed.empty()&&folia::math::js_input::isWhitespace(trimmed.front()))trimmed.remove_prefix(1);
        while(!trimmed.empty()&&folia::math::js_input::isWhitespace(trimmed.back()))trimmed.remove_suffix(1);
        return Value(std::u16string(trimmed));
    });
    define(string_trim,Key(u"length"),Descriptor::data(Value(0),false,false,true));
    define(i.stringPrototype,Key(u"trim"),Descriptor::data(Value(string_trim),true,false,true));
    const auto string_to_primitive=function(u"toString",[](Context& context,Value receiver,
                                                               std::span<const Value>)->Value{
        if(const auto* text=std::get_if<std::u16string>(&receiver.data))return Value(*text);
        const auto* object=std::get_if<ObjectRef>(&receiver.data);
        if(object){
            const auto& payload=std::as_const(context).payload(*object);
            if(const auto* boxed=std::any_cast<BoxedPrimitive>(&payload))
                if(std::holds_alternative<std::u16string>(boxed->value.data))return boxed->value;
        }
        type_error(u"String.prototype.toString called on incompatible receiver");
    });
    define(string_to_primitive,Key(u"length"),Descriptor::data(Value(0),false,false,true));
    define(i.stringPrototype,Key(u"toString"),Descriptor::data(Value(string_to_primitive),true,false,true));
    const auto string_value_of=function(u"valueOf",[](Context& context,Value receiver,
                                                         std::span<const Value>)->Value{
        if(const auto* text=std::get_if<std::u16string>(&receiver.data))return Value(*text);
        const auto* object=std::get_if<ObjectRef>(&receiver.data);
        if(object){
            const auto& payload=std::as_const(context).payload(*object);
            if(const auto* boxed=std::any_cast<BoxedPrimitive>(&payload))
                if(std::holds_alternative<std::u16string>(boxed->value.data))return boxed->value;
        }
        type_error(u"String.prototype.valueOf called on incompatible receiver");
    });
    define(string_value_of,Key(u"length"),Descriptor::data(Value(0),false,false,true));
    define(i.stringPrototype,Key(u"valueOf"),Descriptor::data(Value(string_value_of),true,false,true));

    const auto string_split=function(u"split",[](Context& context,Value receiver,
                                                   std::span<const Value> arguments)->Value{
        if(is_nullish(receiver))type_error(u"String.prototype.split called on null or undefined");
        const Value separator=arguments.empty()?Value(Undefined{}):arguments.front();
        const Value limit=arguments.size()<2?Value(Undefined{}):arguments[1];

        if(!is_nullish(separator)){
            const Value splitter=context.get(separator,Key(context.intrinsics().symbolSplit));
            if(!is_nullish(splitter)){
                const auto* callable=std::get_if<ObjectRef>(&splitter.data);
                if(!callable||context.kind(*callable)!=ObjectKind::function)
                    type_error(u"String split separator Symbol.split is not callable");
                const std::array<Value,2> splitArguments{receiver,limit};
                return context.call(*callable,separator,splitArguments);
            }
        }

        const std::u16string source=primitive_string(context,std::move(receiver));
        const std::uint32_t maximum=std::holds_alternative<Undefined>(limit.data)
            ?std::numeric_limits<std::uint32_t>::max():to_uint32(context,limit);
        const std::u16string separatorText=std::holds_alternative<Undefined>(separator.data)
            ?std::u16string{}:primitive_string(context,separator);
        const auto result=context.array(0);
        if(maximum==0)return Value(result);
        auto append=[&](std::uint32_t index,std::u16string_view part){
            std::u16string key=index_string(index);
            context.define(result,Key(std::move(key)),Descriptor::data(Value(std::u16string(part))));
        };
        if(std::holds_alternative<Undefined>(separator.data)){
            append(0,source);
            return Value(result);
        }
        if(separatorText.empty()){
            for(std::size_t index=0;index<source.size()&&index<maximum;++index)
                append(static_cast<std::uint32_t>(index),source.substr(index,1));
            return Value(result);
        }
        std::size_t start=0;
        std::uint32_t index=0;
        for(;;){
            const auto found=source.find(separatorText,start);
            if(found==std::u16string::npos){append(index,source.substr(start));return Value(result);}
            append(index++,source.substr(start,found-start));
            if(index>=maximum)return Value(result);
            start=found+separatorText.size();
        }
    });
    define(string_split,Key(u"length"),Descriptor::data(Value(2),false,false,true));
    define(i.stringPrototype,Key(u"split"),Descriptor::data(Value(string_split),true,false,true));

    const auto string_slice=function(u"slice",[](Context& context,Value receiver,
                                                    std::span<const Value> arguments)->Value{
        if(is_nullish(receiver))type_error(u"String.prototype.slice called on null or undefined");
        const std::u16string source=primitive_string(context,std::move(receiver));
        const double length=static_cast<double>(source.size());
        const auto integer=[&](const Value& value){
            const double number=context.toNumber(value);
            if(std::isnan(number)||number==0)return 0.0;
            if(!std::isfinite(number))return number;
            return std::trunc(number);
        };
        const double start=arguments.empty()?0.0:integer(arguments.front());
        const double end=arguments.size()<2||std::holds_alternative<Undefined>(arguments[1].data)
            ?length:integer(arguments[1]);
        const auto clamp=[&](double index){
            if(index==-std::numeric_limits<double>::infinity())return std::size_t{0};
            if(index<0)return static_cast<std::size_t>(std::max(length+index,0.0));
            return static_cast<std::size_t>(std::min(index,length));
        };
        const auto first=clamp(start),last=clamp(end);
        return Value(first<last?source.substr(first,last-first):std::u16string{});
    });
    define(string_slice,Key(u"length"),Descriptor::data(Value(2),false,false,true));
    define(i.stringPrototype,Key(u"slice"),Descriptor::data(Value(string_slice),true,false,true));

    for (const auto name : {u"__defineGetter__", u"__defineSetter__", u"hasOwnProperty",
                            u"__lookupGetter__", u"__lookupSetter__", u"isPrototypeOf",
                            u"propertyIsEnumerable"}) {
        const auto method = function(std::u16string(name));
        define(i.objectPrototype, Key(std::u16string(name)), Descriptor::data(Value(method), true, false, true));
    }
    const auto object_to_string=function(u"toString",[](Context& context,Value receiver,std::span<const Value>)->Value {
        if(std::holds_alternative<Undefined>(receiver.data)) return Value(u"[object Undefined]");
        if(std::holds_alternative<Null>(receiver.data)) return Value(u"[object Null]");
        const auto object=to_object(context,receiver);
        std::u16string brand;
        const auto& payload=context.payload(object);
        if(const auto* boxed=std::any_cast<BoxedPrimitive>(&payload)) {
            if(std::holds_alternative<bool>(boxed->value.data)) brand=u"Boolean";
            else if(std::holds_alternative<double>(boxed->value.data)) brand=u"Number";
            else if(std::holds_alternative<std::u16string>(boxed->value.data)) brand=u"String";
            else { (void)context.symbolString(std::get<SymbolRef>(boxed->value.data)); brand=u"Object"; }
        } else {
            switch(context.kind(object)) {
            case ObjectKind::array: brand=u"Array"; break;
            case ObjectKind::function: brand=u"Function"; break;
            case ObjectKind::object: case ObjectKind::instance: brand=u"Object"; break;
            }
        }
        const Value tag=context.get(Value(object),Key(context.intrinsics().symbolToStringTag));
        if(const auto* text=std::get_if<std::u16string>(&tag.data)) brand=*text;
        return Value(u"[object "+brand+u"]");
    });
    define(i.objectPrototype,Key(u"toString"),Descriptor::data(Value(object_to_string),true,false,true));
    const auto object_value_of=function(u"valueOf",[](Context& context,Value receiver,std::span<const Value>)->Value {
        return Value(to_object(context,std::move(receiver)));
    });
    define(i.objectPrototype,Key(u"valueOf"),Descriptor::data(Value(object_value_of),true,false,true));

    const auto proto_getter = function(u"get __proto__", [](Context& context, Value receiver, std::span<const Value>) -> Value {
        const auto* object = std::get_if<ObjectRef>(&receiver.data);
        if (!object) return Value(Undefined{});
        const auto prototype = context.prototype(*object);
        return prototype ? Value(*prototype) : Value(Null{});
    });
    const auto proto_setter = function(u"set __proto__", [](Context& context, Value receiver, std::span<const Value> args) -> Value {
        const auto* object = std::get_if<ObjectRef>(&receiver.data);
        if (!object || args.empty()) return Value(Undefined{});
        if (const auto* prototype = std::get_if<ObjectRef>(&args.front().data)) context.setPrototype(*object, *prototype);
        else if (std::holds_alternative<Null>(args.front().data)) context.setPrototype(*object, std::nullopt);
        return Value(Undefined{});
    });
    define(i.objectPrototype, Key(std::u16string(kProto)), Descriptor::access(proto_getter, proto_setter, false, true));
    const auto to_locale_string = function(u"toLocaleString");
    define(i.objectPrototype, Key(u"toLocaleString"), Descriptor::data(Value(to_locale_string), true, false, true));

    auto initialize_constructor_proto = [&](ObjectRef constructor, ObjectRef prototype) {
        auto& fn = record(constructor);
        for (auto& [key, descriptor] : fn.properties) {
            if (key_is(key, u"prototype")) descriptor = Descriptor::data(Value(prototype), false, false, false);
        }
    };
    initialize_constructor_proto(i.objectConstructor, i.objectPrototype);
    initialize_constructor_proto(i.functionConstructor, i.functionPrototype);
    initialize_constructor_proto(i.arrayConstructor, i.arrayPrototype);
    initialize_constructor_proto(i.expandableConstructor, i.expandablePrototype);
    initialize_constructor_proto(i.numberConstructor, i.numberPrototype);
    initialize_constructor_proto(i.booleanConstructor, i.booleanPrototype);
    initialize_constructor_proto(i.stringConstructor, i.stringPrototype);
    initialize_constructor_proto(i.symbolConstructor, i.symbolPrototype);

    // Function.prototype.call is part of the emitted TypeScript helpers' real
    // property lookup path. Keep it as one ordinary inherited function so
    // deleting or shadowing `call` is observable by dynamic member calls.
    const auto function_call = function(u"call",
        [](Context& context, Value receiver, std::span<const Value> arguments) -> Value {
            const auto* target = std::get_if<ObjectRef>(&receiver.data);
            if (!target || context.kind(*target) != ObjectKind::function)
                type_error(u"Function.prototype.call called on incompatible receiver");
            const Value this_argument = arguments.empty() ? Value(Undefined{}) : arguments.front();
            const auto forwarded = arguments.empty() ? std::span<const Value>{} : arguments.subspan(1);
            return context.call(*target, this_argument, forwarded);
        });
    define(function_call, Key(u"length"), Descriptor::data(Value(1), false, false, true));
    define(i.functionPrototype, Key(u"call"), Descriptor::data(Value(function_call), true, false, true));

    // Function.prototype.apply is also used directly by the transpiled
    // MathJax factory methods. Keep it as an ordinary inherited method so
    // replacing or shadowing `apply` remains observable at the call site.
    const auto function_apply = function(u"apply",
        [](Context& context, Value receiver, std::span<const Value> arguments) -> Value {
            const auto* target = std::get_if<ObjectRef>(&receiver.data);
            if (!target || context.kind(*target) != ObjectKind::function)
                type_error(u"Function.prototype.apply called on incompatible receiver");

            const Value this_argument = arguments.empty() ? Value(Undefined{}) : arguments.front();
            std::vector<Value> forwarded;
            if (arguments.size() > 1 &&
                !std::holds_alternative<Null>(arguments[1].data) &&
                !std::holds_alternative<Undefined>(arguments[1].data)) {
                const auto* argument_array=std::get_if<ObjectRef>(&arguments[1].data);
                if(!argument_array)
                    type_error(u"CreateListFromArrayLike called on a non-object");
                (void)context.kind(*argument_array);
                const Value length_value = context.get(Value(*argument_array), Key(u"length"));
                double length = context.toNumber(length_value);
                if (std::isnan(length) || length <= 0) length = 0;
                else if (!std::isfinite(length)) length = 9007199254740991.0;
                else length = std::min(std::floor(length), 9007199254740991.0);
                if (length > static_cast<double>(std::numeric_limits<std::size_t>::max()))
                    type_error(u"Too many arguments for Function.prototype.apply");
                forwarded.reserve(static_cast<std::size_t>(length));
                for (std::size_t index = 0; index < static_cast<std::size_t>(length); ++index) {
                    const auto ascii = std::to_string(index);
                    forwarded.push_back(context.get(Value(*argument_array), Key(std::u16string(ascii.begin(), ascii.end()))));
                }
            }
            return context.call(*target, this_argument, forwarded);
        });
    define(function_apply, Key(u"length"), Descriptor::data(Value(2), false, false, true));
    define(i.functionPrototype, Key(u"apply"), Descriptor::data(Value(function_apply), true, false, true));

    initializeArrayIntrinsics();
}

Context::~Context() {
    // Native owners retained by records can own canonical identity registries.
    // Release them while every record slot still exists: their destructors may
    // clear payloads in earlier slots. Vector destruction would leave dangling
    // slots behind before those later owners are released.
    for (std::size_t index=0;index<state_->objects.size();++index) {
        if (!state_->objects[index]) continue;
        auto& slot=*state_->objects[index];
        const bool wasRetired=slot.retired;
        auto released=std::exchange(slot,Record{});
        slot.retired=wasRetired;
    }
}

std::shared_ptr<void> Context::getOrCreateServiceImpl(std::u16string identity,
        std::type_index type,std::function<std::shared_ptr<void>()> create){
    if(identity.empty()||!create)throw Exception("TypeError",u"Context service requires an identity and initializer");
    if(const auto found=state_->services.find(identity);found!=state_->services.end()){
        if(auto owner=found->second.owner.lock()){
            if(found->second.type!=type)throw Exception("TypeError",u"Context service identity reused for a different native owner");
            return owner;
        }
        state_->services.erase(found);
    }
    auto owner=create();
    if(!owner)throw Exception("TypeError",u"Context service initializer returned no owner");
    auto [found,inserted]=state_->services.emplace(std::move(identity),ContextState::ServiceEntry{type,owner});
    if(!inserted)throw Exception("TypeError",u"Context service registry changed during initialization");
    return owner;
}

const Intrinsics& Context::intrinsics() const { return state_->intrinsics; }

ObjectRef Context::object() { return allocate(ObjectKind::object, state_->intrinsics.objectPrototype); }
ObjectRef Context::toObject(Value value) { return to_object(*this,std::move(value)); }
ObjectRef Context::nullObject() { return allocate(ObjectKind::object, std::nullopt); }
ObjectRef Context::objectWithPrototype(ObjectRef prototype) {
    (void)record(prototype);
    return allocate(ObjectKind::object, prototype);
}
ObjectRef Context::nativeRecord(NativeRecordViewRef view,std::optional<ObjectRef> prototype_ref){
    if(!view)type_error(u"Native record view is null");
    if(!prototype_ref)prototype_ref=view->prototype();
    if(prototype_ref)(void)record(*prototype_ref);
    const auto result=allocate(ObjectKind::object,prototype_ref);
    auto& r=record(result);
    r.nativeRecordView=std::move(view);
    r.nativeRecordView->setPrototype(r.prototype);
    return result;
}

ObjectRef Context::function(std::u16string label, NativeCall callable, std::any payload_value) {
    const auto result = allocate(ObjectKind::function, state_->intrinsics.functionPrototype);
    auto& r = record(result);
    r.label = label;
    r.callable = std::move(callable);
    r.payload = std::move(payload_value);
    r.properties.emplace_back(Key(u"length"), Descriptor::data(Value(0), false, false, true));
    r.properties.emplace_back(Key(u"name"), Descriptor::data(Value(std::move(label)), false, false, true));
    return result;
}

ObjectRef Context::constructor(std::u16string label, NativeCall callable, std::any payload_value) {
    const auto result = allocate(ObjectKind::function, state_->intrinsics.functionPrototype);
    auto& r = record(result);
    r.label = label;
    r.callable = std::move(callable);
    r.payload = std::move(payload_value);
    r.properties.emplace_back(Key(u"length"), Descriptor::data(Value(0), false, false, true));
    r.properties.emplace_back(Key(u"name"), Descriptor::data(Value(label), false, false, true));
    const auto proto = object();
    r.properties.emplace_back(Key(u"prototype"), Descriptor::data(Value(proto), true, false, false));
    define(proto, Key(std::u16string(kConstructor)), Descriptor::data(Value(result), true, false, true));
    return result;
}

ObjectRef Context::instance(ObjectRef constructor_ref, std::any payload_value) {
    if (kind(constructor_ref) != ObjectKind::function) type_error(u"Instance constructor is not callable");
    auto prototype_value = get(Value(constructor_ref), Key(u"prototype"));
    ObjectRef prototype_ref = state_->intrinsics.objectPrototype;
    if (const auto* object = std::get_if<ObjectRef>(&prototype_value.data)) prototype_ref = *object;
    const auto result = allocate(ObjectKind::instance, prototype_ref);
    record(result).payload = std::move(payload_value);
    return result;
}

ObjectRef Context::nativeConstructor(std::u16string label, NativeConstruct construct_body,
                                     NativeCall callable, std::any payload_value) {
    if (!construct_body) type_error(u"Native constructor has no allocation body");
    if (!callable) callable = [](Context&, Value, std::span<const Value>) -> Value {
        throw Exception("Unimplemented", u"Native constructor ordinary-call body is not registered");
    };
    const auto result = constructor(std::move(label), std::move(callable), std::move(payload_value));
    record(result).construct = std::move(construct_body);
    return result;
}

double Context::toNumber(const Value& value) {
    return primitive_number(toPrimitive(value,PrimitiveHint::numberHint));
}

std::any& Context::payload(ObjectRef object_ref) {
    auto& value=record(object_ref);
    value.payloadMutableAliasEscaped=true;
    std::erase_if(value.ownerTraces,[](const auto& entry){return entry.first==NativeOwnerPart::payload;});
    return value.payload;
}
const std::any& Context::payload(ObjectRef object_ref) const { return record(object_ref).payload; }
void Context::setPayload(ObjectRef object_ref,std::any payload_value){
    auto& value=record(object_ref);
    std::erase_if(value.ownerTraces,[](const auto& entry){return entry.first==NativeOwnerPart::payload;});
    value.payload=std::move(payload_value);
}
void Context::retireObjectRecord(ObjectRef object_ref){
    auto& value=record(object_ref);
    value=Record{};
    value.retired=true;
}
Context::RecordCounts Context::recordCountsForDiagnostics() const noexcept {
    RecordCounts counts;
    counts.slots=state_->objects.size();
    const char* detailValue=std::getenv("FOLIA_MATH_CONTEXT_OWNER_DETAIL");
    const bool ownerDetail=detailValue&&std::string_view(detailValue)=="1";
    std::map<std::string,std::size_t> ownerGroups;
    std::size_t metadataUnavailable=0;
    for(const auto& slot:state_->objects){
        if(!slot)continue;
        const auto& value=*slot;
        if(value.retired){++counts.retired;continue;}
        ++counts.live;
        const bool ownerBearing=value.payload.has_value()||value.callable||value.construct||
           value.nativeIndexedView||value.nativeRecordView||
           !value.nativeFields.empty()||!value.declaredNativeFields.empty()||
           value.attachedOwner||!value.ownerTraces.empty();
        if(!ownerBearing)continue;
        ++counts.ownerBearing;
        if(!ownerDetail)continue;
        try{
            const auto ascii=[](std::u16string_view text){
                std::string result;
                result.reserve(text.size());
                for(const auto ch:text)result.push_back(ch>=0x20&&ch<=0x7e?static_cast<char>(ch):'?');
                return result;
            };
            std::string group="kind="+std::to_string(static_cast<unsigned>(value.kind));
            group+=";payload="+(value.payload.has_value()?std::string(value.payload.type().name()):"-");
            group+=";recordView="+(value.nativeRecordView?std::string(typeid(*value.nativeRecordView).name()):"-");
            group+=";arrayView="+(value.nativeIndexedView?std::string(typeid(*value.nativeIndexedView).name()):"-");
            std::string callableName;
            if((value.callable||value.construct)&&!value.label.empty())callableName=ascii(value.label);
            else for(const auto& [key,descriptor]:value.properties){
                const auto* keyText=std::get_if<std::u16string>(&key.data);
                if(!keyText||*keyText!=u"name"||descriptor.accessor)continue;
                if(const auto* name=std::get_if<std::u16string>(&descriptor.value.data))callableName=ascii(*name);
                break;
            }
            group+=";callableName="+(callableName.empty()?"-":callableName);
            group+=";nativeFields=";
            bool first=true;
            const auto addFields=[&](const auto& fields){
                for(const auto& [name,field]:fields){
                    (void)field;
                    if(!first)group+=",";
                    first=false;
                    if constexpr(std::is_same_v<std::remove_cvref_t<decltype(name)>,Key>){
                        if(const auto* text=std::get_if<std::u16string>(&name.data))group+=ascii(*text);
                        else group+="[symbol]";
                    }else group+=ascii(name);
                }
            };
            addFields(value.nativeFields);
            addFields(value.declaredNativeFields);
            if(first)group+="-";
            ++ownerGroups[group];
        }catch(...){
            ++metadataUnavailable;
        }
    }
    if(ownerDetail){
        try{
            std::vector<std::pair<std::string,std::size_t>> ordered(ownerGroups.begin(),ownerGroups.end());
            std::sort(ordered.begin(),ordered.end(),[](const auto& left,const auto& right){
                return left.second!=right.second?left.second>right.second:left.first<right.first;
            });
            std::cerr<<"Context owner detail groups (top 20; total="<<counts.ownerBearing<<")\n";
            const auto limit=std::min<std::size_t>(20,ordered.size());
            for(std::size_t index=0;index<limit;++index)
                std::cerr<<ordered[index].second<<"\t"<<ordered[index].first<<"\n";
            if(metadataUnavailable)std::cerr<<metadataUnavailable<<"\tmetadata-unavailable\n";
            std::size_t grouped=metadataUnavailable;
            for(const auto& entry:ordered)grouped+=entry.second;
            std::cerr<<"Context owner detail grouped="<<grouped<<"\n";
        }catch(...){
            // Diagnostics must never change the ordinary record-count result.
        }
    }
    return counts;
}
Context::AllocationCounts Context::allocationCountsForDiagnostics() const noexcept {
    return {state_->objects.size(),state_->symbols.size()};
}
ObjectKind Context::kind(ObjectRef object_ref) const { return record(object_ref).kind; }

SymbolRef Context::symbol(std::optional<std::u16string> description) {
    const auto index = static_cast<std::uint64_t>(state_->symbols.size());
    state_->symbols.emplace_back(std::move(description));
    return {state_->identity, index};
}

std::u16string Context::symbolString(SymbolRef symbol_ref) const {
    if (symbol_ref.context != state_->identity || symbol_ref.index >= state_->symbols.size()) {
        type_error(u"Symbol belongs to a different or invalid Context");
    }
    std::u16string result = u"Symbol(";
    if (state_->symbols[symbol_ref.index]) result += *state_->symbols[symbol_ref.index];
    result.push_back(u')');
    return result;
}

Value Context::toPrimitive(Value value, PrimitiveHint hint) {
    const auto* object = std::get_if<ObjectRef>(&value.data);
    if (!object) {
        if (const auto* symbol = std::get_if<SymbolRef>(&value.data)) (void)symbolString(*symbol);
        return value;
    }
    (void)record(*object);

    const Value exotic = get(value, Key(intrinsics().symbolToPrimitive));
    if (!is_nullish(exotic)) {
        const auto* method = std::get_if<ObjectRef>(&exotic.data);
        if (!method || kind(*method) != ObjectKind::function)
            type_error(u"@@toPrimitive must be a function");
        const std::u16string hint_name = hint == PrimitiveHint::stringHint ? u"string" :
                                         hint == PrimitiveHint::numberHint ? u"number" : u"default";
        const std::array<Value, 1> arguments{Value(hint_name)};
        Value result = call(*method, value, arguments);
        if (std::holds_alternative<ObjectRef>(result.data))
            type_error(u"Cannot convert object to primitive value");
        if (const auto* symbol = std::get_if<SymbolRef>(&result.data)) (void)symbolString(*symbol);
        return result;
    }

    const std::array<std::u16string_view, 2> string_order{u"toString", u"valueOf"};
    const std::array<std::u16string_view, 2> number_order{u"valueOf", u"toString"};
    const auto& order = hint == PrimitiveHint::stringHint ? string_order : number_order;
    for (const auto name : order) {
        const Value candidate = get(value, Key(std::u16string(name)));
        const auto* method = std::get_if<ObjectRef>(&candidate.data);
        if (!method || kind(*method) != ObjectKind::function) continue;
        Value result = call(*method, value);
        if (std::holds_alternative<ObjectRef>(result.data)) continue;
        if (const auto* symbol = std::get_if<SymbolRef>(&result.data)) (void)symbolString(*symbol);
        return result;
    }
    type_error(u"Cannot convert object to primitive value");
}

std::u16string Context::keyString(const Key& key) const {
    if (const auto* text = std::get_if<std::u16string>(&key.data)) return *text;
    return symbolString(std::get<SymbolRef>(key.data));
}

bool ordinaryInstanceOf(Context& context,const Value& value,const Value& constructor) {
    const auto* constructorObject=std::get_if<ObjectRef>(&constructor.data);
    if(!constructorObject||context.kind(*constructorObject)!=ObjectKind::function)
        type_error(u"Right-hand side of 'instanceof' is not callable");
    const auto* instanceObject=std::get_if<ObjectRef>(&value.data);
    if(!instanceObject)return false;
    const Value prototypeValue=context.get(constructor,Key(u"prototype"));
    const auto* expected=std::get_if<ObjectRef>(&prototypeValue.data);
    if(!expected)type_error(u"Function has non-object prototype in instanceof check");
    auto current=context.prototype(*instanceObject);
    while(current){if(*current==*expected)return true;current=context.prototype(*current);}
    return false;
}
std::optional<ObjectRef> Context::prototype(ObjectRef object_ref) const {
    const auto& r=record(object_ref);
    return r.nativeRecordView?r.nativeRecordView->prototype():r.prototype;
}

void Context::setPrototype(ObjectRef object_ref, std::optional<ObjectRef> prototype_ref) {
    auto& target = record(object_ref);
    if (prototype_ref) (void)record(*prototype_ref);
    auto cursor = prototype_ref;
    while (cursor) {
        if (*cursor == object_ref) type_error(u"Cyclic prototype value");
        cursor = prototype(*cursor);
    }
    if(target.nativeRecordView)target.nativeRecordView->setPrototype(prototype_ref);
    else target.prototype = prototype_ref;
}

std::vector<Key> Context::ownKeys(Value object_value) const {
    if (is_nullish(object_value)) type_error(u"Cannot convert null or undefined to object");
    if (const auto* symbol_ref = std::get_if<SymbolRef>(&object_value.data)) (void)symbolString(*symbol_ref);
    const auto* object_ref = std::get_if<ObjectRef>(&object_value.data);
    if (!object_ref) {
        std::vector<Key> keys;
        if (const auto* string = std::get_if<std::u16string>(&object_value.data)) {
            keys.reserve(string->size() + 1);
            for (std::uint32_t index = 0; index < string->size(); ++index) keys.emplace_back(index_string(index));
            keys.emplace_back(std::u16string(kLength));
        }
        return keys;
    }
    const auto& r = record(*object_ref);
    struct Indexed { std::uint32_t index; Key key; };
    std::vector<Indexed> indices;
    std::vector<Key> strings;
    std::vector<Key> symbols;
    if(r.nativeIndexedView){
        for(const auto index:r.nativeIndexedView->ownIndexKeys())
            indices.push_back({index,Key(index_string(index))});
        strings.emplace_back(std::u16string(kLength));
    }
    if(r.nativeRecordView){
        for(const auto& key:r.nativeRecordView->ownKeys()){
            if(const auto* text=std::get_if<std::u16string>(&key.data)){
                if(const auto index=array_index(*text))indices.push_back({*index,key});
                else if(std::find(strings.begin(),strings.end(),key)==strings.end())strings.push_back(key);
            }else if(std::find(symbols.begin(),symbols.end(),key)==symbols.end())symbols.push_back(key);
        }
    }
    for (const auto& [key, descriptor_value] : r.properties) {
        (void)descriptor_value;
        if (const auto* text = std::get_if<std::u16string>(&key.data)) {
            if (const auto index = array_index(*text)) indices.push_back({*index, key});
            else strings.push_back(key);
        } else {
            symbols.push_back(key);
        }
    }
    std::sort(indices.begin(), indices.end(), [](const Indexed& a, const Indexed& b) { return a.index < b.index; });
    std::vector<Key> result;
    result.reserve(indices.size() + strings.size() + symbols.size());
    for (auto& item : indices) result.push_back(std::move(item.key));
    result.insert(result.end(), strings.begin(), strings.end());
    result.insert(result.end(), symbols.begin(), symbols.end());
    return result;
}

std::vector<std::u16string> Context::enumerableStringKeys(Value object_value) const {
    std::vector<std::u16string> result;
    for (const auto& key : ownKeys(object_value)) {
        const auto* text = std::get_if<std::u16string>(&key.data);
        if (!text) continue;
        const auto property = rawDescriptor(object_value, key);
        if (property && property->enumerable) result.push_back(*text);
    }
    return result;
}

std::vector<SymbolRef> Context::ownSymbols(Value object_value) const {
    std::vector<SymbolRef> result;
    for (const auto& key : ownKeys(object_value)) {
        if (const auto* symbol_ref = std::get_if<SymbolRef>(&key.data)) result.push_back(*symbol_ref);
    }
    return result;
}

bool Context::ownPropertyEnumerable(Value object_value,const Key& key)const{
    validate_key(*this,key);
    if(is_nullish(object_value))type_error(u"Cannot convert null or undefined to object");
    if(const auto* object=std::get_if<ObjectRef>(&object_value.data)){
        const auto& owner=record(*object);
        if(owner.nativeRecordView)return owner.nativeRecordView->ownPropertyEnumerable(key);
    }
    const auto descriptor=rawDescriptor(std::move(object_value),key);
    return descriptor&&descriptor->enumerable;
}

std::optional<Descriptor> Context::rawDescriptor(Value object_value, const Key& key) const {
    validate_key(*this, key);
    if (is_nullish(object_value)) type_error(u"Cannot convert null or undefined to object");
    if (const auto* symbol_ref = std::get_if<SymbolRef>(&object_value.data)) (void)symbolString(*symbol_ref);
    if (const auto* text = std::get_if<std::u16string>(&object_value.data)) {
        if (key_is(key, kLength)) return Descriptor::data(Value(static_cast<double>(text->size())), false, false, false);
        if (const auto* name = std::get_if<std::u16string>(&key.data)) {
            if (const auto index = array_index(*name); index && *index < text->size()) {
                return Descriptor::data(Value(std::u16string(1, (*text)[*index])), false, true, false);
            }
        }
        return std::nullopt;
    }
    const auto* ref = std::get_if<ObjectRef>(&object_value.data);
    if (!ref) return std::nullopt;
    const auto& r = record(*ref);
    if(r.kind==ObjectKind::array&&r.nativeIndexedView){
        if(key_is(key,kLength))return r.nativeIndexedView->lengthDescriptor();
        if(const auto* name=std::get_if<std::u16string>(&key.data))
            if(const auto index=array_index(*name))return r.nativeIndexedView->indexDescriptor(*index);
    }
    if(r.nativeRecordView){
        if(auto own=r.nativeRecordView->ownDescriptor(key))return own;
    }
    for (const auto& [own_key, own_descriptor] : r.properties) if (own_key == key) return own_descriptor;
    return std::nullopt;
}

NativeDataFieldRef Context::nativeDataField(ObjectRef object,const Key& key)const{
    for(const auto& [name,field]:record(object).nativeFields)if(name==key)return field;
    return {};
}
NativeDataFieldRef Context::declaredNativeDataField(ObjectRef object,const Key& key)const{
    for(const auto& [name,field]:record(object).declaredNativeFields)if(name==key)return field;
    return {};
}

std::optional<Descriptor> Context::descriptor(Value object_value,const Key& key)const{
    auto result=rawDescriptor(object_value,key);
    if(result&&!result->accessor)if(const auto* object=std::get_if<ObjectRef>(&object_value.data))
        if(auto field=nativeDataField(*object,key)){
            result->value=field->read();
            validate_value(*this,result->value);
        }
    return result;
}

bool Context::hasOwn(Value object_value, const Key& key) const { return rawDescriptor(std::move(object_value), key).has_value(); }

bool Context::has(Value object_value, const Key& key) const {
    if (is_nullish(object_value)) type_error(u"Cannot convert null or undefined to object");
    if (rawDescriptor(object_value, key)) return true;
    const auto* object_ref = std::get_if<ObjectRef>(&object_value.data);
    auto cursor = object_ref ? prototype(*object_ref) : std::optional<ObjectRef>{};
    if (!object_ref) {
        if (std::holds_alternative<std::u16string>(object_value.data)) cursor = state_->intrinsics.stringPrototype;
        else if (std::holds_alternative<double>(object_value.data)) cursor = state_->intrinsics.numberPrototype;
        else if (std::holds_alternative<bool>(object_value.data)) cursor = state_->intrinsics.booleanPrototype;
        else if (std::holds_alternative<SymbolRef>(object_value.data)) cursor = state_->intrinsics.symbolPrototype;
    }
    while (cursor) {
        if (rawDescriptor(Value(*cursor), key)) return true;
        cursor = prototype(*cursor);
    }
    return false;
}

Value Context::get(Value object_value, const Key& key) {
    validate_key(*this, key);
    validate_value(*this, object_value);
    if (is_nullish(object_value)) {
        const std::u16string receiver=std::holds_alternative<Null>(object_value.data)
            ?u"null":u"undefined";
        type_error(u"Cannot read properties of "+receiver+u" (reading '"+keyString(key)+u"')");
    }
    auto current = object_value;
    while (true) {
        const auto property = descriptor(current, key);
        if (property) {
            if (!property->accessor) return property->value;
            if (!property->getter) return Value(Undefined{});
            return call(*property->getter, object_value, {});
        }
        const auto* object_ref = std::get_if<ObjectRef>(&current.data);
        if (object_ref) {
            const auto proto = prototype(*object_ref);
            if (proto) { current = Value(*proto); continue; }
        } else if (std::holds_alternative<std::u16string>(current.data)) {
            current = Value(state_->intrinsics.stringPrototype);
            continue;
        } else if (std::holds_alternative<double>(current.data)) {
            current = Value(state_->intrinsics.numberPrototype);
            continue;
        } else if (std::holds_alternative<bool>(current.data)) {
            current = Value(state_->intrinsics.booleanPrototype);
            continue;
        } else if (std::holds_alternative<SymbolRef>(current.data)) {
            current = Value(state_->intrinsics.symbolPrototype);
            continue;
        }
        return Value(Undefined{});
    }
}

void Context::set(Value object_value, const Key& key, Value value) {
    setImpl(std::move(object_value),key,std::move(value));
}

bool Context::assignNativeData(ObjectRef object_ref,const Key& key,NativeDataFieldRef field,Value value){
    if(!field)type_error(u"Native data field is null");
    setImpl(Value(object_ref),key,std::move(value),std::move(field));
    return static_cast<bool>(nativeDataField(object_ref,key));
}

void Context::declareNativeData(ObjectRef object_ref,const Key& key,NativeDataFieldRef field){
    auto& target=record(object_ref);
    validate_key(*this,key);
    if(!field)type_error(u"Native data field is null");
    if(target.kind==ObjectKind::array||target.nativeRecordView)
        throw Exception("Unimplemented",u"Declared native fields require an ordinary object record");
    if(rawDescriptor(Value(object_ref),key)||nativeDataField(object_ref,key)||declaredNativeDataField(object_ref,key))
        type_error(u"Native data field is already declared or present");
    target.declaredNativeFields.emplace_back(key,std::move(field));
}

void Context::setImpl(Value object_value,const Key& key,Value value,NativeDataFieldRef fieldForAbsent) {
    validate_key(*this, key);
    validate_value(*this, object_value);
    validate_value(*this, value);
    const auto* object_ref = std::get_if<ObjectRef>(&object_value.data);
    if (!object_ref) type_error(u"Cannot assign to a primitive value");
    auto& target = record(*object_ref);

    if(target.kind==ObjectKind::array&&target.nativeIndexedView){
        if(key_is(key,kLength)){
            auto own=rawDescriptor(object_value,key);
            if(!own)type_error(u"Array length descriptor is missing");
            if(own->accessor)type_error(u"Array length cannot be an accessor");
            if(!own->writable)type_error(u"Cannot assign to read only property");
            const double requested=primitive_number(value);
            if(!std::isfinite(requested)||requested<0||requested>4294967295.0||std::trunc(requested)!=requested)range_error();
            own->value=Value(requested==0?0.0:requested);
            target.nativeIndexedView->defineLength(std::move(*own));
            return;
        }
        if(const auto* text=std::get_if<std::u16string>(&key.data))if(const auto index=array_index(*text)){
            auto own=target.nativeIndexedView->indexDescriptor(*index);
            if(own){
                if(own->accessor){
                    if(!own->setter)type_error(u"Property has no setter");
                    const std::array<Value,1> arguments{std::move(value)};
                    (void)call(*own->setter,object_value,arguments);
                    return;
                }
                if(!own->writable)type_error(u"Cannot assign to read only property");
                own->value=std::move(value);
                target.nativeIndexedView->defineIndex(*index,std::move(*own));
                return;
            }
        }
    }

    if(target.nativeRecordView){
        auto own=target.nativeRecordView->ownDescriptor(key);
        if(own){
            if(own->accessor){
                if(!own->setter)type_error(u"Property has no setter");
                const std::array<Value,1> arguments{std::move(value)};
                (void)call(*own->setter,object_value,arguments);
                return;
            }
            if(!own->writable)type_error(u"Cannot assign to read only property");
            own->value=std::move(value);
            target.nativeRecordView->defineOwn(key,std::move(*own));
            return;
        }
    }
    auto find_own = [&](Record& r) -> Descriptor* {
        for (auto& [own_key, property] : r.properties) if (own_key == key) return &property;
        return nullptr;
    };
    if (auto* own = find_own(target)) {
        if (own->accessor) {
            if (!own->setter) type_error(u"Property has no setter");
            const std::array<Value, 1> args{std::move(value)};
            (void)call(*own->setter, object_value, args);
            return;
        }
        if (!own->writable) type_error(u"Cannot assign to read only property");
        if(auto field=nativeDataField(*object_ref,key)){
            field->write(std::move(value));
            return;
        }
        if(fieldForAbsent&&target.kind!=ObjectKind::array&&!target.nativeRecordView){
            // Preserve a pre-existing writable data property's descriptor and
            // order while transferring its value authority to the real owner.
            fieldForAbsent->write(std::move(value));
            auto& current=record(*object_ref);
            const auto currentOwn=std::find_if(current.properties.begin(),current.properties.end(),
                [&](const auto& entry){return entry.first==key;});
            if(currentOwn==current.properties.end()||currentOwn->second.accessor||
               nativeDataField(*object_ref,key))return;
            currentOwn->second.value=Undefined{};
            std::erase_if(current.declaredNativeFields,
                [&](const auto& entry){return entry.first==key;});
            current.nativeFields.emplace_back(key,std::move(fieldForAbsent));
            return;
        }
        if (target.kind == ObjectKind::array && key_is(key, kLength)) {
            const auto requested = primitive_number(value);
            if (!std::isfinite(requested) || requested < 0 || requested > 4294967295.0 || std::trunc(requested) != requested) range_error();
            const auto next_length = static_cast<std::uint32_t>(requested);
            const auto old_length = arrayLength(*object_ref);
            if (next_length < old_length) {
                std::vector<std::uint32_t> remove;
                for (const auto& [own_key, descriptor_value] : target.properties) {
                    (void)descriptor_value;
                    if (const auto* name = std::get_if<std::u16string>(&own_key.data)) {
                        if (const auto index = array_index(*name); index && *index >= next_length) remove.push_back(*index);
                    }
                }
                std::sort(remove.rbegin(), remove.rend());
                for (const auto index : remove) {
                    const Key index_key(index_string(index));
                    const auto property = std::find_if(target.properties.begin(), target.properties.end(), [&](const auto& pair) { return pair.first == index_key; });
                    if (property != target.properties.end() && !property->second.configurable) {
                        const auto length = std::find_if(target.properties.begin(), target.properties.end(), [](const auto& pair) { return key_is(pair.first, kLength); });
                        if (length != target.properties.end()) length->second.value = Value(static_cast<double>(index + 1));
                        type_error(u"Cannot delete non-configurable array element");
                    }
                    if (property != target.properties.end()) target.properties.erase(property);
                }
            }
            const auto length = std::find_if(target.properties.begin(), target.properties.end(), [](const auto& pair) { return key_is(pair.first, kLength); });
            if (length != target.properties.end()) length->second.value = Value(static_cast<double>(next_length));
            return;
        }
        own->value = std::move(value);
        return;
    }

    // Assignment resolves inherited setters and non-writable data before it
    // creates an own property or activates a dormant native data cell.
    auto cursor=prototype(*object_ref);
    while(cursor){
        (void)record(*cursor);
        const auto inherited=rawDescriptor(Value(*cursor),key);
        if(inherited){
            if(inherited->accessor){
                if(!inherited->setter)type_error(u"Inherited property has no setter");
                const std::array<Value,1> arguments{std::move(value)};
                (void)call(*inherited->setter,object_value,arguments);
                return;
            }
            if(!inherited->writable)type_error(u"Cannot assign to inherited read only property");
            break;
        }
        cursor=prototype(*cursor);
    }

    // A native indexed array keeps numeric properties and its length in its
    // provider. Its absent index must not become a Record shadow property.
    if(target.kind==ObjectKind::array&&target.nativeIndexedView){
        if(const auto* name=std::get_if<std::u16string>(&key.data))if(const auto index=array_index(*name)){
            const auto length=target.nativeIndexedView->length();
            const auto lengthDescriptor=target.nativeIndexedView->lengthDescriptor();
            if(*index>=length&&!lengthDescriptor.writable)type_error(u"Array length is not writable");
            target.nativeIndexedView->defineIndex(*index,Descriptor::data(std::move(value)));
            return;
        }
    }
    if(target.kind==ObjectKind::array&&key_is(key,kLength))
        type_error(u"Cannot redefine array length");
    if(target.nativeRecordView){
        target.nativeRecordView->defineOwn(key,Descriptor::data(std::move(value)));
        return;
    }

    if(!fieldForAbsent)fieldForAbsent=declaredNativeDataField(*object_ref,key);
    if(fieldForAbsent){
        if(target.kind==ObjectKind::array)
            throw Exception("Unimplemented",u"Native data fields require an ordinary object record");
        fieldForAbsent->write(std::move(value));

        // A native write callback may reenter this Context. Preserve any
        // property it created and avoid a second descriptor for the same key.
        auto& current=record(*object_ref);
        const auto reentrantOwn=std::find_if(current.properties.begin(),current.properties.end(),
            [&](const auto& entry){return entry.first==key;});
        if(reentrantOwn!=current.properties.end()){
            std::erase_if(current.declaredNativeFields,[&](const auto& entry){return entry.first==key;});
            return;
        }
        const auto activeField=nativeDataField(*object_ref,key);
        if(!activeField){
            std::erase_if(current.declaredNativeFields,[&](const auto& entry){return entry.first==key;});
            current.nativeFields.emplace_back(key,std::move(fieldForAbsent));
        }
        current.properties.emplace_back(key,Descriptor::data(Undefined{}));
        return;
    }
    define(*object_ref,key,Descriptor::data(std::move(value)));
}

void Context::define(ObjectRef object_ref, const Key& key, Descriptor new_descriptor) {
    auto& target = record(object_ref);
    validate_key(*this, key);
    validate_value(*this, new_descriptor.value);
    if (new_descriptor.getter) (void)record(*new_descriptor.getter);
    if (new_descriptor.setter) (void)record(*new_descriptor.setter);
    if ((new_descriptor.getter && kind(*new_descriptor.getter) != ObjectKind::function) ||
        (new_descriptor.setter && kind(*new_descriptor.setter) != ObjectKind::function)) type_error(u"Getter or setter is not callable");

    auto find_own = [&]() -> std::vector<std::pair<Key, Descriptor>>::iterator {
        return std::find_if(target.properties.begin(), target.properties.end(), [&](const auto& pair) { return pair.first == key; });
    };

    const bool array_length = target.kind == ObjectKind::array && key_is(key, kLength);
    if(target.kind==ObjectKind::array&&target.nativeIndexedView){
        if(array_length){
            if(new_descriptor.accessor)type_error(u"Array length cannot be an accessor");
            const double number=primitive_number(new_descriptor.value);
            if(!std::isfinite(number)||number<0||number>4294967295.0||std::trunc(number)!=number)range_error();
            new_descriptor.value=Value(number==0?0.0:number);
            const auto old=target.nativeIndexedView->lengthDescriptor();
            if(!old.configurable&&new_descriptor.configurable)type_error(u"Cannot redefine non-configurable property");
            if(!old.enumerable&&new_descriptor.enumerable)type_error(u"Cannot redefine non-configurable property");
            if(!old.writable&&new_descriptor.writable)type_error(u"Cannot redefine non-writable property");
            if(!old.writable&&!sameValue(old.value,new_descriptor.value))type_error(u"Cannot redefine non-writable property");
            target.nativeIndexedView->defineLength(std::move(new_descriptor));
            return;
        }
        if(const auto* text=std::get_if<std::u16string>(&key.data))if(const auto index=array_index(*text)){
            const auto length=target.nativeIndexedView->length();
            const auto lengthDescriptor=target.nativeIndexedView->lengthDescriptor();
            if(*index>=length&&!lengthDescriptor.writable)type_error(u"Array length is not writable");
            const auto old=target.nativeIndexedView->indexDescriptor(*index);
            if(old&&!old->configurable){
                if(new_descriptor.configurable||new_descriptor.enumerable!=old->enumerable||
                   !data_descriptor_compatible(*old,new_descriptor))type_error(u"Cannot redefine non-configurable property");
                if(!old->accessor&&!old->writable&&
                   (new_descriptor.writable||!sameValue(old->value,new_descriptor.value)))
                    type_error(u"Cannot redefine non-writable property");
            }
            target.nativeIndexedView->defineIndex(*index,std::move(new_descriptor));
            return;
        }
    }
    if(target.nativeRecordView){
        const auto old=target.nativeRecordView->ownDescriptor(key);
        if(old&&!old->configurable){
            if(new_descriptor.enumerable!=old->enumerable||new_descriptor.configurable)
                type_error(u"Cannot redefine non-configurable property");
            if(old->accessor!=new_descriptor.accessor)
                type_error(u"Cannot redefine non-configurable property");
            if(!old->accessor&&!old->writable&&(new_descriptor.writable||!sameValue(old->value,new_descriptor.value)))
                type_error(u"Cannot redefine non-writable property");
        }
        target.nativeRecordView->defineOwn(key,std::move(new_descriptor));
        return;
    }
    if (array_length) {
        if (new_descriptor.accessor) type_error(u"Array length cannot be an accessor");
        const auto number = primitive_number(new_descriptor.value);
        if (!std::isfinite(number) || number < 0 || number > 4294967295.0 || std::trunc(number) != number) range_error();
        new_descriptor.value = Value(number == 0 ? 0.0 : number);
        auto old = find_own();
        if (old == target.properties.end()) type_error(u"Array length descriptor is missing");
        if (!old->second.configurable && new_descriptor.configurable) type_error(u"Cannot redefine non-configurable property");
        if (!old->second.enumerable && new_descriptor.enumerable) type_error(u"Cannot redefine non-configurable property");
        if (!old->second.writable && new_descriptor.writable) type_error(u"Cannot redefine non-writable property");
        if (!old->second.writable && !sameValue(old->second.value, new_descriptor.value)) type_error(u"Cannot redefine non-writable property");
        const auto requested = static_cast<std::uint32_t>(number);
        const auto previous = arrayLength(object_ref);
        if (requested < previous) {
            std::vector<std::uint32_t> indices;
            for (const auto& [own_key, descriptor_value] : target.properties) {
                (void)descriptor_value;
                if (const auto* name = std::get_if<std::u16string>(&own_key.data)) {
                    if (const auto index = array_index(*name); index && *index >= requested) indices.push_back(*index);
                }
            }
            std::sort(indices.rbegin(), indices.rend());
            for (const auto index : indices) {
                const Key index_key(index_string(index));
                const auto property = std::find_if(target.properties.begin(), target.properties.end(), [&](const auto& pair) { return pair.first == index_key; });
                if (property != target.properties.end() && !property->second.configurable) {
                    const auto length = std::find_if(target.properties.begin(), target.properties.end(), [](const auto& pair) { return key_is(pair.first, kLength); });
                    if (length != target.properties.end()) {
                        length->second.value = Value(static_cast<double>(index + 1));
                        if (!new_descriptor.writable) length->second.writable = false;
                    }
                    type_error(u"Cannot delete non-configurable array element");
                }
                if (property != target.properties.end()) target.properties.erase(property);
            }
        }
        old = find_own();
        old->second = std::move(new_descriptor);
        return;
    }

    bool grows_array_length = false;
    if (target.kind == ObjectKind::array) {
        if (const auto* name = std::get_if<std::u16string>(&key.data)) {
            if (const auto index = array_index(*name)) {
                const auto length = std::find_if(target.properties.begin(), target.properties.end(), [](const auto& pair) { return key_is(pair.first, kLength); });
                if (length == target.properties.end()) type_error(u"Array length descriptor is missing");
                if (*index >= arrayLength(object_ref) && !length->second.writable) type_error(u"Array length is not writable");
                grows_array_length = *index >= arrayLength(object_ref);
            }
        }
    }

    auto existing = find_own();
    if (existing == target.properties.end()) {
        auto field=nativeDataField(object_ref,key);
        const bool declared=!field&&static_cast<bool>(declaredNativeDataField(object_ref,key));
        if(declared)field=declaredNativeDataField(object_ref,key);
        if(field){
            if(new_descriptor.accessor){
                // A configurable accessor replaces the Source data property.
                // Remove its native cell first, then detach that binding so
                // future reads and writes use only the accessor descriptor.
                if(!declared)field->remove();
                auto& current=record(object_ref);
                std::erase_if(declared?current.declaredNativeFields:current.nativeFields,
                    [&](const auto& entry){return entry.first==key;});
            }else{
                field->write(std::move(new_descriptor.value));
                if(declared){
                    auto& current=record(object_ref);
                    std::erase_if(current.declaredNativeFields,[&](const auto& entry){return entry.first==key;});
                    current.nativeFields.emplace_back(key,field);
                }
                new_descriptor.value=Undefined{};
            }
        }
        target.properties.emplace_back(key, std::move(new_descriptor));
        if (grows_array_length) {
            const auto length = std::find_if(target.properties.begin(), target.properties.end(), [](const auto& pair) { return key_is(pair.first, kLength); });
            length->second.value = Value(static_cast<double>(*array_index(std::get<std::u16string>(key.data)) + 1));
        }
        return;
    }
    const auto old = existing->second;
    if (!old.configurable) {
        if (new_descriptor.configurable || new_descriptor.enumerable != old.enumerable ||
            !data_descriptor_compatible(old, new_descriptor)) type_error(u"Cannot redefine non-configurable property");
        const auto field=nativeDataField(object_ref,key);
        const Value oldValue=field&&!old.accessor&&!old.writable?field->read():old.value;
        if (!old.accessor && !old.writable && (new_descriptor.writable || !sameValue(oldValue, new_descriptor.value))) {
            type_error(u"Cannot redefine non-writable property");
        }
    }
    if(auto field=nativeDataField(object_ref,key)){
        if(new_descriptor.accessor){
            // Descriptor compatibility was checked above. Preserve the
            // native owner's removal semantics before changing authority.
            field->remove();
            auto& current=record(object_ref);
            std::erase_if(current.nativeFields,[&](const auto& entry){return entry.first==key;});
            existing=std::find_if(current.properties.begin(),current.properties.end(),
                [&](const auto& pair){return pair.first==key;});
            if(existing==current.properties.end())type_error(u"Native field changed its own descriptor during definition");
        }else{
            if(old.configurable||old.writable)field->write(std::move(new_descriptor.value));
            // The ordinary descriptor stores metadata, never the owner's value.
            new_descriptor.value=Undefined{};
            existing=find_own();
            if(existing==target.properties.end())type_error(u"Native field changed its own descriptor during definition");
        }
    }
    existing->second = std::move(new_descriptor);
    if (grows_array_length) {
        const auto length = std::find_if(target.properties.begin(), target.properties.end(), [](const auto& pair) { return key_is(pair.first, kLength); });
        length->second.value = Value(static_cast<double>(*array_index(std::get<std::u16string>(key.data)) + 1));
    }
}

void Context::bindData(ObjectRef object,const Key& key,NativeDataFieldRef field,
                       bool writable,bool enumerable,bool configurable){
    auto& target=record(object);
    validate_key(*this,key);
    if(!field)type_error(u"Native data field is null");
    if(target.kind==ObjectKind::array)
        throw Exception("Unimplemented",u"Native array storage requires a complete array record binding");
    const auto identity=std::to_string(object.context)+":"+std::to_string(object.index);
    const auto describeCollision=[&](std::u16string_view reason){
        std::u16string message=u"Native data field '"+keyString(key)+u"' on Context object ";
        message.append(identity.begin(),identity.end());
        message+=reason;
        return message;
    };
    if(rawDescriptor(Value(object),key))
        type_error(describeCollision(u" already has an own property"));
    if(nativeDataField(object,key))
        type_error(describeCollision(u" already has its actual owner binding"));
    target.properties.emplace_back(key,Descriptor::data(Undefined{},writable,enumerable,configurable));
    try{target.nativeFields.emplace_back(key,std::move(field));}
    catch(...){target.properties.pop_back();throw;}
}

void Context::erase(ObjectRef object_ref, const Key& key) {
    auto& target = record(object_ref);
    validate_key(*this, key);
    if(target.kind==ObjectKind::array&&target.nativeIndexedView){
        if(key_is(key,kLength))type_error(u"Cannot delete array length");
        if(const auto* text=std::get_if<std::u16string>(&key.data))if(const auto index=array_index(*text)){
            const auto own=target.nativeIndexedView->indexDescriptor(*index);
            if(!own)return;
            if(!own->configurable)type_error(u"Cannot delete non-configurable array element");
            target.nativeIndexedView->deleteIndex(*index);
            return;
        }
    }
    if(target.nativeRecordView){
        const auto old=target.nativeRecordView->ownDescriptor(key);
        if(!old)return;
        if(!old->configurable)type_error(u"Cannot delete non-configurable property");
        target.nativeRecordView->deleteOwn(key);
        return;
    }
    const auto found = std::find_if(target.properties.begin(), target.properties.end(), [&](const auto& pair) { return pair.first == key; });
    if (found == target.properties.end()) return;
    if (!found->second.configurable) type_error(u"Cannot delete non-configurable property");
    if (target.kind == ObjectKind::array && key_is(key, kLength)) type_error(u"Cannot delete array length");
    if(auto field=nativeDataField(object_ref,key)){
        field->remove();
        // Keep the actual class field binding when its Source own property
        // disappears. A later data-property definition uses this same cell;
        // it must not become a detached Context value beside the C++ field.
        // The owner operation can reenter; locate the property again.
        const auto current=std::find_if(target.properties.begin(),target.properties.end(),[&](const auto& pair){return pair.first==key;});
        if(current!=target.properties.end())target.properties.erase(current);
        const auto binding=std::find_if(target.nativeFields.begin(),target.nativeFields.end(),
            [&](const auto& entry){return entry.first==key;});
        if(binding!=target.nativeFields.end()){
            auto retained=std::move(binding->second);
            target.nativeFields.erase(binding);
            target.declaredNativeFields.emplace_back(key,std::move(retained));
        }
        return;
    }
    target.properties.erase(found);
}

Value Context::call(ObjectRef function_ref, Value receiver, std::span<const Value> arguments) {
    auto& fn = record(function_ref);
    validate_value(*this, receiver);
    for (const auto& argument : arguments) validate_value(*this, argument);
    if (fn.kind != ObjectKind::function || !fn.callable) type_error(u"Value is not callable in this native context");
    return fn.callable(*this, std::move(receiver), arguments);
}

ObjectRef Context::construct(ObjectRef constructor_ref, std::span<const Value> arguments) {
    auto& constructor_record = record(constructor_ref);
    for (const auto& argument : arguments) validate_value(*this, argument);
    if (constructor_record.kind != ObjectKind::function)
        type_error(u"Value is not a constructor");
    if (!constructor_record.construct)
        throw Exception("Unimplemented", u"Constructor has no registered native allocation body");
    // Callback can allocate records or reenter the Context. Keep the callback
    // alive independently of any bindings it may change during construction.
    const auto body = constructor_record.construct;
    const auto result = body(*this, arguments);
    (void)record(result);
    return result;
}

bool Context::isConstructor(ObjectRef constructor_ref) const {
    const auto& value=record(constructor_ref);
    return value.kind==ObjectKind::function&&static_cast<bool>(value.construct);
}

ObjectRef Context::descriptorObject(const Descriptor& descriptor_value) {
    const auto result = object();
    if (descriptor_value.accessor) {
        define(result, Key(u"get"), Descriptor::data(descriptor_value.getter ? Value(*descriptor_value.getter) : Value(Undefined{})));
        define(result, Key(u"set"), Descriptor::data(descriptor_value.setter ? Value(*descriptor_value.setter) : Value(Undefined{})));
    } else {
        define(result, Key(u"value"), Descriptor::data(descriptor_value.value));
        define(result, Key(u"writable"), Descriptor::data(Value(descriptor_value.writable)));
    }
    define(result, Key(u"enumerable"), Descriptor::data(Value(descriptor_value.enumerable)));
    define(result, Key(u"configurable"), Descriptor::data(Value(descriptor_value.configurable)));
    return result;
}

Descriptor Context::descriptorFromObject(Value descriptor_value) {
    if (is_nullish(descriptor_value) || !std::holds_alternative<ObjectRef>(descriptor_value.data)) type_error(u"Property description must be an object");
    auto field = [&](std::u16string_view name) -> std::optional<Value> {
        const Key key{std::u16string(name)};
        if (!has(descriptor_value, key)) return std::nullopt;
        return get(descriptor_value, key);
    };
    const auto enumerable = field(u"enumerable");
    const auto configurable = field(u"configurable");
    const auto value = field(u"value");
    const auto writable = field(u"writable");
    const auto get_value = field(u"get");
    const auto set_value = field(u"set");
    const bool accessor = get_value.has_value() || set_value.has_value();
    if (accessor && (value || writable)) type_error(u"Invalid property descriptor");
    auto as_function = [&](const std::optional<Value>& item) -> std::optional<ObjectRef> {
        if (!item || std::holds_alternative<Undefined>(item->data)) return std::nullopt;
        const auto* ref = std::get_if<ObjectRef>(&item->data);
        if (!ref || kind(*ref) != ObjectKind::function) type_error(u"Getter or setter is not callable");
        return *ref;
    };
    const bool enumerable_flag = enumerable && truthy(*enumerable);
    const bool configurable_flag = configurable && truthy(*configurable);
    if (accessor) return Descriptor::access(as_function(get_value), as_function(set_value), enumerable_flag, configurable_flag);
    const Value data_value = value.value_or(Value(Undefined{}));
    const bool writable_flag = writable && truthy(*writable);
    return Descriptor::data(data_value, writable_flag, enumerable_flag, configurable_flag);
}

} // namespace folia::math::options
