// MathJax 4.1.3 TokenMap.ts; Apache-2.0.
module folia.math.input.tex.parser;
import std;

namespace folia::math::tex {
DispatchOutcome parseResult(DispatchOutcome result) {
    return std::holds_alternative<NoParseResult>(result) ? DispatchOutcome{Handled{}} : std::move(result);
}
AbstractTokenMap::AbstractTokenMap(TokenMapInit init)
    : name_(std::move(init.name)) { setParser(init.parser); }
void AbstractTokenMap::setParser(std::optional<TokenMapParser> method) {
    if (method && !std::visit([](auto callback) { return callback != nullptr; }, *method)) method.reset();
    parser_ = method;
}
std::optional<TokenMapParser> AbstractTokenMap::parserFor(std::u16string_view token) const {
    return contains(token) ? parser_ : std::nullopt;
}
DispatchOutcome AbstractTokenMap::parseValue(TexParser& parser, TokenMapParser const& method,
                                            TokenInput const& input) {
    auto callback = std::get_if<TokenInputHandler>(&method);
    if (!callback) throw std::logic_error("Token map requires a typed token callback");
    return *callback ? parseResult((*callback)(parser, input)) : DispatchOutcome{ContinueDispatch{}};
}
DispatchOutcome AbstractTokenMap::parse(TexParser& parser, std::u16string_view token) {
    auto method = parserFor(token);
    auto mapped = lookup(token); // Source evaluates this even when parserFor returned null.
    if (!method) return ContinueDispatch{};
    if (auto text = std::get_if<std::u16string_view>(&mapped)) {
        if (text->empty()) return ContinueDispatch{};
        std::u16string captured(*text);
        return parseValue(parser, *method, TokenInput{std::u16string_view(captured)});
    }
    if (auto value = std::get_if<Token const*>(&mapped); value && *value) {
        Token captured = **value;
        return parseValue(parser, *method, TokenInput{&captured});
    }
    return ContinueDispatch{};
}
RegExpMap::RegExpMap(RegExpMapInit init)
    : AbstractTokenMap(std::move(init.base)), regexp_(std::move(init.regexp)) {
    if (!regexp_) throw std::invalid_argument("A regular expression map needs its expression");
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
DispatchOutcome DelimiterMap::parse(TexParser& parser, std::u16string_view token) {
    auto escaped = u"\\" + std::u16string(token);
    return CharacterMap::parse(parser, escaped);
}
MacroMap::MacroMap(MacroMapInit init)
    : AbstractParseMap(std::move(init.base), std::move(init.entries)) {}
MacroMap::MacroMap(SessionMacroMapInit init)
    : AbstractParseMap(std::move(init.base), {}), session_(&init.session),
      callbacks_(&init.callbacks), expansion_(init.expansion) {
    if (!expansion_) throw std::invalid_argument("A session macro map needs its expansion callback");
}
bool MacroMap::contains(std::u16string_view token) const {
    return session_ ? session_->findCommand(token) != nullptr : AbstractParseMap::contains(token);
}
TokenLookup MacroMap::lookup(std::u16string_view token) const {
    if (!session_) return AbstractParseMap::lookup(token);
    auto definition = session_->findCommand(token);
    return definition ? TokenLookup{definition} : TokenLookup{};
}
std::optional<TokenMapParser> MacroMap::parserFor(std::u16string_view token) const {
    auto mapped = lookup(token);
    if (auto macro = std::get_if<Macro const*>(&mapped); macro && *macro)
        return std::visit([](auto method) -> std::optional<TokenMapParser> {
            return method ? std::optional<TokenMapParser>(TokenMapParser{method}) : std::nullopt;
        }, (*macro)->function);
    if (auto definition = std::get_if<CommandDefinition const*>(&mapped); definition && *definition) {
        return std::visit([&](auto const& body) -> std::optional<TokenMapParser> {
            using T = std::decay_t<decltype(body)>;
            if constexpr (std::is_same_v<T, MacroDefinition>) return TokenMapParser{expansion_};
            else if constexpr (std::is_same_v<T, NativeMacroAlias>)
                return TokenMapParser{callbacks_->macroHandler(body.handler)};
            else if constexpr (std::is_same_v<T, CharacterMacroAlias>)
                return TokenMapParser{callbacks_->tokenParser(body.parser)};
            else return std::nullopt;
        }, (*definition)->body);
    }
    return std::nullopt;
}
DispatchOutcome MacroMap::parse(TexParser& parser, std::u16string_view input) {
    auto mapped = lookup(input);
    auto method = parserFor(input);
    if (!method) return ContinueDispatch{};
    std::u16string received(input);
    if (auto definition = std::get_if<CommandDefinition const*>(&mapped); definition && *definition) {
        // Snapshot the selected mathematical definition. A callback can replace
        // the same session entry; it must not invalidate its in-flight arguments.
        CommandDefinition captured = **definition;
        if (auto character = std::get_if<CharacterMacroAlias>(&captured.body))
            return parseValue(parser, *method, TokenInput{&character->captured});
        auto callback = std::get_if<MacroHandler>(&*method);
        if (!callback) throw std::logic_error("Macro map requires a typed macro callback");
        if (!*callback) return ContinueDispatch{};
        auto const& dispatch = captured.source_token;
        MacroArguments arguments = std::cref(captured);
        if (auto alias = std::get_if<NativeMacroAlias>(&captured.body))
            arguments = std::span<TexArgument const>(alias->arguments);
        return parseResult((*callback)(parser, MacroInvocation{received, dispatch, arguments}));
    }
    if (auto macro = std::get_if<Macro const*>(&mapped); macro && *macro) {
        Macro captured = **macro;
        auto callback = std::get_if<MacroHandler>(&*method);
        if (!callback) throw std::logic_error("Macro map requires a typed macro callback");
        if (!*callback) return ContinueDispatch{};
        auto const& dispatch = captured.token;
        return parseResult((*callback)(parser, MacroInvocation{received, dispatch,
            std::span<TexArgument const>(captured.arguments)}));
    }
    return ContinueDispatch{};
}
EnvironmentMap::EnvironmentMap(EnvironmentMapInit init) : MacroMap(std::move(init.macros)) {
    setParser(TokenMapParser{init.parser});
}
DispatchOutcome EnvironmentMap::parse(TexParser& parser, std::u16string_view token) {
    auto mapped = lookup(token);
    auto method = parserFor(token);
    auto generic = this->parser();
    auto macro = std::get_if<Macro const*>(&mapped);
    if (!method || !generic || !macro || !*macro) return ContinueDispatch{};
    auto callback = std::get_if<EnvironmentHandler>(&*generic);
    auto body = std::get_if<EnvironmentBodyHandler>(&*method);
    if (!callback || !body) throw std::logic_error("Environment map requires its typed environment callbacks");
    if (!*callback || !*body) return ContinueDispatch{};
    Macro captured = **macro;
    return parseResult((*callback)(parser, captured.token, *body, captured.arguments));
}
}
