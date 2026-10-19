/*************************************************************
 * MathJax 4.1.3 output/svg/Wrapper.ts paint and wrapper-decoration stage.
 * Geometry uses the actual CommonWrapper metrics; it never recomputes layout.
 *************************************************************/
export module folia.math.output.scene:svg_paint;
export import folia.math.output.scene.foundation;
export import folia.math.output.common;
import std;

export namespace folia::math::output::scene {
struct SvgLineStack {
    NodeRef outer;
    std::vector<NodeRef> lines;
};

// Source createSvgNodes/handleHref phase. Roots and hit boxes are created
// before wrapper-specific children, preserving source paint/DOM order.
std::vector<NodeRef> createSvgRoots(common::CommonWrapper&,Builder&,
                                    std::span<const NodeRef> parents);
SvgLineStack createSvgLineStack(common::CommonWrapper&,Builder&,std::size_t breakCount,
                                std::span<const NodeRef> parents);
// Applies the Source SvgWrapper standard-node decoration order to the already
// placed scene roots: styles, scale, border, colors/background, then MathML
// attributes/classes. `lineIndex` selects the corresponding wrapped line for
// multi-root output (inline breaks).
void decorateSvgWrapper(common::CommonWrapper&,Builder&,std::span<const NodeRef>,
                        std::size_t lineIndex=0);

// Source SvgWrapper.place. `wrapperRoot` selects the default-root path, which
// applies dx and the id alignment-box rewrite before composing its transform.
void placeSvgNode(common::CommonWrapper&,Builder&,const NodeRef& target,
                  double x,double y,bool wrapperRoot);
}
