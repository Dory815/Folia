// MathJax 4.1.3 output/common/Wrappers/ms.ts.
export module folia.math.output.common:ms;
export import :wrapper;
export import folia.math.core.mml_tree.mml_nodes.ms;
import std;

export namespace folia::math::output::common {
class CommonMs : public CommonWrapper {
  public:
    using CommonWrapper::CommonWrapper;
    void initializeSource() override;
    CommonWrapper &createText(std::u16string_view text);
};
} // namespace folia::math::output::common
