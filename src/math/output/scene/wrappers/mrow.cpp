/*************************************************************
 * Source-mapped SVG Mrow toSVG/addChildren/placeLines methods.
 *************************************************************/
module folia.math.output.scene;
import :mrow;
import folia.math.util.property_host;
import std;

namespace folia::math::output::scene {
namespace {
double jsMax(double left,double right){
    if(std::isnan(left)||std::isnan(right))return std::numeric_limits<double>::quiet_NaN();
    return std::max(left,right);
}
double jsOrZero(double value){
    return value==0||std::isnan(value)?0:value;
}
std::span<const NodeRef> slicedParents(std::span<const NodeRef> parents,
                                      std::size_t start,std::size_t count){
    if(start>=parents.size())return {};
    return parents.subspan(start,std::min(count,parents.size()-start));
}
double wrapperDx(common::CommonWrapper& wrapper){
    // SvgWrapper.handleStyles is skipped by the inferred-mrow fast path.
    if(auto* row=dynamic_cast<common::CommonMrow*>(&wrapper);
       row&&row->node->isInferred()&&row->breakCount()==0)return 0;
    if(!wrapper.styles||!wrapper.styleData)return 0;
    return wrapper.styleData->padding[3]+
        (wrapper.styleData->border?wrapper.styleData->border->width[3]:0);
}
void addChildren(common::CommonMrow& row,Builder& builder,WrapperFactory& placements,
                 std::span<const NodeRef> parents){
    double x=0;
    std::size_t parentIndex=0;
    if(!row.childNodes)return;
    const auto children=*row.childNodes;
    if(!children)throw options::Exception("TypeError",
        u"object null is not iterable (cannot read property Symbol(Symbol.iterator))");
    for(std::size_t index=0;index<children->size();++index){
        auto* child=children->get(index).requireNode("toSVG");
        const auto breaks=row.node->isEmbellished()?0:child->breakCount();
        const auto childParents=slicedParents(parents,parentIndex,breaks+1);
        const auto dom=placements.build(*child,builder,childParents);
        std::size_t line=0;
        for(const auto& node:dom){
            if(node){
                const double dx=line?0:wrapperDx(*child);
                const auto box=child->getLineBBox(line++);
                x+=(box->L+dx)*box->rscale;
                placeSvgNode(row,builder,node,x,0,false);
                x+=(box->w+box->R-dx)*box->rscale;
            }
            if(breaks)x=0;
        }
        if(breaks){
            const auto box=child->getLineBBox(breaks);
            x+=(box->w+box->R)*box->rscale;
        }
        parentIndex+=breaks;
    }
}
void placeLines(common::CommonMrow& row,Builder& builder,
                std::span<const NodeRef> parents){
    const auto math=row.jax().math;
    if(!math)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'display')");
    const bool display=property_host::truthy(math->display);
    double y=row.dh;
    for(std::size_t index=0;index<parents.size();++index){
        if(index>=row.lineBBox.size()||!row.lineBBox[index])
            throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'L')");
        const auto& line=*row.lineBBox[index];
        placeSvgNode(row,builder,parents[index],jsOrZero(line.L),y,false);
        const double nextHeight=index+1<row.lineBBox.size()&&row.lineBBox[index+1]
            ?row.lineBBox[index+1]->h:0;
        const double leading=display
            ?line.lineLeading.value_or(std::numeric_limits<double>::quiet_NaN()):0;
        y-=jsMax(.25,line.d)+leading+jsMax(.75,jsOrZero(nextHeight));
    }
}
}

std::vector<NodeRef> MrowPlacement::build(common::CommonWrapper& base,Builder& builder,
    WrapperFactory& placements,std::span<const NodeRef> parents){
    auto* row=dynamic_cast<common::CommonMrow*>(&base);
    if(!row)throw options::Exception("TypeError",u"Mrow SVG placement received a non-Mrow wrapper");
    row->getBBox();
    const std::size_t breaks=row->isStack?0:row->breakCount();
    std::vector<NodeRef> roots;
    std::vector<NodeRef> childParents;
    if(breaks||!row->node->isInferred()){
        if(breaks){
            auto lineStack=createSvgLineStack(*row,builder,breaks,parents);
            roots.push_back(lineStack.outer);
            childParents=std::move(lineStack.lines);
        }else{
            roots=createSvgRoots(*row,builder,parents);
            decorateSvgWrapper(*row,builder,roots);
            childParents=roots;
        }
    }else if(row->dh){
        if(parents.empty())throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'append')");
        const auto transform="translate(0 "+row->jax().fixed(row->dh*1000,1)+")";
        auto group=builder.node(u"g",PropertyList{{"transform",Property(transform)}});
        builder.transform(group,Transform{0,row->dh,1,1});
        builder.append(parents.front(),group);
        roots.push_back(group);
        childParents.push_back(group);
    }else{
        roots.assign(parents.begin(),parents.end());
        childParents=roots;
    }
    addChildren(*row,builder,placements,childParents);
    if(breaks)placeLines(*row,builder,childParents);
    return roots;
}

void registerMrowSvgWrapper(WrapperFactory& factory){
    factory.registerKind("mrow",std::make_shared<MrowPlacement>());
    // SvgWrappers.inferredMrow is a distinct Source constructor (and has its
    // own CommonInferredMrow measurement behavior), but its Source toSVG
    // implementation is the inherited Mrow implementation. Dispatch it to
    // that same placement algorithm while retaining the actual wrapper object.
    factory.registerKind("inferredMrow",std::make_shared<MrowPlacement>());
}
}
