module folia.math.output.common;
import :maction;
import :mo;
import :wrapper;
import folia.math.output.common.maction_source;
import folia.math.util.property_host;
import std;

namespace folia::math::output::common {
void CommonMaction::initializeSource(){
    CommonWrapper::initializeSource();
    initializeCommonMactionSource(*this);
}

CommonWrapper* CommonMaction::selected(){
    const auto selection=node->attributes()->get("selection");
    double raw=property_host::number(selection);
    const auto count=childNodes&&*childNodes?(*childNodes)->size():0;
    std::size_t index=0;
    if(!std::isnan(raw)){
        const double selected=std::max(1.0,std::min(static_cast<double>(count),raw));
        if(selected>=1&&selected<=static_cast<double>(count)&&std::floor(selected)==selected)index=static_cast<std::size_t>(selected)-1;
        else index=count;
    }else index=count;
    if(index<count){
        const auto children=*childNodes;
        const auto selected=children->get(index);
        if(selected.isNode())return selected.nodeOrNull();
    }
    auto* action=dynamic_cast<mml::MmlMaction*>(node);
    if(!action)throw options::Exception("TypeError",u"maction wrapper has incompatible MML node");
    auto* child=action->selected();
    if(!child)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'kind')");
    return wrapNode(*child,this);
}
void CommonMaction::computeBBox(BBox& box,bool recompute){
    auto* child=selected();
    if(!child)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'getOuterBBox')");
    box.updateFrom(*child->getOuterBBox());
    (void)child->setChildPWidths(recompute);
}
std::size_t CommonMaction::breakCount(){
    auto* child=selected();
    if(node->isEmbellished()){
        auto* mo=child?child->coreMO():nullptr;
        if(!mo)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'embellishedBreakCount')");
        return mo->embellishedBreakCount();
    }
    return child?child->breakCount():0;
}
std::shared_ptr<LineBBox> CommonMaction::computeLineBBox(std::size_t index){
    auto* child=selected();
    if(!child)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'getLineBBox')");
    return getChildLineBBox(*child,index);
}
}
