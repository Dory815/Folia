// MathJax 4.1.3 input/tex/bboldx/BboldxConfiguration.ts; Apache-2.0.
module folia.math.input.tex.bboldx.configuration;
import folia.math.input.tex.bboldx.mappings;
import std;
namespace folia::math::tex {
namespace {
void configureBboldx(ParserConfiguration &, TeX &jax) {
    if (auto data = jax.parseOptions().textmacros) {
        data->parseOptions->options.textmacros->packages->push_back({u"text-bboldx", {}});
        data->textConf->add(u"text-bboldx", jax, {});
    }
}
} // namespace
ConfigurationRef BboldxConfiguration::initializeModule(ConfigurationHandler &registry,
                                                       MapHandler &maps) {
    BboldxMappings::initializeModule(maps);
    ConfigurationDefinition text;
    text.parser = u"text";
    text.handler = {
        {HandlerKind::macro,
         {u"text-bboldx", u"text-bboldx-mathchar0miNormal", u"text-bboldx-delimiterNormal",
          u"text-bboldx-mathchar0miBold", u"text-bboldx-delimiterBold"}}};
    Configuration::create(registry, u"text-bboldx", std::move(text));
    ConfigurationDefinition definition;
    definition.handler = {{HandlerKind::macro,
                           {u"bboldx", u"bboldx-mathchar0miNormal", u"bboldx-delimiterNormal",
                            u"bboldx-mathchar0miBold", u"bboldx-delimiterBold"}}};
    definition.options.bboldx = BboldxOptions{.bfbb = false, .light = false};
    definition.config = ProcessorSpec<ConfigMethod>{&configureBboldx};
    definition.priority = 3;
    return Configuration::create(registry, u"bboldx", std::move(definition));
}
} // namespace folia::math::tex
