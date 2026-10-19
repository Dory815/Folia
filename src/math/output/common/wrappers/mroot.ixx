/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/mroot.ts.
 *************************************************************/
export module folia.math.output.common:mroot;
export import :msqrt;
import std;
export namespace folia::math::output::common {
class CommonMroot : public CommonMsqrt {
public:
    using CommonMsqrt::CommonMsqrt;
    std::optional<std::size_t> root()const override{return 1;}
    void combineRootBBox(BBox&,const BBox&,double)const override;
    std::array<double,3> getRootDimens(const BBox&,double)const override;
    double rootHeight(const BBox&,const BBox&,double size,double H)const;
    double rootWidth()const override;
};
}
