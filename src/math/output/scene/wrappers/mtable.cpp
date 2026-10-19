/*************************************************************
 * MathJax output/svg/Wrappers/mtable.ts.
 * Table sizing/stretching remains in CommonMtable. This module performs the
 * Source SVG row/cell placement and emits typed line/frame geometry.
 *************************************************************/
module folia.math.output.scene;
import :mtable;
import :mtd;
import :mtr;
import :svg_paint;
import folia.math.util.property_host;
import std;

namespace folia::math::output::scene {
namespace {
std::string fixed(common::CommonWrapper& wrapper,double value,unsigned digits=1){
    return wrapper.jax().fixed(value*1000,digits);
}
Property fixedProperty(common::CommonWrapper& wrapper,double value,unsigned digits=1){
    return Property(fixed(wrapper,value,digits));
}
Transform matrix(double a,double b,double c,double d,double e=0,double f=0){
    Transform result;TransformOperation operation;operation.kind=TransformKind::Matrix;
    operation.a=a;operation.b=b;operation.c=c;operation.d=d;operation.e=e;operation.f=f;
    result.operations.push_back(operation);return result;
}
NodeRef adoptExistingSvgNode(Builder& builder,common::CommonMtable& table,DOMNodeRef source){
    auto& adaptor=table.jax().domAdaptor();
    const auto kind=adaptor.kind(source);
    PropertyList attributes;
    for(const auto& attribute:adaptor.allAttributes(source))
        attributes.set(property_host::debugText(attribute.name),attribute.value);
    const auto children=adaptor.childNodes(source);

    // SvgMtable's real constructor already created this node through this.svg.
    // Adopt its current DOM state into the Scene; do not report a second paint
    // creation or temporarily replace the observer.
    auto result=builder.adoptExisting(kind,std::move(attributes),
        kind==u"#text"?adaptor.value(source):std::u16string{});
    if(kind==u"g"){
        const auto transform=property_host::string(adaptor.getAttribute(source,u"transform"));
        if(transform==u"matrix(1 0 0 -1 0 0)")
            builder.transform(result,matrix(1,0,0,-1));
    }
    if(kind!=u"#text"){
        for(std::size_t i=0;i<children.slots.size();++i){
            const auto child=propertyNode(children.slots.get(i));
            const auto* reference=std::get_if<DOMNodeRef>(&child);
            if(!reference)throw options::Exception("TypeError",u"Source SVG node has a non-node child");
            builder.append(result,adoptExistingSvgNode(builder,table,*reference));
        }
    }
    return result;
}
std::vector<double> surroundingLines(const std::vector<double>& lines,double edge){
    std::vector<double> result;result.reserve(lines.size()+2);result.push_back(edge);
    result.insert(result.end(),lines.begin(),lines.end());result.push_back(edge);return result;
}
PropertyList lineAttributes(common::CommonMtable& table,double thickness,
                            std::string_view style,std::string_view axis,
                            std::array<std::pair<std::string_view,double>,4> values){
    PropertyList attrs;
    attrs.set("data-line",Property(std::string(axis)));
    attrs.set("class",Property(std::string("mjx-")+std::string(style)));
    for(const auto& [name,value]:values)attrs.set(name,fixedProperty(table,value));
    if(thickness!=.07){
        attrs.set("stroke-thickness",fixedProperty(table,thickness));
        if(style!="solid")attrs.set("stroke-dasharray",Property(
            std::string(style=="dotted"?"0,":"")+fixed(table,2*thickness)));
    }
    return attrs;
}
NodeRef makeVerticalLine(common::CommonMtable& table,Builder& builder,
                         double x,std::string_view style,double thickness){
    const auto box=table.getBBox();
    const double dt=style=="dotted"?thickness/2:0;
    const double X=x+thickness/2;
    auto line=builder.node(u"line",lineAttributes(table,thickness,style,"v",{{
        {"x1",X},{"y1",dt-box->d},{"x2",X},{"y2",box->h-dt}}}));
    // Source setLineThickness writes stroke-thickness, not stroke-width.
    // The actual .mjx table stylesheet therefore keeps its 70-unit stroke.
    line->geometry=LineGeometry{{X,dt-box->d},{X,box->h-dt},.07};
    std::get<LineGeometry>(line->geometry).emitStrokeWidth=false;
    ShapePaint paint;paint.fill="none";
    paint.emitAttributes=false;
    if(style=="dotted"){
        paint.lineCap=LineCap::Round;
        paint.dash=StrokeDash{{0,2*thickness},0,","};
    }else if(style=="dashed")paint.dash=StrokeDash{{2*thickness}};
    line->paint=paint;return line;
}
NodeRef makeHorizontalLine(common::CommonMtable& table,Builder& builder,
                           double y,std::string_view style,double thickness){
    const auto box=table.getBBox();
    const double dt=style=="dotted"?thickness/2:0;
    const double Y=y-thickness/2;
    auto line=builder.node(u"line",lineAttributes(table,thickness,style,"h",{{
        {"x1",dt},{"y1",Y},{"x2",box->w-dt},{"y2",Y}}}));
    line->geometry=LineGeometry{{dt,Y},{box->w-dt,Y},.07};
    std::get<LineGeometry>(line->geometry).emitStrokeWidth=false;
    ShapePaint paint;paint.fill="none";
    paint.emitAttributes=false;
    if(style=="dotted"){
        paint.lineCap=LineCap::Round;
        paint.dash=StrokeDash{{0,2*thickness},0,","};
    }else if(style=="dashed")paint.dash=StrokeDash{{2*thickness}};
    line->paint=paint;return line;
}
NodeRef makeFrame(common::CommonMtable& table,Builder& builder,std::string_view style){
    const auto box=table.getBBox();const double t=table.fLine;
    PropertyList attrs{{"data-frame",Property(true)},
        {"class",Property(std::string("mjx-")+std::string(style))},
        {"width",fixedProperty(table,box->w-t)},
        {"height",fixedProperty(table,box->h+box->d-t)},
        {"x",fixedProperty(table,t/2)},
        {"y",fixedProperty(table,t/2-box->d)}};
    if(t!=.07)attrs.set("stroke-thickness",fixedProperty(table,t));
    auto rect=builder.node(u"rect",std::move(attrs));
    RectGeometry geometry{t/2,t/2-box->d,box->w-t,box->h+box->d-t};
    geometry.fields={RectField::Width,RectField::Height,RectField::X,RectField::Y};
    geometry.fieldCount=4;rect->geometry=geometry;
    ShapePaint paint;paint.fill="none";paint.strokeWidth=.07;
    if(style=="dotted"){
        paint.lineCap=LineCap::Round;
        paint.dash=StrokeDash{{0,2*t},0,","};
    }else if(style=="dashed")paint.dash=StrokeDash{{2*t}};
    paint.emitAttributes=false;
    rect->paint=paint;return rect;
}
void placeRows(common::CommonMtable& table,Builder& builder,WrapperFactory& placements,
               const NodeRef& root,const NodeRef& labels){
    const auto data=table.getTableData();
    const bool equal=property_host::truthy(table.node->attributes()->get("equalrows"));
    const double equalHeight=table.getEqualRowHeight();
    const auto spaces=table.getRowHalfSpacing();
    const auto lines=surroundingLines(table.rLines,table.fLine);
    double y=table.getBBox()->h-lines.front();
    const auto rows=table.tableRows();
    for(std::size_t i=0;i<table.numRows;++i){
        if(i>=rows.size()||i>=data.H.size()||i>=data.D.size())
            throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'H')");
        const double H=equal?(equalHeight+data.H[i]-data.D[i])/2:data.H[i];
        const double D=equal?(equalHeight-data.H[i]+data.D[i])/2:data.D[i];
        const std::array<NodeRef,1> parent={root};
        const auto rowRoots=placements.build(*rows[i],builder,parent);
        // SvgMlabeledtr.toSVG renders childNodes[0] after the SvgMtr body but
        // before the caller places the row itself.
        if(rows[i]->labeled())MtrPlacement::buildLabel(*rows[i],builder,placements,labels);
        if(!rowRoots.empty())placeSvgNode(*rows[i],builder,rowRoots.front(),0,
            y-(i<spaces.size()?spaces[i]:0)-H,true);
        y-=(i<spaces.size()?spaces[i]:0)+H+D+(i+1<spaces.size()?spaces[i+1]:0)+
            (i+1<lines.size()?lines[i+1]:0);
    }
}
void handleColumnLines(common::CommonMtable& table,Builder& builder,const NodeRef& root){
    if(property_host::string(table.node->attributes()->get("columnlines"))==u"none")return;
    const auto lines=table.getColumnAttributes("columnlines");if(!lines)return;
    const auto spaces=table.getColumnHalfSpacing();const auto widths=table.getComputedWidths();
    double x=table.fLine;
    for(std::size_t i=0;i<lines->size();++i){
        x+=(i<spaces.size()?spaces[i]:0)+(i<widths.size()?widths[i]:0)+
            (i+1<spaces.size()?spaces[i+1]:0);
        if((*lines)[i]!="none"){
            const double width=i<table.cLines.size()?table.cLines[i]:0;
            builder.append(root,makeVerticalLine(table,builder,x,(*lines)[i],width));
        }
        if(i<table.cLines.size())x+=table.cLines[i];
    }
}
void handleRowLines(common::CommonMtable& table,Builder& builder,const NodeRef& root){
    if(property_host::string(table.node->attributes()->get("rowlines"))==u"none")return;
    const auto lines=table.getRowAttributes("rowlines");if(!lines)return;
    const bool equal=property_host::truthy(table.node->attributes()->get("equalrows"));
    const auto data=table.getTableData();const double equalHeight=table.getEqualRowHeight();
    const auto spaces=table.getRowHalfSpacing();double y=table.getBBox()->h-table.fLine;
    for(std::size_t i=0;i<lines->size();++i){
        if(i>=data.H.size()||i>=data.D.size())break;
        const double H=equal?(equalHeight+data.H[i]-data.D[i])/2:data.H[i];
        const double D=equal?(equalHeight-data.H[i]+data.D[i])/2:data.D[i];
        y-=(i<spaces.size()?spaces[i]:0)+H+D+(i+1<spaces.size()?spaces[i+1]:0);
        if((*lines)[i]!="none")builder.append(root,makeHorizontalLine(table,builder,y,
            (*lines)[i],i<table.rLines.size()?table.rLines[i]:0));
        if(i<table.rLines.size())y-=table.rLines[i];
    }
}
void handleFrame(common::CommonMtable& table,Builder& builder,const NodeRef& root){
    if(table.frame&&table.fLine)
        builder.append(root,makeFrame(table,builder,property_host::debugText(
            property_host::string(table.node->attributes()->get("frame")))));
}
double handlePWidth(common::CommonMtable& table,Builder& builder,const NodeRef& root){
    if(!table.pWidth)return 0;
    const auto box=table.getBBox();const double W=box->L+table.pWidth+box->R;
    const auto align=table.getAlignShift().first;
    if(!table.container)throw options::Exception("TypeError",
        u"Cannot read properties of null (reading 'getWrapWidth')");
    const double maximum=std::max(table.isTop?W:0,
        table.container->getWrapWidth(table.containerI));
    const double CW=maximum-box->L-box->R;
    const double dw=box->w-(table.pWidth>CW?CW:table.pWidth);
    const double dx=align=="left"?0:align=="right"?dw:dw/2;
    if(dx){
        const auto sourceChildCount=root->children.size();
        auto group=builder.node(u"g",{}, {},sourceChildCount);
        group->children=std::move(root->children);root->children.clear();
        for(std::size_t index=0;index<group->children.size();++index)
            if(const auto child=group->children[index])child->parent=group;
        placeSvgNode(table,builder,group,dx,0,false);builder.append(root,group);
    }
    return dx;
}
void handleLabels(common::CommonMtable& table,Builder& builder,
                  const NodeRef& root,const NodeRef& labels,double dx){
    if(!table.hasLabels)return;
    const auto data=table.getTableData();const auto spaces=table.getRowHalfSpacing();
    const auto rows=table.tableRows();
    const bool equal=property_host::truthy(table.node->attributes()->get("equalrows"));
    const double equalHeight=table.getEqualRowHeight();
    double y=table.getBBox()->h-table.fLine;
    std::size_t labelIndex=0;
    for(std::size_t i=0;i<table.numRows&&i<rows.size();++i){
        auto* row=rows[i];
        const double rawH=i<data.H.size()?(equal?(equalHeight+data.H[i]-data.D[i])/2:data.H[i]):0;
        const double rawD=i<data.D.size()?(equal?(equalHeight-data.H[i]+data.D[i])/2:data.D[i]):0;
        if(row->labeled()){
            if(!row->childNodes||!*row->childNodes||(*row->childNodes)->size()==0)
                throw options::Exception("TypeError",u"Cannot read properties of undefined (reading '0')");
            auto* cell=dynamic_cast<common::CommonMtd*>((*row->childNodes)->get(0).nodeOrNull());
            if(!cell)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'placeCell')");
            y-=(i<spaces.size()?spaces[i]:0)+rawH;
            if(labelIndex>=labels->children.size())
                throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'placeCell')");
            const auto cellRoot=labels->children[labelIndex++];
            if(cellRoot){
                const double rowScale=1/row->getBBox()->rscale;
                const auto [ax,ay]=MtdPlacement::placeCell(*cell,builder,cellRoot,
                    0,y,data.L,rawH*rowScale,rawD*rowScale);
                const double h=rawH*rowScale,d=rawD*rowScale;
                MtdPlacement::placeColor(*cell,cellRoot,-ax,-(d+ay),
                    data.L,h+d);
            }
            y-=rawD+(i+1<spaces.size()?spaces[i+1]:0)+
                (i<table.rLines.size()?table.rLines[i]:0);
        }else y-=(i<spaces.size()?spaces[i]:0)+rawH+rawD+
            (i+1<spaces.size()?spaces[i+1]:0)+(i<table.rLines.size()?table.rLines[i]:0);
    }
    const auto box=table.getBBox();const double W=box->L+(table.pWidth?table.pWidth:box->w)+box->R;
    const double labelWidth=data.L;
    if(table.isTop){
        const auto [pad,align,shift]=table.getPadAlignShift(property_host::debugText(
            property_host::string(table.node->attributes()->get("side"))));
        (void)pad;
        const double x=shift+(align=="right"?-W:align=="center"?-W/2:0)+box->L;
        const double scale=(table.font->params().x_height*1000)/table.metrics().ex;
        Transform outerTransform;
        outerTransform.operations.push_back({.kind=TransformKind::Translate,.x=0,.y=box->h,
            .spaceSeparated=true});
        outerTransform.operations.push_back({.kind=TransformKind::Matrix,.a=1,.d=-1});
        outerTransform.operations.push_back({.kind=TransformKind::Scale,.x=scale,.y=scale,.scalarScale=true});
        auto tableGroup=builder.node(u"g",PropertyList{{"transform",Property("matrix(1 0 0 -1 0 0)")}},
            {},root->children.size());
        builder.transform(tableGroup,matrix(1,0,0,-1));
        tableGroup->children=std::move(root->children);root->children.clear();
        for(std::size_t index=0;index<tableGroup->children.size();++index)
            if(const auto child=tableGroup->children[index])child->parent=tableGroup;
        auto tableView=builder.node(u"svg",PropertyList{
            {"data-table",Property(true)},
            {"preserveAspectRatio",Property(align=="left"?"xMinYMid":align=="right"?"xMaxYMid":"xMidYMid")},
            {"viewBox",Property(fixed(table,-x)+" "+fixed(table,-box->h)+" 1 "+fixed(table,box->h+box->d))}}, {},1);
        ViewportGeometry tableGeometry;tableGeometry.width=tableGeometry.height=1;
        tableGeometry.widthRelative=tableGeometry.heightRelative=true;
        tableGeometry.clipToViewport=false;
        tableGeometry.alignment=align=="left"?ViewportAlignment::XMinYMid:
            align=="right"?ViewportAlignment::XMaxYMid:ViewportAlignment::XMidYMid;
        tableGeometry.emitRect=false;
        tableGeometry.viewX=-x;tableGeometry.viewY=-box->h;tableGeometry.viewWidth=.001;
        tableGeometry.viewHeight=box->h+box->d;tableView->geometry=tableGeometry;
        builder.append(tableView,tableGroup);
        const auto side=property_host::debugText(property_host::string(table.node->attributes()->get("side")));
        auto labelsView=builder.node(u"svg",PropertyList{
            {"data-labels",Property(true)},
            {"preserveAspectRatio",Property(side=="left"?"xMinYMid":"xMaxYMid")},
            {"viewBox",Property((side=="left"?"0":fixed(table,labelWidth))+" "+
                fixed(table,-box->h)+" 1 "+fixed(table,box->h+box->d))}}, {},1);
        ViewportGeometry labelGeometry;labelGeometry.width=labelGeometry.height=1;
        labelGeometry.widthRelative=labelGeometry.heightRelative=true;
        labelGeometry.clipToViewport=false;
        labelGeometry.alignment=side=="left"?ViewportAlignment::XMinYMid:ViewportAlignment::XMaxYMid;
        labelGeometry.viewX=side=="left"?0:labelWidth;labelGeometry.viewY=-box->h;
        labelGeometry.emitRect=false;
        labelGeometry.viewWidth=.001;labelGeometry.viewHeight=box->h+box->d;labelsView->geometry=labelGeometry;
        builder.append(labelsView,labels);
        const std::string transform="translate(0 "+fixed(table,box->h)+") matrix(1 0 0 -1 0 0) scale("+
            table.jax().fixed(scale,2)+")";
        auto outer=builder.node(u"g",PropertyList{{"transform",Property(transform)}}, {},2);
        builder.transform(outer,std::move(outerTransform));
        builder.append(outer,tableView);builder.append(outer,labelsView);
        builder.append(root,outer);placeSvgNode(table,builder,root,-box->L,0,false);
    }else{
        const auto align=table.getAlignShift().first;
        if(!table.container)throw options::Exception("TypeError",
            u"Cannot read properties of null (reading 'getWrapWidth')");
        const double CW=std::max(W,table.container->getWrapWidth(table.containerI));
        const auto side=property_host::debugText(property_host::string(table.node->attributes()->get("side")));
        const double labelX=(side=="left"?(align=="left"?0:align=="right"?W-CW+dx:(W-CW)/2+dx):
            align=="left"?CW:align=="right"?W+dx:(CW+W)/2+dx)-box->L-labelWidth;
        placeSvgNode(table,builder,labels,labelX,0,false);builder.append(root,labels);
    }
}
}

std::vector<NodeRef> MtablePlacement::build(common::CommonWrapper& base,Builder& builder,
    WrapperFactory& placements,std::span<const NodeRef> parents){
    auto* table=dynamic_cast<common::CommonMtable*>(&base);
    if(!table)throw options::Exception("TypeError",u"Mtable placement received a non-Mtable wrapper");
    const auto sourceLabels=table->jax().sourceDOMField(*table,u"labels");
    auto labels=adoptExistingSvgNode(builder,*table,sourceLabels);
    auto roots=createSvgRoots(*table,builder,parents);decorateSvgWrapper(*table,builder,roots);
    if(roots.empty())return roots;
    placeRows(*table,builder,placements,roots.front(),labels);
    handleColumnLines(*table,builder,roots.front());
    handleRowLines(*table,builder,roots.front());
    handleFrame(*table,builder,roots.front());
    const double dx=handlePWidth(*table,builder,roots.front());
    handleLabels(*table,builder,roots.front(),labels,dx);
    return roots;
}
void registerMtablePlacement(WrapperFactory& factory){
    factory.registerKind("mtable",std::make_shared<MtablePlacement>());
}
}
