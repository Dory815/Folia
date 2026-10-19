/*************************************************************
 * Native CommonMroot class boundary from MathJax 4.1.3.
 *************************************************************/
export module folia.math.output.common:mroot;
export import :msqrt;
import std;

export namespace folia::math::output::common {
class CommonMroot : public CommonMsqrt {
  public:
    using CommonMsqrt::CommonMsqrt;

    std::optional<std::size_t> root() const override {
        return 1;
    }

    void combineRootBBox(BBox &, BBox const &, double) const override;
    std::array<double, 3> getRootDimens(BBox const &, double) const override;
    double rootHeight(BBox const &, BBox const &, double size, double height) const;
    double rootWidth() const override;
};
} // namespace folia::math::output::common
