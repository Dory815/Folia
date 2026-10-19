/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/munderover.ts.
 *************************************************************/
module folia.math.output.common;
import :munderover;
import :scriptbase;
import :wrapper;
import folia.math.core.mml_tree.mml_nodes.munderover;
import folia.math.util.property_host;
import std;

namespace folia::math::output::common {
namespace {
CommonWrapper* child(CommonWrapper& wrapper,std::size_t index){
    if(!wrapper.childNodes)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading '0')");
    const auto children=*wrapper.childNodes;
    if(!children)throw options::Exception("TypeError",u"Cannot read properties of null (reading '0')");
    auto* result=dynamic_cast<CommonWrapper*>(children->get(index).requireNode("getOuterBBox"));
    if(!result)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'getOuterBBox')");
    return result;
}
const mml::MmlMunderover& limitsNode(const mml::MmlNode& node){
    const auto* result=dynamic_cast<const mml::MmlMunderover*>(&node);
    if(!result)throw options::Exception("TypeError",u"Limits wrapper requires an MmlMunderover node");
    return *result;
}
}

CommonMunder::CommonMunder(CommonWrapperFactory& f,mml::MmlNode& n,CommonWrapper* p)
    :CommonMsub(f,n,p,limitsNode(n).under()){}
void CommonMunder::initializeSource(){CommonMsub::initializeSource();stretchChildren();}
CommonWrapper* CommonMunder::scriptChild()const{return child(*const_cast<CommonMunder*>(this),limitsNode(*node).under());}
void CommonMunder::computeBBox(BBox& bbox,bool recompute){
    if(hasMovableLimits()){CommonScriptbase::computeBBox(bbox,recompute);return;}
    bbox.empty();auto base=baseChild()->getOuterBBox();auto under=scriptChild()->getOuterBBox();
    const auto v=getUnderKV(*base,*under)[1];const auto delta=isLineBelow?0:getDelta(*scriptChild(),true);
    const auto widths=getDeltaW({*base,*under},{0,-delta});
    bbox.combine(*base,widths[0],0);bbox.combine(*under,widths[1],v);
    bbox.d+=font->params().big_op_spacing5;bbox.clean();setChildPWidths(recompute);
}

CommonMover::CommonMover(CommonWrapperFactory& f,mml::MmlNode& n,CommonWrapper* p)
    :CommonMsup(f,n,p,limitsNode(n).over()){}
void CommonMover::initializeSource(){CommonMsup::initializeSource();stretchChildren();}
CommonWrapper* CommonMover::scriptChild()const{return child(*const_cast<CommonMover*>(this),limitsNode(*node).over());}
void CommonMover::computeBBox(BBox& bbox,bool recompute){
    if(hasMovableLimits()){CommonScriptbase::computeBBox(bbox,recompute);return;}
    bbox.empty();auto base=baseChild()->getOuterBBox();auto over=scriptChild()->getOuterBBox();
    if(property_host::truthy(node->attributes()->get("accent")))base->h=std::max(base->h,font->params().x_height*baseScale);
    const auto u=getOverKU(*base,*over)[1];const auto delta=isLineAbove?0:getDelta(*scriptChild());
    const auto widths=getDeltaW({*base,*over},{0,delta});
    bbox.combine(*base,widths[0],0);bbox.combine(*over,widths[1],u);
    bbox.h+=font->params().big_op_spacing5;bbox.clean();setChildPWidths(recompute);
}

CommonMunderover::CommonMunderover(CommonWrapperFactory& f,mml::MmlNode& n,CommonWrapper* p)
    :CommonMsubsup(f,n,p,limitsNode(n).over()){}
void CommonMunderover::initializeSource(){CommonMsubsup::initializeSource();stretchChildren();}
CommonWrapper* CommonMunderover::underChild()const{return child(*const_cast<CommonMunderover*>(this),limitsNode(*node).under());}
CommonWrapper* CommonMunderover::overChild()const{return child(*const_cast<CommonMunderover*>(this),limitsNode(*node).over());}
CommonWrapper* CommonMunderover::subChild()const{return underChild();}
CommonWrapper* CommonMunderover::supChild()const{return overChild();}
void CommonMunderover::computeBBox(BBox& bbox,bool recompute){
    if(hasMovableLimits()){CommonScriptbase::computeBBox(bbox,recompute);return;}
    bbox.empty();auto over=overChild()->getOuterBBox();auto base=baseChild()->getOuterBBox();auto under=underChild()->getOuterBBox();
    if(property_host::truthy(node->attributes()->get("accent")))base->h=std::max(base->h,font->params().x_height*baseScale);
    const auto u=getOverKU(*base,*over)[1],v=getUnderKV(*base,*under)[1];
    const auto odelta=getDelta(*overChild()),udelta=getDelta(*underChild(),true);
    const auto widths=getDeltaW({*base,*under,*over},{0,isLineBelow?0:-udelta,isLineAbove?0:odelta});
    bbox.combine(*base,widths[0],0);bbox.combine(*over,widths[2],u);bbox.combine(*under,widths[1],v);
    const auto z=font->params().big_op_spacing5;bbox.h+=z;bbox.d+=z;bbox.clean();setChildPWidths(recompute);
}
}
