/*************************************************************
 * Owned HTML values used by the SVG foreignObject boundary.
 *************************************************************/
export module folia.math.output.scene.html_types;
import std;

export namespace folia::math::output::scene {
struct HtmlNodePrimitive {
    std::u16string kind;
    // Source-directed target namespace; Lite DOM itself has no namespace model.
    std::u16string namespaceHint;
    std::vector<std::pair<std::u16string, std::u16string>> attributes;
    std::u16string text;
    std::vector<HtmlNodePrimitive> children;
};

struct ForeignObjectGeometry {
    // CSS-pixel bounds; em_per_pixel converts CSS pixels to Scene em units.
    double x{}, y{}, width{}, height{};
    double em_per_pixel{1};
};
} // namespace folia::math::output::scene
