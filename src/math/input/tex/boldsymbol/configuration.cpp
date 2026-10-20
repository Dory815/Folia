// The original boldsymbol token creator and mathematical variant rewrite.
module folia.math.input.tex.boldsymbol.configuration;
import std;

namespace folia::math::tex {
mml::MmlNode& createBoldToken(NodeFactory& factory, mml::NodeKind kind,
                             NodeDefinition definition, std::u16string_view text) {
    auto& token = NodeFactory::createToken(factory, kind, std::move(definition), text);
    // Source short-circuits mtext before reading the current parser environment.
    if (kind != mml::NodeKind::mtext && factory.configuration().environment().bold_symbol.value_or(false)) {
        token.properties().tex.fix_bold = true;
        factory.configuration().addNode(u"fixBold", token);
    }
    return token;
}

void rewriteBoldTokens(std::span<mml::MmlNode* const> nodes) {
    static constexpr std::pair<std::u16string_view, std::u16string_view> variants[]{
        {u"normal", u"bold"}, {u"italic", u"bold-italic"}, {u"fraktur", u"bold-fraktur"},
        {u"script", u"bold-script"}, {u"sans-serif", u"bold-sans-serif"},
        {u"-tex-calligraphic", u"-tex-bold-calligraphic"},
        {u"-tex-oldstyle", u"-tex-bold-oldstyle"}, {u"-tex-mathit", u"bold-italic"}
    };
    for (auto* node : nodes) {
        if (!node || !node->properties().tex.fix_bold.value_or(false)) continue;
        if (auto const* current = node->attributes().lookup(u"mathvariant")) {
            auto value = *current;
            if (auto const* text = std::get_if<std::u16string>(&value))
                for (auto const& [from, to] : variants)
                    if (*text == from) { value = std::u16string(to); break; }
            node->attributes().set(u"mathvariant", std::move(value));
        }
        node->properties().tex.fix_bold.reset();
    }
}
}
