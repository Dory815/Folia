// MathJax 4.1.3 input/tex/textcomp/TextcompMappings.ts; Apache-2.0.
module folia.math.input.tex.textcomp.mappings;
import folia.math.input.tex.textmacros.methods;
import folia.math.input.tex.parse_util;
import std;
namespace folia::math::tex {
namespace {
struct Entry {
    const char *name;
    std::u16string_view character;
    const char *variant;
};
#include "mapping_data.inc"
} // namespace
DispatchOutcome TextcompMappings::Insert(TexParser &parser, MacroInvocation const &invocation) {
    const auto arguments = std::get<std::span<TexArgument const>>(invocation.arguments);
    const auto &character = std::get<std::u16string>(arguments[0]);
    const auto font =
        arguments.size() > 1 ? std::get<std::u16string>(arguments[1]) : std::u16string{};
    if (auto text = dynamic_cast<TextParser *>(&parser)) {
        if (font.empty())
            return TextMacrosMethods::Insert(parser, invocation);
        text->saveText();
    }
    NodeDefinition definition;
    if (!font.empty())
        definition.attributes.set(u"mathvariant", font);
    parser.Push(&parse_util::internalText(parser, character, std::move(definition)));
    return NoParseResult{};
}
void TextcompMappings::initializeModule(MapHandler &maps) {
    std::vector<Macro> commands;
    for (auto const &entry : entries) {
        const std::string_view name(entry.name);
        std::vector<TexArgument> arguments{std::u16string(entry.character)};
        if (entry.variant && *entry.variant)
            arguments.emplace_back(std::u16string(std::string_view(entry.variant) == "oldstyle"
                                                      ? u"-tex-oldstyle"
                                                      : u"-tex-bold-oldstyle"));
        commands.emplace_back(std::u16string(name.begin(), name.end()), &Insert,
                              std::move(arguments));
    }
    maps.registerMap(std::make_shared<CommandMap>(
        CommandMapInit{MacroMapInit{{u"textcomp-macros", std::nullopt}, std::move(commands)}}));
}
} // namespace folia::math::tex
