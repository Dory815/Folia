/*************************************************************
 * Shared low-level operations used by Source-mapped SVG script placements.
 *************************************************************/
export module folia.math.output.scene:script_support;
export import :wrapper;
import std;
export namespace folia::math::output::scene {
std::vector<NodeRef> scriptChildScene(common::CommonWrapper&,Builder&,WrapperFactory&,std::span<const NodeRef>);
void placeScriptRoots(common::CommonWrapper&,Builder&,const std::vector<NodeRef>&,double,double,bool wrapperRoot=true);
void placeScriptChild(common::CommonWrapper&,Builder&,WrapperFactory&,double,double,std::span<const NodeRef>,bool wrapperRoot=true);
}
