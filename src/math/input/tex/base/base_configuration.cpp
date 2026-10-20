/*************************************************************
 * Copyright (c) 2018-2026 The MathJax Consortium.
 * MathJax 4.1.3 input/tex/base/BaseConfiguration.ts; Apache-2.0.
 *************************************************************/
module folia.math.input.tex.base_configuration;
import folia.math.input.tex.parse_methods;
import folia.math.input.tex.parse_util;
import folia.math.input.tex.node_util;
import folia.math.core.mml_tree.operator_dictionary;
import std;

namespace folia::math::tex {
namespace {
void csUndefined(TexParser &, std::u16string_view name) {
    const std::array<std::u16string, 1> args{u"\\" + std::u16string(name)};
    throw TexError("UndefinedControlSequence", u"Undefined control sequence %1", args);
}
void envUndefined(TexParser &, std::u16string_view name) {
    const std::array<std::u16string, 1> args{std::u16string(name)};
    throw TexError("UnknownEnv", u"Unknown environment '%1'", args);
}
double scriptlevel(mml::MmlNode &node) {
    auto value = node.attributes().lookup(u"scriptlevel");
    if (!value || std::holds_alternative<mml::AttributeNull>(*value))
        return 0;
    if (auto number = std::get_if<double>(value))
        return *number;
    if (auto boolean = std::get_if<bool>(value))
        return *boolean ? 1 : 0;
    auto const &text = std::get<std::u16string>(*value);
    std::string narrow(text.begin(), text.end());
    char *end{};
    auto result = std::strtod(narrow.c_str(), &end);
    return end == narrow.c_str() ? std::numeric_limits<double>::quiet_NaN() : result;
}
void filterNonscript(TexFilterData &arg) {
    auto &data = arg.data;
    for (auto node : data.getList(u"nonscript")) {
        if (scriptlevel(*node) > 0) {
            auto parent = node->parent();
            parent->removeChild(*node);
            const std::array removed{node};
            auto kind = mml::nodeKindName(node->kind());
            data.removeFromList(std::u16string(kind.begin(), kind.end()), removed);
            if (node->isKind(mml::NodeKind::mrow)) {
                auto style = node->children()[0];
                const std::array styles{style};
                data.removeFromList(u"mstyle", styles);
                data.removeFromList(u"mspace", style->children()[0]->children());
            }
        } else if (node->isKind(mml::NodeKind::mrow)) {
            node->parent()->replaceChild(*node->children()[0], *node);
            const std::array removed{node};
            data.removeFromList(u"mrow", removed);
        }
    }
}
std::unique_ptr<Tags> createBaseTags(ParseOptions &options) {
    return std::make_unique<BaseTags>(options);
}
void configure(ParserConfiguration &config, TeX &jax) {
    auto &options = jax.parseOptions().options;
    if (options.digits) {
        if (auto pattern = std::get_if<std::u16string>(&*options.digits)) {
            if (!pattern->empty())
                options.numberPattern = std::make_shared<regexp_host::RegExp>(*pattern);
        } else if (auto expression = std::get<regexp_host::RegExpRef>(*options.digits))
            options.numberPattern = std::move(expression);
    }
    config.maps().registerMap(std::make_shared<RegExpMap>(RegExpMapInit{
        {u"digit", TokenMapParser{&ParseMethods::digit}}, options.initialDigit.value()}));
    config.maps().registerMap(std::make_shared<RegExpMap>(RegExpMapInit{
        {u"letter", TokenMapParser{&ParseMethods::variable}}, options.initialLetter.value()}));
    const std::array<std::u16string, 2> maps{u"letter", u"digit"};
    config.handlers.get(HandlerKind::character)->add(maps, nullptr, 4);
}
} // namespace
void Other(TexParser &parser, std::u16string_view character) {
    auto const &environment = parser.stack.env();
    auto definition = parse_util::getFontDef(parser);
    auto mapped = parser.configuration.maps().getMap(u"remap")->lookup(character);
    auto remap = std::get_if<Token const *>(&mapped);
    auto range = mml::operator_dict::getRange(character);
    auto kind = mml::nodeKindFromName(std::string(range.kind.begin(), range.kind.end())).value();
    auto &node = parser.createToken(kind, remap && *remap ? (*remap)->character.value() : character,
                                    std::move(definition));
    std::u16string style;
    if (parse_util::isLatinOrGreekChar(character)) {
        style = parser.configuration.mathStyle(character, true);
        if (style.empty())
            style = environment.italic_font.value_or(u"");
    }
    auto variant = range.variant.value_or(
        (environment.font && !environment.font->empty() && style == u"normal") ? std::u16string{}
                                                                               : style);
    if (!variant.empty())
        node.attributes().set(u"mathvariant", variant);
    if (kind == mml::NodeKind::mo) {
        node.properties().tex.fix_stretchy = true;
        parser.configuration.addNode(u"fixStretchy", node);
    }
    parser.Push(&node);
}
ConfigurationRef BaseConfiguration::create(ConfigurationHandler &registry) {
    ConfigurationDefinition definition;
    definition.config = ProcessorSpec<ConfigMethod>{&configure};
    definition.handler = {{HandlerKind::character, {u"command", u"special"}},
                          {HandlerKind::delimiter, {u"delimiter"}},
                          {HandlerKind::macro,
                           {u"delimiter", u"macros", u"lcGreek", u"ucGreek", u"mathchar0mi",
                            u"mathchar0mo", u"mathchar7"}},
                          {HandlerKind::environment, {u"environment"}}};
    definition.fallback = {{HandlerKind::character, &Other},
                           {HandlerKind::macro, &csUndefined},
                           {HandlerKind::environment, &envUndefined}};
    definition.items = baseItemClasses();
    definition.tags = {{u"base", &createBaseTags}};
    auto &options = definition.options;
    options.maxMacros = 1000;
    options.digits = std::u16string{};
    options.numberPattern = std::make_shared<regexp_host::RegExp>(
        u"^(?:[0-9]+(?:\\{,\\}[0-9]{3})*(?:\\.[0-9]*)?|\\.[0-9]+)");
    options.initialDigit = std::make_shared<regexp_host::RegExp>(u"[0-9.,]");
    options.identifierPattern = std::make_shared<regexp_host::RegExp>(u"^[a-zA-Z]+");
    options.initialLetter = std::make_shared<regexp_host::RegExp>(u"[a-zA-Z]");
    options.baseURL = u"";
    definition.postprocessors.emplace_back(FilterList::Callback{&filterNonscript}, -4);
    return Configuration::create(registry, u"base", std::move(definition));
}
ConfigurationRef BasePackage::initialize(ConfigurationHandler &registry, MapHandler &maps) {
    BaseMappings::initialize(maps);
    std::vector<Token> remap;
    remap.emplace_back(u"-", std::u16string(u"\u2212"));
    remap.emplace_back(u"*", std::u16string(u"\u2217"));
    remap.emplace_back(u"`", std::u16string(u"\u2018"));
    maps.registerMap(std::make_shared<CharacterMap>(
        CharacterMapInit{{u"remap", std::nullopt}, std::move(remap)}));
    return BaseConfiguration::create(registry);
}
} // namespace folia::math::tex
