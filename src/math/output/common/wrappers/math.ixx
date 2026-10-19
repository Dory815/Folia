// MathJax 4.1.3 output/common/Wrappers/math.ts.
export module folia.math.output.common:math;
export import :wrapper;
export import folia.math.core.mml_tree.mml_nodes.math;

export namespace folia::math::output::common {
class CommonMath : public CommonWrapper {
  public:
    using CommonWrapper::CommonWrapper;
    double getWrapWidth(std::size_t) override;

  protected:
    void computeBBox(BBox &, bool recompute = false) override;
};
} // namespace folia::math::output::common
