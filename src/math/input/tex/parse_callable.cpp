// Shared native ParseMethod bridge for MathJax 4.1.3 TeX input.
// Apache-2.0; see src/math/LICENSE-MathJax.txt.
module folia.math.input.tex.types;

import folia.math.core.tree.factory_types;
import folia.math.util.callable_types;
import std;

namespace {

[[noreturn]] void invalid_callable(std::u16string message) {
    throw folia::math::tree::FactoryError("TypeError", std::move(message));
}

void validate_receiver(const folia::math::FunctionReceiver& receiver) {
    if (const auto* object = std::get_if<folia::math::NativeObjectRef>(&receiver)) {
        if (!*object) invalid_callable(u"ParseMethod receiver is null");
        return;
    }
    if (!std::holds_alternative<folia::math::Missing>(receiver)) {
        invalid_callable(u"Invalid ParseMethod receiver");
    }
}

std::any flatten_result(const folia::math::tex::ParseResult& result) {
    return std::visit([](const auto& value) -> std::any {
        return std::any(value);
    }, result);
}

folia::math::tex::ParseResult restore_result(const std::any& result) {
    using folia::math::tex::ParseSymbolRef;
    using folia::math::tex::StackItemRef;
    if (!result.has_value()) return folia::math::Property(folia::math::Missing{});
    if (const auto* value = std::any_cast<folia::math::Property>(&result)) return *value;
    if (const auto* value = std::any_cast<StackItemRef>(&result)) return *value;
    if (const auto* value = std::any_cast<ParseSymbolRef>(&result)) return *value;
    invalid_callable(u"Invalid ParseMethod result type");
}

} // namespace

folia::math::tex::ParseCallableRef folia::math::tex::makeParseCallable(
    ParseBody body) {
    if (!body) invalid_callable(u"ParseMethod body is empty");

    auto function = std::make_shared<ParseCallable>();
    function->call = [body = std::move(body)](
        const FunctionReceiver& receiver, FunctionArgs args) -> std::any {
        validate_receiver(receiver);
        if (args.size() < 2) {
            invalid_callable(u"ParseMethod requires parser and atom slots");
        }

        const auto* parser = std::any_cast<std::reference_wrapper<ParserContext>>(&args[0]);
        if (!parser) invalid_callable(u"Invalid ParseMethod parser slot");
        const auto* atom = std::any_cast<ParseAtom>(&args[1]);
        if (!atom) invalid_callable(u"Invalid ParseMethod atom slot");

        const auto rest = args.subspan(2);
        return flatten_result(body(receiver, parser->get(), *atom, rest));
    };
    return function;
}

folia::math::tex::ParseResult folia::math::tex::invokeParseMethod(
    const ParseMethod& method, const FunctionReceiver& receiver,
    ParserContext& parser, const ParseAtom& atom, ParseRest rest) {
    validate_receiver(receiver);

    const auto* candidate = std::get_if<ParseCallableRef>(&method);
    if (!candidate || !*candidate) invalid_callable(u"ParseMethod is not callable");
    const ParseCallableRef callable = *candidate;
    const auto body = callable->call;
    if (!body) invalid_callable(u"ParseMethod body is empty");

    std::vector<std::any> slots;
    slots.reserve(rest.size() + 2);
    slots.emplace_back(std::ref(parser));
    slots.emplace_back(atom);
    for (const auto& value : rest) slots.push_back(value);

    const std::any result = body(receiver, FunctionArgs(slots));
    return restore_result(result);
}
