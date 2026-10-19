/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/mfrac.ts.
 * Source-aligned fraction wrapper.
 *************************************************************/
export module folia.math.output.common:mfrac;
export import :wrapper;
export import :mo;
import folia.math.core.mml_tree.mml_nodes.mfrac;
import std;

export namespace folia::math::output::common {
class CommonMfrac : public CommonWrapper {
public:
    CommonMo* bevel=nullptr;
    double pad=0;
    CommonMfrac(CommonWrapperFactory&,mml::MmlNode&,CommonWrapper* parent=nullptr);
    void initializeSource() override;
    void getFractionBBox(BBox&,bool display,double thickness);
    struct TUV { double T=0,u=0,v=0; };
    TUV getTUV(bool display,double thickness)const;
    void getAtopBBox(BBox&,bool display);
    struct UVQ { double u=0,v=0,q=0; BBox nbox,dbox; };
    UVQ getUVQ(bool display)const;
    void getBevelledBBox(BBox&,bool display);
    struct BevelData { double H=0,delta=0,u=0,v=0; BBox nbox,dbox; };
    BevelData getBevelData(bool display)const;
    bool isDisplay()const;
    bool canStretch(std::string_view)override;
    std::string getChildAlign(std::size_t)const override;
    double getWrapWidth(std::size_t)override;
    double sourceAlignX(double,const BBox&,std::string_view) const;
    double sourceLength2em(const Property&,double size=1,std::optional<double> scale={}) const;
    // Source-call bridge used by SVG placement; delegates to the same
    // CommonWrapper alignment/length algorithms.
    double getAlignXForSource(double width,const BBox& box,std::string_view align) const {
        return sourceAlignX(width,box,align);
    }
    double length2emForSource(const Property& value,double size=1,
                              std::optional<double> scale={}) const {
        return sourceLength2em(value,size,scale);
    }
protected:
    void computeBBox(BBox&,bool recompute=false)override;
};
}
