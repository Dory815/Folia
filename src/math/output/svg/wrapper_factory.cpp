/*************************************************************
 * Native SVG catalog composition over the accepted Common profile.
 *************************************************************/
module folia.math.output.svg.wrapper_factory;
import folia.math.output.svg.menclose_notations;
import folia.math.output.svg.merror;
import folia.math.core.mml_tree.mml_document;
import std;

namespace folia::math::output::svg {
common::CommonWrapperCatalog SvgWrapperFactory::defaultNodes() {
    auto catalog = common::CommonWrapperFactory::standardNodes(mencloseNotations());
    auto const merror = std::ranges::find(catalog, folia::math::mml::NodeKind::merror,
                                          &common::CommonWrapperRegistration::kind);
    if (merror == catalog.end())
        throw std::logic_error("Common standard catalog has no merror registration");
    *merror = merrorRegistration();
    return catalog;
}

SvgWrapperFactory::SvgWrapperFactory(common::CommonOutputJax &jax,
                                     common::CommonWrapperCatalog actual)
    : common::CommonWrapperFactory(jax, std::move(actual)) {}
} // namespace folia::math::output::svg
