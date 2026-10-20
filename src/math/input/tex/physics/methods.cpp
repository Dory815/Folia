// MathJax 4.1.3 input/tex/physics/PhysicsMethods.ts; Apache-2.0.
module folia.math.input.tex.physics.methods;
import folia.math.input.tex.parse_util;
import folia.math.util.js_numeric_input;
import folia.math.util.js_number;
import std;

namespace folia::math::tex {
namespace {
using mml::MmlNode;
using mml::NodeKind;
using Args = std::span<TexArgument const>;
Args arguments(MacroInvocation const &invocation) {
    return std::get<Args>(invocation.arguments);
}
std::u16string stringArg(Args args, std::size_t i, std::u16string_view fallback = {}) {
    return i < args.size() ? std::get<std::u16string>(args[i]) : std::u16string(fallback);
}
bool flagArg(Args args, std::size_t i, bool fallback = false) {
    return i < args.size() ? std::get<bool>(args[i]) : fallback;
}
std::u16string numberText(double value) {
    const auto text = js_number::toString(value);
    return {text.begin(), text.end()};
}
MmlNode &nested(TexParser &parser, std::u16string source) {
    auto *node = TexParser(std::move(source), ParserEnvironmentSeed{parser.stack.env()},
                           parser.configuration)
                     .mml();
    if (!node)
        throw std::logic_error("Physics subexpression requires MML");
    return *node;
}
DispatchOutcome pushNested(TexParser &parser, std::u16string source) {
    parser.Push(&nested(parser, std::move(source)));
    return NoParseResult{};
}
void replaceRemaining(TexParser &parser, std::u16string replacement) {
    replacement += parser.string().substr(parser.position());
    parser.setString(std::move(replacement));
    parser.setPosition(0);
}
void insertAtPosition(TexParser &parser, std::u16string replacement) {
    auto source = std::u16string(parser.string());
    source.insert(parser.position(), replacement);
    parser.setString(std::move(source));
}
FrameHandle autoOpen(TexParser &parser, std::u16string open, std::u16string close) {
    auto item = parser.itemFactory().create(StackKind::auto_open);
    auto &frame = dynamic_cast<AutoOpen &>(*item);
    frame.open = std::move(open);
    frame.close = std::move(close);
    return item;
}
char16_t closingFence(std::u16string_view fence) {
    if (fence == u"(")
        return u')';
    if (fence == u"[")
        return u']';
    if (fence == u"{")
        return u'}';
    if (fence == u"|")
        return u'|';
    return 0;
}
bool bigFence(std::u16string_view value) {
    return value == u"big" || value == u"Big" || value == u"bigg" || value == u"Bigg";
}
[[noreturn]] void missingArgument(TexParser &parser) {
    const std::array args{parser.currentCS()};
    throw TexError("MissingArgFor", u"Missing argument for %1", args);
}
NodeDefinition operatorDefinition() {
    NodeDefinition result;
    result.properties.tex.explicit_tex_class = mml::TexClass::op;
    return result;
}
std::u16string fencedSource(std::u16string const &open, std::u16string const &body,
                            std::u16string const &close, bool star, std::u16string const &big) {
    if (star)
        return open + u" " + body + u" " + close;
    if (!big.empty())
        return u"\\" + big + u"l" + open + u" " + body + u" \\" + big + u"r" + close;
    return u"\\left" + open + u" " + body + u" \\right" + close;
}
MmlNode &createVectorToken(NodeFactory &factory, NodeKind kind, NodeDefinition definition,
                           std::optional<std::u16string_view> text) {
    auto &token = NodeFactory::createToken(factory, kind, std::move(definition), text);
    auto const &env = factory.configuration().environment();
    if (!text || text->size() != 1 || (env.font && !env.font->empty()) || !env.vector_font ||
        env.vector_font->empty())
        return token;
    const auto code = (*text)[0];
    auto between = [&](char16_t low, char16_t high) { return low <= code && code <= high; };
    const auto *accent = token.attributes().lookup(u"accent");
    const auto *accentFlag = accent ? std::get_if<bool>(accent) : nullptr;
    if (between(0x41, 0x5a) || between(0x61, 0x7a) || between(0x391, 0x3a9) ||
        between(0x30, 0x39) || (between(0x3b1, 0x3c9) && env.vector_star.value_or(false)) ||
        (accentFlag && *accentFlag))
        token.attributes().set(u"mathvariant", *env.vector_font);
    return token;
}
void vectorApplication(TexParser &parser, MacroInvocation const &invocation, bool vector) {
    auto args = arguments(invocation);
    auto &op = nested(parser, stringArg(args, 0));
    parser.Push(vector ? parser.itemFactory().create(StackKind::mml, MmlItemInit{op})
                       : parser.itemFactory().create(StackKind::fn, FnItemInit{op}));
    auto left = parser.GetNext();
    const auto right = closingFence(left);
    if (!right)
        return;
    const bool enlarge = std::ranges::any_of(
        args.subspan(1), [&](auto const &arg) { return std::get<std::u16string>(arg) == left; });
    if (left == u"{") {
        auto argument = parser.GetArgument(invocation.dispatch_token).value();
        auto macro = (enlarge ? std::u16string(u"\\left\\{") : u"") + u" " + argument + u" " +
                     (enlarge ? u"\\right\\}" : u"");
        // Source concatenates directly here; it does not insert macro argument spacing.
        replaceRemaining(parser, std::move(macro));
    } else if (enlarge) {
        parser.setPosition(parser.position() + 1);
        parser.Push(autoOpen(parser, left, std::u16string(1, right)));
    }
}
std::u16string outputBraket(std::array<std::u16string, 3> const &args, bool star1, bool star2) {
    if (star1 && star2)
        return u"\\left\\langle{" + args[0] + u"}\\middle\\vert{" + args[1] + u"}\\middle\\vert{" +
               args[2] + u"}\\right\\rangle";
    if (star1)
        return u"\\langle{" + args[0] + u"}\\vert{" + args[1] + u"}\\vert{" + args[2] +
               u"}\\rangle";
    return u"\\left\\langle{" + args[0] + u"}\\right\\vert{" + args[1] + u"}\\left\\vert{" +
           args[2] + u"}\\right\\rangle";
}
std::size_t matrixDimension(double value) {
    // Source Array(length) has a finite integral uint32 length. Validate before native conversion.
    if (!std::isfinite(value) || value < 0 || std::trunc(value) != value ||
        value > std::numeric_limits<std::uint32_t>::max())
        throw std::length_error("Invalid matrix dimension");
    return static_cast<std::size_t>(value);
}
} // namespace

DispatchOutcome PhysicsMethods::Quantity(TexParser &parser, MacroInvocation const &invocation) {
    auto args = arguments(invocation);
    auto open = stringArg(args, 0, u"("), close = stringArg(args, 1, u")");
    const bool arg = flagArg(args, 2), star = arg ? parser.GetStar() : false;
    auto next = parser.GetNext();
    const auto position = parser.position();
    std::u16string big;
    if (next == u"\\") {
        parser.setPosition(parser.position() + 1);
        big = parser.GetCS();
        if (!bigFence(big)) {
            auto &empty = parser.create(NodeKind::mrow);
            parser.Push(&parse_util::fenced(parser.configuration, open, empty, close));
            parser.setPosition(position);
            return NoParseResult{};
        }
        next = parser.GetNext();
    }
    const auto right = closingFence(next);
    if (arg && next != u"{")
        missingArgument(parser);
    if (!right) {
        auto &empty = parser.create(NodeKind::mrow);
        parser.Push(&parse_util::fenced(parser.configuration, open, empty, close));
        parser.setPosition(position);
        return NoParseResult{};
    }
    auto named = stringArg(args, 3), variant = stringArg(args, 4);
    if (!named.empty()) {
        auto &function = parser.createToken(NodeKind::mi, named, operatorDefinition());
        if (!variant.empty())
            function.attributes().set(u"mathvariant", variant);
        parser.Push(parser.itemFactory().create(StackKind::fn, FnItemInit{function}));
    }
    if (next == u"{") {
        auto body = parser.GetArgument(invocation.dispatch_token).value();
        return pushNested(parser,
                          fencedSource(arg ? open : u"\\{", body, arg ? close : u"\\}", star, big));
    }
    parser.setPosition(parser.position() + 1);
    auto frame = autoOpen(parser, next, std::u16string(1, right));
    dynamic_cast<AutoOpen &>(*frame).big = std::move(big);
    parser.Push(std::move(frame));
    return NoParseResult{};
}
DispatchOutcome PhysicsMethods::Eval(TexParser &parser, MacroInvocation const &invocation) {
    const bool star = parser.GetStar();
    auto next = parser.GetNext();
    if (next == u"(" || next == u"[") {
        parser.setPosition(parser.position() + 1);
        auto frame = autoOpen(parser, next, u"|");
        auto &open = dynamic_cast<AutoOpen &>(*frame);
        open.smash = star;
        open.right = u"\\vphantom{\\int}";
        parser.Push(std::move(frame));
    } else {
        std::u16string replacement = u"\\left.\\vphantom{\\int}\\right|";
        if (next == u"{") {
            auto body = parser.GetArgument(invocation.dispatch_token).value();
            replacement = u"\\left." + (star ? u"\\smash{" + body + u"}" : body) +
                          u"\\vphantom{\\int}\\right|";
        }
        insertAtPosition(parser, std::move(replacement));
    }
    return NoParseResult{};
}
DispatchOutcome PhysicsMethods::Commutator(TexParser &parser, MacroInvocation const &invocation) {
    auto args = arguments(invocation);
    const bool star = parser.GetStar();
    auto next = parser.GetNext();
    std::u16string big;
    if (next == u"\\") {
        parser.setPosition(parser.position() + 1);
        big = parser.GetCS();
        if (!bigFence(big))
            missingArgument(parser);
        next = parser.GetNext();
    }
    if (next != u"{")
        missingArgument(parser);
    auto first = parser.GetArgument(invocation.dispatch_token).value();
    auto second = parser.GetArgument(invocation.dispatch_token).value();
    return pushNested(parser, fencedSource(stringArg(args, 0, u"["), first + u"," + second,
                                           stringArg(args, 1, u"]"), star, big));
}
DispatchOutcome PhysicsMethods::VectorBold(TexParser &parser, MacroInvocation const &invocation) {
    const bool star = parser.GetStar();
    auto source = parser.GetArgument(invocation.dispatch_token).value();
    auto &factory = parser.configuration.nodeFactory;
    auto environment = parser.stack.environmentScope();
    MmlNode *result{};
    {
        struct Restore {
            NodeFactory &factory;
            TokenCreator token;
            EnvironmentScope environment;
            std::optional<std::u16string> font;
            ~Restore() {
                environment->font = font && !font->empty() ? std::move(font) : std::nullopt;
                // Source removes these fields after nested parsing, even inside a vector
                // expression.
                environment->vector_font.reset();
                environment->vector_star.reset();
                factory.setCreators(NodeCreators{.token = token});
            }
        } restore{factory, factory.creators().token.value(), environment, environment->font};
        environment->font.reset();
        factory.setCreators(NodeCreators{.token = &createVectorToken});
        environment->vector_font = star ? u"bold-italic" : u"bold";
        environment->vector_star = star;
        result = &nested(parser, std::move(source));
    }
    parser.Push(result);
    return NoParseResult{};
}
DispatchOutcome PhysicsMethods::StarMacro(TexParser &parser, MacroInvocation const &invocation) {
    auto args = arguments(invocation);
    const bool star = parser.GetStar();
    const double count = std::get<double>(args[0]);
    std::vector<std::u16string> values;
    for (std::size_t i = 0; static_cast<double>(i) < count; ++i)
        values.push_back(parser.GetArgument(invocation.dispatch_token).value());
    std::u16string macro;
    for (std::size_t i = 1; i < args.size(); ++i) {
        if (star && i > 1)
            macro += u"*";
        macro += std::get<std::u16string>(args[i]);
    }
    macro = parse_util::substituteArgs(parser, values, macro);
    auto combined = parse_util::addArgs(parser, macro, parser.string().substr(parser.position()));
    parser.setString(std::move(combined));
    parser.setPosition(0);
    parse_util::checkMaxMacros(parser);
    return NoParseResult{};
}
DispatchOutcome PhysicsMethods::OperatorApplication(TexParser &parser,
                                                    MacroInvocation const &invocation) {
    vectorApplication(parser, invocation, false);
    return NoParseResult{};
}
DispatchOutcome PhysicsMethods::VectorOperator(TexParser &parser,
                                               MacroInvocation const &invocation) {
    vectorApplication(parser, invocation, true);
    return NoParseResult{};
}
DispatchOutcome PhysicsMethods::Expression(TexParser &parser, MacroInvocation const &invocation) {
    auto args = arguments(invocation);
    auto id = stringArg(args, 1);
    if (id.empty())
        id = std::u16string(invocation.dispatch_token.substr(1));
    auto exponent =
        flagArg(args, 0, true) ? parser.GetBrackets(invocation.dispatch_token) : std::nullopt;
    auto *function = &parser.createToken(NodeKind::mi, id, operatorDefinition());
    if (exponent && !exponent->empty()) {
        auto &power = nested(parser, *exponent);
        const std::array children{function, &power};
        function = &parser.create(NodeKind::msup, children);
    }
    parser.Push(parser.itemFactory().create(StackKind::fn, FnItemInit{*function}));
    if (parser.GetNext() == u"(") {
        parser.setPosition(parser.position() + 1);
        parser.Push(autoOpen(parser, u"(", u")"));
    }
    return NoParseResult{};
}
DispatchOutcome PhysicsMethods::Qqtext(TexParser &parser, MacroInvocation const &invocation) {
    const bool star = parser.GetStar();
    auto text = stringArg(arguments(invocation), 0);
    if (text.empty())
        text = parser.GetArgument(invocation.dispatch_token).value();
    insertAtPosition(parser,
                     (star ? std::u16string{} : u"\\quad") + u"\\text{" + text + u"}\\quad ");
    return NoParseResult{};
}
DispatchOutcome PhysicsMethods::Differential(TexParser &parser, MacroInvocation const &invocation) {
    auto exponent = parser.GetBrackets(invocation.dispatch_token);
    auto macro = stringArg(arguments(invocation), 0) + (exponent ? u"^{" + *exponent + u"}" : u" ");
    const bool parens = parser.GetNext() == u"(", braces = parser.GetNext() == u"{";
    if (!parens && !braces) {
        macro += parser.GetArgument(invocation.dispatch_token, true).value_or(u"");
        return pushNested(parser, std::move(macro));
    }
    if (braces) {
        macro += parser.GetArgument(invocation.dispatch_token).value();
        auto &content = nested(parser, std::move(macro));
        const std::array children{&content};
        parser.Push(&parser.create(NodeKind::tex_atom, children, operatorDefinition()));
    } else {
        pushNested(parser, std::move(macro));
        parser.setPosition(parser.position() + 1);
        parser.Push(autoOpen(parser, u"(", u")"));
    }
    return NoParseResult{};
}
DispatchOutcome PhysicsMethods::Derivative(TexParser &parser, MacroInvocation const &invocation) {
    auto args = arguments(invocation);
    const double maximum = std::get<double>(args[0]);
    const auto op = stringArg(args, 1);
    const bool star = parser.GetStar();
    auto exponent = parser.GetBrackets(invocation.dispatch_token);
    std::vector<std::u16string> values{parser.GetArgument(invocation.dispatch_token).value()};
    while (parser.GetNext() == u"{" && static_cast<double>(values.size()) < maximum)
        values.push_back(parser.GetArgument(invocation.dispatch_token).value());
    bool ignore = false;
    std::u16string power1 = u" ", power2 = u" ";
    if (maximum > 2 && values.size() > 2) {
        power1 = u"^{" + numberText(static_cast<double>(values.size() - 1)) + u"}";
        ignore = true;
    } else if (exponent) {
        ignore = maximum > 2 && values.size() > 1;
        power1 = power2 = u"^{" + *exponent + u"}";
    }
    auto first = values.size() > 1 ? values[0] : u"";
    auto second = values.size() > 1 ? values[1] : values[0];
    std::u16string rest;
    for (std::size_t i = 2; i < values.size() && !values[i].empty(); ++i)
        rest += op + u" " + values[i];
    auto fraction = star ? std::u16string(u"\\flatfrac") : u"\\frac";
    pushNested(parser, fraction + u"{" + op + power1 + first + u"}{" + op + u" " + second + power2 +
                           u" " + rest + u"}");
    if (parser.GetNext() == u"(") {
        parser.setPosition(parser.position() + 1);
        auto frame = autoOpen(parser, u"(", u")");
        dynamic_cast<AutoOpen &>(*frame).ignore = ignore;
        parser.Push(std::move(frame));
    }
    return NoParseResult{};
}
DispatchOutcome PhysicsMethods::Bra(TexParser &parser, MacroInvocation const &invocation) {
    const bool starBra = parser.GetStar();
    auto bra = parser.GetArgument(invocation.dispatch_token).value();
    std::u16string ket;
    bool hasKet = false, starKet = false;
    if (parser.GetNext() == u"\\") {
        auto saved = parser.position();
        parser.setPosition(saved + 1);
        auto cs = parser.GetCS();
        auto found = parser.lookup(HandlerKind::macro, cs);
        std::u16string_view token;
        if (auto macro = std::get_if<tex::Macro const *>(&found); macro && *macro)
            token = (*macro)->token;
        if (auto command = std::get_if<CommandDefinition const *>(&found); command && *command)
            token = (*command)->source_token;
        if (token == u"ket") {
            hasKet = true;
            saved = parser.position();
            starKet = parser.GetStar();
            if (parser.GetNext() == u"{")
                ket = parser.GetArgument(cs, true).value_or(u"");
            else {
                parser.setPosition(saved);
                starKet = false;
            }
        } else
            parser.setPosition(saved);
    }
    std::u16string macro;
    if (hasKet)
        macro = starBra || starKet
                    ? u"\\langle{" + bra + u"}\\vert{" + ket + u"}\\rangle"
                    : u"\\left\\langle{" + bra + u"}\\middle\\vert{" + ket + u"}\\right\\rangle";
    else
        macro = starBra ? u"\\langle{" + bra + u"}\\vert"
                        : u"\\left\\langle{" + bra + u"}\\right\\vert{" + ket + u"}";
    return pushNested(parser, std::move(macro));
}
DispatchOutcome PhysicsMethods::Ket(TexParser &parser, MacroInvocation const &invocation) {
    const bool star = parser.GetStar();
    auto ket = parser.GetArgument(invocation.dispatch_token).value();
    return pushNested(parser, star ? u"\\vert{" + ket + u"}\\rangle"
                                   : u"\\left\\vert{" + ket + u"}\\right\\rangle");
}
DispatchOutcome PhysicsMethods::BraKet(TexParser &parser, MacroInvocation const &invocation) {
    const bool star = parser.GetStar();
    auto bra = parser.GetArgument(invocation.dispatch_token).value();
    auto ket = parser.GetNext() == u"{" ? parser.GetArgument(invocation.dispatch_token, true)
                                        : std::nullopt;
    auto const &right = ket ? *ket : bra;
    return pushNested(parser, star ? u"\\langle{" + bra + u"}\\vert{" + right + u"}\\rangle"
                                   : u"\\left\\langle{" + bra + u"}\\middle\\vert{" + right +
                                         u"}\\right\\rangle");
}
DispatchOutcome PhysicsMethods::KetBra(TexParser &parser, MacroInvocation const &invocation) {
    const bool star = parser.GetStar();
    auto ket = parser.GetArgument(invocation.dispatch_token).value();
    auto bra = parser.GetNext() == u"{" ? parser.GetArgument(invocation.dispatch_token, true)
                                        : std::nullopt;
    auto const &right = bra ? *bra : ket;
    return pushNested(parser,
                      star ? u"\\vert{" + ket + u"}\\rangle\\!\\langle{" + right + u"}\\vert"
                           : u"\\left\\vert{" + ket + u"}\\middle\\rangle\\!\\middle\\langle{" +
                                 right + u"}\\right\\vert");
}
DispatchOutcome PhysicsMethods::Expectation(TexParser &parser, MacroInvocation const &invocation) {
    const bool star1 = parser.GetStar(), star2 = star1 && parser.GetStar();
    auto op = parser.GetArgument(invocation.dispatch_token).value();
    auto state = parser.GetNext() == u"{" ? parser.GetArgument(invocation.dispatch_token, true)
                                          : std::nullopt;
    return pushNested(parser, !op.empty() && state && !state->empty()
                                  ? outputBraket({*state, op, *state}, star1, star2)
                              : star1 ? u"\\langle {" + op + u"} \\rangle"
                                      : u"\\left\\langle {" + op + u"} \\right\\rangle");
}
DispatchOutcome PhysicsMethods::MatrixElement(TexParser &parser,
                                              MacroInvocation const &invocation) {
    const bool star1 = parser.GetStar(), star2 = star1 && parser.GetStar();
    std::array<std::u16string, 3> values;
    for (auto &value : values)
        value = parser.GetArgument(invocation.dispatch_token).value();
    return pushNested(parser, outputBraket(values, star1, star2));
}
DispatchOutcome PhysicsMethods::MatrixQuantity(TexParser &parser,
                                               MacroInvocation const &invocation) {
    const bool small = flagArg(arguments(invocation), 0), star = parser.GetStar();
    auto next = parser.GetNext();
    const std::u16string array = small ? u"smallmatrix" : u"array";
    std::u16string body, open, close;
    if (next == u"{")
        body = parser.GetArgument(invocation.dispatch_token).value();
    else if (next == u"(" || next == u"[" || next == u"|") {
        parser.setPosition(parser.position() + 1);
        open = next == u"(" ? (star ? u"\\lgroup" : u"(") : next;
        close = next == u"(" ? (star ? u"\\rgroup" : u")") : next == u"[" ? u"]" : u"|";
        const auto delimiter = next == u"(" ? u")" : next == u"[" ? u"]" : u"|";
        body = parser.GetUpTo(invocation.dispatch_token, delimiter);
    } else {
        open = u"(";
        close = u")";
    }
    return pushNested(parser, (open.empty() ? std::u16string{} : u"\\left") + open + u"\\begin{" +
                                  array + u"}{} " + body + u"\\end{" + array + u"}" +
                                  (open.empty() ? u"" : u"\\right") + close);
}
DispatchOutcome PhysicsMethods::IdentityMatrix(TexParser &parser,
                                               MacroInvocation const &invocation) {
    auto source = parser.GetArgument(invocation.dispatch_token).value();
    const double parsed = js_input::parseInt(source, js_input::IntegerRadix::Decimal);
    if (std::isnan(parsed))
        throw TexError("InvalidNumber", u"Invalid number");
    if (parsed <= 1) {
        replaceRemaining(parser, u"1");
        return NoParseResult{};
    }
    auto size = matrixDimension(parsed);
    std::u16string matrix;
    for (std::size_t i = 0; i < size; ++i) {
        if (i)
            matrix += u"\\\\ ";
        for (std::size_t j = 0; j < size; ++j) {
            if (j)
                matrix += u" & ";
            matrix += i == j ? u"1" : u"0";
        }
    }
    replaceRemaining(parser, std::move(matrix));
    return NoParseResult{};
}
DispatchOutcome PhysicsMethods::XMatrix(TexParser &parser, MacroInvocation const &invocation) {
    const bool star = parser.GetStar();
    auto fill = parser.GetArgument(invocation.dispatch_token).value();
    auto ns = parser.GetArgument(invocation.dispatch_token).value();
    auto ms = parser.GetArgument(invocation.dispatch_token).value();
    double nd = js_input::parseInt(ns, js_input::IntegerRadix::Decimal);
    double md = js_input::parseInt(ms, js_input::IntegerRadix::Decimal);
    if (std::isnan(nd) || std::isnan(md) || numberText(nd) != ns || numberText(md) != ms)
        throw TexError("InvalidNumber", u"Invalid number");
    const auto n = matrixDimension(std::max(nd, 1.0)), m = matrixDimension(std::max(md, 1.0));
    std::u16string matrix;
    for (std::size_t i = 1; i <= n; ++i) {
        if (i > 1)
            matrix += u"\\\\ ";
        for (std::size_t j = 1; j <= m; ++j) {
            if (j > 1)
                matrix += u" & ";
            matrix += fill;
            if (star && n > 1 && m > 1)
                matrix += u"_{{" + numberText(static_cast<double>(i)) + u"}{" +
                          numberText(static_cast<double>(j)) + u"}}";
            else if (star && (n > 1 || m > 1))
                matrix += u"_{" + numberText(static_cast<double>(n > 1 ? i : j)) + u"}";
        }
    }
    replaceRemaining(parser, std::move(matrix));
    return NoParseResult{};
}
DispatchOutcome PhysicsMethods::PauliMatrix(TexParser &parser, MacroInvocation const &invocation) {
    auto arg = parser.GetArgument(invocation.dispatch_token).value();
    std::u16string matrix = arg.empty() ? u"" : arg.substr(1);
    if (!arg.empty())
        switch (arg[0]) {
        case u'0':
            matrix += u" 1 & 0\\\\ 0 & 1";
            break;
        case u'1':
        case u'x':
            matrix += u" 0 & 1\\\\ 1 & 0";
            break;
        case u'2':
        case u'y':
            matrix += u" 0 & -i\\\\ i & 0";
            break;
        case u'3':
        case u'z':
            matrix += u" 1 & 0\\\\ 0 & -1";
            break;
        default:
            break;
        }
    replaceRemaining(parser, std::move(matrix));
    return NoParseResult{};
}
DispatchOutcome PhysicsMethods::DiagonalMatrix(TexParser &parser,
                                               MacroInvocation const &invocation) {
    const bool anti = flagArg(arguments(invocation), 0);
    if (parser.GetNext() != u"{")
        return NoParseResult{};
    const auto start = parser.position();
    (void)parser.GetArgument(invocation.dispatch_token);
    const auto end = parser.position();
    parser.setPosition(start + 1);
    std::vector<std::u16string> elements;
    auto current = parser.position();
    while (current < end) {
        try {
            auto element = parser.GetUpTo(invocation.dispatch_token, u",");
            current = parser.position();
            elements.push_back(std::move(element));
        } catch (...) {
            parser.setPosition(end);
            elements.emplace_back(parser.string().substr(current, end - 1 - current));
            break;
        }
    }
    std::u16string matrix;
    for (std::size_t i = 0; i < elements.size(); ++i) {
        if (i)
            matrix += u"\\\\ ";
        matrix.append(anti ? elements.size() - i - 1 : i, u'&');
        matrix += u"\\mqty{" + elements[i] + u"}";
    }
    matrix += parser.string().substr(end);
    parser.setString(std::move(matrix));
    parser.setPosition(0);
    return NoParseResult{};
}
DispatchOutcome PhysicsMethods::AutoClose(TexParser &parser, MacroInvocation const &invocation) {
    auto *top = parser.stack.Top();
    if (top && top->isKind(StackKind::over))
        top = parser.stack.Top(2);
    auto *open = dynamic_cast<AutoOpen *>(top);
    if (!open || !open->closing(invocation.dispatch_token))
        return ContinueDispatch{};
    NodeDefinition definition;
    auto args = arguments(invocation);
    if (!args.empty())
        definition.properties.tex.explicit_tex_class =
            static_cast<mml::TexClass>(static_cast<int>(std::get<double>(args[0])));
    auto &fence =
        parser.createToken(NodeKind::mo, invocation.dispatch_token, std::move(definition));
    auto close = parser.itemFactory().create(StackKind::close);
    dynamic_cast<CloseItem &>(*close).pre_autoclose = true;
    parser.Push(std::move(close));
    auto item = parser.itemFactory().create(StackKind::mml, MmlItemInit{fence});
    dynamic_cast<MmlItem &>(*item).autoclose = true;
    parser.Push(std::move(item));
    return Handled{};
}
DispatchOutcome PhysicsMethods::Vnabla(TexParser &parser, MacroInvocation const &) {
    const bool arrow =
        parser.options().physics && parser.options().physics->arrowdel.value_or(false);
    return pushNested(parser, arrow ? u"\\vec{\\gradientnabla}" : u"{\\gradientnabla}");
}
DispatchOutcome PhysicsMethods::DiffD(TexParser &parser, MacroInvocation const &) {
    const bool italic =
        parser.options().physics && parser.options().physics->italicdiff.value_or(false);
    return pushNested(parser, italic ? u"d" : u"{\\rm d}");
}
} // namespace folia::math::tex
