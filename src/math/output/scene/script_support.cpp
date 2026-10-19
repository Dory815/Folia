module folia.math.output.scene;
import :script_support;
import :svg_paint;
import std;
namespace folia::math::output::scene {
std::vector<NodeRef> scriptChildScene(common::CommonWrapper& child,Builder& builder,WrapperFactory& factory,std::span<const NodeRef> parents){return factory.build(child,builder,parents);}
void placeScriptRoots(common::CommonWrapper& wrapper,Builder& builder,const std::vector<NodeRef>& roots,double x,double y,bool wrapperRoot){for(const auto& root:roots)if(root)placeSvgNode(wrapper,builder,root,x,y,wrapperRoot);}
void placeScriptChild(common::CommonWrapper& wrapper,Builder& builder,WrapperFactory& factory,double x,double y,std::span<const NodeRef> parents,bool wrapperRoot){auto result=scriptChildScene(wrapper,builder,factory,parents);placeScriptRoots(wrapper,builder,result,x,y,wrapperRoot);}
}
