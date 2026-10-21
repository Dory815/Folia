/*************************************************************
 * Native SVG catalog composition over the accepted Common profile.
 *************************************************************/
module folia.math.output.svg.wrapper_factory;
import folia.math.output.svg.menclose_notations;
import folia.math.output.svg.merror;
import folia.math.output.svg.maction;
import folia.math.output.svg.xml_wrapper;
import folia.math.output.svg.mglyph;
import folia.math.core.mml_tree.mml_document;
import std;

namespace folia::math::output::svg {
common::CommonWrapperCatalog SvgWrapperFactory::defaultNodes() {
    auto catalog = common::CommonWrapperFactory::standardNodes(mencloseNotations());
    const auto maction = std::ranges::find(catalog, folia::math::mml::NodeKind::maction,
                                           &common::CommonWrapperRegistration::kind);
    if (maction == catalog.end())
        throw std::logic_error("Common standard catalog has no maction registration");
    *maction = mactionRegistration();
    auto const merror = std::ranges::find(catalog, folia::math::mml::NodeKind::merror,
                                          &common::CommonWrapperRegistration::kind);
    if (merror == catalog.end())
        throw std::logic_error("Common standard catalog has no merror registration");
    *merror = merrorRegistration();
    const auto xml = std::ranges::find(catalog, folia::math::mml::NodeKind::xml,
                                       &common::CommonWrapperRegistration::kind);
    if (xml == catalog.end())
        throw std::logic_error("Common standard catalog has no XML registration");
    *xml = xmlRegistration();
    const auto html = std::ranges::find(catalog, folia::math::mml::NodeKind::html,
                                        &common::CommonWrapperRegistration::kind);
    if (html == catalog.end())
        throw std::logic_error("Common standard catalog has no HTML registration");
    *html = htmlRegistration();
    const auto annotationXml =
        std::ranges::find(catalog, folia::math::mml::NodeKind::annotation_xml,
                          &common::CommonWrapperRegistration::kind);
    if (annotationXml == catalog.end())
        throw std::logic_error("Common standard catalog has no annotation-xml registration");
    *annotationXml = annotationXmlRegistration();
    const auto mglyph = std::ranges::find(catalog, folia::math::mml::NodeKind::mglyph,
                                          &common::CommonWrapperRegistration::kind);
    if (mglyph == catalog.end())
        throw std::logic_error("Common standard catalog has no mglyph registration");
    *mglyph = mglyphRegistration();
    return catalog;
}

SvgWrapperFactory::SvgWrapperFactory(common::CommonOutputJax &jax,
                                     common::CommonWrapperCatalog actual)
    : common::CommonWrapperFactory(jax, std::move(actual)) {}
} // namespace folia::math::output::svg
