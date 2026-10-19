/*************************************************************
 * MathJax output/svg/Wrappers/semantics.ts and HtmlNode.ts.
 * SVG XML wrappers are the backend subclasses of the shared Common XML
 * measurement owner. They never create or own a second DOM graph.
 *************************************************************/
export module folia.math.output.svg.xml_wrapper;
export import folia.math.output.common;

export namespace folia::math::output::svg {
class SvgXmlNode : public common::CommonXmlNode {
public:
    using CommonXmlNode::CommonXmlNode;
    DOMNodeRef addHDW(DOMNodeRef html,PropertyList& styles) override;
};

// Source SvgHtmlNode extends SvgXmlNode and adds only its actual HtmlNode
// kind. It shares the same DOM adaptor, bbox and foreignObject implementation.
class SvgHtmlNode final : public SvgXmlNode {
public:
    using SvgXmlNode::SvgXmlNode;
};

common::CommonWrapperClassRef registerSvgXmlNodeClass(
    common::CommonFactoryContextBindings&,options::ObjectRef sourceConstructor);
common::CommonWrapperClassRef registerSvgHtmlNodeClass(
    common::CommonFactoryContextBindings&,options::ObjectRef sourceConstructor);
}
