// MathJax 4.1.3 input/tex/textcomp/TextcompConfiguration.ts; Apache-2.0.
module folia.math.input.tex.textcomp.configuration;
import folia.math.input.tex.textcomp.mappings;
import std;
namespace folia::math::tex {
namespace {
void configureTextcomp(ParserConfiguration &, TeX &jax) {
    if (auto data = jax.parseOptions().textmacros) {
        data->parseOptions->options.textmacros->packages->push_back({u"text-textcomp", {}});
        data->textConf->add(u"text-textcomp", jax, {});
    }
}
} // namespace
ConfigurationRef TextcompConfiguration::initializeModule(ConfigurationHandler &registry,
                                                         MapHandler &maps) {
    TextcompMappings::initializeModule(maps);
    ConfigurationDefinition text;
    text.parser = u"text";
    text.handler = {{HandlerKind::macro, {u"textcomp-macros"}}};
    (void)Configuration::create(registry, u"text-textcomp", std::move(text));
    ConfigurationDefinition definition;
    definition.handler = {{HandlerKind::macro, {u"textcomp-macros"}}};
    definition.config = ProcessorSpec<ConfigMethod>{&configureTextcomp};
    return Configuration::create(registry, u"textcomp", std::move(definition));
}
} // namespace folia::math::tex
