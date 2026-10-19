/*************************************************************
 * Source output/svg/Wrappers/scriptbase.ts (msub/msup inherited script placement).
 *************************************************************/
export module folia.math.output.scene:scriptbase;
export import :wrapper;
export import :script_support;
import std;
export namespace folia::math::output::scene {
void registerScriptbasePlacement(WrapperFactory&);
std::vector<NodeRef> buildScriptbasePlacement(common::CommonWrapper&, Builder&, WrapperFactory&,
                                               std::span<const NodeRef>);
}
