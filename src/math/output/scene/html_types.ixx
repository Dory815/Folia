/*************************************************************
 * Context-free immutable payloads for SVG foreignObject content.
 * The HTML snapshot is built from the canonical output DOM adaptor at the
 * consumer boundary; it contains no DOMNodeRef, Context or wrapper owner.
 *************************************************************/
export module folia.math.output.scene.foundation:html_types;
export import :transform_types;
import std;

export namespace folia::math::output::scene {
struct HtmlNodePrimitive {
    std::u16string kind;
    // Source-directed target namespace. Lite DOM itself has no namespace
    // model; this records the namespace required by the consuming SVG
    // foreignObject boundary, rather than claiming a Lite DOM observation.
    std::u16string namespaceHint;
    std::vector<std::pair<std::u16string,std::u16string>> attributes;
    std::u16string text;
    std::vector<HtmlNodePrimitive> children;
};

struct ForeignObjectGeometry {
    // The HTML viewport is in CSS pixels. Its transform converts those pixels
    // into mathematical em units; SVG projects those em units to 1000 units.
    double x=0,y=0,width=0,height=0;
    Transform transform;
    // Controls whether typed geometry fills a missing SVG attribute during
    // serialization. This is a stable emission policy, not a presence cache:
    // Source svg() helper nodes keep their current attributes authoritative,
    // while direct typed Builder geometry retains its declared defaults.
    bool synthesizeMissingAttributes=true;
};
}
