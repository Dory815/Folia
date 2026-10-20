// MathJax 4.1.3 input/tex/bussproofs/BussproofsConfiguration.ts; Apache-2.0.
module folia.math.input.tex.bussproofs.configuration;
import folia.math.input.tex.bussproofs.mappings;
import folia.math.input.tex.bussproofs.util;
import std;
namespace folia::math::tex {
ConfigurationRef BussproofsConfiguration::initializeModule(ConfigurationHandler &registry,
                                                           MapHandler &maps) {
    BussproofsMappings::initializeModule(maps);
    ConfigurationDefinition definition;
    definition.handler = {{HandlerKind::macro, {u"Bussproofs-macros"}},
                          {HandlerKind::environment, {u"Bussproofs-environments"}}};
    definition.items = {
        {StackKind::proof_tree,
         StackItemClass{[](StackBuildArguments const &args) -> std::unique_ptr<BaseItem> {
                            return std::make_unique<ProofTreeItem>(args.factory);
                        },
                        [](BaseItem const &item) {
                            return dynamic_cast<ProofTreeItem const *>(&item) != nullptr;
                        }}}};
    definition.preprocessors.emplace_back(&bussproofs_util::saveDocument, 1);
    definition.postprocessors = {{&bussproofs_util::clearDocument, 3},
                                 {&bussproofs_util::makeBsprAttributes, 2},
                                 {&bussproofs_util::balanceRules, 1}};
    return Configuration::create(registry, u"bussproofs", std::move(definition));
}
} // namespace folia::math::tex
