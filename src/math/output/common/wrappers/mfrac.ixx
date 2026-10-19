/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/mfrac.ts.
 * Source-aligned fraction wrapper, using native MML and values.
 *************************************************************/
export module folia.math.output.common:mfrac;
export import :wrapper;
export import :mo;
import folia.math.core.mml_tree.mml_nodes.mfrac;
import std;

export namespace folia::math::output::common {
class CommonMfrac : public CommonWrapper {
  public:
    // Borrowed from the same CommonWrapperFactory arena as this wrapper.
    CommonMo *bevel{};
    double pad{};

    CommonMfrac(CommonWrapperFactory &, mml::MmlNode &, CommonWrapper *parent = nullptr);
    void initializeSource() override;
    void getFractionBBox(BBox &, bool display, double thickness);

    struct TUV {
        double T{}, u{}, v{};
    };

    TUV getTUV(bool display, double thickness) const;
    void getAtopBBox(BBox &, bool display);

    struct UVQ {
        double u{}, v{}, q{};
        BBox nbox, dbox;
    };

    UVQ getUVQ(bool display) const;
    void getBevelledBBox(BBox &, bool display);

    struct BevelData {
        double H{}, delta{}, u{}, v{};
        BBox nbox, dbox;
    };

    BevelData getBevelData(bool display) const;
    bool isDisplay() const;
    bool canStretch(StretchDirection) override;
    std::string getChildAlign(std::size_t) const override;
    double getWrapWidth(std::size_t) override;

  protected:
    void computeBBox(BBox &, bool recompute = false) override;
};
} // namespace folia::math::output::common
