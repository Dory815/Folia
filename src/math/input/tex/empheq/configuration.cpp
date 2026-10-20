// MathJax 4.1.3 input/tex/empheq/EmpheqConfiguration.ts; Apache-2.0.
module folia.math.input.tex.empheq.configuration;
import folia.math.input.tex.empheq.util;
import folia.math.input.tex.parse_methods;
import std;

namespace folia::math::tex {
StackEntry EmpheqMethods::Empheq(TexParser &parser, FrameHandle const &frame,
                                 std::span<TexArgument const>) {
    auto &begin = dynamic_cast<BeginItem &>(*frame);
    auto name = std::u16string(begin.getName().value_or(u"undefined"));
    if (parser.stack.env().closing_environment == name) {
        parser.stack.env().closing_environment.reset();
        auto end = parser.itemFactory().create(StackKind::end);
        dynamic_cast<EndItem &>(*end).name = parser.stack.global().empheq;
        parser.Push(std::move(end));
        parser.stack.global().empheq = u"";
        auto empheq = dynamic_cast<BeginItem *>(parser.stack.Top());
        if (!empheq)
            throw std::logic_error("Empheq environment has no active begin frame");
        EmpheqUtil::adjustTable(*empheq, parser);
        end = parser.itemFactory().create(StackKind::end);
        dynamic_cast<EndItem &>(*end).name = u"empheq";
        parser.Push(std::move(end));
        return {};
    }
    parse_util::checkEqnEnv(parser);
    auto command = u"\\begin{" + name + u"}";
    auto options = parser.GetBrackets(command).value_or(u"");
    auto argument = parser.GetArgument(command).value();
    // Source destructures only the first two pieces; it does not trim them.
    const auto equal = argument.find(u'=');
    auto env = argument.substr(0, equal);
    std::u16string number;
    if (equal != argument.npos) {
        const auto next = argument.find(u'=', equal + 1);
        number = argument.substr(equal + 1, next == argument.npos ? next : next - equal - 1);
    }
    if (!EmpheqUtil::checkEnv(env))
        throw TexError("EmpheqInvalidEnv", u"Invalid environment \"%1\" for %2",
                       std::array{env, name});
    begin.nestStart = true;
    if (!options.empty()) {
        const std::array<KeyValueOption, 2> allowed{{{u"left", {}}, {u"right", {}}}};
        auto values = EmpheqUtil::splitOptions(options, allowed);
        for (auto const &[key, value] : values.ownEntries()) {
            auto &side = key == u"left" ? begin.left : begin.right;
            auto text = std::get_if<std::u16string>(&value);
            if (text)
                side = *text;
            else if (auto boolean = std::get_if<bool>(&value); boolean && !*boolean)
                side = u""; // Source's false side is disabled by adjustTable.
            else
                throw std::invalid_argument("Empheq side material requires a TeX string");
        }
    }
    parser.stack.global().empheq = env;
    parser.setString(u"\\begin{" + env + u"}" +
                     (number.empty() ? std::u16string{} : u"{" + number + u"}") +
                     std::u16string(parser.string().substr(parser.position())));
    parser.setPosition(0);
    parser.Push(frame);
    return {};
}
DispatchOutcome EmpheqMethods::EmpheqMO(TexParser &parser, MacroInvocation const &invocation) {
    const auto args = std::get<std::span<TexArgument const>>(invocation.arguments);
    parser.Push(&parser.createToken(mml::NodeKind::mo, std::get<std::u16string>(args[0])));
    return NoParseResult{};
}
DispatchOutcome EmpheqMethods::EmpheqDelim(TexParser &parser, MacroInvocation const &invocation) {
    auto delimiter = parser.GetDelimiter(invocation.dispatch_token);
    NodeDefinition definition;
    definition.attributes.set(u"stretchy", true);
    definition.attributes.set(u"symmetric", true);
    parser.Push(&parser.createToken(mml::NodeKind::mo,
                                    delimiter ? std::optional<std::u16string_view>(*delimiter)
                                              : std::nullopt,
                                    std::move(definition)));
    return NoParseResult{};
}
ConfigurationRef EmpheqConfiguration::initializeModule(ConfigurationHandler &registry,
                                                       MapHandler &maps) {
    maps.registerMap(std::make_shared<EnvironmentMap>(EnvironmentMapInit{
        MacroMapInit{{u"empheq-env", {}},
                     {Macro(u"empheq", EnvironmentBodyHandler{&EmpheqMethods::Empheq},
                            {std::u16string(u"empheq")})}},
        &ParseMethods::environment}));
    constexpr std::array<std::pair<std::u16string_view, std::u16string_view>, 32> symbols{
        {{u"empheqlbrace", u"{"},         {u"empheqrbrace", u"}"},
         {u"empheqlbrack", u"["},         {u"empheqrbrack", u"]"},
         {u"empheqlangle", u"\u27E8"},    {u"empheqrangle", u"\u27E9"},
         {u"empheqlparen", u"("},         {u"empheqrparen", u")"},
         {u"empheqlvert", u"|"},          {u"empheqrvert", u"|"},
         {u"empheqlVert", u"\u2016"},     {u"empheqrVert", u"\u2016"},
         {u"empheqlfloor", u"\u230A"},    {u"empheqrfloor", u"\u230B"},
         {u"empheqlceil", u"\u2308"},     {u"empheqrceil", u"\u2309"},
         {u"empheqbiglbrace", u"{"},      {u"empheqbigrbrace", u"}"},
         {u"empheqbiglbrack", u"["},      {u"empheqbigrbrack", u"]"},
         {u"empheqbiglangle", u"\u27E8"}, {u"empheqbigrangle", u"\u27E9"},
         {u"empheqbiglparen", u"("},      {u"empheqbigrparen", u")"},
         {u"empheqbiglvert", u"|"},       {u"empheqbigrvert", u"|"},
         {u"empheqbiglVert", u"\u2016"},  {u"empheqbigrVert", u"\u2016"},
         {u"empheqbiglfloor", u"\u230A"}, {u"empheqbigrfloor", u"\u230B"},
         {u"empheqbiglceil", u"\u2308"},  {u"empheqbigrceil", u"\u2309"}}};
    MacroMapInit commands{{u"empheq-macros", {}}, {}};
    for (auto const &[name, character] : symbols)
        commands.entries.emplace_back(std::u16string(name), &EmpheqMethods::EmpheqMO,
                                      std::vector<TexArgument>{std::u16string(character)});
    for (auto name : {u"empheql", u"empheqr", u"empheqbigl", u"empheqbigr"})
        commands.entries.emplace_back(name, &EmpheqMethods::EmpheqDelim);
    maps.registerMap(std::make_shared<CommandMap>(CommandMapInit{std::move(commands)}));
    ConfigurationDefinition definition;
    definition.handler = {{HandlerKind::macro, {u"empheq-macros"}},
                          {HandlerKind::environment, {u"empheq-env"}}};
    return Configuration::create(registry, u"empheq", std::move(definition));
}
} // namespace folia::math::tex
