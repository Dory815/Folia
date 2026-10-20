// MathJax 4.1.3 input/tex/tagformat/TagFormatConfiguration.ts; Apache-2.0.
module folia.math.input.tex.tagformat.tag_format_configuration;
import std;

namespace folia::math::tex {
void tagformatConfig(ParserConfiguration &configuration, TeX &jax) {
    const auto selected = jax.options.tags.value_or(u"none");
    // The catalog belongs to this parser configuration. Reconfiguration keeps
    // one entry rather than producing globally named, permanently retained classes.
    if (selected == u"configTags")
        return;
    auto parent = TagsFactory::creator(selected, configuration.defaultTags, configuration.tags);
    TagsFactory::add(
        configuration,
        {u"configTags", [parent = std::move(parent)](ParseOptions &owner) -> std::unique_ptr<Tags> {
             auto product = parent(owner);
             if (!product)
                 throw std::runtime_error("Tags creator returned no product");
             product->setFormatters(owner.options);
             return product;
         }});
    jax.options.tags = u"configTags";
}
ConfigurationRef TagFormatConfiguration::initializeModule(ConfigurationHandler &registry) {
    ConfigurationDefinition definition;
    definition.config = ProcessorSpec<ConfigMethod>{&tagformatConfig, 10};
    definition.options.tagformat = TagsFactory::formatters();
    return Configuration::create(registry, u"tagformat", std::move(definition));
}
} // namespace folia::math::tex
