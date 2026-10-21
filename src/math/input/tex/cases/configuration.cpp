// MathJax 4.1.3 input/tex/cases/CasesConfiguration.ts; Apache-2.0.
module folia.math.input.tex.cases.configuration;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.empheq.util;
import std;

namespace folia::math::tex {
StackKind CasesBeginItem::kind() const {
    return StackKind::cases_begin;
}
CheckResult CasesBeginItem::checkItem(FrameHandle const &item) {
    if (item->isKind(StackKind::end) && item->getName() == getName() && end) {
        end = false;
        // Source [[], true]: pop one activation, replay no items.
        return CheckResult(std::vector<ReplayEntry>{});
    }
    return BeginItem::checkItem(item);
}
void CasesTags::start(std::string_view env, bool taggable, bool defaultTags) {
    subcounter_ = 0;
    AmsTags::start(env, taggable, defaultTags);
}
void CasesTags::autoTag() {
    auto &equation = configuration().request().equation;
    if (equation.current_tag.tag)
        return;
    auto increment = [](std::int64_t &counter) {
        if (counter == std::numeric_limits<std::int64_t>::max())
            throw std::overflow_error("Equation counter exhausted");
        ++counter;
    };
    if (equation.current_tag.environment == "subnumcases") {
        if (subcounter_ == 0)
            increment(equation.counter.equation);
        increment(subcounter_);
        tag(formatNumber(equation.counter.equation, subcounter_), false);
    } else {
        if (equation.current_tag.environment != "numcases-left")
            increment(equation.counter.equation);
        tag(formatNumber(equation.counter.equation), false);
    }
}
std::u16string CasesTags::formatNumberDefault(std::int64_t number,
                                              std::optional<std::int64_t> subnumber) {
    const auto ascii = std::to_string(number);
    std::u16string result(ascii.begin(), ascii.end());
    if (subnumber)
        result += static_cast<char16_t>((static_cast<std::uint64_t>(*subnumber) + 0x60U) & 0xFFFFU);
    return result;
}
StackEntry CasesMethods::NumCases(TexParser &parser, FrameHandle const &frame,
                                  std::span<TexArgument const>) {
    auto &begin = dynamic_cast<CasesBeginItem &>(*frame);
    auto name = std::u16string(begin.getName().value_or(u"undefined"));
    if (parser.stack.env().closing_environment == name) {
        parser.stack.env().closing_environment.reset();
        auto end = parser.itemFactory().create(StackKind::end);
        dynamic_cast<EndItem &>(*end).name = name;
        parser.Push(std::move(end));
        auto cases = dynamic_cast<CasesBeginItem *>(parser.stack.Top());
        if (!cases || !cases->Last())
            throw std::logic_error("Cases environment has no completed table");
        auto &table = *cases->Last();
        auto &original = parse_util::copyNode(table, parser);
        EmpheqUtil::left(table, original, cases->left.value_or(u"") + u"\\mmlToken{mo}{\\U{7B}}\\,",
                         parser, u"numcases-left");
        end = parser.itemFactory().create(StackKind::end);
        dynamic_cast<EndItem &>(*end).name = name;
        parser.Push(std::move(end));
        return {};
    }
    begin.left = parser.GetArgument(u"\\begin{" + name + u"}").value();
    const std::array<TexArgument, 4> args{true, true, std::u16string(u"ll"), std::u16string(u"tt")};
    auto result = BaseMethods::EqnArray(parser, frame, args);
    auto &array = dynamic_cast<EqnArrayItem &>(*std::get<FrameHandle>(result));
    array.arraydef.set(u"displaystyle", false);
    array.arraydef.set(u"rowspacing", u".2em");
    array.numCases = true;
    // EqnArray already pushed frame. This second activation consumes the first end.
    parser.Push(frame);
    return result;
}
DispatchOutcome CasesMethods::Entry(TexParser &parser, MacroInvocation const &invocation) {
    auto array = dynamic_cast<ArrayItem *>(parser.stack.Top());
    if (!array || !array->numCases)
        return BaseMethods::Entry(parser, invocation);
    auto cell = parser.itemFactory().create(StackKind::cell);
    auto &entry = dynamic_cast<CellItem &>(*cell);
    entry.isEntry = true;
    entry.name = invocation.dispatch_token;
    parser.Push(std::move(cell));
    const std::u16string source(parser.string());
    std::size_t braces{}, i = parser.position();
    regexp_host::RegExp control(u"^[a-z]+|.", u"i");
    while (i < source.size()) {
        const auto c = source[i];
        if (c == u'{') {
            ++braces;
            ++i;
        } else if (c == u'}') {
            if (braces == 0)
                break;
            --braces;
            ++i;
        } else if (c == u'&' && braces == 0)
            throw TexError("ExtraCasesAlignTab",
                           u"Extra alignment tab in text for numcase environment");
        else if (c == u'\\' && braces == 0) {
            auto match = control.exec(source.substr(i + 1));
            auto cs = match ? match->at(0) : std::nullopt;
            if (!cs || *cs == u"\\" || *cs == u"cr" || *cs == u"end" || *cs == u"label")
                break;
            i += cs->size();
        } else
            ++i;
    }
    auto text = source.substr(parser.position(), i - parser.position());
    auto leading = regexp_host::RegExp(u"^\\s*").exec(text);
    if (leading)
        text.erase(0, leading->at(0)->size());
    parser.PushAll(parse_util::internalMath(parser, text, ScriptLevel{0.0}));
    parser.setPosition(i);
    // Source returns null, so character dispatch continues to the base Entry map.
    return ContinueDispatch{};
}
DispatchOutcome CasesMethods::environment(TexParser &parser, std::u16string_view env,
                                          EnvironmentBodyHandler method,
                                          std::span<TexArgument const> args) {
    auto item = parser.itemFactory().create(StackKind::cases_begin);
    auto &begin = dynamic_cast<CasesBeginItem &>(*item);
    begin.name = env;
    begin.end = true;
    parser.Push(method(parser, item, args));
    return NoParseResult{};
}
ConfigurationRef CasesConfiguration::initializeModule(ConfigurationHandler &registry,
                                                      MapHandler &maps) {
    maps.registerMap(std::make_shared<EnvironmentMap>(EnvironmentMapInit{
        MacroMapInit{{u"cases-env", {}},
                     {Macro(u"numcases", EnvironmentBodyHandler{&CasesMethods::NumCases},
                            {std::u16string(u"cases")}),
                      Macro(u"subnumcases", EnvironmentBodyHandler{&CasesMethods::NumCases},
                            {std::u16string(u"cases")})}},
        &CasesMethods::environment}));
    maps.registerMap(std::make_shared<MacroMap>(
        MacroMapInit{{u"cases-macros", {}}, {Macro(u"&", &CasesMethods::Entry)}}));
    ConfigurationDefinition definition;
    definition.handler = {{HandlerKind::environment, {u"cases-env"}},
                          {HandlerKind::character, {u"cases-macros"}}};
    definition.items = {
        {StackKind::cases_begin,
         StackItemClass{[](StackBuildArguments const &args) -> std::unique_ptr<BaseItem> {
                            return std::make_unique<CasesBeginItem>(args.factory);
                        },
                        [](BaseItem const &item) {
                            return dynamic_cast<CasesBeginItem const *>(&item) != nullptr;
                        }}}};
    definition.tags = {{u"cases", [](ParseOptions &options) -> std::unique_ptr<Tags> {
                            return std::make_unique<CasesTags>(options);
                        }}};
    return Configuration::create(registry, u"cases", std::move(definition));
}
} // namespace folia::math::tex
