/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/ms.ts.
 * Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.output.common;
import :ms;
import :wrapper;
import folia.math.core.tree.child_array;
import folia.math.util.property_host;
import std;

namespace folia::math::output::common {
CommonWrapper* CommonMs::createText(std::u16string_view text) {
    auto* textNode=mmlText(std::u16string(text));
    auto* result=wrap(textNode);
    result->parent=this;
    return result;
}

CommonMs::CommonMs(CommonWrapperFactory& factory,mml::MmlNode& source,CommonWrapper* parent)
    :CommonWrapper(factory,source,parent) {}
void CommonMs::initializeSource(){
    CommonWrapper::initializeSource();
    const auto attributes=node->attributes();
    const auto quotes=attributes->getList({"lquote","rquote"});
    Property left=quotes.get("lquote");
    Property right=quotes.get("rquote");
    if(variant!="monospace") {
        if(!attributes->isSet("lquote")&&left==Property("\""))left=Property("“");
        if(!attributes->isSet("rquote")&&right==Property("\""))right=Property("”");
    }
    if(!childNodes)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'unshift')");
    auto children=*childNodes;
    if(!children)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'unshift')");
    const std::array<tree::ChildValue<CommonWrapper>,1> opening{createText(property_host::string(left))};
    children->splice(0,0,opening);
    const std::array<tree::ChildValue<CommonWrapper>,1> closing{createText(property_host::string(right))};
    children->push_back(closing[0]);
}

}
