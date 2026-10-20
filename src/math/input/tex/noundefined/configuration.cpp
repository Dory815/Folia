// MathJax 4.1.3 input/tex/noundefined/NoUndefinedConfiguration.ts; Apache-2.0.
module folia.math.input.tex.noundefined.configuration;
import std;

namespace folia::math::tex {
namespace {
void noUndefined(TexParser &parser, std::u16string_view name) {
    auto &text = parser.createText(u"\\" + std::u16string(name));
    auto const &options = parser.options().noundefined.value();
    NodeDefinition definition;
    for (auto const &[attribute, value] : {
             std::pair{u"mathcolor", &options.color},
             std::pair{u"mathbackground", &options.background},
             std::pair{u"mathsize", &options.size}}) {
        if (value->has_value() && !value->value().empty())
            definition.attributes.set(attribute, value->value());
    }
    parser.Push(&parser.create(mml::NodeKind::mtext, {}, std::move(definition), &text));
}
}
ConfigurationRef NoUndefinedConfiguration::initializeModule(ConfigurationHandler &registry) {
    ConfigurationDefinition definition;
    definition.fallback = {{HandlerKind::macro, &noUndefined}};
    definition.options.noundefined = NoUndefinedOptions{u"red", u"", u""};
    definition.priority = 3;
    return Configuration::create(registry, u"noundefined", std::move(definition));
}
}
