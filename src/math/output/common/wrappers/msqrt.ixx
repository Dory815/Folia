/*************************************************************
 * Native CommonMsqrt class boundary from MathJax 4.1.3.
 *************************************************************/
export module folia.math.output.common:msqrt;
export import :wrapper;
export import :mo;
export import folia.math.output.scene.types;
import std;

export namespace folia::math::output::common {
struct RadicalGeometry {
    scene::Point surd_origin{};
    scene::Point base_origin{};
    std::optional<scene::Point> degree_origin;
    scene::Rect overbar{};
    double placed_height{};
    double placed_surd_height{};
};

class CommonMsqrt : public CommonWrapper {
    // Menclose borrows an existing source node for its detached radical.
    // The request factory owns the new wrapper; this does not reparent MML.
    mml::MmlNode* projectedBaseNode_{};
  public:
    CommonMo *surd{}; // borrowed from the request-local wrapper factory
    double surdH{};

    CommonMsqrt(CommonWrapperFactory &, mml::MmlNode &, CommonWrapper *parent = nullptr,
                mml::MmlNode* projectedBaseNode = nullptr);
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
    RadicalGeometry layoutGeometry();
    virtual double addRoot(RadicalGeometry &, BBox const &surdBox, double height);
    void getStretchedSurd();
    void invalidateBBox(bool bubble = true) override;

  protected:
    void computeBBox(BBox &, bool recompute = false) override;
};
} // namespace folia::math::output::common
