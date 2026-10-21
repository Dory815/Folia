/*************************************************************
 * Candidate native port of MathJax 4.1.3 output/common/Wrappers/mrow.ts.
 * See src/math/LICENSE-MathJax.txt.
 *************************************************************/
export module folia.math.output.common:mrow;
export import :wrapper;
export import :mo;
import std;

export namespace folia::math::output::common {
class CommonMrow : public CommonWrapper {
  public:
    bool isStack{};
    double dh{};

    CommonMrow(CommonWrapperFactory &, mml::MmlNode &, CommonWrapper *parent = nullptr);
    void initializeSource() override;
    void stretchChildren();
    bool fixesPWidth() const override;
    std::size_t breakCount() override;
    CommonWrapper *breakTop(CommonWrapper *, CommonWrapper *) override;
    std::shared_ptr<LineBBox> getLineBBox(std::size_t) override;
    bool setChildPWidths(bool, std::optional<double> width = {}, bool clear = true) override;
    void breakToWidth(double) override;
    SceneNodes emitScene(scene::Builder &, std::span<scene::NodeId const> parents) override;
    double sceneDx() override;

  protected:
    SceneContainers createSceneNodes(scene::Builder &, std::span<scene::NodeId const>) override;
    void computeBBox(BBox &, bool recompute = false) override;
    void addSceneChildren(scene::Builder &, std::span<scene::NodeId const>) override;

  private:
    void computeChildLineBBox(CommonWrapper &, std::size_t);
    void computeLinebreakBBox(BBox &);
    void shiftLines(BBox &);
    void vboxAdjust(BBox &);
};
} // namespace folia::math::output::common
