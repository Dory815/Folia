/*************************************************************
 * Copyright (c) 2018-2026 The MathJax Consortium.
 * MathJax 4.1.3 input/tex/newcommand/NewcommandMethods.ts; Apache-2.0.
 *************************************************************/
module folia.math.input.tex.newcommand.methods;
import folia.math.input.tex.newcommand.util;
import folia.math.input.tex.newcommand.items;
import folia.math.input.tex.parse_util;
import folia.math.input.tex.unit_util;
import std;
namespace folia::math::tex {
namespace {
using Arguments = std::span<TexArgument const>;
std::u16string text(Arguments args, std::size_t i) {
    if (i >= args.size() || std::holds_alternative<NullArgument>(args[i]))
        return {};
    return std::get<std::u16string>(args[i]);
}
bool present(Arguments args, std::size_t i) {
    return i < args.size() && !std::holds_alternative<NullArgument>(args[i]);
}
bool flag(Arguments args, std::size_t i) {
    if (!present(args, i))
        return false;
    if (auto string = std::get_if<std::u16string>(&args[i]))
        return !string->empty();
    if (auto number = std::get_if<double>(&args[i]))
        return *number != 0 && !std::isnan(*number);
    return std::get<bool>(args[i]);
}
double number(std::u16string_view value) {
    if (value.empty())
        return 0;
    std::string narrow(value.begin(), value.end());
    char *end{};
    auto result = std::strtod(narrow.c_str(), &end);
    return end == narrow.c_str() ? std::numeric_limits<double>::quiet_NaN() : result;
}
double number(Arguments args, std::size_t i) {
    if (!present(args, i))
        return 0;
    if (auto numeric = std::get_if<double>(&args[i]))
        return *numeric;
    return number(std::get<std::u16string>(args[i]));
}
[[noreturn]] void error(std::string id, std::u16string format, std::u16string_view argument) {
    const std::array<std::u16string, 1> args{std::u16string(argument)};
    throw TexError(std::move(id), std::move(format), args);
}
DispatchOutcome pushNull(TexParser &p) {
    p.Push(p.itemFactory().create(StackKind::null_item));
    return NoParseResult{};
}
} // namespace
DispatchOutcome NewcommandMethods::NewCommand(TexParser &p, MacroInvocation const &invocation) {
    auto name = invocation.dispatch_token;
    auto cs = NewcommandUtil::GetCsNameArgument(p, name);
    auto count = NewcommandUtil::GetArgCount(p, name);
    auto optional = p.GetBrackets(name);
    MacroDefinition definition;
    definition.replacement_source = p.GetArgument(name).value();
    definition.arity = count ? number(*count) : 0;
    definition.default_first_argument = optional;
    definition.substitute_parameters = count && !count->empty();
    NewcommandUtil::addMacro(p, cs, CommandDefinition{cs, std::move(definition)});
    return pushNull(p);
}
DispatchOutcome NewcommandMethods::NewEnvironment(TexParser &p, MacroInvocation const &invocation) {
    auto name = invocation.dispatch_token;
    auto env = UnitUtil::instance().trimSpaces(p.GetArgument(name).value());
    auto count = NewcommandUtil::GetArgCount(p, name);
    auto optional = p.GetBrackets(name);
    auto begin = p.GetArgument(name).value();
    auto end = p.GetArgument(name).value();
    NewcommandUtil::addEnvironment(
        p, env,
        EnvironmentDefinition{std::move(begin), std::move(end), count ? number(*count) : 0,
                              std::move(optional), count && !count->empty()});
    return pushNull(p);
}
DispatchOutcome NewcommandMethods::MacroDef(TexParser &p, MacroInvocation const &invocation) {
    auto name = invocation.dispatch_token;
    auto cs = NewcommandUtil::GetCSname(p, name);
    auto definition = NewcommandUtil::GetTemplate(p, name, u"\\" + cs);
    definition.replacement_source = p.GetArgument(name).value();
    NewcommandUtil::addMacro(p, cs, CommandDefinition{cs, std::move(definition)});
    return pushNull(p);
}
DispatchOutcome NewcommandMethods::Let(TexParser &p, MacroInvocation const &invocation) {
    auto name = invocation.dispatch_token;
    auto cs = NewcommandUtil::GetCSname(p, name);
    auto c = p.GetNext();
    if (c == u"=") {
        p.setPosition(p.position() + 1);
        c = p.GetNext();
    }
    auto &handlers = p.configuration.handlers;
    p.Push(p.itemFactory().create(StackKind::null_item));
    if (c == u"\\") {
        auto target = NewcommandUtil::GetCSname(p, name);
        if (cs == target)
            return NoParseResult{};
        auto map = handlers.get(HandlerKind::macro)->applicable(target);
        if (dynamic_cast<MacroMap *>(map.get())) {
            auto lookup = map->lookup(target);
            if (auto command = std::get_if<CommandDefinition const *>(&lookup);
                command && *command) {
                auto definition = **command;
                NewcommandUtil::addMacro(p, cs, std::move(definition));
                return NoParseResult{};
            }
            auto const &macro = **std::get_if<tex::Macro const *>(&lookup);
            auto method = std::get<MacroHandler>(macro.function);
            auto binding = p.configuration.callbacks().macroBinding(method);
            NewcommandUtil::addMacro(
                p, cs, CommandDefinition{macro.token, NativeMacroAlias{binding, macro.arguments}});
            return NoParseResult{};
        }
        if (auto characterMap = dynamic_cast<CharacterMap *>(map.get());
            characterMap && !dynamic_cast<DelimiterMap *>(map.get())) {
            auto lookup = characterMap->lookup(target);
            auto captured = **std::get_if<Token const *>(&lookup);
            auto method = std::get<TokenInputHandler>(*characterMap->parser());
            auto binding = p.configuration.callbacks().tokenBinding(method);
            NewcommandUtil::addMacro(
                p, cs, CommandDefinition{cs, CharacterMacroAlias{binding, std::move(captured)}});
            return NoParseResult{};
        }
        auto lookup = handlers.get(HandlerKind::delimiter)->lookup(u"\\" + target);
        if (auto delimiter = std::get_if<Token const *>(&lookup); delimiter && *delimiter) {
            Token captured = **delimiter;
            NewcommandUtil::addDelimiter(p, u"\\" + cs, captured.character, captured.attributes);
            return NoParseResult{};
        }
        NewcommandUtil::checkProtectedMacros(p, cs);
        NewcommandUtil::undefineMacro(p, cs);
        NewcommandUtil::undefineDelimiter(p, u"\\" + cs);
        return NoParseResult{};
    }
    p.setPosition(p.position() + 1);
    auto lookup = handlers.get(HandlerKind::delimiter)->lookup(c);
    if (auto delimiter = std::get_if<Token const *>(&lookup); delimiter && *delimiter) {
        Token captured = **delimiter;
        NewcommandUtil::addDelimiter(p, u"\\" + cs, captured.character, captured.attributes);
        return NoParseResult{};
    }
    MacroDefinition definition;
    definition.replacement_source = c;
    NewcommandUtil::addMacro(p, cs, CommandDefinition{cs, std::move(definition)});
    return NoParseResult{};
}
DispatchOutcome NewcommandMethods::MacroWithTemplate(TexParser &p,
                                                     MacroInvocation const &invocation) {
    MacroDefinition definition;
    if (auto owned =
            std::get_if<std::reference_wrapper<CommandDefinition const>>(&invocation.arguments))
        definition = std::get<MacroDefinition>(owned->get().body);
    else {
        auto args = std::get<Arguments>(invocation.arguments);
        definition.replacement_source = text(args, 0);
        definition.arity = std::trunc(number(args, 1));
        definition.parameter_template = args.size() > 2;
        if (definition.parameter_template) {
            definition.parameter_prefix = text(args, 2);
            for (std::size_t i = 0; static_cast<double>(i) < definition.arity; ++i)
                definition.parameter_delimiters.push_back(text(args, i + 3));
        }
    }
    auto replacement = definition.replacement_source;
    if (definition.parameter_template) {
        std::vector<std::u16string> args;
        p.GetNext();
        if (!definition.parameter_prefix.empty() &&
            !NewcommandUtil::MatchParam(p, definition.parameter_prefix))
            error("MismatchUseDef", u"Use of %1 doesn't match its definition",
                  invocation.dispatch_token);
        if (definition.arity != 0 && !std::isnan(definition.arity)) {
            for (std::size_t i = 0; static_cast<double>(i) < definition.arity; ++i) {
                auto delimiter = i < definition.parameter_delimiters.size()
                                     ? std::u16string_view(definition.parameter_delimiters[i])
                                     : std::u16string_view{};
                args.push_back(NewcommandUtil::GetParameter(
                    p, invocation.dispatch_token,
                    delimiter.empty() ? std::nullopt
                                      : std::optional<std::u16string_view>(delimiter)));
            }
            replacement = parse_util::substituteArgs(p, args, replacement);
        }
    }
    p.setString(parse_util::addArgs(p, replacement, p.string().substr(p.position())));
    p.setPosition(0);
    parse_util::checkMaxMacros(p);
    return NoParseResult{};
}
StackEntry NewcommandMethods::BeginEnv(TexParser &p, FrameHandle const &begin, Arguments args) {
    auto name = std::u16string(begin->getName().value_or(u"undefined"));
    auto bdef = text(args, 0), edef = text(args, 1);
    auto count = number(args, 2);
    if (p.stack.env().closing_environment == name) {
        p.stack.env().closing_environment.reset();
        auto &depth = p.stack.global().begin_env_depth;
        if (depth.value_or(0)) {
            --*depth;
            if (!edef.empty()) {
                auto rest = std::u16string(p.string().substr(p.position()));
                p.setString(parse_util::addArgs(p, p.string().substr(0, p.position()), edef));
                p.Parse();
                p.setString(std::move(rest));
                p.setPosition(0);
            }
        }
        auto result = p.itemFactory().create(StackKind::end);
        dynamic_cast<EndItem &>(*result).name = name;
        return result;
    }
    if (flag(args, 2)) {
        std::vector<std::u16string> arguments;
        auto command = u"\\begin{" + name + u"}";
        if (present(args, 3)) {
            auto optional = p.GetBrackets(command);
            arguments.push_back(optional ? *optional : text(args, 3));
        }
        for (double i = static_cast<double>(arguments.size()); i < count; ++i)
            arguments.push_back(p.GetArgument(command).value());
        bdef = parse_util::substituteArgs(p, arguments, bdef);
        edef = parse_util::substituteArgs(p, {}, edef);
    }
    p.setString(parse_util::addArgs(p, bdef, p.string().substr(p.position())));
    p.setPosition(0);
    p.stack.global().begin_env_depth = p.stack.global().begin_env_depth.value_or(0) + 1;
    auto result = p.itemFactory().create(StackKind::begin_env);
    dynamic_cast<BeginEnvItem &>(*result).name = name;
    return result;
}
} // namespace folia::math::tex
