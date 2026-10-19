/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/mtr.ts.
 * Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.output.common;
import :mtr;
import :wrapper;
import folia.math.core.tree.child_array;
import std;

namespace folia::math::output::common {
namespace {
tree::FactoryChildren<CommonWrapper> requiredChildren(const CommonMtr& row,std::string_view member) {
    const auto& value=row.childNodes;
    const std::u16string suffix=u" (reading '"+std::u16string(member.begin(),member.end())+u"')";
    if(!value)throw options::Exception("TypeError",u"Cannot read properties of undefined"+suffix);
    if(!*value)throw options::Exception("TypeError",u"Cannot read properties of null"+suffix);
    return *value;
}
CommonWrapper* firstChild(CommonWrapper& cell) {
    if(!cell.childNodes)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading '0')");
    if(!*cell.childNodes)throw options::Exception("TypeError",u"Cannot read properties of null (reading '0')");
    return dynamic_cast<CommonWrapper*>((*cell.childNodes)->get(0).requireNode("canStretch"));
}
}

std::size_t CommonMtr::numCells() const { return requiredChildren(*this,"length")->size(); }
tree::FactoryChildren<CommonWrapper> CommonMtr::tableCells() const { return requiredChildren(*this,"length"); }
CommonWrapper* CommonMtr::getChild(std::size_t index) const {
    return dynamic_cast<CommonWrapper*>(requiredChildren(*this,"length")->get(index).nodeOrNull());
}
std::vector<std::shared_ptr<BBox>> CommonMtr::getChildBBoxes() const {
    const auto cells=requiredChildren(*this,"map");
    std::vector<std::shared_ptr<BBox>> result;
    result.reserve(cells->size());
    for(std::size_t i=0;i<cells->size();++i) {
        auto* cell=dynamic_cast<CommonWrapper*>(cells->get(i).requireNode("getBBox"));
        result.push_back(cell->getBBox());
    }
    return result;
}
void CommonMtr::stretchChildren(std::optional<std::array<double,2>> HD) {
    const auto original=requiredChildren(*this,"slice");
    const auto cells=labeled()?original->slice(1):original;
    std::vector<CommonWrapper*> stretchy;
    for(std::size_t i=0;i<cells->size();++i) {
        auto* cell=dynamic_cast<CommonWrapper*>(cells->get(i).requireNode("childNodes"));
        auto* child=firstChild(*cell);
        if(child->canStretch("v"))stretchy.push_back(child);
    }
    const auto count=stretchy.size();
    const auto nodeCount=requiredChildren(*this,"length")->size();
    if(count&&nodeCount>1&&!HD) {
        double H=0,D=0;
        const bool all=count>1&&count==nodeCount;
        for(std::size_t i=0;i<cells->size();++i) {
            auto* cell=dynamic_cast<CommonWrapper*>(cells->get(i).requireNode("childNodes"));
            auto* child=firstChild(*cell);
            if(!child->stretch)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'dir')");
            const bool noStretch=child->stretch->dir==NONE;
            if(all||noStretch) {
                const auto box=child->getBBox(noStretch);
                if(box->h>H)H=box->h;
                if(box->d>D)D=box->d;
            }
        }
        HD=std::array<double,2>{H,D};
    }
    if(HD)for(auto* child:stretchy) {
        const auto scale=child->coreRScale();
        auto* mo=child->coreMO();
        if(!mo)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'getStretchedVariant')");
        mo->getStretchedVariant(std::array<double,2>{(*HD)[0]*scale,(*HD)[1]*scale});
    }
}

std::size_t CommonMlabeledtr::numCells() const {
    const auto count=CommonMtr::numCells();
    return count?count-1:0;
}
tree::FactoryChildren<CommonWrapper> CommonMlabeledtr::tableCells() const {
    return requiredChildren(*this,"slice")->slice(1);
}
CommonWrapper* CommonMlabeledtr::getChild(std::size_t index) const {
    return dynamic_cast<CommonWrapper*>(requiredChildren(*this,"length")->get(index+1).nodeOrNull());
}
std::vector<std::shared_ptr<BBox>> CommonMlabeledtr::getChildBBoxes() const {
    const auto cells=requiredChildren(*this,"slice")->slice(1);
    std::vector<std::shared_ptr<BBox>> result;
    result.reserve(cells->size());
    for(std::size_t i=0;i<cells->size();++i) {
        auto* cell=dynamic_cast<CommonWrapper*>(cells->get(i).requireNode("getBBox"));
        result.push_back(cell->getBBox());
    }
    return result;
}
}
