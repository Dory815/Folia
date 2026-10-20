// MathJax 4.1.3 input/tex/mathtools/MathtoolsConfiguration.ts; Apache-2.0.
module folia.math.input.tex.mathtools.configuration;
import folia.math.input.tex.newcommand.configuration;
import folia.math.input.tex.node_util;
import std;
namespace folia::math::tex {
void configMathtools(ParserConfiguration &config, TeX &jax) {
    NewcommandConfig(config, jax);
    auto &options = jax.options.mathtools.value();
    for (auto const &[name, paired] : options.pairedDelimiters.value()) {
        const auto id = config.callbacks().macroBinding(&MathtoolsMethods::PairedDelimiters);
        if (!id.value)
            throw std::logic_error("PairedDelimiters is absent from the compiled callback table");
        jax.session().defineCommand(
            name, CommandDefinition{name, NativeMacroAlias{id,
                                                           {paired.left, paired.right, paired.body,
                                                            static_cast<double>(paired.arity),
                                                            paired.pre, paired.post}}});
    }
    auto const &legacy = options.legacycolonsymbols.value();
    const bool active = std::visit(
        [](auto const &value) {
            if constexpr (std::is_same_v<std::remove_cvref_t<decltype(value)>, bool>)
                return value;
            else
                return !value.empty();
        },
        legacy);
    if (active)
        config.handlers.add(MathtoolsMethods::LEGACYCONFIG);
    MathtoolsTagFormat(config, jax);
}
void fixPrescripts(TexFilterData &filter) {
    auto &data = filter.data;
    for (auto node : data.getList(u"mmultiscripts")) {
        if (!node->properties().tex.fix_prescript.value_or(false))
            continue;
        int missing = 0;
        for (std::size_t index : {1u, 2u}) {
            if (index >= node->children().size() || !node->children()[index]) {
                auto &none = data.nodeFactory.makeNode(mml::NodeKind::none);
                NodeUtil::setChild(*node, index, none);
                ++missing;
            }
        }
        if (missing == 2) {
            std::vector<mml::MmlNode *> children(node->children().begin(), node->children().end());
            children.erase(children.begin() + 1, children.begin() + 3);
            node->setChildren(children);
        }
    }
}
ConfigurationRef MathtoolsConfiguration::initializeModule(ConfigurationHandler &registry,
                                                          MapHandler &maps) {
    MathtoolsMappings::initialize(maps);
    ConfigurationDefinition definition;
    definition.handler = {{HandlerKind::macro, {u"mathtools-macros", u"mathtools-delimiters"}},
                          {HandlerKind::environment, {u"mathtools-environments"}},
                          {HandlerKind::delimiter, {u"mathtools-delimiters"}},
                          {HandlerKind::character, {u"mathtools-characters"}}};
    definition.items = {
        {StackKind::multlined,
         StackItemClass{[](StackBuildArguments const &args) -> std::unique_ptr<BaseItem> {
                            std::get<MultlinedItemInit>(args.init);
                            return std::make_unique<MultlinedItem>(args.factory);
                        },
                        [](BaseItem const &item) {
                            return dynamic_cast<MultlinedItem const *>(&item) != nullptr;
                        }}}};
    definition.config = ProcessorSpec<ConfigMethod>{&configMathtools};
    definition.postprocessors.emplace_back(FilterList::Callback{&fixPrescripts}, -6);
    MathtoolsOptions defaults;
    defaults.multlinedGap = u"1em";
    defaults.multlinedPos = u"c";
    defaults.multlinedWidth = u"";
    defaults.firstlineAfterskip = u"";
    defaults.lastlinePreskip = u"";
    defaults.smallmatrixAlign = u"c";
    defaults.shortvdotsAdjustAbove = u".2em";
    defaults.shortvdotsAdjustBelow = u".2em";
    defaults.centercolon = false;
    defaults.centercolonOffset = u".04em";
    defaults.thincolonDx = u"-.04em";
    defaults.thincolonDw = u"-.08em";
    defaults.useUnicode = false;
    defaults.legacycolonsymbols = false;
    defaults.prescriptSubFormat = u"";
    defaults.prescriptSupFormat = u"";
    defaults.prescriptArgFormat = u"";
    defaults.allowMathtoolsset = true;
    defaults.pairedDelimiters.emplace();
    defaults.tagforms.emplace();
    definition.options.mathtools = std::move(defaults);
    return Configuration::create(registry, u"mathtools", std::move(definition));
}
} // namespace folia::math::tex
