// MathJax 4.1.3 output/common/Wrappers/math.ts.
export module folia.math.output.common:math;
export import :wrapper;
export import folia.math.core.mml_tree.mml_nodes.math;

export namespace folia::math::output::common {
class CommonMath : public CommonWrapper {
  public:
    using CommonWrapper::CommonWrapper;
    double getWrapWidth(std::size_t) override;
    bool setChildPWidths(bool recompute, std::optional<double> width = std::nullopt,
                         bool clear = true) override;
    SceneNodes emitScene(scene::Builder &, std::span<scene::NodeId const> parents) override;

  protected:
    void computeBBox(BBox &, bool recompute = false) override;
};
} // namespace folia::math::output::common
