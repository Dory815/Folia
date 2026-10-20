// MathJax 4.1.3 input/tex/extpfeil/ExtpfeilConfiguration.ts; Apache-2.0.
module folia.math.input.tex.extpfeil.configuration;
import folia.math.input.tex.newcommand.configuration;
import folia.math.input.tex.newcommand.util;
import folia.math.util.js_numeric_input;
import std;

namespace folia::math::tex {
DispatchOutcome ExtpfeilMethods::NewExtArrow(TexParser &parser, MacroInvocation const &invocation) {
    const auto control = parser.GetArgument(invocation.dispatch_token).value();
    const auto spacing = parser.GetArgument(invocation.dispatch_token).value();
    const auto character = parser.GetArgument(invocation.dispatch_token).value();
    const std::array<std::u16string, 1> name{std::u16string(invocation.dispatch_token)};
    if (!regexp_host::RegExp(u"^\\\\([a-z]+|.)$", u"i").exec(control))
        throw TexError("NewextarrowArg1", u"First argument to %1 must be a control sequence name",
                       name);
    if (!regexp_host::RegExp(u"^(\\d+),(\\d+)$").exec(spacing))
        throw TexError("NewextarrowArg2",
                       u"Second argument to %1 must be two integers separated by a comma", name);
    if (!regexp_host::RegExp(u"^(\\d+|0x[0-9A-F]+)$", u"i").exec(character))
        throw TexError("NewextarrowArg3",
                       u"Third argument to %1 must be a unicode character number", name);
    const auto command = control.substr(1);
    const auto comma = spacing.find(u',');
    const auto code = js_input::parseInt(character);
    const auto left = js_input::parseInt(std::u16string_view(spacing).substr(0, comma));
    const auto right = js_input::parseInt(std::u16string_view(spacing).substr(comma + 1));
    const auto id = parser.configuration.callbacks().macroBinding(&AmsMethods::xArrow);
    NewcommandUtil::addMacro(parser, command,
                             CommandDefinition{command, NativeMacroAlias{id, {code, left, right}}});
    parser.Push(parser.itemFactory().create(StackKind::null_item));
    return NoParseResult{};
}
ConfigurationRef ExtpfeilConfiguration::initializeModule(ConfigurationHandler &registry,
                                                         MapHandler &maps) {
    maps.registerMap(std::make_shared<CommandMap>(CommandMapInit{
        MacroMapInit{{u"extpfeil", std::nullopt},
                     {Macro(u"xtwoheadrightarrow", ExtpfeilMethods::xArrow, {8608.0, 12.0, 16.0}),
                      Macro(u"xtwoheadleftarrow", ExtpfeilMethods::xArrow, {8606.0, 17.0, 13.0}),
                      Macro(u"xmapsto", ExtpfeilMethods::xArrow, {8614.0, 6.0, 7.0}),
                      Macro(u"xlongequal", ExtpfeilMethods::xArrow, {61.0, 7.0, 7.0}),
                      Macro(u"xtofrom", ExtpfeilMethods::xArrow, {8644.0, 12.0, 12.0}),
                      Macro(u"Newextarrow", &ExtpfeilMethods::NewExtArrow)}}}));
    ConfigurationDefinition definition;
    definition.handler = {{HandlerKind::macro, {u"extpfeil"}}};
    definition.config = ProcessorSpec<ConfigMethod>{&NewcommandConfig};
    return Configuration::create(registry, u"extpfeil", std::move(definition));
}
} // namespace folia::math::tex
