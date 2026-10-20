/*************************************************************
 * Native CommonMpadded declaration mirroring MathJax 4.1.3.
 * See src/math/LICENSE-MathJax.txt.
 *************************************************************/
export module folia.math.output.common:mpadded;
export import :wrapper;
import folia.math.core.mml_tree.mml_nodes.mpadded;
import std;

export namespace folia::math::output::common {
class CommonMpadded : public CommonWrapper {
  public:
    using CommonWrapper::CommonWrapper;

    SceneNodes emitScene(scene::Builder &, std::span<scene::NodeId const> parents) override;
    double containerWidth() const override;
    std::array<double, 9> getDimens();
    double dimen(std::u16string_view length, BBox const &, std::string_view direction = {},
                 std::optional<double> minimum = {}) const;
    void setBBoxDimens(BBox &);
    std::string getChildAlign(std::size_t) const override;
    double getWrapWidth(std::size_t) override;

  protected:
    void computeBBox(BBox &, bool recompute = false) override;
};
} // namespace folia::math::output::common
