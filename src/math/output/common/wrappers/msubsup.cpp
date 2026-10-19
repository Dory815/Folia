/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/msubsup.ts.
 * Source implementation for msub, msup and msubsup.
 *************************************************************/
module folia.math.output.common;
import :msubsup;
import :scriptbase;
import :wrapper;
import folia.math.core.mml_tree.mml_nodes.msubsup;
import folia.math.util.property_host;
import std;

namespace folia::math::output::common {
namespace {
CommonWrapper* scriptAt(CommonWrapper& wrapper,std::size_t index){
    if(!wrapper.childNodes)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading '0')");
    const auto children=*wrapper.childNodes;
    if(!children)throw options::Exception("TypeError",u"Cannot read properties of null (reading '0')");
    return dynamic_cast<CommonWrapper*>(children->get(index).nodeOrNull());
}
const mml::MmlMsubsup& msubsupNode(const mml::MmlNode& node){
    const auto* result=dynamic_cast<const mml::MmlMsubsup*>(&node);
    if(!result)throw options::Exception("TypeError",u"Script wrapper requires an MmlMsubsup node");
    return *result;
}
}

CommonMsub::CommonMsub(CommonWrapperFactory& factory,mml::MmlNode& node,CommonWrapper* parent,
                       std::optional<std::size_t> scriptChildIndex)
    :CommonScriptbase(factory,node,parent,useIC,
        scriptChildIndex?*scriptChildIndex:msubsupNode(node).sub()){}
CommonWrapper* CommonMsub::scriptChild()const{return scriptAt(*const_cast<CommonMsub*>(this),msubsupNode(*node).sub());}
std::array<double,2> CommonMsub::getOffset()const{
    const double x=baseIsChar?0:getAdjustedIc();
    return {x,-getV()};
}

CommonMsup::CommonMsup(CommonWrapperFactory& factory,mml::MmlNode& node,CommonWrapper* parent,
                       std::optional<std::size_t> scriptChildIndex)
    :CommonScriptbase(factory,node,parent,useIC,
        scriptChildIndex?*scriptChildIndex:msubsupNode(node).sup()){}
CommonWrapper* CommonMsup::scriptChild()const{return scriptAt(*const_cast<CommonMsup*>(this),msubsupNode(*node).sup());}
std::array<double,2> CommonMsup::getOffset()const{
    const double x=getAdjustedIc()-(baseRemoveIc?0:baseIc);
    return {x,getU()};
}

CommonMsubsup::CommonMsubsup(CommonWrapperFactory& factory,mml::MmlNode& node,CommonWrapper* parent,
                             std::optional<std::size_t> scriptChildIndex)
    :CommonScriptbase(factory,node,parent,useIC,
        scriptChildIndex?*scriptChildIndex:msubsupNode(node).sup()){}
CommonWrapper* CommonMsubsup::subChild()const{return scriptAt(*const_cast<CommonMsubsup*>(this),msubsupNode(*node).sub());}
CommonWrapper* CommonMsubsup::supChild()const{return scriptAt(*const_cast<CommonMsubsup*>(this),msubsupNode(*node).sup());}
CommonWrapper* CommonMsubsup::scriptChild()const{return supChild();}

std::shared_ptr<std::array<double,3>> CommonMsubsup::getUVQ(const BBox* subboxValue,const BBox* supboxValue){
    // Source default parameters perform both child getter calls before entering
    // the method, even when the cached UVQ is already populated.
    const auto subbox=subboxValue?std::make_shared<BBox>(*subboxValue):subChild()->getOuterBBox();
    const auto supbox=supboxValue?std::make_shared<BBox>(*supboxValue):supChild()->getOuterBBox();
    const auto base=baseCore;
    const auto bbox=base->getLineBBox(base->breakCount());
    if(UVQ)return UVQ;
    const auto& tex=font->params();const double t=3*tex.rule_thickness;
    const double subscriptshift=length2em(node->attributes()->get("subscriptshift"),tex.sub2);
    const double drop=baseCharZero(bbox->d*baseScale+tex.sub_drop*subbox->rscale);
    const double supd=supbox->d*supbox->rscale;
    const double subh=subbox->h*subbox->rscale;
    double u=getU();double v=std::max(drop,subscriptshift);
    double q=u-supd-(subh-v);
    if(q<t){
        v+=t-q;
        const double p=0.8*tex.x_height-(u-supd);
        if(p>0){u+=p;v-=p;}
    }
    u=std::max(length2em(node->attributes()->get("superscriptshift"),u),u);
    v=std::max(length2em(node->attributes()->get("subscriptshift"),v),v);
    q=u-supd-(subh-v);
    UVQ=std::make_shared<std::array<double,3>>(std::array<double,3>{u,-v,q});
    return UVQ;
}

BBox CommonMsubsup::appendScripts(BBox& bbox)const{
    const auto subbox=subChild()->getOuterBBox();
    const auto supbox=supChild()->getOuterBBox();
    const double w=getBaseWidth();const double x=getAdjustedIc();
    const auto uvq=const_cast<CommonMsubsup*>(this)->getUVQ();
    const auto line=baseChild()->getLineBBox(baseChild()->breakCount());
    const double y=bbox.d-line->d;
    bbox.combine(*subbox,w+(baseIsChar?0:x),(*uvq)[1]-y);
    bbox.combine(*supbox,w+x,(*uvq)[0]-y);
    bbox.w+=font->params().scriptspace;
    return bbox;
}
}
