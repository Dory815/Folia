// MathJax 4.1.3 input/tex/braket/BraketConfiguration.ts; Apache-2.0.
module folia.math.input.tex.braket.configuration;
import folia.math.input.tex.braket.items;
import folia.math.input.tex.braket.mappings;
import std;
namespace folia::math::tex {
ConfigurationRef BraketConfiguration::initializeModule(ConfigurationHandler &registry,
                                                       MapHandler &maps) {
    BraketMappings::initializeModule(maps);
    ConfigurationDefinition definition;
    definition.handler = {{HandlerKind::character, {u"Braket-characters"}},
                          {HandlerKind::macro, {u"Braket-macros"}}};
    definition.items = {
        {StackKind::braket,
         StackItemClass{[](StackBuildArguments const &arguments) -> std::unique_ptr<BaseItem> {
                            return std::make_unique<BraketItem>(arguments.factory);
                        },
                        [](BaseItem const &item) {
                            return dynamic_cast<BraketItem const *>(&item) != nullptr;
                        }}}};
    definition.priority = 3;
    return Configuration::create(registry, u"braket", std::move(definition));
}
} // namespace folia::math::tex
