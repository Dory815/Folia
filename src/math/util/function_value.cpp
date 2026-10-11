// Root-frozen identity boundary; implementation delegated under P22a contract.
module folia.math.util.function_types;
import std;
namespace folia::math {
namespace functions {
FunctionRef reference(const FunctionValue& value) {
    if (const auto* function = std::get_if<FunctionRef>(&value)) return *function;
    const Property& property = std::get<Property>(value);
    if (const auto* function = std::get_if<FunctionRef>(&property.value)) return *function;
    return {};
}

FunctionValue fromProperty(const Property& property) {
    if (const auto* function = std::get_if<FunctionRef>(&property.value)) {
        return FunctionValue{*function};
    }
    return FunctionValue{property};
}

Property toProperty(const FunctionValue& value) {
    if (const auto* property = std::get_if<Property>(&value)) return *property;
    return Property(std::get<FunctionRef>(value));
}
}

bool operator==(const FunctionValue& left, const FunctionValue& right) {
    const auto functionValue = [](const FunctionValue& value) -> const FunctionRef* {
        if (const auto* function = std::get_if<FunctionRef>(&value)) return function;
        const Property& property = std::get<Property>(value);
        return std::get_if<FunctionRef>(&property.value);
    };

    const FunctionRef* leftFunction = functionValue(left);
    const FunctionRef* rightFunction = functionValue(right);
    if (leftFunction || rightFunction) {
        return leftFunction && rightFunction && *leftFunction == *rightFunction;
    }

    const auto* leftProperty = std::get_if<Property>(&left);
    const auto* rightProperty = std::get_if<Property>(&right);
    return leftProperty && rightProperty && *leftProperty == *rightProperty;
}
}
