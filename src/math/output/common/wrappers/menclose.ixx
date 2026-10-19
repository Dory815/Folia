/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/menclose.ts.
 * The selected constructor's real Notations Map remains the source of all
 * notation definitions; this class stores the Source instance's live fields.
 *************************************************************/
export module folia.math.output.common:menclose;
export import :wrapper;
export import :msqrt;
import folia.math.core.mml_tree.mml_nodes.menclose;
import std;

export namespace folia::math::output::common {
class CommonMenclose : public CommonWrapper {
    options::ObjectRef sourceInstance() const;
    options::Value sourceField(std::u16string_view) const;
    void setSourceField(std::u16string_view,options::Value);
    options::Value callSourceMethod(std::u16string_view,
                                    std::span<const options::Value> args={}) const;
    options::Value notation(std::u16string_view) const;
    options::Value callNotation(const options::Value&,std::u16string_view,
                                std::span<const options::Value> args={}) const;
    double paddingValue() const;
    double thicknessValue() const;
    std::array<double,3> arrowheadValue() const;
    void setArrowhead(std::array<double,3>);
    std::vector<std::u16string> notationNames() const;
public:
    CommonMenclose(CommonWrapperFactory&,mml::MmlNode&,CommonWrapper* parent=nullptr);
    void initializeSource() override;
    void getParameters();
    void getNotations();
    void removeRedundantNotations();
    void initializeNotations();
    std::array<double,4> getBBoxExtenders() const;
    std::array<double,4> getPadding() const;
    void maximizeEntries(std::array<double,4>&,const std::array<double,4>&) const;
    double getOffset(std::string_view direction) const;
    std::array<double,2> getArgMod(double width,double height) const;
    options::Value arrow(double width,double angle,bool doubleHeaded,
                         std::string_view offset={},double distance=0) const;
    struct ArrowData { double angle=0,width=0,x=0,y=0; };
    ArrowData arrowData() const;
    std::array<double,2> arrowAW() const;
    CommonMsqrt& createMsqrt(CommonWrapper& child);
    std::array<double,4> sqrtTRBL() const;
protected:
    void computeBBox(BBox&,bool recompute=false) override;
};
}
