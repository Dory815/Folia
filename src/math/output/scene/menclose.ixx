/*************************************************************
 * Source SvgMenclose placement and typed notation paint.
 *************************************************************/
export module folia.math.output.scene:menclose;
export import :wrapper;
export import :svg_paint;
export import folia.math.output.scene.menclose_source;
import std;

export namespace folia::math::output::scene {
class MenclosePlacement final:public SvgWrapperPlacement {
    MencloseSourceService* source_;
protected:
    void placeContent(common::CommonWrapper&,Builder&,WrapperFactory&,
                      std::span<const NodeRef>)override;
public:
    explicit MenclosePlacement(MencloseSourceService& source):source_(&source){}
};
void registerMenclosePlacement(WrapperFactory&,MencloseSourceService&);
}
