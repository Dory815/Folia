/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/mglyph.ts.
 * Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.output.common;
import :mglyph;
import :wrapper;
import folia.math.core.mml_tree.mml_nodes.mglyph;
import folia.math.util.js_numeric_input;
import folia.math.util.js_number;
import folia.math.util.property_host;
import folia.math.util.utf16;
import std;

namespace folia::math::output::common {
namespace {
std::u16string fromCodePoint(double point) {
    if (!std::isfinite(point) || std::trunc(point) != point || point < 0 || point > 0x10FFFF)
        throw options::Exception("RangeError",u"Invalid code point "+property_host::text(js_number::toString(point)));
    const auto cp=static_cast<std::uint32_t>(point);
    if(cp<=0xFFFF)return std::u16string(1,static_cast<char16_t>(cp));
    const auto scalar=cp-0x10000;
    return {static_cast<char16_t>(0xD800+(scalar>>10)),static_cast<char16_t>(0xDC00+(scalar&0x3FF))};
}
}

CommonMglyph::CommonMglyph(CommonWrapperFactory& factory,mml::MmlNode& source,CommonWrapper* parent)
    :CommonWrapper(factory,source,parent){}
void CommonMglyph::initializeSource(){
    CommonWrapper::initializeSource();
    if(!dynamic_cast<mml::MmlMglyph*>(node))
        throw options::Exception("TypeError",u"CommonMglyph requires an MmlMglyph node");
    getParameters();
}

void CommonMglyph::getParameters(){
    const auto attributes=node->attributes();
    const auto values=attributes->getList({"width","height","valign","src","index"});
    const Property widthValue=values.get("width");
    const Property heightValue=values.get("height");
    const Property valignValue=values.get("valign");
    const Property srcValue=values.get("src");
    const Property indexValue=values.get("index");
    if(property_host::truthy(srcValue)){
        width=widthValue==Property("auto")?1:length2em(widthValue);
        height=heightValue==Property("auto")?1:length2em(heightValue);
        valign=length2em(property_host::truthy(valignValue)?valignValue:Property("0"));
        return;
    }
    const double index=js_input::parseInt(property_host::string(indexValue));
    const auto text=fromCodePoint(index);
    auto* raw=dynamic_cast<mml::TextNode*>(node->factoryOwner().create("text",tree::FactoryArgs{}));
    if(!raw)throw options::Exception("TypeError",u"MML factory did not create a TextNode");
    raw->setText(text);
    charWrapper_=wrap(raw);
    if(!charWrapper_)throw options::Exception("TypeError",u"Wrapper factory did not create a text wrapper");
    charWrapper_->parent=this;
}

void CommonMglyph::computeBBox(BBox& bbox,bool){
    if(charWrapper_){
        bbox.updateFrom(*charWrapper_->getBBox());
    }else{
        bbox.w=width;
        bbox.h=height+valign;
        bbox.d=-valign;
    }
}
}
