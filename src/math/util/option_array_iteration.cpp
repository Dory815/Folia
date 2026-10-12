// Native ordinary-array forEach primitive needed by actual FindTeX options.
module folia.math.util.option_types;
import std;
import folia.math.util.js_number;
namespace folia::math::options {
ObjectRef Context::arrayMap(ObjectRef object,Value callback,Value thisArgument){
    if(kind(object)!=ObjectKind::array)
        throw Exception("TypeError",u"Array method called on incompatible receiver");

    const std::uint32_t length=arrayLength(object);
    const auto* callbackObject=std::get_if<ObjectRef>(&callback.data);
    if(!callbackObject||kind(*callbackObject)!=ObjectKind::function)
        throw Exception("TypeError",u"Array map callback must be callable");

    // ArraySpeciesCreate first observes the receiver's current constructor,
    // even for empty arrays. Only the intrinsic Array and undefined cases are
    // part of this host profile; custom species need their own constructor host.
    const Value constructorValue=get(Value(object),Key(u"constructor"));
    ObjectRef intrinsicArray=intrinsics().arrayConstructor;
    if(!std::holds_alternative<Undefined>(constructorValue.data)){
        if(const auto* constructorObject=std::get_if<ObjectRef>(&constructorValue.data)){
            if(*constructorObject!=intrinsicArray)
                throw Exception("Unimplemented",u"Custom Array species requires a native constructor binding");
        }else{
            throw Exception("TypeError",u"Array constructor is not a constructor");
        }
    }

    const ObjectRef result=array(length);
    const Value receiver(object);
    for(std::uint64_t index=0;index<length;++index){
        const auto ascii=std::to_string(index);
        const Key key(std::u16string(ascii.begin(),ascii.end()));
        if(!has(receiver,key))continue;
        const Value value=get(receiver,key);
        const std::array<Value,3> arguments{
            value,Value(static_cast<double>(index)),receiver
        };
        const Value mapped=call(*callbackObject,thisArgument,arguments);
        define(result,key,Descriptor::data(mapped));
    }
    return result;
}
void Context::arrayForEach(ObjectRef object,Value callback,Value thisArgument){
    // This is Array.prototype.forEach's ordinary-array profile: length is
    // captured once, while HasProperty/Get and callback effects remain live.
    const ObjectRef array = object;
    if (kind(array) != ObjectKind::array) {
        throw Exception("TypeError",u"Array method called on incompatible receiver");
    }
    const std::uint32_t length = arrayLength(array);

    const auto* callback_object = std::get_if<ObjectRef>(&callback.data);
    if (!callback_object || kind(*callback_object) != ObjectKind::function) {
        throw Exception("TypeError",u"Array forEach callback must be callable");
    }

    const Value receiver(array);
    for (std::uint64_t index = 0; index < length; ++index) {
        const auto ascii = std::to_string(index);
        const Key key(std::u16string(ascii.begin(),ascii.end()));
        if (!has(receiver,key)) continue;

        const Value value = get(receiver,key);
        const std::array<Value,3> arguments{
            value, Value(static_cast<double>(index)), receiver
        };
        (void)call(*callback_object,thisArgument,arguments);
    }
}

namespace {
std::u16string ordinary_index(std::uint64_t index) {
    const auto ascii = std::to_string(index);
    return std::u16string(ascii.begin(), ascii.end());
}
Key ordinary_key(std::uint64_t index) { return Key(ordinary_index(index)); }
void ordinary_array_receiver(Context& context, ObjectRef object) {
    if (context.kind(object) != ObjectKind::array)
        throw Exception("TypeError",u"Array method called on incompatible receiver");
}
ObjectRef ordinary_callback(Context& context, Value callback) {
    const auto* object = std::get_if<ObjectRef>(&callback.data);
    if (!object || context.kind(*object) != ObjectKind::function)
        throw Exception("TypeError",u"Array callback must be callable");
    return *object;
}
bool same_value_zero(const Value& left, const Value& right) {
    if (left.data.index() != right.data.index()) return false;
    if (const auto* a = std::get_if<double>(&left.data)) {
        const auto b = std::get<double>(right.data);
        return (*a == b) || (std::isnan(*a) && std::isnan(b));
    }
    return left == right;
}
[[noreturn]] void array_unimplemented(std::u16string message) {
    throw Exception("Unimplemented", std::move(message));
}
}

ObjectRef Context::arrayReverse(ObjectRef object) {
    ordinary_array_receiver(*this, object);
    const std::uint64_t length = arrayLength(object);
    const Value receiver(object);
    for (std::uint64_t lower = 0; lower < length / 2; ++lower) {
        const std::uint64_t upper = length - lower - 1;
        const Key lower_key = ordinary_key(lower);
        const Key upper_key = ordinary_key(upper);
        const bool lower_exists = has(receiver, lower_key);
        Value lower_value = Undefined{};
        if (lower_exists) lower_value = get(receiver, lower_key);
        const bool upper_exists = has(receiver, upper_key);
        Value upper_value = Undefined{};
        if (upper_exists) upper_value = get(receiver, upper_key);
        if (lower_exists && upper_exists) {
            set(receiver, lower_key, upper_value);
            set(receiver, upper_key, lower_value);
        } else if (!lower_exists && upper_exists) {
            set(receiver, lower_key, upper_value);
            erase(object, upper_key);
        } else if (lower_exists && !upper_exists) {
            erase(object, lower_key);
            set(receiver, upper_key, lower_value);
        }
    }
    return object;
}

std::uint32_t Context::arrayPush(ObjectRef object, std::span<const Value> values) {
    ordinary_array_receiver(*this, object);
    const std::vector<Value> stable_values(values.begin(), values.end());
    std::uint64_t length = arrayLength(object);
    const Value receiver(object);
    for (const auto& value : stable_values) {
        set(receiver, Key(ordinary_index(length)), value);
        ++length;
    }
    set(receiver, Key(u"length"), Value(static_cast<double>(length)));
    return arrayLength(object);
}

Value Context::arrayPop(ObjectRef object) {
    ordinary_array_receiver(*this, object);
    const std::uint64_t length = arrayLength(object);
    if (length == 0) {
        set(Value(object), Key(u"length"), Value(0));
        return Undefined{};
    }
    const auto index = length - 1;
    const Key key = ordinary_key(index);
    const Value result = get(Value(object), key);
    erase(object, key);
    set(Value(object), Key(u"length"), Value(static_cast<double>(index)));
    return result;
}

Value Context::arrayFind(ObjectRef object, Value callback, Value thisArgument) {
    ordinary_array_receiver(*this, object);
    const std::uint64_t length = arrayLength(object);
    const ObjectRef function = ordinary_callback(*this, callback);
    const Value receiver(object);
    for (std::uint64_t index = 0; index < length; ++index) {
        const Value value = get(receiver, ordinary_key(index));
        const std::array<Value,3> arguments{value, Value(static_cast<double>(index)), receiver};
        if (truthy(call(function, thisArgument, arguments))) return value;
    }
    return Undefined{};
}

bool Context::arrayIncludes(ObjectRef object, Value search, Value fromIndex) {
    ordinary_array_receiver(*this, object);
    const std::uint64_t length = arrayLength(object);
    if (length == 0) return false;
    const double number = toNumber(fromIndex);
    double integer = std::isnan(number) || number == 0 ? 0 : std::trunc(number);
    std::uint64_t index = 0;
    if (integer == std::numeric_limits<double>::infinity()) return false;
    if (integer == -std::numeric_limits<double>::infinity()) integer = 0;
    if (integer >= 0) {
        if (integer >= static_cast<double>(length)) return false;
        index = static_cast<std::uint64_t>(integer);
    } else {
        const double candidate = static_cast<double>(length) + integer;
        index = candidate <= 0 ? 0 : static_cast<std::uint64_t>(candidate);
    }
    const Value receiver(object);
    for (; index < length; ++index) {
        if (same_value_zero(get(receiver, ordinary_key(index)), search)) return true;
    }
    return false;
}

ObjectRef Context::arraySlice(ObjectRef object, Value start, Value end) {
    ordinary_array_receiver(*this, object);
    const std::uint64_t length = arrayLength(object);
    auto relative = [&](const Value& value, bool is_end) -> std::uint64_t {
        if (std::holds_alternative<Undefined>(value.data)) return is_end ? length : 0;
        const double number = toNumber(value);
        if (number == -std::numeric_limits<double>::infinity()) return 0;
        if (number == std::numeric_limits<double>::infinity()) return length;
        const double integer = std::isnan(number) || number == 0 ? 0 : std::trunc(number);
        if (integer < 0) {
            const double adjusted = static_cast<double>(length) + integer;
            return adjusted <= 0 ? 0 : static_cast<std::uint64_t>(adjusted);
        }
        return integer >= static_cast<double>(length) ? length : static_cast<std::uint64_t>(integer);
    };
    const std::uint64_t first = relative(start, false);
    const std::uint64_t final = relative(end, true);
    const std::uint64_t count = final > first ? final - first : 0;
    const Value constructor = get(Value(object), Key(u"constructor"));
    Value species = Undefined{};
    if (std::holds_alternative<ObjectRef>(constructor.data)) {
        species = get(constructor, Key(intrinsics().symbolSpecies));
        if (std::holds_alternative<Null>(species.data)) species = Undefined{};
    } else if (!std::holds_alternative<Undefined>(constructor.data)) {
        throw Exception("TypeError",u"Array constructor is not a constructor");
    }
    if (!std::holds_alternative<Undefined>(species.data)) {
        const auto* species_function = std::get_if<ObjectRef>(&species.data);
        if (!species_function || kind(*species_function) != ObjectKind::function)
            throw Exception("TypeError",u"Array species is not a constructor");
        if (*species_function != intrinsics().arrayConstructor)
            array_unimplemented(u"Custom Array species requires a native constructor binding");
    }
    const ObjectRef result = array(static_cast<std::uint32_t>(count));
    for (std::uint64_t offset = 0; offset < count; ++offset) {
        const Key source = ordinary_key(first + offset);
        if (!has(Value(object), source)) continue;
        const Value value = get(Value(object), source);
        define(result, ordinary_key(offset), Descriptor::data(value));
    }
    return result;
}

ObjectRef Context::arrayFill(ObjectRef object, Value value, Value start, Value end) {
    ordinary_array_receiver(*this, object);
    const std::uint64_t length = arrayLength(object);
    auto relative = [&](const Value& input, bool is_end) -> std::uint64_t {
        if (std::holds_alternative<Undefined>(input.data)) return is_end ? length : 0;
        const double number = toNumber(input);
        if (number == -std::numeric_limits<double>::infinity()) return 0;
        if (number == std::numeric_limits<double>::infinity()) return length;
        const double integer = std::isnan(number) || number == 0 ? 0 : std::trunc(number);
        if (integer < 0) {
            const double adjusted = static_cast<double>(length) + integer;
            return adjusted <= 0 ? 0 : static_cast<std::uint64_t>(adjusted);
        }
        return integer >= static_cast<double>(length) ? length : static_cast<std::uint64_t>(integer);
    };
    const std::uint64_t first = relative(start, false);
    const std::uint64_t final = relative(end, true);
    for (std::uint64_t index = first; index < final; ++index)
        set(Value(object), ordinary_key(index), value);
    return object;
}

std::u16string Context::arrayJoin(ObjectRef object, Value separator) {
    ordinary_array_receiver(*this, object);
    const std::uint64_t length = arrayLength(object);
    auto primitive_string = [&](const Value& value) -> std::u16string {
        if (std::holds_alternative<Undefined>(value.data)) return u"undefined";
        if (std::holds_alternative<Null>(value.data)) return u"null";
        if (const auto* boolean = std::get_if<bool>(&value.data)) return *boolean ? u"true" : u"false";
        if (const auto* number = std::get_if<double>(&value.data)) {
            const std::string ascii = folia::math::js_number::toString(*number);
            return std::u16string(ascii.begin(), ascii.end());
        }
        if (const auto* text = std::get_if<std::u16string>(&value.data)) return *text;
        if (std::holds_alternative<SymbolRef>(value.data))
            throw Exception("TypeError",u"Cannot convert a Symbol value to a string");
        array_unimplemented(u"Object-to-primitive conversion has no native binding");
    };
    const std::u16string separator_string = std::holds_alternative<Undefined>(separator.data)
        ? std::u16string(u",") : primitive_string(separator);
    std::u16string result;
    for (std::uint64_t index = 0; index < length; ++index) {
        if (index != 0) result += separator_string;
        const Value value = get(Value(object), ordinary_key(index));
        if (std::holds_alternative<Undefined>(value.data) || std::holds_alternative<Null>(value.data)) continue;
        result += primitive_string(value);
    }
    return result;
}
}
