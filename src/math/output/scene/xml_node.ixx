/*************************************************************
 * Backend-independent foreignObject payload for Source SvgXmlNode and
 * SvgHtmlNode. This is a value snapshot; it retains no DOM or wrapper owner.
 *************************************************************/
export module folia.math.output.scene:xml_node;
export import folia.math.output.scene.foundation;
export import :wrapper;
export import folia.math.output.common;

export namespace folia::math::output::scene {
struct XmlNodePlacementData {
    ForeignObjectGeometry geometry;
    // Source SVG paint attributes are retained in their original insertion
    // order. Numeric values in geometry remain the D2D authority.
    PropertyList attributes;
    DOMNodeRef html;
};

// Evaluation order follows SvgXmlNode.toSVG: metrics, getBBox, then getHTML.
XmlNodePlacementData captureXmlNode(common::CommonXmlNode&);

class XmlNodePlacement final:public Placement {
public:
    std::vector<NodeRef> build(common::CommonWrapper&,Builder&,WrapperFactory&,
        std::span<const NodeRef> parents)override;
};
void registerXmlNodePlacement(WrapperFactory&);
}
