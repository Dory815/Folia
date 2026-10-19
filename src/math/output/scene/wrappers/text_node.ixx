/*************************************************************
 * MathJax 4.1.3 output/svg/Wrappers/TextNode.ts placement.
 *************************************************************/
export module folia.math.output.scene:text_node;
export import :wrapper;
export import :glyph_paint;
export import folia.math.output.common;
import std;

export namespace folia::math::output::scene {
class TextNodePlacement final : public Placement {
public:
    std::vector<NodeRef> build(common::CommonWrapper&,Builder&,WrapperFactory&,
                               std::span<const NodeRef>) override;
};
void registerTextNodePlacement(WrapperFactory&);
}
