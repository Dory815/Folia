/*************************************************************
 * MathJax output/svg/Wrappers/mtd.ts inherited wrapper body.
 *************************************************************/
export module folia.math.output.scene:mtd;
export import :wrapper;
export import folia.math.output.common;
import std;

export namespace folia::math::output::scene {
class MtdPlacement final : public SvgWrapperPlacement {
protected:
    void placeContent(common::CommonWrapper&,Builder&,WrapperFactory&,
                      std::span<const NodeRef>) override;
public:
    static std::array<double,2> placeCell(common::CommonMtd&,Builder&,const NodeRef&,
        double x,double y,double width,double height,double depth);
    static void placeColor(common::CommonMtd&,const NodeRef&,double x,double y,
        double width,double height);
    std::vector<NodeRef> build(common::CommonWrapper&,Builder&,WrapperFactory&,
                               std::span<const NodeRef>) override;
};
void registerMtdPlacement(WrapperFactory&);
}
