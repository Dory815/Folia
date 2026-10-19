/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/semantics.ts.
 * Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
export module folia.math.output.common:semantics;
export import :wrapper;
export import folia.math.core.mml_tree.mml_nodes.semantics;

export namespace folia::math::output::common {
class CommonSemantics : public CommonWrapper {
public:
    using CommonWrapper::CommonWrapper;
    std::size_t breakCount() override;
protected:
    void computeBBox(BBox&, bool recompute=false) override;
};
}
