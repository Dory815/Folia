/*************************************************************
 * Source output/svg/Wrappers/msubsup.ts.
 *************************************************************/
export module folia.math.output.scene:msubsup;
export import :wrapper;
export import :script_support;
import std;
export namespace folia::math::output::scene {
void registerMsubsupPlacement(WrapperFactory&);
std::vector<NodeRef> buildMsubsupPlacement(common::CommonWrapper&, Builder&, WrapperFactory&,
                                            std::span<const NodeRef>);
}
