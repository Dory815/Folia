/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/mn.ts.
 *************************************************************/
export module folia.math.output.common:mn;
export import :wrapper;
import std;
export namespace folia::math::output::common {
class CommonMn:public CommonWrapper {
public:
    using CommonWrapper::CommonWrapper;
    std::shared_ptr<std::vector<std::uint32_t>> remapChars(
        std::shared_ptr<std::vector<std::uint32_t>>)const override;
};
}
