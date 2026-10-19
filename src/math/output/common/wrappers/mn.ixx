// MathJax 4.1.3 output/common/Wrappers/mn.ts.
export module folia.math.output.common:mn;
export import :wrapper;
import std;

export namespace folia::math::output::common {
class CommonMn : public CommonWrapper {
  public:
    using CommonWrapper::CommonWrapper;
    std::vector<std::uint32_t> remapChars(std::span<std::uint32_t const>) const override;
};
} // namespace folia::math::output::common
