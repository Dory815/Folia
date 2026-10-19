/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/scriptbase.ts.
 * Shared measurement behavior for sub/sup and under/over wrappers.
 *************************************************************/
export module folia.math.output.common:scriptbase;
export import :wrapper;
export import :mo;
import folia.math.core.mml_tree.mml_nodes.msubsup;
import folia.math.core.mml_tree.mml_nodes.munderover;
import std;

export namespace folia::math::output::common {
class CommonScriptbase : public CommonWrapper {
    bool useItalicCorrection_ = true;
    std::size_t constructorScriptChild_ = 1;

  public:
    CommonWrapper *baseCore = nullptr;
    double baseScale = 1;
    double baseIc = 0;
    bool baseRemoveIc = false;
    bool baseIsChar = false;
    std::optional<bool> baseHasAccentOver;
    std::optional<bool> baseHasAccentUnder;
    bool isLineAbove = false;
    bool isLineBelow = false;
    bool isMathAccent = false;

    CommonScriptbase(CommonWrapperFactory &, mml::MmlNode &, CommonWrapper *parent = nullptr,
                     bool useItalicCorrection = true, std::size_t constructorScriptChild = 1);
    void initializeSource() override;
    virtual CommonWrapper *baseChild() const;
    virtual CommonWrapper *scriptChild() const;
    virtual CommonWrapper *getBaseCore();
    virtual void setBaseAccentsFor(CommonWrapper &);
    virtual CommonWrapper *getSemanticBase();
    virtual CommonWrapper *getBaseFence(CommonWrapper *, std::u16string_view);
    virtual double getBaseScale() const;
    virtual double getBaseIc() const;
    virtual double getAdjustedIc() const;
    virtual bool isCharBase() const;
    virtual void checkLineAccents();
    virtual bool isLineAccent(CommonWrapper *) const;
    virtual double getBaseWidth() const;
    virtual std::array<double, 2> getOffset() const;
    virtual double baseCharZero(double) const;
    virtual double getV() const;
    virtual double getU() const;
    virtual bool hasMovableLimits() const;
    virtual std::array<double, 2> getOverKU(const BBox &, const BBox &) const;
    virtual std::array<double, 2> getUnderKV(const BBox &, const BBox &) const;
    virtual std::vector<double> getDeltaW(const std::vector<BBox> &,
                                          std::array<double, 3> delta = {0, 0, 0}) const;
    virtual double getDelta(CommonWrapper &, bool noskew = false) const;
    virtual void stretchChildren();
    virtual void computeBBox(BBox &, bool recompute = false) override;
    virtual BBox appendScripts(BBox &) const;
    std::size_t breakCount() override;
    CommonWrapper *breakTop(CommonWrapper *, CommonWrapper *) override;
    std::shared_ptr<LineBBox> computeLineBBox(std::size_t) override;
};
} // namespace folia::math::output::common
