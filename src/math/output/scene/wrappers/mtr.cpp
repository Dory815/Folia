/*************************************************************
 * MathJax output/svg/Wrappers/mtr.ts and mlabeledtr.ts.
 * Row/cell sizes and table spacing come only from the Common table owner.
 *************************************************************/
module folia.math.output.scene;
import :mtr;
import :mtd;
import :svg_paint;
import folia.math.util.property_host;
import std;

namespace folia::math::output::scene {
namespace {
std::string fixed(common::CommonWrapper& wrapper,double value,unsigned digits=1){
    return wrapper.jax().fixed(value*1000,digits);
}
}

std::vector<NodeRef> MtrPlacement::build(common::CommonWrapper& base,Builder& builder,
    WrapperFactory& placements,std::span<const NodeRef> parents){
    auto* row=dynamic_cast<common::CommonMtr*>(&base);
    if(!row)throw options::Exception("TypeError",u"Mtr placement received a non-Mtr wrapper");
    auto* table=dynamic_cast<common::CommonMtable*>(row->parent);
    if(!table)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'getColumnHalfSpacing')");
    auto roots=createSvgRoots(*row,builder,parents);
    decorateSvgWrapper(*row,builder,roots);
    if(roots.empty())return roots;

    const auto rows=table->tableRows();
    const auto found=std::find(rows.begin(),rows.end(),row);
    if(found==rows.end())throw options::Exception("TypeError",u"Mtr is absent from its parent table");
    const auto index=static_cast<std::size_t>(found-rows.begin());
    const auto rowBox=row->getBBox();
    const double scale=1/rowBox->rscale;
    const auto data=table->getTableData();
    if(index>=data.H.size()||index>=data.D.size())
        throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'H')");
    const bool equal=property_host::truthy(table->node->attributes()->get("equalrows"));
    const double equalHeight=table->getEqualRowHeight();
    const double H=(equal?(equalHeight+data.H[index]-data.D[index])/2:data.H[index])*scale;
    const double D=(equal?(equalHeight-data.H[index]+data.D[index])/2:data.D[index])*scale;
    const auto columnSpaces=table->getColumnHalfSpacing();
    const auto computedWidths=table->getComputedWidths();
    const auto rowSpaces=table->getRowHalfSpacing();
    std::vector<double> rowLines;
    rowLines.reserve(table->rLines.size()+2);
    rowLines.push_back(table->fLine);
    rowLines.insert(rowLines.end(),table->rLines.begin(),table->rLines.end());
    rowLines.push_back(table->fLine);
    std::vector<double> columnLines;
    columnLines.reserve(table->cLines.size()+2);
    columnLines.push_back(table->fLine);
    columnLines.insert(columnLines.end(),table->cLines.begin(),table->cLines.end());
    columnLines.push_back(table->fLine);
    double x=columnLines.empty()?0:columnLines.front();
    const auto count=row->numCells();
    for(std::size_t i=0;i<count;++i){
        auto* cell=row->getChild(i);
        if(!cell)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'toSVG')");
        const auto cellRoots=placements.build(*cell,builder,roots);
        const double leftSpace=i<columnSpaces.size()?columnSpaces[i]*scale:0;
        const double rightSpace=i+1<columnSpaces.size()?columnSpaces[i+1]*scale:0;
        const double width=i<computedWidths.size()?computedWidths[i]*scale:0;
        const double leftLine=i<columnLines.size()?columnLines[i]*scale:0;
        const double rightLine=i+1<columnLines.size()?columnLines[i+1]*scale:0;
        if(!cellRoots.empty()){
            auto* mtd=dynamic_cast<common::CommonMtd*>(cell);
            if(!mtd)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'placeCell')");
            const auto [ax,ay]=MtdPlacement::placeCell(*mtd,builder,cellRoots.front(),
                x+leftSpace,0,width,H,D);
            const double h=std::max(mtd->getBBox()->h,0.75),d=std::max(mtd->getBBox()->d,0.25);
            const double topSpace=index<rowSpaces.size()?rowSpaces[index]*scale:0;
            const double bottomSpace=index+1<rowSpaces.size()?rowSpaces[index+1]*scale:0;
            const double cellWidth=leftSpace+width+rightSpace;
            MtdPlacement::placeColor(*mtd,cellRoots.front(),
                -(ax+leftSpace+leftLine/2),-(D+bottomSpace+ay),
                cellWidth+(leftLine+rightLine)/2,H+D+topSpace+bottomSpace);
        }
        x+=leftSpace+width+rightSpace+rightLine;
    }
    // SvgMtr.placeColor runs after all cells and updates the row's first
    // existing background rectangle, when that primitive is present.
    if(!roots.front()->children.empty()){
        const auto background=roots.front()->children.front();
        if(background&&background->kind==u"rect"&&
           property_host::truthy(background->attributes.get("data-bgcolor"))){
            const double topSpace=index<rowSpaces.size()?rowSpaces[index]*scale:0;
            const double bottomSpace=index+1<rowSpaces.size()?rowSpaces[index+1]*scale:0;
            const double topLine=index<rowLines.size()?rowLines[index]*scale:0;
            const double bottomLine=index+1<rowLines.size()?rowLines[index+1]*scale:0;
            const double y=-(D+bottomSpace+bottomLine/2);
            const double width=table->getWidth()*scale;
            const double height=topLine/2+topSpace+H+D+bottomSpace+bottomLine/2;
            const auto box=std::get_if<RectGeometry>(&background->geometry);
            if(!box)throw options::Exception("TypeError",u"Row background has no rectangle geometry");
            auto updated=*box;updated.y=y;updated.width=width;updated.height=height;
            background->geometry=updated;
            background->attributes.set("y",Property(fixed(*row,y)));
            background->attributes.set("width",Property(fixed(*row,width)));
            background->attributes.set("height",Property(fixed(*row,height)));
        }
    }
    return roots;
}

NodeRef MtrPlacement::buildLabel(common::CommonMtr& row,Builder& builder,
    WrapperFactory& placements,const NodeRef& labels){
    if(!row.labeled())return {};
    if(!row.childNodes||!*row.childNodes||(*row.childNodes)->size()==0)
        throw options::Exception("TypeError",u"Cannot read properties of undefined (reading '0')");
    auto* cell=dynamic_cast<common::CommonWrapper*>((*row.childNodes)->get(0).nodeOrNull());
    if(!cell)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'toSVG')");
    const std::array<NodeRef,1> parent={labels};
    const auto roots=placements.build(*cell,builder,parent);
    return roots.empty()?NodeRef{}:roots.front();
}

void registerMtrPlacement(WrapperFactory& factory){
    factory.registerKind("mtr",std::make_shared<MtrPlacement>());
    factory.registerKind("mlabeledtr",std::make_shared<MtrPlacement>());
}
}
