// Native Array operations used by MathJax's Options.ts profile.
// Copyright (c) 2017-2026 The MathJax Consortium.
// MathJax source is Apache-2.0 licensed; see ../LICENSE-MathJax.txt.
module folia.math.util.option_types;
import std;
#include "option_storage.hpp"

namespace {
using namespace folia::math::options;

struct ArrayIteratorState {
    Value source;
    std::uint64_t index=0;
    bool done=false;
    bool string_iterator=false;
};

[[noreturn]] void type_error(std::u16string message) {
    throw Exception("TypeError", std::move(message));
}

std::u16string index_key(std::uint32_t index) {
    const auto ascii = std::to_string(index);
    return std::u16string(ascii.begin(), ascii.end());
}

void require_array(Context& context, ObjectRef array) {
    if (context.kind(array) != ObjectKind::array) type_error(u"Array method called on incompatible receiver");
}

void validate_value_context(Context& context, const Value& value) {
    if (const auto* object = std::get_if<ObjectRef>(&value.data)) (void)context.kind(*object);
    if (const auto* symbol = std::get_if<SymbolRef>(&value.data)) (void)context.symbolString(*symbol);
}

ObjectRef require_function(Context& context, const Value& value) {
    const auto* function = std::get_if<ObjectRef>(&value.data);
    if (!function || context.kind(*function) != ObjectKind::function) type_error(u"Array filter callback must be callable");
    return *function;
}

double to_integer_or_infinity(const Value& value) {
    if (std::holds_alternative<Undefined>(value.data)) return 0;
    if (std::holds_alternative<Null>(value.data)) return 0;
    if (const auto* boolean = std::get_if<bool>(&value.data)) return *boolean ? 1 : 0;
    if (const auto* number = std::get_if<double>(&value.data)) {
        if (std::isnan(*number) || *number == 0) return 0;
        if (std::isinf(*number)) return *number;
        return std::trunc(*number);
    }
    // The source profile does not provide JS ToNumber for strings, symbols, or
    // objects. Ordinary Options.ts callers pass numeric fromIndex values only.
    type_error(u"Cannot convert array index to a number in this native context");
}

void register_method(Context& context, ObjectRef prototype, std::u16string name,
                     std::uint32_t arity, NativeCall callable = {}) {
    const auto method = context.function(name, std::move(callable));
    context.define(method, Key(u"length"), Descriptor::data(Value(static_cast<double>(arity)), false, false, true));
    context.define(prototype, Key(std::move(name)), Descriptor::data(Value(method), true, false, true));
}
} // namespace

namespace folia::math::options {

void Context::initializeArrayIntrinsics() {
    auto& prototype = record(state_->intrinsics.arrayPrototype);
    const auto has_length = std::find_if(prototype.properties.begin(), prototype.properties.end(),
        [](const auto& property) {
            const auto* name = std::get_if<std::u16string>(&property.first.data);
            return name && *name == u"length";
        });
    if (has_length == prototype.properties.end()) {
        prototype.properties.emplace(prototype.properties.begin(), Key(u"length"),
                                     Descriptor::data(Value(0), true, false, false));
    }

    auto& intrinsics=state_->intrinsics;
    intrinsics.iteratorPrototype=objectWithPrototype(intrinsics.objectPrototype);
    intrinsics.arrayIteratorPrototype=objectWithPrototype(intrinsics.iteratorPrototype);
    intrinsics.stringIteratorPrototype=objectWithPrototype(intrinsics.iteratorPrototype);
    intrinsics.mapIteratorPrototype=objectWithPrototype(intrinsics.iteratorPrototype);
    const auto iteratorSelf=function(u"[Symbol.iterator]",[](Context&,Value receiver,std::span<const Value>){return receiver;});
    define(intrinsics.iteratorPrototype,Key(intrinsics.symbolIterator),Descriptor::data(Value(iteratorSelf),true,false,true));
    const auto iteratorNext=function(u"next",[](Context& context,Value receiver,std::span<const Value>)->Value {
        const auto* iterator=std::get_if<ObjectRef>(&receiver.data);
        if(!iterator) type_error(u"Array Iterator next called on incompatible receiver");
        auto* state=std::any_cast<ArrayIteratorState>(&context.payload(*iterator));
        if(!state) type_error(u"Array Iterator state is missing");
        auto result=context.object();
        if(state->done) {
            context.define(result,Key(u"value"),Descriptor::data(Value(Undefined{})));
            context.define(result,Key(u"done"),Descriptor::data(Value(true)));
            return Value(result);
        }
        if(state->string_iterator) {
            const auto* text=std::get_if<std::u16string>(&state->source.data);
            if(!text) type_error(u"String iterator state is invalid");
            if(state->index>=text->size()) {
                state->done=true;
                context.define(result,Key(u"value"),Descriptor::data(Value(Undefined{})));
                context.define(result,Key(u"done"),Descriptor::data(Value(true)));
                return Value(result);
            }
            const std::size_t start=static_cast<std::size_t>(state->index);
            std::size_t width=1;
            const char16_t lead=(*text)[start];
            if(lead>=0xD800&&lead<=0xDBFF&&start+1<text->size()&&(*text)[start+1]>=0xDC00&&(*text)[start+1]<=0xDFFF) width=2;
            state->index+=width;
            context.define(result,Key(u"value"),Descriptor::data(Value(text->substr(start,width))));
            context.define(result,Key(u"done"),Descriptor::data(Value(false)));
            return Value(result);
        }
        const double rawLength=context.toNumber(context.get(state->source,Key(u"length")));
        const double maxLength=9007199254740991.0;
        const std::uint64_t length=std::isnan(rawLength)||rawLength<=0 ? 0 :
            rawLength>=maxLength ? static_cast<std::uint64_t>(maxLength) :
            static_cast<std::uint64_t>(std::floor(rawLength));
        if(state->index>=length) {
            state->done=true;
            context.define(result,Key(u"value"),Descriptor::data(Value(Undefined{})));
            context.define(result,Key(u"done"),Descriptor::data(Value(true)));
            return Value(result);
        }
        const std::uint64_t index=state->index++;
        const auto ascii=std::to_string(index);
        const Value value=context.get(state->source,Key(std::u16string(ascii.begin(),ascii.end())));
        context.define(result,Key(u"value"),Descriptor::data(value));
        context.define(result,Key(u"done"),Descriptor::data(Value(false)));
        return Value(result);
    });
    define(intrinsics.arrayIteratorPrototype,Key(u"next"),Descriptor::data(Value(iteratorNext),true,false,true));
    const auto values=function(u"values",[](Context& context,Value receiver,std::span<const Value>)->Value {
        if(std::holds_alternative<Undefined>(receiver.data)||std::holds_alternative<Null>(receiver.data))
            type_error(u"Array.prototype.values called on null or undefined");
        auto iterator=context.objectWithPrototype(context.intrinsics().arrayIteratorPrototype);
        context.payload(iterator)=ArrayIteratorState{receiver,0,false,false};
        return Value(iterator);
    });
    define(values,Key(u"length"),Descriptor::data(Value(0),false,false,true));
    define(intrinsics.arrayPrototype,Key(u"values"),Descriptor::data(Value(values),true,false,true));
    define(intrinsics.arrayPrototype,Key(intrinsics.symbolIterator),Descriptor::data(Value(values),true,false,true));
    const auto stringIterator=function(u"[Symbol.iterator]",[](Context& context,Value receiver,std::span<const Value>)->Value {
        const auto* text=std::get_if<std::u16string>(&receiver.data);
        if(!text) type_error(u"String iterator called on incompatible receiver");
        auto iterator=context.objectWithPrototype(context.intrinsics().stringIteratorPrototype);
        context.payload(iterator)=ArrayIteratorState{receiver,0,false,true};
        return Value(iterator);
    });
    define(intrinsics.stringPrototype,Key(intrinsics.symbolIterator),Descriptor::data(Value(stringIterator),true,false,true));
    define(intrinsics.stringIteratorPrototype,Key(u"next"),Descriptor::data(Value(iteratorNext),true,false,true));
    const auto mapIteratorNext=function(u"next",[](Context& context,Value receiver,std::span<const Value>)->Value {
        const auto* iterator=std::get_if<ObjectRef>(&receiver.data);
        if(!iterator) type_error(u"Map Iterator next called on incompatible receiver");
        const auto* next=std::any_cast<std::function<Value(Context&)>>(&context.payload(*iterator));
        if(!next||!*next) type_error(u"Map Iterator state is missing");
        return (*next)(context);
    });
    define(intrinsics.mapIteratorPrototype,Key(u"next"),Descriptor::data(Value(mapIteratorNext),true,false,true));

    register_method(*this, state_->intrinsics.arrayPrototype, u"indexOf", 1,
        [](Context& context, Value receiver, std::span<const Value> arguments) -> Value {
            const auto* array = std::get_if<ObjectRef>(&receiver.data);
            if (!array) type_error(u"Array method called on incompatible receiver");
            require_array(context, *array);
            const Value needle = arguments.empty() ? Value(Undefined{}) : arguments.front();
            validate_value_context(context, needle);
            std::uint32_t start = 0;
            if (arguments.size() > 1) {
                const auto from = to_integer_or_infinity(arguments[1]);
                const double length = context.arrayLength(*array);
                if (from == std::numeric_limits<double>::infinity()) start = static_cast<std::uint32_t>(length);
                else if (from == -std::numeric_limits<double>::infinity()) start = 0;
                else if (from < 0) start = static_cast<std::uint32_t>(std::max(0.0, length + from));
                else start = static_cast<std::uint32_t>(std::min(from, length));
            }
            const auto length = context.arrayLength(*array);
            for (std::uint64_t index = start; index < length; ++index) {
                const Key key(index_key(static_cast<std::uint32_t>(index)));
                if (context.has(Value(*array), key) && context.get(Value(*array), key) == needle) return Value(static_cast<double>(index));
            }
            return Value(-1);
        });

    register_method(*this, state_->intrinsics.arrayPrototype, u"filter", 1,
        [](Context& context, Value receiver, std::span<const Value> arguments) -> Value {
            const auto* array = std::get_if<ObjectRef>(&receiver.data);
            if (!array) type_error(u"Array method called on incompatible receiver");
            require_array(context, *array);
            if (arguments.empty()) type_error(u"Array filter callback must be callable");
            const auto callback = require_function(context, arguments.front());
            const Value this_arg = arguments.size() > 1 ? arguments[1] : Value(Undefined{});
            const auto initial_length = context.arrayLength(*array);
            auto result = context.array();
            for (std::uint64_t index = 0; index < initial_length; ++index) {
                const Key key(index_key(static_cast<std::uint32_t>(index)));
                if (!context.has(Value(*array), key)) continue;
                const auto value = context.get(Value(*array), key);
                const std::array<Value, 3> callback_args{
                    value, Value(static_cast<double>(index)), Value(*array)
                };
                if (truthy(context.call(callback, this_arg, callback_args))) {
                    context.set(Value(result), Key(index_key(context.arrayLength(result))), value);
                }
            }
            return Value(result);
        });

    register_method(*this, state_->intrinsics.arrayPrototype, u"forEach", 1,
        [](Context& context, Value receiver, std::span<const Value> arguments) -> Value {
            const auto* array=std::get_if<ObjectRef>(&receiver.data);
            if(!array)type_error(u"Array method called on incompatible receiver");
            const Value callback=arguments.empty()?Value(Undefined{}):arguments.front();
            const Value thisArgument=arguments.size()>1?arguments[1]:Value(Undefined{});
            context.arrayForEach(*array,callback,thisArgument);
            return Undefined{};
        });

    register_method(*this, state_->intrinsics.arrayPrototype, u"map", 1,
        [](Context& context, Value receiver, std::span<const Value> arguments) -> Value {
            const auto* array=std::get_if<ObjectRef>(&receiver.data);
            if(!array)type_error(u"Array method called on incompatible receiver");
            const Value callback=arguments.empty()?Value(Undefined{}):arguments.front();
            const Value thisArgument=arguments.size()>1?arguments[1]:Value(Undefined{});
            return context.arrayMap(*array,callback,thisArgument);
        });

    register_method(*this, state_->intrinsics.arrayPrototype, u"slice", 2,
        [](Context& context, Value receiver, std::span<const Value> arguments) -> Value {
            const auto* array = std::get_if<ObjectRef>(&receiver.data);
            if (!array) type_error(u"Array method called on incompatible receiver");
            return Value(context.arraySlice(*array,
                arguments.empty() ? Value(Undefined{}) : arguments[0],
                arguments.size() < 2 ? Value(Undefined{}) : arguments[1]));
        });
    register_method(*this, state_->intrinsics.arrayPrototype, u"reverse", 0,
        [](Context& context, Value receiver, std::span<const Value>) -> Value {
            const auto* array = std::get_if<ObjectRef>(&receiver.data);
            if (!array) type_error(u"Array method called on incompatible receiver");
            return Value(context.arrayReverse(*array));
        });
    register_method(*this, state_->intrinsics.arrayPrototype, u"includes", 1,
        [](Context& context, Value receiver, std::span<const Value> arguments) -> Value {
            const auto* array = std::get_if<ObjectRef>(&receiver.data);
            if (!array) type_error(u"Array method called on incompatible receiver");
            return Value(context.arrayIncludes(*array,
                arguments.empty() ? Value(Undefined{}) : arguments[0],
                arguments.size() < 2 ? Value(Undefined{}) : arguments[1]));
        });
    register_method(*this, state_->intrinsics.arrayPrototype, u"push", 1,
        [](Context& context, Value receiver, std::span<const Value> arguments) -> Value {
            const auto* array = std::get_if<ObjectRef>(&receiver.data);
            if (!array) type_error(u"Array method called on incompatible receiver");
            return Value(static_cast<double>(context.arrayPush(*array, arguments)));
        });
    register_method(*this, state_->intrinsics.arrayPrototype, u"pop", 0,
        [](Context& context, Value receiver, std::span<const Value>) -> Value {
            const auto* array = std::get_if<ObjectRef>(&receiver.data);
            if (!array) type_error(u"Array method called on incompatible receiver");
            return context.arrayPop(*array);
        });
    register_method(*this, state_->intrinsics.arrayPrototype, u"join", 1,
        [](Context& context, Value receiver, std::span<const Value> arguments) -> Value {
            const auto* array = std::get_if<ObjectRef>(&receiver.data);
            if (!array) type_error(u"Array method called on incompatible receiver");
            return Value(context.arrayJoin(*array,
                arguments.empty() ? Value(Undefined{}) : arguments[0]));
        });
    register_method(*this, state_->intrinsics.arrayPrototype, u"fill", 1,
        [](Context& context, Value receiver, std::span<const Value> arguments) -> Value {
            const auto* array = std::get_if<ObjectRef>(&receiver.data);
            if (!array) type_error(u"Array method called on incompatible receiver");
            return Value(context.arrayFill(*array,
                arguments.empty() ? Value(Undefined{}) : arguments[0],
                arguments.size() < 2 ? Value(Undefined{}) : arguments[1],
                arguments.size() < 3 ? Value(Undefined{}) : arguments[2]));
        });
    register_method(*this, state_->intrinsics.arrayPrototype, u"find", 1,
        [](Context& context, Value receiver, std::span<const Value> arguments) -> Value {
            const auto* array = std::get_if<ObjectRef>(&receiver.data);
            if (!array) type_error(u"Array method called on incompatible receiver");
            return context.arrayFind(*array,
                arguments.empty() ? Value(Undefined{}) : arguments[0],
                arguments.size() < 2 ? Value(Undefined{}) : arguments[1]);
        });
}

ObjectRef Context::array(std::uint32_t length) {
    const auto result = allocate(ObjectKind::array, state_->intrinsics.arrayPrototype);
    auto& properties = record(result).properties;
    properties.emplace_back(Key(u"length"), Descriptor::data(Value(0), true, false, false));
    if (length != 0) {
        define(result, Key(u"length"), Descriptor::data(Value(static_cast<double>(length)), true, false, false));
    }
    return result;
}

std::uint32_t Context::arrayLength(ObjectRef object) const {
    const auto& target = record(object);
    if (target.kind != ObjectKind::array) type_error(u"Array length requested from non-array");
    const auto found = std::find_if(target.properties.begin(), target.properties.end(), [](const auto& property) {
        const auto* name = std::get_if<std::u16string>(&property.first.data);
        return name && *name == u"length";
    });
    if (found == target.properties.end() || found->second.accessor) type_error(u"Array length descriptor is missing");
    const auto* number = std::get_if<double>(&found->second.value.data);
    if (!number || !std::isfinite(*number) || *number < 0 || *number > 4294967295.0 || std::trunc(*number) != *number) {
        type_error(u"Array length is invalid");
    }
    return static_cast<std::uint32_t>(*number);
}

std::vector<Value> Context::arrayValues(ObjectRef object) {
    require_array(*this, object);
    std::vector<Value> values;
    // Model the live ordinary-array iteration consumed by Options.ts spread:
    // read length anew after each element, and use Get so holes yield inherited
    // values or undefined. This is not a general JS iterator API.
    for (std::uint64_t index = 0; index < arrayLength(object); ++index) {
        values.push_back(get(Value(object), Key(index_key(static_cast<std::uint32_t>(index)))));
    }
    return values;
}

std::ptrdiff_t Context::arrayIndexOf(ObjectRef object, const Value& needle) {
    require_array(*this, object);
    validate_value_context(*this, needle);
    const auto length = arrayLength(object);
    for (std::uint64_t index = 0; index < length; ++index) {
        const Key key(index_key(static_cast<std::uint32_t>(index)));
        if (has(Value(object), key) && get(Value(object), key) == needle) return static_cast<std::ptrdiff_t>(index);
    }
    return -1;
}

ObjectRef Context::arrayFilter(ObjectRef object, const std::function<bool(Value)>& predicate) {
    require_array(*this, object);
    if (!predicate) type_error(u"Array filter callback must be callable");
    const auto initial_length = arrayLength(object);
    const auto result = array();
    for (std::uint64_t index = 0; index < initial_length; ++index) {
        const Key key(index_key(static_cast<std::uint32_t>(index)));
        if (!has(Value(object), key)) continue;
        auto value = get(Value(object), key);
        if (predicate(value)) set(Value(result), Key(index_key(arrayLength(result))), std::move(value));
    }
    return result;
}

} // namespace folia::math::options
