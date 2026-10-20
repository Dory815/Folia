// MathJax 4.1.3 TokenMap.ts; Apache-2.0.
module folia.math.input.tex.parser;
import folia.math.util.js_number;
import std;

namespace folia::math::tex {
DispatchOutcome parseResult(DispatchOutcome result) {
    return std::holds_alternative<NoParseResult>(result) ? DispatchOutcome{Handled{}}
                                                         : std::move(result);
}
AbstractTokenMap::AbstractTokenMap(TokenMapInit init) : name_(std::move(init.name)) {
    setParser(init.parser);
}
void AbstractTokenMap::setParser(std::optional<TokenMapParser> method) {
    if (method && !std::visit([](auto callback) { return callback != nullptr; }, *method))
        method.reset();
    parser_ = method;
}
std::optional<TokenMapParser> AbstractTokenMap::parserFor(std::u16string_view token) const {
    return contains(token) ? parser_ : std::nullopt;
}
DispatchOutcome AbstractTokenMap::parseValue(TexParser &parser, TokenMapParser const &method,
                                             TokenInput const &input) {
    auto callback = std::get_if<TokenInputHandler>(&method);
    if (!callback)
        throw std::logic_error("Token map requires a typed token callback");
    return *callback ? parseResult((*callback)(parser, input))
                     : DispatchOutcome{ContinueDispatch{}};
}
DispatchOutcome AbstractTokenMap::parse(TexParser &parser, std::u16string_view token) {
    auto method = parserFor(token);
    auto mapped = lookup(token); // Source evaluates this even when parserFor returned null.
    if (!method)
        return ContinueDispatch{};
    if (auto text = std::get_if<std::u16string_view>(&mapped)) {
        if (text->empty())
            return ContinueDispatch{};
        std::u16string captured(*text);
        return parseValue(parser, *method, TokenInput{std::u16string_view(captured)});
    }
    if (auto value = std::get_if<Token const *>(&mapped); value && *value) {
        Token captured = **value;
        return parseValue(parser, *method, TokenInput{&captured});
    }
    return ContinueDispatch{};
}
RegExpMap::RegExpMap(RegExpMapInit init)
    : AbstractTokenMap(std::move(init.base)), regexp_(std::move(init.regexp)) {
    if (!regexp_)
        throw std::invalid_argument("A regular expression map needs its expression");
}
bool RegExpMap::contains(std::u16string_view token) const {
    return regexp_->exec(std::u16string(token)).has_value();
}
TokenLookup RegExpMap::lookup(std::u16string_view token) const {
    return contains(token) ? TokenLookup{token} : TokenLookup{};
}
CharacterMap::CharacterMap(CharacterMapInit init)
    : AbstractParseMap(std::move(init.base), std::move(init.entries)) {}
DelimiterMap::DelimiterMap(CharacterMapInit init) : CharacterMap(std::move(init)) {}
DelimiterMap::DelimiterMap(SessionDelimiterMapInit init)
    : CharacterMap(CharacterMapInit{std::move(init.base), {}}), session_(&init.session) {}
bool DelimiterMap::contains(std::u16string_view token) const {
    return session_ ? session_->findDelimiter(token) != nullptr : CharacterMap::contains(token);
}
TokenLookup DelimiterMap::lookup(std::u16string_view token) const {
    if (!session_)
        return CharacterMap::lookup(token);
    auto definition = session_->findDelimiter(token);
    return definition ? TokenLookup{definition} : TokenLookup{};
}
DispatchOutcome DelimiterMap::parse(TexParser &parser, std::u16string_view token) {
    auto escaped = u"\\" + std::u16string(token);
    return CharacterMap::parse(parser, escaped);
}
MacroMap::MacroMap(MacroMapInit init)
    : AbstractParseMap(std::move(init.base), std::move(init.entries)) {}
MacroMap::MacroMap(SessionMacroMapInit init)
    : AbstractParseMap(std::move(init.base), {}), session_(&init.session),
      callbacks_(&init.callbacks), expansion_(init.expansion),
      templateExpansion_(init.templateExpansion) {
    if (!expansion_)
        throw std::invalid_argument("A session macro map needs its expansion callback");
}
bool MacroMap::contains(std::u16string_view token) const {
    return session_ ? session_->findCommand(token) != nullptr : AbstractParseMap::contains(token);
}
TokenLookup MacroMap::lookup(std::u16string_view token) const {
    if (!session_)
        return AbstractParseMap::lookup(token);
    auto definition = session_->findCommand(token);
    return definition ? TokenLookup{definition} : TokenLookup{};
}
std::optional<TokenMapParser> MacroMap::parserFor(std::u16string_view token) const {
    auto mapped = lookup(token);
    if (auto macro = std::get_if<Macro const *>(&mapped); macro && *macro)
        return std::visit(
            [](auto method) -> std::optional<TokenMapParser> {
                return method ? std::optional<TokenMapParser>(TokenMapParser{method})
                              : std::nullopt;
            },
            (*macro)->function);
    if (auto definition = std::get_if<CommandDefinition const *>(&mapped);
        definition && *definition) {
        return std::visit(
            [&](auto const &body) -> std::optional<TokenMapParser> {
                using T = std::decay_t<decltype(body)>;
                if constexpr (std::is_same_v<T, MacroDefinition>) {
                    if (body.parameter_template && !templateExpansion_)
                        throw std::logic_error(
                            "A parameter template needs its compiled expansion callback");
                    return TokenMapParser{body.parameter_template ? templateExpansion_
                                                                  : expansion_};
                } else if constexpr (std::is_same_v<T, NativeMacroAlias>)
                    return TokenMapParser{callbacks_->macroHandler(body.handler)};
                else if constexpr (std::is_same_v<T, CharacterMacroAlias>)
                    return TokenMapParser{callbacks_->tokenParser(body.parser)};
                else
                    return std::nullopt;
            },
            (*definition)->body);
    }
    return std::nullopt;
}
DispatchOutcome MacroMap::parse(TexParser &parser, std::u16string_view input) {
    auto mapped = lookup(input);
    if (auto definition = std::get_if<CommandDefinition const *>(&mapped);
        definition && *definition && std::holds_alternative<MacroSuppression>((*definition)->body))
        return InvokeFallback{};
    auto method = parserFor(input);
    if (!method)
        return ContinueDispatch{};
    return parseResult(invoke(parser, mapped, *method, input, {}));
}
DispatchOutcome MacroMap::invoke(TexParser &parser, TokenLookup const &mapped,
                                 TokenMapParser const &method, std::u16string_view input,
                                 std::u16string_view prefix) {
    std::u16string received(input);
    if (auto definition = std::get_if<CommandDefinition const *>(&mapped);
        definition && *definition) {
        // Snapshot the selected mathematical definition. A callback can replace
        // the same session entry; it must not invalidate its in-flight arguments.
        CommandDefinition captured = **definition;
        if (auto character = std::get_if<CharacterMacroAlias>(&captured.body))
            return parseValue(parser, method, TokenInput{&character->captured});
        auto callback = std::get_if<MacroHandler>(&method);
        if (!callback)
            throw std::logic_error("Macro map requires a typed macro callback");
        if (!*callback)
            return ContinueDispatch{};
        auto dispatch = std::u16string(prefix) + captured.source_token;
        MacroArguments arguments = std::cref(captured);
        if (auto alias = std::get_if<NativeMacroAlias>(&captured.body))
            arguments = std::span<TexArgument const>(alias->arguments);
        return (*callback)(parser, MacroInvocation{received, dispatch, arguments});
    }
    if (auto macro = std::get_if<Macro const *>(&mapped); macro && *macro) {
        Macro captured = **macro;
        auto callback = std::get_if<MacroHandler>(&method);
        if (!callback)
            throw std::logic_error("Macro map requires a typed macro callback");
        if (!*callback)
            return ContinueDispatch{};
        auto dispatch = std::u16string(prefix) + captured.token;
        return (*callback)(
            parser,
            MacroInvocation{received, dispatch, std::span<TexArgument const>(captured.arguments)});
    }
    return ContinueDispatch{};
}
CommandMap::CommandMap(CommandMapInit init) : MacroMap(std::move(init.macros)) {}
CommandMap::CommandMap(SessionMacroMapInit init) : MacroMap(std::move(init)) {}
DispatchOutcome CommandMap::parse(TexParser &parser, std::u16string_view input) {
    auto mapped = lookup(input);
    if (auto definition = std::get_if<CommandDefinition const *>(&mapped);
        definition && *definition && std::holds_alternative<MacroSuppression>((*definition)->body))
        return InvokeFallback{};
    auto method = parserFor(input);
    if (!method || std::holds_alternative<std::monostate>(mapped))
        return ContinueDispatch{};
    auto previous = parser.currentCS();
    parser.setCurrentCS(u"\\" + std::u16string(input));
    // Source retains the failed command for error reporting on an exception.
    auto result = invoke(parser, mapped, *method, input, u"\\");
    parser.setCurrentCS(std::move(previous));
    return parseResult(std::move(result));
}
EnvironmentMap::EnvironmentMap(EnvironmentMapInit init) : MacroMap(std::move(init.macros)) {
    setParser(TokenMapParser{init.parser});
}
EnvironmentMap::EnvironmentMap(SessionEnvironmentMapInit init)
    : MacroMap(MacroMapInit{std::move(init.base), {}}), environmentSession_(&init.session),
      environmentBody_(init.body) {
    if (!init.parser || !environmentBody_)
        throw std::invalid_argument("A session environment map needs its compiled callbacks");
    setParser(TokenMapParser{init.parser});
}
bool EnvironmentMap::contains(std::u16string_view token) const {
    return environmentSession_ ? environmentSession_->findEnvironment(token) != nullptr
                               : MacroMap::contains(token);
}
TokenLookup EnvironmentMap::lookup(std::u16string_view token) const {
    if (!environmentSession_)
        return MacroMap::lookup(token);
    auto definition = environmentSession_->findEnvironment(token);
    return definition ? TokenLookup{definition} : TokenLookup{};
}
std::optional<TokenMapParser> EnvironmentMap::parserFor(std::u16string_view token) const {
    if (!environmentSession_)
        return MacroMap::parserFor(token);
    return contains(token) ? std::optional<TokenMapParser>{environmentBody_} : std::nullopt;
}
DispatchOutcome EnvironmentMap::parse(TexParser &parser, std::u16string_view token) {
    auto mapped = lookup(token);
    if (auto entry = std::get_if<EnvironmentDefinition const *>(&mapped); entry && *entry) {
        EnvironmentDefinition captured = **entry;
        auto generic = this->parser();
        if (!generic)
            return ContinueDispatch{};
        auto callback = std::get_if<EnvironmentHandler>(&*generic);
        if (!callback || !*callback)
            throw std::logic_error("An environment map needs its typed parser");
        std::vector<TexArgument> arguments{true, captured.begin_source, captured.end_source,
                                           captured.arity};
        if (captured.substitute_parameters) {
            auto count = js_number::toString(captured.arity);
            arguments[3] = std::u16string(count.begin(), count.end());
        }
        if (captured.default_first_argument)
            arguments.emplace_back(*captured.default_first_argument);
        return parseResult((*callback)(parser, token, environmentBody_, arguments));
    }
    auto method = parserFor(token);
    auto generic = this->parser();
    auto macro = std::get_if<Macro const *>(&mapped);
    if (!method || !generic || !macro || !*macro)
        return ContinueDispatch{};
    auto callback = std::get_if<EnvironmentHandler>(&*generic);
    auto body = std::get_if<EnvironmentBodyHandler>(&*method);
    if (!callback || !body)
        throw std::logic_error("Environment map requires its typed environment callbacks");
    if (!*callback || !*body)
        return ContinueDispatch{};
    Macro captured = **macro;
    return parseResult((*callback)(parser, captured.token, *body, captured.arguments));
}
} // namespace folia::math::tex
