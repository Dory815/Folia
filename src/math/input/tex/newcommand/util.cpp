/*************************************************************
 * Copyright (c) 2009-2026 The MathJax Consortium.
 * MathJax 4.1.3 input/tex/newcommand/NewcommandUtil.ts; Apache-2.0.
 *************************************************************/
module folia.math.input.tex.newcommand.util;
import folia.math.input.tex.unit_util;
import folia.math.input.tex.begingroup.stack;
import std;
namespace folia::math::tex {
namespace {
[[noreturn]] void error(std::string id, std::u16string format, std::u16string_view arg) {
    const std::array<std::u16string, 1> args{std::u16string(arg)};
    throw TexError(std::move(id), std::move(format), args);
}
bool match(std::u16string pattern, std::u16string_view text) {
    return regexp_host::RegExp(std::move(pattern)).exec(std::u16string(text)).has_value();
}
std::u16string trim(std::u16string_view text) {
    return UnitUtil::instance().trimSpaces(text);
}
} // namespace
std::u16string NewcommandUtil::GetCSname(TexParser &p, std::u16string_view cmd) {
    if (p.GetNext() != u"\\")
        error("MissingCS", u"%1 must be followed by a control sequence", cmd);
    auto name = trim(p.GetArgument(cmd).value()).substr(1);
    checkProtectedMacros(p, name);
    return name;
}
std::u16string NewcommandUtil::GetCsNameArgument(TexParser &p, std::u16string_view cmd) {
    auto name = trim(p.GetArgument(cmd).value());
    if (name.starts_with(u"\\"))
        name.erase(0, 1);
    if (!match(u"^(.|[a-zA-Z]+)$", name))
        error("IllegalControlSequenceName", u"Illegal control sequence name for %1", cmd);
    checkProtectedMacros(p, name);
    return name;
}
std::optional<std::u16string> NewcommandUtil::GetArgCount(TexParser &p, std::u16string_view cmd) {
    auto count = p.GetBrackets(cmd);
    if (count && !count->empty()) {
        *count = trim(*count);
        if (!match(u"^[0-9]+$", *count))
            error("IllegalParamNumber", u"Illegal number of parameters specified in %1", cmd);
    }
    return count;
}
MacroDefinition NewcommandUtil::GetTemplate(TexParser &p, std::u16string_view cmd,
                                            std::u16string_view cs) {
    p.GetNext();
    std::vector<std::optional<std::u16string>> params;
    std::size_t count{}, start = p.position();
    auto finish = [&](bool templated) {
        MacroDefinition result;
        result.arity = static_cast<double>(count);
        result.parameter_template = templated;
        result.substitute_parameters = count != 0;
        if (templated) {
            if (!params.empty())
                result.parameter_prefix = params[0].value_or(u"");
            result.parameter_delimiters.resize(count);
            for (std::size_t i = 0; i < count; ++i)
                if (i + 1 < params.size())
                    result.parameter_delimiters[i] = params[i + 1].value_or(u"");
        }
        return result;
    };
    while (p.position() < p.string().size()) {
        auto c = p.GetNext();
        if (c == u"#") {
            if (start != p.position()) {
                params.resize(count + 1);
                params[count] = p.string().substr(start, p.position() - start);
            }
            p.setPosition(p.position() + 1);
            auto at = p.position();
            auto digit = at < p.string().size() ? p.string()[at] : char16_t{};
            if (digit < u'1' || digit > u'9')
                error("CantUseHash2", u"Illegal use of # in template for %1", cs);
            if (static_cast<std::size_t>(digit - u'0') != ++count)
                error("SequentialParam", u"Parameters for %1 must be numbered sequentially", cs);
            start = p.position() + 1;
        } else if (c == u"{") {
            if (start != p.position()) {
                params.resize(count + 1);
                params[count] = p.string().substr(start, p.position() - start);
                auto trailing = *params[count];
                auto notSpace = trailing.find_first_not_of(u' ');
                bool priorEmpty = true;
                for (std::size_t n = 0; n < count && n < params.size(); ++n)
                    if (params[n] && !params[n]->empty())
                        priorEmpty = false;
                if (notSpace == trailing.npos && priorEmpty)
                    return finish(false);
            }
            return finish(!params.empty());
        }
        p.setPosition(p.position() + 1);
    }
    error("MissingReplacementString", u"Missing replacement string for definition of %1", cmd);
}
std::u16string NewcommandUtil::GetParameter(TexParser &p, std::u16string_view name,
                                            std::optional<std::u16string_view> param) {
    if (!param)
        return p.GetArgument(name).value();
    auto start = p.position();
    std::size_t length{};
    bool hasBraces{};
    while (p.position() < p.string().size()) {
        auto c = p.string()[p.position()];
        if (c == u'{') {
            hasBraces = p.position() == start;
            p.GetArgument(name);
            length = p.position() - start;
        } else if (MatchParam(p, *param)) {
            if (hasBraces) {
                ++start;
                length -= 2;
            }
            return std::u16string(p.string().substr(start, length));
        } else if (c == u'\\') {
            p.setPosition(p.position() + 1);
            ++length;
            hasBraces = false;
            auto rest = std::u16string(p.string().substr(p.position()));
            auto next = regexp_host::RegExp(u"[a-z]+|.", u"i").exec(rest);
            if (next) {
                p.setPosition(p.position() + next->at(0)->size());
                length = p.position() - start;
            }
        } else {
            p.setPosition(p.position() + 1);
            ++length;
            hasBraces = false;
        }
    }
    error("RunawayArgument", u"Runaway argument for %1?", name);
}
bool NewcommandUtil::MatchParam(TexParser &p, std::u16string_view param) {
    if (p.string().substr(p.position(), param.size()) != param)
        return false;
    if (match(u"\\\\[a-zA-Z]+$", param)) {
        auto at = p.position() + param.size();
        auto c = at < p.string().size() ? p.string()[at] : char16_t{};
        if ((c >= u'a' && c <= u'z') || (c >= u'A' && c <= u'Z'))
            return false;
    }
    p.setPosition(p.position() + param.size());
    return true;
}
DefinitionLayer &NewcommandUtil::checkGlobal(TexParser &p, std::span<DefinitionKey const> keys) {
    auto &session = p.configuration.packageData;
    if (p.stack.env().is_global.value_or(false))
        return BegingroupStack(session.definitions()).checkGlobal(keys);
    return session.currentDefinitionLayer();
}
void NewcommandUtil::checkProtectedMacros(TexParser &p, std::u16string_view cs) {
    auto const &names = p.options().protectedMacros;
    if (names && std::ranges::find(*names, cs) != names->end())
        error("ProtectedMacro", u"The control sequence %1 can't be redefined",
              u"\\" + std::u16string(cs));
}
void NewcommandUtil::addDelimiter(TexParser &p, std::u16string cs,
                                  std::optional<std::u16string> character,
                                  std::optional<mml::AttributeSet> attributes) {
    auto name = cs.substr(1);
    checkProtectedMacros(p, name);
    const std::array keys{DefinitionKey{DefinitionKind::command, name},
                          DefinitionKey{DefinitionKind::delimiter, cs}};
    auto &layer = checkGlobal(p, keys);
    if (name != cs)
        layer.commands.erase(name);
    layer.delimiters.insert_or_assign(cs, Token(cs, std::move(character), std::move(attributes)));
    p.stack.env().is_global.reset();
}
void NewcommandUtil::addMacro(TexParser &p, std::u16string cs, CommandDefinition definition) {
    checkProtectedMacros(p, cs);
    const std::array keys{DefinitionKey{DefinitionKind::command, cs}};
    auto &layer = checkGlobal(p, keys);
    undefineDelimiter(p, u"\\" + cs);
    if (definition.source_token.empty())
        definition.source_token = cs;
    definition.validate();
    layer.commands.insert_or_assign(std::move(cs), std::move(definition));
    p.stack.env().is_global.reset();
}
void NewcommandUtil::addEnvironment(TexParser &p, std::u16string name,
                                    EnvironmentDefinition definition) {
    const std::array keys{DefinitionKey{DefinitionKind::environment, name}};
    auto &layer = checkGlobal(p, keys);
    layer.environments.insert_or_assign(std::move(name), std::move(definition));
    p.stack.env().is_global.reset();
}
void NewcommandUtil::undefineMacro(TexParser &p, std::u16string_view cs) {
    const std::array keys{DefinitionKey{DefinitionKind::command, std::u16string(cs)}};
    auto &layer = checkGlobal(p, keys);
    layer.commands.erase(std::u16string(cs));
    if (p.configuration.handlers.get(HandlerKind::macro)->applicable(cs)) {
        layer.commands.insert_or_assign(std::u16string(cs),
                                        CommandDefinition{std::u16string(cs), MacroSuppression{}});
        undefineDelimiter(p, u"\\" + std::u16string(cs));
    }
    p.stack.env().is_global.reset();
}
void NewcommandUtil::undefineDelimiter(TexParser &p, std::u16string_view cs) {
    const std::array keys{DefinitionKey{DefinitionKind::delimiter, std::u16string(cs)}};
    auto &layer = checkGlobal(p, keys);
    layer.delimiters.erase(std::u16string(cs));
    if (p.configuration.handlers.get(HandlerKind::delimiter)->applicable(cs))
        layer.delimiters.insert_or_assign(
            std::u16string(cs), Token(std::u16string(cs), std::nullopt, mml::AttributeSet{}));
    p.stack.env().is_global.reset();
}
} // namespace folia::math::tex
