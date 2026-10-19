/*************************************************************
 * Source output/svg/Wrappers/merror.ts placement.
 *************************************************************/
export module folia.math.output.scene:merror;
export import :wrapper;
import std;

export namespace folia::math::output::scene {
class MerrorPlacement final:public Placement {
public:
    std::vector<NodeRef> build(common::CommonWrapper&,Builder&,WrapperFactory&,
                              std::span<const NodeRef>)override;
};
void registerMerrorPlacement(WrapperFactory&);
}
