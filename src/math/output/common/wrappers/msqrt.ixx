/*************************************************************
 * Native CommonMsqrt class boundary from MathJax 4.1.3.
 *************************************************************/
export module folia.math.output.common:msqrt;
export import :wrapper;
export import :mo;
import std;

export namespace folia::math::output::common {
class CommonMsqrt : public CommonWrapper {
  public:
    CommonMo *surd{}; // borrowed from the request-local wrapper factory
    double surdH{};

    CommonMsqrt(CommonWrapperFactory &, mml::MmlNode &, CommonWrapper *parent = nullptr);
    void initializeSource() override;

    virtual std::size_t base() const {
        return 0;
    }

    virtual std::optional<std::size_t> root() const {
        return std::nullopt;
    }

    virtual void combineRootBBox(BBox &, BBox const &, double) const;
    virtual std::array<double, 2> getPQ(BBox const &) const;
    virtual std::array<double, 3> getRootDimens(BBox const &, double) const;
    virtual double rootWidth() const;
    void getStretchedSurd();
    void invalidateBBox(bool bubble = true) override;

  protected:
    void computeBBox(BBox &, bool recompute = false) override;
};
} // namespace folia::math::output::common
