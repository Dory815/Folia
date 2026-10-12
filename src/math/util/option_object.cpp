module folia.math.util.option_types;
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

[[noreturn]] void type_error(std::u16string message = u"Invalid operation") {
    throw Exception("TypeError", std::move(message));
}

[[noreturn]] void range_error(std::u16string message = u"Invalid array length") {
    throw Exception("RangeError", std::move(message));
}

bool is_nullish(const Value& value) {
    return std::holds_alternative<Undefined>(value.data) || std::holds_alternative<Null>(value.data);
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

Record& Context::record(ObjectRef object) const {
    if (object.context != state_->identity || object.index >= state_->objects.size() || !state_->objects[object.index]) {
        type_error(u"Object belongs to a different or invalid Context");
    }
    return *state_->objects[object.index];
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

    i.objectPrototype = allocate(ObjectKind::object, std::nullopt);
    i.functionPrototype = allocate(ObjectKind::function, i.objectPrototype);
    i.arrayPrototype = allocate(ObjectKind::array, i.objectPrototype);
    i.expandablePrototype = allocate(ObjectKind::object, i.objectPrototype);
    i.numberPrototype = allocate(ObjectKind::object, i.objectPrototype);
    i.booleanPrototype = allocate(ObjectKind::object, i.objectPrototype);
    i.stringPrototype = allocate(ObjectKind::object, i.objectPrototype);
    i.symbolPrototype = allocate(ObjectKind::object, i.objectPrototype);

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

    // Object.prototype method identities/order are represented; their bodies
    // are outside the Options source closure, so call() rejects them.
    for (const auto name : {u"__defineGetter__", u"__defineSetter__", u"hasOwnProperty",
                            u"__lookupGetter__", u"__lookupSetter__", u"isPrototypeOf",
                            u"propertyIsEnumerable", u"toString", u"valueOf"}) {
        const auto method = function(std::u16string(name));
        define(i.objectPrototype, Key(std::u16string(name)), Descriptor::data(Value(method), true, false, true));
    }

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

    initializeArrayIntrinsics();
}

Context::~Context() = default;

const Intrinsics& Context::intrinsics() const { return state_->intrinsics; }

ObjectRef Context::object() { return allocate(ObjectKind::object, state_->intrinsics.objectPrototype); }
ObjectRef Context::nullObject() { return allocate(ObjectKind::object, std::nullopt); }
ObjectRef Context::objectWithPrototype(ObjectRef prototype) {
    (void)record(prototype);
    return allocate(ObjectKind::object, prototype);
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

double Context::toNumber(const Value& value) const {
    return primitive_number(value);
}

std::any& Context::payload(ObjectRef object_ref) { return record(object_ref).payload; }
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

std::u16string Context::keyString(const Key& key) const {
    if (const auto* text = std::get_if<std::u16string>(&key.data)) return *text;
    return symbolString(std::get<SymbolRef>(key.data));
}

std::optional<ObjectRef> Context::prototype(ObjectRef object_ref) const { return record(object_ref).prototype; }

void Context::setPrototype(ObjectRef object_ref, std::optional<ObjectRef> prototype_ref) {
    auto& target = record(object_ref);
    if (prototype_ref) (void)record(*prototype_ref);
    auto cursor = prototype_ref;
    while (cursor) {
        if (*cursor == object_ref) type_error(u"Cyclic prototype value");
        cursor = record(*cursor).prototype;
    }
    target.prototype = prototype_ref;
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
    for (const auto& [own_key, own_descriptor] : r.properties) if (own_key == key) return own_descriptor;
    return std::nullopt;
}

NativeDataFieldRef Context::nativeDataField(ObjectRef object,const Key& key)const{
    for(const auto& [name,field]:record(object).nativeFields)if(name==key)return field;
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
    auto cursor = object_ref ? record(*object_ref).prototype : std::optional<ObjectRef>{};
    if (!object_ref) {
        if (std::holds_alternative<std::u16string>(object_value.data)) cursor = state_->intrinsics.stringPrototype;
        else if (std::holds_alternative<double>(object_value.data)) cursor = state_->intrinsics.numberPrototype;
        else if (std::holds_alternative<bool>(object_value.data)) cursor = state_->intrinsics.booleanPrototype;
        else if (std::holds_alternative<SymbolRef>(object_value.data)) cursor = state_->intrinsics.symbolPrototype;
    }
    while (cursor) {
        if (rawDescriptor(Value(*cursor), key)) return true;
        cursor = record(*cursor).prototype;
    }
    return false;
}

Value Context::get(Value object_value, const Key& key) {
    validate_key(*this, key);
    validate_value(*this, object_value);
    if (is_nullish(object_value)) type_error(u"Cannot read properties of null or undefined");
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
            const auto proto = record(*object_ref).prototype;
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
    validate_key(*this, key);
    validate_value(*this, object_value);
    validate_value(*this, value);
    const auto* object_ref = std::get_if<ObjectRef>(&object_value.data);
    if (!object_ref) type_error(u"Cannot assign to a primitive value");
    auto& target = record(*object_ref);

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

    auto cursor = target.prototype;
    while (cursor) {
        auto& proto = record(*cursor);
        if (auto* inherited = find_own(proto)) {
            if (inherited->accessor) {
                if (!inherited->setter) type_error(u"Inherited property has no setter");
                const std::array<Value, 1> args{std::move(value)};
                (void)call(*inherited->setter, object_value, args);
                return;
            }
            if (!inherited->writable) type_error(u"Cannot assign to inherited read only property");
            break;
        }
        cursor = proto.prototype;
    }

    if (target.kind == ObjectKind::array && key_is(key, kLength)) type_error(u"Cannot redefine array length");
    define(*object_ref, key, Descriptor::data(std::move(value)));
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
        if(auto field=nativeDataField(object_ref,key)){
            if(new_descriptor.accessor)
                throw Exception("Unimplemented",u"Replacing a native data field with an accessor requires its native owner operation");
            field->write(std::move(new_descriptor.value));
            new_descriptor.value=Undefined{};
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
        if(new_descriptor.accessor)
            throw Exception("Unimplemented",u"Replacing a native data field with an accessor requires its native owner operation");
        if(old.configurable||old.writable)field->write(std::move(new_descriptor.value));
        // The ordinary descriptor stores metadata, never the owner's value.
        new_descriptor.value=Undefined{};
        existing=find_own();
        if(existing==target.properties.end())type_error(u"Native field changed its own descriptor during definition");
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
    if(rawDescriptor(Value(object),key))type_error(u"Native data field already has an own property");
    if(nativeDataField(object,key))type_error(u"Native data field already has its actual owner binding");
    target.properties.emplace_back(key,Descriptor::data(Undefined{},writable,enumerable,configurable));
    try{target.nativeFields.emplace_back(key,std::move(field));}
    catch(...){target.properties.pop_back();throw;}
}

void Context::erase(ObjectRef object_ref, const Key& key) {
    auto& target = record(object_ref);
    validate_key(*this, key);
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
