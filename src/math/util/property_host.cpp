// Root-owned coercion traversal state. Keep nontrivial TLS outside exported
// inline module functions so importing MSVC translation units share its owner.
module folia.math.util.property_host;
import folia.math.util.option_types;
import folia.math.util.js_numeric_input;
import folia.math.util.js_number;
import std;
namespace folia::math::property_host {
std::vector<Property>& activeArrays() {
    static thread_local std::vector<Property> active;
    return active;
}

namespace {
using options::Context;
using options::Value;
using options::Undefined;
using options::Null;
using options::SymbolRef;

[[noreturn]] void addTypeError(std::u16string message) {
    throw options::Exception("TypeError", std::move(message));
}

std::u16string primitiveString(Context& context, const Value& value) {
    if (std::holds_alternative<Undefined>(value.data)) return u"undefined";
    if (std::holds_alternative<Null>(value.data)) return u"null";
    if (const auto* boolean = std::get_if<bool>(&value.data)) return *boolean ? u"true" : u"false";
    if (const auto* number = std::get_if<double>(&value.data))
        return text(js_number::toString(*number));
    if (const auto* string = std::get_if<std::u16string>(&value.data)) return *string;
    if (const auto* symbol = std::get_if<SymbolRef>(&value.data)) {
        (void)context.symbolString(*symbol);
        addTypeError(u"Cannot convert a Symbol value to a string");
    }
    addTypeError(u"Cannot convert object to primitive value");
}
}

Value add(Context& context, const Value& left, const Value& right) {
    // Abstract Addition performs the left ToPrimitive before the right one.
    const Value lhs = context.toPrimitive(left, options::PrimitiveHint::defaultHint);
    const Value rhs = context.toPrimitive(right, options::PrimitiveHint::defaultHint);
    if (std::holds_alternative<std::u16string>(lhs.data) ||
        std::holds_alternative<std::u16string>(rhs.data)) {
        return Value(primitiveString(context, lhs) + primitiveString(context, rhs));
    }
    return Value(context.toNumber(lhs) + context.toNumber(rhs));
}
}
