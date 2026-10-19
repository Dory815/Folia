/*************************************************************
 * MathJax output/svg/Wrappers/mtd.ts.
 * The cell owns no new layout calculations: CommonMtd supplies its BBox and
 * alignment methods, and the row placement applies the returned translation.
 *************************************************************/
module folia.math.output.scene;
import :mtd;
import :svg_paint;
import folia.math.util.property_host;
import std;

namespace folia::math::output::scene {
std::array<double,2> MtdPlacement::placeCell(common::CommonMtd& cell,Builder& builder,
    const NodeRef& root,double x,double y,double width,double height,double depth){
    const auto box=cell.getBBox();
    const double h=std::max(box->h,0.75),d=std::max(box->d,0.25);
    const auto attrs=cell.node->attributes();
    const double dx=cell.sourceGetAlignX(width,*box,
        property_host::debugText(property_host::string(attrs->get("columnalign"))));
    const double dy=cell.sourceGetAlignY(height,depth,h,d,
        property_host::debugText(property_host::string(attrs->get("rowalign"))));
    placeSvgNode(cell,builder,root,x+dx,y+dy,true);
    return {dx,dy};
}

void MtdPlacement::placeColor(common::CommonMtd& cell,const NodeRef& root,
    double x,double y,double width,double height){
    if(!root||root->children.empty())return;
    const auto background=root->children.front();
    if(!background||background->kind!=u"rect"||
       !property_host::truthy(background->attributes.get("data-bgcolor")))return;
    auto geometry=std::get_if<RectGeometry>(&background->geometry);
    if(!geometry)throw options::Exception("TypeError",u"Cell background has no rectangle geometry");
    auto updated=*geometry;updated.x=x;updated.y=y;updated.width=width;updated.height=height;
    background->geometry=updated;
    background->attributes.set("x",Property(cell.jax().fixed(x*1000,1)));
    background->attributes.set("y",Property(cell.jax().fixed(y*1000,1)));
    background->attributes.set("width",Property(cell.jax().fixed(width*1000,1)));
    background->attributes.set("height",Property(cell.jax().fixed(height*1000,1)));
}

void MtdPlacement::placeContent(common::CommonWrapper& base,Builder& builder,
    WrapperFactory& placements,std::span<const NodeRef> roots){
    auto* cell=dynamic_cast<common::CommonMtd*>(&base);
    if(!cell)throw options::Exception("TypeError",u"Mtd placement received a non-Mtd wrapper");
    addSvgBaseChildren(*cell,builder,placements,roots);
}
std::vector<NodeRef> MtdPlacement::build(common::CommonWrapper& base,Builder& builder,
    WrapperFactory& placements,std::span<const NodeRef> parents){
    auto* cell=dynamic_cast<common::CommonMtd*>(&base);
    if(!cell)throw options::Exception("TypeError",u"Mtd placement received a non-Mtd wrapper");
    return SvgWrapperPlacement::build(base,builder,placements,parents);
}
void registerMtdPlacement(WrapperFactory& factory){
    factory.registerKind("mtd",std::make_shared<MtdPlacement>());
}
}
