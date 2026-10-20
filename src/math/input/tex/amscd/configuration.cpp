// MathJax 4.1.3 input/tex/amscd/AmsCdConfiguration.ts; Apache-2.0.
module folia.math.input.tex.amscd.configuration;
import std;

namespace folia::math::tex {
ConfigurationRef AmsCdConfiguration::initializeModule(ConfigurationHandler &registry,
                                                      MapHandler &maps) {
    AmsCdMappings::initialize(maps);
    ConfigurationDefinition definition;
    definition.handler = {{HandlerKind::character, {u"amscd_special"}},
                          {HandlerKind::macro, {u"amscd_macros"}},
                          {HandlerKind::environment, {u"amscd_environment"}}};
    AmscdOptions options;
    options.colspace = u"5pt";
    options.rowspace = u"5pt";
    options.harrowsize = u"2.75em";
    options.varrowsize = u"1.75em";
    options.hideHorizontalLabels = false;
    definition.options.amscd = std::move(options);
    return Configuration::create(registry, u"amscd", std::move(definition));
}
} // namespace folia::math::tex