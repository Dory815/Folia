// MathJax 4.1.3 input/tex/begingroup/BegingroupConfiguration.ts; Apache-2.0.
module folia.math.input.tex.begingroup.configuration;
import std;

namespace folia::math::tex {
namespace {
void configure(ParserConfiguration &, TeX &jax) {
    jax.parseOptions().begingroup =
        std::make_unique<BegingroupStack>(jax.session().definitions());
}
void removeGroups(TexFilterData &argument) {
    if (!argument.data.begingroup)
        throw std::logic_error("The begingroup package has not been configured");
    argument.data.begingroup->remove();
}
void finishGroups(TexFilterData &argument) {
    if (!argument.data.begingroup)
        throw std::logic_error("The begingroup package has not been configured");
    argument.data.begingroup->finish();
}
}
ConfigurationRef BegingroupConfiguration::initializeModule(ConfigurationHandler &registry,
                                                          MapHandler &maps) {
    maps.registerMap(std::make_shared<CommandMap>(CommandMapInit{MacroMapInit{
        {u"begingroup", std::nullopt},
        {Macro(u"begingroup", &BegingroupMethods::begingroup),
         Macro(u"endgroup", &BegingroupMethods::endgroup),
         Macro(u"global", &BegingroupMethods::global),
         Macro(u"gdef", BegingroupMethods::macro, {std::u16string(u"\\global\\def")}),
         Macro(u"begingroupReset", &BegingroupMethods::reset),
         Macro(u"begingroupSandbox", &BegingroupMethods::sandbox)}}}));
    ConfigurationDefinition definition;
    definition.handler = {{HandlerKind::macro, {u"begingroup"}}};
    definition.options.begingroup = BegingroupOptions{
        std::vector<std::u16string>{u"let", u"def", u"newcommand", u"DeclareMathOperator",
                                     u"Newextarrow"}};
    definition.config = ProcessorSpec<ConfigMethod>{&configure};
    definition.preprocessors.emplace_back(FilterList::Callback{&removeGroups});
    definition.postprocessors.emplace_back(FilterList::Callback{&finishGroups});
    return Configuration::create(registry, u"begingroup", std::move(definition));
}
}
