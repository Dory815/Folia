// MathJax 4.1.3 input/tex/mathtools/MathtoolsUtil.ts; Apache-2.0.
module folia.math.input.tex.mathtools.util;
import folia.math.input.tex.unit_util;
import folia.math.input.tex.newcommand.util;
import folia.math.input.tex.mathtools.methods;
import folia.math.util.js_numeric_input;
import std;
namespace folia::math::tex {
void MathtoolsUtil::setDisplayLevel(mml::MmlNode &node, std::u16string_view style) {
    bool display = false;
    double script = 0;
    if (style == u"\\displaystyle")
        display = true;
    else if (style == u"\\textstyle") {
    } else if (style == u"\\scriptstyle")
        script = 1;
    else if (style == u"\\scriptscriptstyle")
        script = 2;
    else
        return;
    node.attributes().set(u"displaystyle", display);
    node.attributes().set(u"scriptlevel", script);
}
EqnArrayItem &MathtoolsUtil::checkAlignment(TexParser &parser, std::u16string_view name) {
    auto top = dynamic_cast<EqnArrayItem *>(parser.stack.Top());
    if (!top || top->kind() != StackKind::eqnarray)
        throw TexError("NotInAlignment", u"%1 can only be used in aligment environments",
                       std::array{std::u16string(name)});
    return *top;
}
void MathtoolsUtil::addPairedDelims(TexParser &parser, std::u16string cs,
                                    std::vector<TexArgument> args) {
    auto macros = parser.configuration.handlers.get(HandlerKind::macro);
    if (macros && macros->contains(cs))
        throw TexError("CommadExists", u"Command %1 already defined", std::array{u"\\" + cs});
    auto id = parser.configuration.callbacks().macroBinding(&MathtoolsMethods::PairedDelimiters);
    if (!id.value)
        throw std::logic_error("PairedDelimiters is absent from the compiled callback table");
    CommandDefinition definition{cs, NativeMacroAlias{id, std::move(args)}};
    NewcommandUtil::addMacro(parser, std::move(cs), std::move(definition));
}
void MathtoolsUtil::spreadLines(mml::MmlNode &table, std::u16string_view spread) {
    if (!table.isKind(mml::NodeKind::mtable))
        return;
    const auto value = table.attributes().lookup(u"rowspacing");
    if (!value || !std::holds_alternative<std::u16string>(*value))
        throw std::invalid_argument("Mathtools rowspacing must be a string");
    const auto spacing = std::get<std::u16string>(*value);
    const auto add = UnitUtil::instance().dimen2em(spread);
    std::u16string result;
    std::size_t begin = 0;
    while (true) {
        auto end = spacing.find(u' ', begin);
        if (end == std::u16string::npos)
            end = spacing.size();
        auto dimension =
            UnitUtil::instance().dimen2em(std::u16string_view(spacing).substr(begin, end - begin));
        if (begin)
            result += u' ';
        auto text = UnitUtil::instance().em(std::max(0.0, dimension + add));
        result.append(text.begin(), text.end());
        if (end == spacing.size())
            break;
        begin = end + 1;
    }
    table.attributes().set(u"rowspacing", std::move(result));
}
std::u16string MathtoolsUtil::plusOrMinus(std::u16string_view name, std::u16string value) {
    while (!value.empty() && js_input::isWhitespace(value.front()))
        value.erase(value.begin());
    while (!value.empty() && js_input::isWhitespace(value.back()))
        value.pop_back();
    std::size_t index = 0;
    if (index < value.size() && (value[index] == u'+' || value[index] == u'-'))
        ++index;
    bool digit = false;
    while (index < value.size() && value[index] >= u'0' && value[index] <= u'9') {
        digit = true;
        ++index;
    }
    if (index < value.size() && value[index] == u'.') {
        ++index;
        while (index < value.size() && value[index] >= u'0' && value[index] <= u'9') {
            digit = true;
            ++index;
        }
    }
    if (!digit || index != value.size())
        throw TexError("NotANumber", u"Argument to %1 is not a number",
                       std::array{std::u16string(name)});
    if (value.front() != u'+' && value.front() != u'-')
        value.insert(value.begin(), u'+');
    return value;
}
mml::MmlNode &MathtoolsUtil::getScript(TexParser &parser, std::u16string_view name,
                                       std::u16string_view position) {
    auto source = UnitUtil::instance().trimSpaces(parser.GetArgument(name).value());
    if (source.empty())
        return parser.create(mml::NodeKind::none);
    auto const &options = parser.options().mathtools.value();
    auto format = position == u"sub"   ? &options.prescriptSubFormat
                  : position == u"sup" ? &options.prescriptSupFormat
                  : position == u"arg" ? &options.prescriptArgFormat
                                       : nullptr;
    if (!format)
        throw std::invalid_argument("Unknown Mathtools prescript position");
    if (format->has_value() && !format->value().empty())
        source = format->value() + u"{" + source + u"}";
    TexParser nested(std::move(source), ParserEnvironmentSeed{parser.stack.env()},
                     parser.configuration);
    auto result = nested.mml();
    if (!result)
        throw std::logic_error("Prescript parser returned no MML");
    return result->isKind(mml::NodeKind::tex_atom) && result->isEmpty()
               ? parser.create(mml::NodeKind::none)
               : *result;
}
} // namespace folia::math::tex
