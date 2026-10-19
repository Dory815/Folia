// MathJax 4.1.3 output/common/Wrappers/mo.ts Source-aligned wrapper class.
export module folia.math.output.common:mo;
export import :wrapper;
import std;

export namespace folia::math::output::common {
class CommonMo : public CommonWrapper {
public:
    std::optional<double> size;
    bool isAccent=false;
    std::string breakStyle;
    CommonMo* multChar=nullptr;
    CommonMo(CommonWrapperFactory&,mml::MmlNode&,CommonWrapper* parent=nullptr);
    void initializeSource() override;
    std::size_t breakCount() override;
    std::size_t embellishedBreakCount() const;
    std::string embellishedBreakStyle() const;
    void setBreakStyle(std::string_view linebreak={});
    std::string getBreakStyle(std::string_view linebreak={}) const;
    void protoBBox(BBox&);
    void computeBBox(BBox&,bool recompute=false) override;
    std::shared_ptr<LineBBox> computeLineBBox(std::size_t) override;
    LineBBox moLineBBox(std::size_t,std::string_view,const BBox* obox=nullptr);
    double getAccentOffset();
    double getCenterOffset(BBox* bbox=nullptr);
    void getStretchedVariant(std::span<const double> dimensions,bool exact=false);
    double getSize(std::string_view name,double fallback);
    double getWH(std::span<const double> dimensions)const;
    void getStretchBBox(std::span<const double> requested,double dimension,const DelimiterData&);
    std::array<double,2> getBaseline(std::span<const double> requested,double dimension,const DelimiterData&)const;
    double checkExtendedHeight(double dimension,const DelimiterData&)const;
    void getVariant() override;
    std::shared_ptr<std::vector<std::uint32_t>> remapChars(
        std::shared_ptr<std::vector<std::uint32_t>> chars) const override;
    bool canStretch(std::string_view direction) override;
private:
    void setDelimSize(double character,double index);
};
}
