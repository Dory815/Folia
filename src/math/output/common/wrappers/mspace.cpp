module folia.math.output.common;
import :mspace;
import :output_jax;
import folia.math.core.mml_tree.mml_nodes.mspace;
import folia.math.util.property_host;
import std;

namespace folia::math::output::common {
CommonMspace::CommonMspace(CommonWrapperFactory& factory,mml::MmlNode& node,CommonWrapper* parent)
    :CommonWrapper(factory,node,parent){}
void CommonMspace::initializeSource(){CommonWrapper::initializeSource();setBreakStyle();}

bool CommonMspace::canBreak()const{
    const auto* space=dynamic_cast<const mml::MmlMspace*>(node);
    if(!space)throw options::Exception("TypeError",u"CommonMspace requires an MmlMspace node");
    return space->canBreak();
}

std::size_t CommonMspace::breakCount(){return breakStyle.empty()?0:1;}

void CommonMspace::setBreakStyle(std::string_view linebreak){
    if(!linebreak.empty()){breakStyle=std::string(linebreak);return;}
    const auto* space=dynamic_cast<const mml::MmlMspace*>(node);
    if(!space)throw options::Exception("TypeError",u"CommonMspace requires an MmlMspace node");
    breakStyle=(space->hasNewline()||property_host::truthy(node->getProperty("forcebreak")))?"before":"";
}

void CommonMspace::computeBBox(BBox& bbox,bool){
    const auto attributes=node->attributes();
    bbox.w=length2em(attributes->get("width"),0);
    bbox.h=length2em(attributes->get("height"),0);
    bbox.d=length2em(attributes->get("depth"),0);
}

std::shared_ptr<LineBBox> CommonMspace::computeLineBBox(std::size_t index){
    const auto leadingString=node->attributes()->get("data-lineleading");
    const double leading=length2em(leadingString,jax().currentLinebreakOptions().lineleading);
    auto bbox=std::make_shared<LineBBox>(LineBBox::from(BBox::zero(),leading));
    if(index==1){
        bbox->getIndentData(*node);
        bbox->w=getBBox()->w;
        bbox->isFirst=bbox->w==0;
    }
    return bbox;
}
}
