// MathJax 4.1.3 output/common/Wrappers/mglyph.ts.
export module folia.math.output.common:mglyph;
export import :wrapper;
export import folia.math.core.mml_tree.mml_nodes.mglyph;
import std;

export namespace folia::math::output::common {
class CommonTextNode;

class CommonMglyph : public CommonWrapper {
    CommonTextNode *charWrapper_{}; // borrowed from the factory's wrapper arena

  public:
    double width{};
    double height{};
    double valign{};
    using CommonWrapper::CommonWrapper;
    void initializeSource() override;
    void getParameters();

    CommonTextNode *charWrapper() const noexcept {
        return charWrapper_;
    }

  protected:
    void computeBBox(BBox &, bool recompute = false) override;
};
} // namespace folia::math::output::common
