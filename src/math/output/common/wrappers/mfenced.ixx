/*************************************************************
 * Native CommonMfenced declaration mirroring MathJax 4.1.3.
 * See src/math/LICENSE-MathJax.txt.
 *************************************************************/
export module folia.math.output.common:mfenced;
export import :wrapper;
export import :mrow;
import folia.math.core.mml_tree.mml_nodes.mfenced;
import std;

export namespace folia::math::output::common {
class CommonMfenced : public CommonWrapper {
  public:
    // Borrowed wrapper owned by the same CommonWrapperFactory arena.
    CommonMrow *mrow{};

    CommonMfenced(CommonWrapperFactory &, mml::MmlNode &, CommonWrapper *parent = nullptr);
    SceneNodes emitScene(scene::Builder &, std::span<scene::NodeId const> parents) override;
    void initializeSource() override;
    void createMrow();
    void addMrowChildren();
    void addMo(mml::MmlNode *);
    std::size_t breakCount() override;

  protected:
    void computeBBox(BBox &, bool recompute = false) override;
    std::shared_ptr<LineBBox> computeLineBBox(std::size_t) override;
};
} // namespace folia::math::output::common
