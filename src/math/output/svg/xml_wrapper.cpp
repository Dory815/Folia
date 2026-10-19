/*************************************************************
 * Source SvgXmlNode.addHDW and SvgHtmlNode inheritance.
 *************************************************************/
module folia.math.output.svg.xml_wrapper;
import folia.math.output.common;
import folia.math.util.property_host;

namespace folia::math::output::svg {
DOMNodeRef SvgXmlNode::addHDW(DOMNodeRef html,PropertyList& styles){
    auto& output=jax();
    const auto holder=output.html(u"mjx-html-holder",
        PropertyList{{"style",Property(styles)}},
        PropertyArray{Property(nodeProperty(html))});
    const auto box=getBBox();
    const double scale=metrics().scale;
    styles.setOwn("height",Property(sourceEm((box->h+box->d)*scale)));
    styles.setOwn("width",Property(sourceEm(box->w*scale)));
    styles.setOwn("vertical-align",Property(sourceEm(-box->d*scale)));
    styles.erase("font-size");
    styles.erase("font-family");
    return holder;
}

common::CommonWrapperClassRef registerSvgXmlNodeClass(
    common::CommonFactoryContextBindings& bindings,options::ObjectRef sourceConstructor){
    common::CommonWrapperClassData metadata;
    metadata.kind="XML";
    return common::registerCommonWrapperClass<SvgXmlNode>(
        bindings,sourceConstructor,std::move(metadata));
}

common::CommonWrapperClassRef registerSvgHtmlNodeClass(
    common::CommonFactoryContextBindings& bindings,options::ObjectRef sourceConstructor){
    common::CommonWrapperClassData metadata;
    metadata.kind="html";
    return common::registerCommonWrapperClass<SvgHtmlNode>(
        bindings,sourceConstructor,std::move(metadata));
}
}
