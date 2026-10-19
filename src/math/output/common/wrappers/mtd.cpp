/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/mtd.ts.
 *************************************************************/
module folia.math.output.common;
import :mtd;
import :mtr;
import :mtable;
import folia.math.core.mml_tree.mml_nodes.mtd;
import folia.math.util.property_host;
import std;
namespace folia::math::output::common {
void CommonMtd::invalidateBBox(bool){bboxComputed_=false;lineBBox.clear();}
double CommonMtd::getWrapWidth(std::size_t){
    auto* row=dynamic_cast<CommonMtr*>(parent);
    auto* table=row?dynamic_cast<CommonMtable*>(row->parent):nullptr;
    if(!table)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'cWidths')");
    auto position=node->childPosition();
    if(!position)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'childPosition')");
    const auto index=*position-(row->labeled()?1u:0u);
    if(index<table->cWidths.size())if(auto* width=std::get_if<double>(&table->cWidths[index]))return *width;
    auto data=table->getTableData();
    if(index>=data.W.size())throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'getOuterBBox')");
    return data.W[index];
}
std::string CommonMtd::getChildAlign(std::size_t)const{
    const auto value=node->attributes()->get("columnalign");
    if(value.missing()||std::holds_alternative<PropertyNull>(value.value))return {};
    return property_host::debugText(property_host::string(value));
}
}
