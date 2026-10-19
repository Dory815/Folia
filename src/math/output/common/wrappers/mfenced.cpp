module folia.math.output.common;
import :mfenced;
import :wrapper;
import :mrow;
import folia.math.core.mml_tree.mml_nodes.mfenced;
import std;
namespace folia::math::output::common {
CommonMfenced::CommonMfenced(CommonWrapperFactory& factory,mml::MmlNode& node,CommonWrapper* parentValue)
    :CommonWrapper(factory,node,parentValue){}
void CommonMfenced::initializeSource(){
    CommonWrapper::initializeSource();
    createMrow();addMrowChildren();
}
void CommonMfenced::createMrow(){
    const auto rowValue=node->factoryOwner().create("inferredMrow",{});
    auto* row=rowValue;
    row->inheritAttributesFrom(mml::NodeValue(node));
    auto* wrapped=wrapNode(*row,nullptr);
    mrow=dynamic_cast<CommonMrow*>(wrapped);
    if(!mrow)throw options::Exception("TypeError",u"inferredMrow wrapper is not a CommonMrow");
    mrow->parent=this;
}
void CommonMfenced::addMo(mml::MmlNode* value){
    if(!value)return;
    if(!mrow||!mrow->childNodes||!(*mrow->childNodes))throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'push')");
    CommonWrapper* wrapped=wrapNode(*value,nullptr);
    (*mrow->childNodes)->push_back(wrapped);
    wrapped->parent=mrow;
}
void CommonMfenced::addMrowChildren(){
    auto* fenced=dynamic_cast<mml::MmlMfenced*>(node);
    if(!fenced)throw options::Exception("TypeError",u"CommonMfenced requires an MmlMfenced node");
    addMo(fenced->open);
    if(!childNodes)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'length')");
    const auto captured=*childNodes;
    if(!captured)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'length')");
    if(captured->size())(*mrow->childNodes)->push_back(captured->get(0));
    if(!fenced->separators)throw options::Exception("TypeError",u"Cannot read properties of null (reading '0')");
    const auto rest=captured->slice(1);
    std::size_t index=0;
    for(std::size_t i=0;i<rest->size();++i){
        const auto separator=fenced->separators->get(index++);
        addMo(separator.nodeOrNull());
        (*mrow->childNodes)->push_back(rest->get(i));
    }
    addMo(fenced->close);
    mrow->stretchChildren();
}
std::size_t CommonMfenced::breakCount(){return mrow->breakCount();}
void CommonMfenced::computeBBox(BBox& box,bool recompute){
    box.updateFrom(*mrow->getOuterBBox());setChildPWidths(recompute);
}
std::shared_ptr<LineBBox> CommonMfenced::computeLineBBox(std::size_t index){return mrow->getLineBBox(index);}
}
