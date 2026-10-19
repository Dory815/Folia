/*************************************************************
 * Native CommonMaction declaration mirroring MathJax 4.1.3.
 * See src/math/LICENSE-MathJax.txt.
 *************************************************************/
export module folia.math.output.common:maction;
export import :wrapper;
import folia.math.core.mml_tree.mml_nodes.maction;
import std;

export namespace folia::math::output::common {
class CommonMaction : public CommonWrapper {
  public:
    using CommonWrapper::CommonWrapper;
    void initializeSource() override;
    CommonWrapper *selected();
    std::size_t breakCount() override;

  protected:
    void computeBBox(BBox &, bool recompute = false) override;
    std::shared_ptr<LineBBox> computeLineBBox(std::size_t) override;
};
} // namespace folia::math::output::common
