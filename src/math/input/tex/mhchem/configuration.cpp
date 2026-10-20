// Native port of MathJax 4.1.3 input/tex/mhchem/MhchemConfiguration.ts.
// Copyright (c) 2018-2026 The MathJax Consortium. Apache-2.0.
module folia.math.input.tex.mhchem.configuration;
import folia.math.dependencies.mhchem_parser;
import folia.math.input.tex.token;
import folia.math.util.utf16;
import std;

namespace folia::math::tex {
namespace {
class MhchemCommandMap final : public CommandMap {
    std::shared_ptr<CharacterMap> originalCharacters_;

  public:
    MhchemCommandMap(CommandMapInit init, std::shared_ptr<CharacterMap> characters)
        : CommandMap(std::move(init)), originalCharacters_(std::move(characters)) {}
    CharacterMap const &originalCharacters() const noexcept { return *originalCharacters_; }
};

constexpr std::array<MhchemReplacement, 6> replacements{{
    {uR"rx(\\underset{\\lower2mu{(.*?)}}{\\overset{(.*?)}{\\long(.*?)}})rx",
     uR"(\mhchemx$3[$1]{$2})", MhchemReplacementKind::text},
    {uR"rx(\\overset{(.*?)}{\\long(.*?)})rx", uR"(\mhchemx$2{$1})",
     MhchemReplacementKind::text},
    {uR"rx(\\rlap\{\\lower\.1em\{-\}\}\\raise\.1em\{\\tripledash\})rx",
     uR"(\mhchemBondTD)", MhchemReplacementKind::text},
    {uR"rx(\\rlap\{\\lower\.2em\{-\}\}\\rlap\{\\raise\.2em\{\\tripledash\}\}-)rx",
     uR"(\mhchemBondTDD)", MhchemReplacementKind::text},
    {uR"rx(\\rlap\{\\lower\.2em\{-\}\}\\rlap\{\\raise.2em\{-\}\}\\tripledash)rx",
     uR"(\mhchemBondDTD)", MhchemReplacementKind::text},
    {uR"rx(\\(x?(?:long)?(?:left|right|[Ll]eftright|[Rr]ightleft)(?:arrow|harpoons)))rx",
     {}, MhchemReplacementKind::arrow},
}};

bool mapHas(TokenMap const &map, std::u16string_view name) {
    const auto value = map.lookup(name);
    if (auto token = std::get_if<Token const *>(&value))
        return *token != nullptr;
    if (auto macro = std::get_if<Macro const *>(&value))
        return *macro != nullptr;
    if (auto command = std::get_if<CommandDefinition const *>(&value))
        return *command != nullptr;
    if (auto text = std::get_if<std::u16string_view>(&value))
        return !text->empty();
    return false;
}

std::u16string arrowReplace(std::u16string_view whole, std::u16string_view arrow,
                            TokenMap const &macros, TokenMap const &characters) {
    const auto name = u"mhchem" + std::u16string(arrow);
    return mapHas(characters, name) || mapHas(macros, name)
               ? u"\\" + name
               : std::u16string(whole);
}

} // namespace

std::span<MhchemReplacement const> mhchemReplacements() noexcept { return replacements; }

std::u16string applyMhchemReplacement(std::u16string tex, MhchemReplacement const &rule,
                                      TokenMap const &macros, TokenMap const &characters) {
    if (rule.kind == MhchemReplacementKind::text)
        return mhchem::replaceRegex(std::move(tex), rule.pattern, rule.replacement, true);

    regexp_host::RegExp expression(std::u16string(rule.pattern), u"g");
    std::u16string result;
    std::size_t offset = 0;
    while (const auto found = expression.exec(tex)) {
        const auto whole = found->at(0).value_or(u"");
        const auto arrow = found->at(1).value_or(u"");
        result.append(tex, offset, found->index - offset);
        result += arrowReplace(whole, arrow, macros, characters);
        offset = found->index + whole.size();
    }
    result.append(tex, offset, tex.size() - offset);
    return result;
}

std::u16string applyMhchemReplacements(std::u16string tex, TokenMap const &macros,
                                      TokenMap const &characters) {
    for (const auto &rule : replacements)
        tex = applyMhchemReplacement(std::move(tex), rule, macros, characters);
    return tex;
}

DispatchOutcome MhchemMethods::Machine(TexParser &parser, MacroInvocation const &invocation) {
    const auto machine = std::get<std::u16string>(
        std::get<std::span<TexArgument const>>(invocation.arguments).front());
    const auto argument = parser.GetArgument(invocation.dispatch_token).value();
    std::u16string tex;
    try {
        auto *handler = parser.configuration.handlers.get(HandlerKind::macro);
        auto registered = handler ? handler->retrieve(u"mhchem") : nullptr;
        auto *macros = dynamic_cast<MhchemCommandMap *>(registered.get());
        if (!macros)
            throw std::logic_error("Mhchem maps are not registered");
        tex = applyMhchemReplacements(mhchem::toTex(argument, machine), *macros,
                                      macros->originalCharacters());
    } catch (mhchem::Error const &error) {
        throw TexError(utf16::encode(error.id), error.message);
    }
    const auto input = parser.string();
    const auto position = std::min(parser.position(), input.size());
    parser.setString(tex + std::u16string(input.substr(position)));
    parser.setPosition(0);
    return NoParseResult{};
}

DispatchOutcome MhchemUtils::relmo(TexParser &parser, TokenInput const &input) {
    const auto token = std::get_if<Token const *>(&input.value);
    if (!token || !*token || !(*token)->character)
        return ContinueDispatch{};
    NodeDefinition definition;
    definition.attributes.set(u"stretchy", true);
    definition.attributes.set(u"texClass", static_cast<double>(mml::TexClass::rel));
    definition.attributes.set(u"mathvariant", u"-mhchem");
    if ((*token)->attributes)
        for (const auto &[name, value] : (*token)->attributes->ownEntries())
            definition.attributes.set(name, value);
    parser.Push(&parser.createToken(mml::NodeKind::mo, *(*token)->character,
                                    std::move(definition)));
    return NoParseResult{};
}

ConfigurationRef MhchemConfiguration::initializeModule(ConfigurationHandler &registry,
                                                       MapHandler &maps) {
    if (auto existing = registry.get(u"mhchem"))
        return existing;
    std::vector<Macro> commands{
        Macro(u"ce", MacroMethod{&MhchemMethods::Machine}, {std::u16string(u"ce")}),
        Macro(u"pu", MacroMethod{&MhchemMethods::Machine}, {std::u16string(u"pu")}),
        Macro(u"mhchemxrightarrow", MacroMethod{MhchemMethods::xArrow},
              {58409.0, 5.0, 9.0}),
        Macro(u"mhchemxleftarrow", MacroMethod{MhchemMethods::xArrow},
              {58408.0, 9.0, 5.0}),
        Macro(u"mhchemxleftrightarrow", MacroMethod{MhchemMethods::xArrow},
              {58410.0, 9.0, 9.0}),
        Macro(u"mhchemxleftrightarrows", MacroMethod{MhchemMethods::xArrow},
              {58411.0, 9.0, 9.0}),
        Macro(u"mhchemxrightleftharpoons", MacroMethod{MhchemMethods::xArrow},
              {58376.0, 5.0, 9.0}),
        Macro(u"mhchemxRightleftharpoons", MacroMethod{MhchemMethods::xArrow},
              {58377.0, 5.0, 9.0}),
        Macro(u"mhchemxLeftrightharpoons", MacroMethod{MhchemMethods::xArrow},
              {58378.0, 9.0, 11.0}),
    };
    const auto character = [](std::u16string token, std::u16string value, bool fixed) {
        std::optional<mml::AttributeSet> attributes;
        if (fixed) {
            attributes.emplace();
            attributes->set(u"stretchy", false);
        }
        return Token(std::move(token), std::move(value), std::move(attributes));
    };
    std::vector<Token> characterEntries{
        character(u"tripledash", u"\uE410", true),
        character(u"mhchemBondTD", u"\uE411", true),
        character(u"mhchemBondTDD", u"\uE412", true),
        character(u"mhchemBondDTD", u"\uE413", true),
        character(u"mhchemlongleftarrow", u"\uE428", false),
        character(u"mhchemlongrightarrow", u"\uE429", false),
        character(u"mhchemlongleftrightarrow", u"\uE42A", false),
        character(u"mhchemlongrightleftharpoons", u"\uE408", false),
        character(u"mhchemlongRightleftharpoons", u"\uE409", false),
        character(u"mhchemlongLeftrightharpoons", u"\uE40A", false),
        character(u"mhchemlongleftrightarrows", u"\uE42B", false),
        character(u"mhchemrightarrow", u"\uE42D", false),
        character(u"mhchemleftarrow", u"\uE42C", false),
        character(u"mhchemleftrightarrow", u"\uE42E", false),
    };
    auto characterMap = std::make_shared<CharacterMap>(CharacterMapInit{
        {u"mhchem-chars", TokenMapParser{TokenInputHandler{&MhchemUtils::relmo}}},
        std::move(characterEntries)});
    maps.registerMap(characterMap);
    maps.registerMap(std::make_shared<MhchemCommandMap>(CommandMapInit{
        MacroMapInit{{u"mhchem", std::nullopt}, std::move(commands)}}, characterMap));

    ConfigurationDefinition definition;
    definition.handler = {{HandlerKind::macro, {u"mhchem", u"mhchem-chars"}}};
    return Configuration::create(registry, u"mhchem", std::move(definition));
}
} // namespace folia::math::tex
