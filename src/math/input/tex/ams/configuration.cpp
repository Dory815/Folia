// MathJax 4.1.3 input/tex/ams/AmsConfiguration.ts; Apache-2.0.
module folia.math.input.tex.ams.configuration;
import folia.math.input.tex.newcommand.configuration;
import std;

namespace folia::math::tex {
namespace {
std::unique_ptr<Tags> createAmsTags(ParseOptions &options) {
    return std::make_unique<AmsTags>(options);
}
} // namespace
ConfigurationRef AmsConfiguration::initializeModule(ConfigurationHandler &registry,
                                                    MapHandler &maps) {
    AmsMappings::initialize(maps);
    ConfigurationDefinition definition;
    definition.handler = {
        {HandlerKind::character, {u"AMSmath-operatorLetter"}},
        {HandlerKind::delimiter, {u"AMSsymbols-delimiter", u"AMSmath-delimiter"}},
        {HandlerKind::macro,
         {u"AMSsymbols-mathchar0mi", u"AMSsymbols-mathchar0mo", u"AMSsymbols-delimiter",
          u"AMSsymbols-macros", u"AMSmath-mathchar0mo", u"AMSmath-macros", u"AMSmath-delimiter"}},
        {HandlerKind::environment, {u"AMSmath-environment"}}};
    definition.items = {
        {StackKind::multline,
         StackItemClass{[](StackBuildArguments const &arguments) -> std::unique_ptr<BaseItem> {
                            auto const &init = std::get<MultlineItemInit>(arguments.init);
                            return std::make_unique<MultlineItem>(arguments.factory, init.numbered);
                        },
                        [](BaseItem const &item) {
                            return dynamic_cast<MultlineItem const *>(&item) != nullptr;
                        }}},
        {StackKind::flalign,
         StackItemClass{[](StackBuildArguments const &arguments) -> std::unique_ptr<BaseItem> {
                            auto const &init = std::get<FlalignItemInit>(arguments.init);
                            return std::make_unique<FlalignItem>(arguments.factory, init.name,
                                                                 init.numbered, init.padded,
                                                                 init.center);
                        },
                        [](BaseItem const &item) {
                            return dynamic_cast<FlalignItem const *>(&item) != nullptr;
                        }}}};
    definition.tags = {{u"ams", &createAmsTags}};
    definition.options.multlineWidth = u"";
    AmsOptions options;
    options.operatornamePattern = std::make_shared<regexp_host::RegExp>(u"^[-*a-zA-Z0-9]+");
    options.multlineWidth = u"100%";
    options.multlineIndent = u"1em";
    definition.options.ams = std::move(options);
    definition.config = ProcessorSpec<ConfigMethod>{&NewcommandConfig};
    return Configuration::create(registry, u"ams", std::move(definition));
}
} // namespace folia::math::tex
