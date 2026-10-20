/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/TeXAtom.ts.
 * Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
export module folia.math.output.common:tex_atom;
export import :wrapper;
export import folia.math.core.mml_tree.mml_nodes.tex_atom;

export namespace folia::math::output::common {
class CommonTeXAtom : public CommonWrapper {
public:
    using CommonWrapper::CommonWrapper;
    SceneNodes emitScene(scene::Builder &, std::span<scene::NodeId const> parents) override;

  protected:
    void computeBBox(BBox&, bool recompute=false) override;
};
}
