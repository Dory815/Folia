// MathJax 4.1.3 input/tex/textmacros/TextMacrosMethods.ts; Apache-2.0.
module folia.math.input.tex.textmacros.methods;
import std;

namespace folia::math::tex {
namespace {
TextParser &textParser(TexParser &parser) {
    return dynamic_cast<TextParser &>(parser);
}
std::span<TexArgument const> arguments(MacroInvocation const &invocation) {
    return std::get<std::span<TexArgument const>>(invocation.arguments);
}
std::u16string const &argumentText(MacroInvocation const &invocation) {
    return std::get<std::u16string>(arguments(invocation)[0]);
}
} // namespace
DispatchOutcome TextMacrosMethods::Comment(TexParser &parser, MacroInvocation const &) {
    while (parser.position() < parser.string().size() &&
           parser.string()[parser.position()] != u'\n')
        parser.setPosition(parser.position() + 1);
    parser.setPosition(parser.position() + 1);
    return NoParseResult{};
}
DispatchOutcome TextMacrosMethods::Math(TexParser &base, MacroInvocation const &invocation) {
    auto &parser = textParser(base);
    parser.saveText();
    const auto start = parser.position();
    std::size_t braces = 0;
    while (true) {
        auto c = parser.GetNext();
        if (c.empty())
            break;
        const auto end = parser.position();
        parser.setPosition(end + 1);
        if (c == u"\\" || c == u"$") {
            if (c == u"\\" && parser.GetCS() == u")")
                c = u"\\(";
            if (braces == 0 && invocation.dispatch_token == c) {
                auto source = std::u16string(parser.string().substr(start, end - start));
                auto *math = TexParser(std::move(source), ParserEnvironmentSeed{parser.stack.env()},
                                       parser.texParser().configuration)
                                 .mml();
                parser.PushMath(*math);
                return NoParseResult{};
            }
        } else if (c == u"{") {
            ++braces;
        } else if (c == u"}") {
            if (braces == 0)
                parser.Error("ExtraCloseMissingOpen", u"Extra close brace or missing open brace");
            --braces;
        }
    }
    parser.Error("MathNotTerminated", u"Math mode is not properly terminated");
}
DispatchOutcome TextMacrosMethods::MathModeOnly(TexParser &base,
                                                MacroInvocation const &invocation) {
    const std::u16string token(invocation.dispatch_token);
    textParser(base).Error("MathModeOnly", u"'%1' allowed only in math mode", {&token, 1});
}
DispatchOutcome TextMacrosMethods::Misplaced(TexParser &base, MacroInvocation const &invocation) {
    const std::u16string token(invocation.dispatch_token);
    textParser(base).Error("Misplaced", u"Misplaced '%1'", {&token, 1});
}
DispatchOutcome TextMacrosMethods::OpenBrace(TexParser &base, MacroInvocation const &) {
    auto &parser = textParser(base);
    parser.envStack.push_back(parser.stack.environmentScope());
    parser.stack.setEnv(std::make_shared<TexEnvironment>(parser.stack.env()));
    return NoParseResult{};
}
DispatchOutcome TextMacrosMethods::CloseBrace(TexParser &base, MacroInvocation const &) {
    auto &parser = textParser(base);
    if (parser.envStack.empty())
        parser.Error("ExtraCloseMissingOpen", u"Extra close brace or missing open brace");
    parser.saveText();
    parser.stack.setEnv(parser.envStack.back());
    parser.envStack.pop_back();
    return NoParseResult{};
}
DispatchOutcome TextMacrosMethods::OpenQuote(TexParser &base, MacroInvocation const &invocation) {
    auto &parser = textParser(base);
    if (parser.position() < parser.string().size() &&
        parser.string()[parser.position()] == invocation.dispatch_token[0]) {
        parser.text() += u'\u201c';
        parser.setPosition(parser.position() + 1);
    } else
        parser.text() += u'\u2018';
    return NoParseResult{};
}
DispatchOutcome TextMacrosMethods::CloseQuote(TexParser &base, MacroInvocation const &invocation) {
    auto &parser = textParser(base);
    if (parser.position() < parser.string().size() &&
        parser.string()[parser.position()] == invocation.dispatch_token[0]) {
        parser.text() += u'\u201d';
        parser.setPosition(parser.position() + 1);
    } else
        parser.text() += u'\u2019';
    return NoParseResult{};
}
DispatchOutcome TextMacrosMethods::Tilde(TexParser &base, MacroInvocation const &) {
    textParser(base).text() += u'\u00a0';
    return NoParseResult{};
}
DispatchOutcome TextMacrosMethods::Space(TexParser &base, MacroInvocation const &) {
    auto &parser = textParser(base);
    parser.text() += u' ';
    parser.GetNext();
    return NoParseResult{};
}
DispatchOutcome TextMacrosMethods::SelfQuote(TexParser &base, MacroInvocation const &invocation) {
    textParser(base).text().append(invocation.dispatch_token.substr(1));
    return NoParseResult{};
}
DispatchOutcome TextMacrosMethods::Insert(TexParser &base, MacroInvocation const &invocation) {
    textParser(base).text() += argumentText(invocation);
    return NoParseResult{};
}
DispatchOutcome TextMacrosMethods::Accent(TexParser &base, MacroInvocation const &invocation) {
    auto &parser = textParser(base);
    auto *math = parser.ParseArg(invocation.dispatch_token);
    auto &accent = parser.createToken(mml::NodeKind::mo, argumentText(invocation));
    parser.addAttributes(accent);
    const std::array children{math, &accent};
    parser.Push(&parser.create(mml::NodeKind::mover, children));
    return NoParseResult{};
}
DispatchOutcome TextMacrosMethods::Emph(TexParser &base, MacroInvocation const &invocation) {
    auto &parser = textParser(base);
    TexEnvironment environment;
    environment.math_variant =
        parser.stack.env().math_variant == u"-tex-mathit" ? u"normal" : u"-tex-mathit";
    parser.Push(parser.ParseTextArg(invocation.dispatch_token, std::move(environment)));
    return NoParseResult{};
}
DispatchOutcome TextMacrosMethods::TextFont(TexParser &base, MacroInvocation const &invocation) {
    auto &parser = textParser(base);
    parser.saveText();
    TexEnvironment environment;
    environment.math_variant = argumentText(invocation);
    parser.Push(parser.ParseTextArg(invocation.dispatch_token, std::move(environment)));
    return NoParseResult{};
}
DispatchOutcome TextMacrosMethods::SetFont(TexParser &base, MacroInvocation const &invocation) {
    auto &parser = textParser(base);
    parser.saveText();
    parser.stack.env().math_variant = argumentText(invocation);
    return NoParseResult{};
}
DispatchOutcome TextMacrosMethods::SetSize(TexParser &base, MacroInvocation const &invocation) {
    auto &parser = textParser(base);
    parser.saveText();
    parser.stack.env().math_size = std::get<double>(arguments(invocation)[0]);
    return NoParseResult{};
}
DispatchOutcome TextMacrosMethods::CheckAutoload(TexParser &base,
                                                 MacroInvocation const &invocation) {
    auto &parser = textParser(base);
    auto &primary = parser.texParser();
    const std::u16string name(invocation.dispatch_token.substr(1));
    auto macro = primary.lookup(HandlerKind::macro, name);
    const bool present = std::visit(
        [](auto const &entry) {
            using T = std::decay_t<decltype(entry)>;
            if constexpr (std::same_as<T, std::monostate>)
                return false;
            else if constexpr (std::same_as<T, std::u16string_view>)
                return !entry.empty();
            else
                return entry != nullptr;
        },
        macro);
    if (!present) {
        (void)primary.parse(HandlerKind::macro, ParseInput{primary, name});
        return NoParseResult{};
    }
    (void)primary.parse(HandlerKind::macro, ParseInput{parser, name});
    return NoParseResult{};
}
} // namespace folia::math::tex
