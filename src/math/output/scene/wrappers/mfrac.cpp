/*************************************************************
 * MathJax 4.1.3 output/svg/Wrappers/mfrac.ts placement equations.
 * Fraction metrics and child BBoxes come from CommonMfrac; this file only
 * transfers those Source placements into full-precision scene geometry.
 *************************************************************/
module folia.math.output.scene;
import :mfrac;
import :svg_paint;
import folia.math.util.property_host;
import std;

namespace folia::math::output::scene {
namespace {
common::CommonWrapper& childAt(common::CommonMfrac& wrapper,std::size_t index){
    if(!wrapper.childNodes)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'getOuterBBox')");
    const auto children=*wrapper.childNodes;
    if(!children)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'getOuterBBox')");
    auto* value=dynamic_cast<common::CommonWrapper*>((*children).get(index).requireNode("getOuterBBox"));
    return *value;
}
std::vector<NodeRef> childScene(common::CommonWrapper& child,Builder& builder,
                               WrapperFactory& factory,std::span<const NodeRef> parents){
    return factory.build(child,builder,parents);
}
NodeRef first(std::vector<NodeRef>& nodes){
    if(nodes.empty()||!nodes.front())throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'transform')");
    return nodes.front();
}
double jsMax(double a,double b){
    if(std::isnan(a)||std::isnan(b))return std::numeric_limits<double>::quiet_NaN();
    return std::max(a,b);
}
}

std::vector<NodeRef> MfracPlacement::build(common::CommonWrapper& base,Builder& builder,
    WrapperFactory& placements,std::span<const NodeRef> parents){
    auto* wrapper=dynamic_cast<common::CommonMfrac*>(&base);
    if(!wrapper)throw options::Exception("TypeError",u"Mfrac placement received a non-Mfrac wrapper");
    if(auto split=tryEmbellishedSvg(*wrapper,builder,placements,parents))return *split;
    auto roots=createSvgRoots(*wrapper,builder,parents);
    decorateSvgWrapper(*wrapper,builder,roots);
    if(roots.empty())return roots;
    const auto attributes=wrapper->node->attributes();
    const bool display=wrapper->isDisplay();
    const bool bevelled=property_host::truthy(attributes->get("bevelled"));
    auto& numerator=childAt(*wrapper,0);
    auto& denominator=childAt(*wrapper,1);
    auto nbox=numerator.getOuterBBox();
    auto dbox=denominator.getOuterBBox();
    if(bevelled){
        if(!wrapper->bevel)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'getOuterBBox')");
        const auto data=wrapper->getBevelData(display);
        auto n=childScene(numerator,builder,placements,roots);
        auto slash=childScene(*wrapper->bevel,builder,placements,roots);
        auto d=childScene(denominator,builder,placements,roots);
        const double w=(data.nbox.L+data.nbox.w+data.nbox.R)*data.nbox.rscale;
        placeSvgNode(numerator,builder,first(n),data.nbox.L*data.nbox.rscale,data.u,true);
        placeSvgNode(*wrapper->bevel,builder,first(slash),w-data.delta/2,0,true);
        placeSvgNode(denominator,builder,first(d),w+wrapper->bevel->getOuterBBox()->w+
                     data.dbox.L*data.dbox.rscale-data.delta,data.v,true);
        return roots;
    }
    const auto line=attributes->get("linethickness");
    const double thickness=wrapper->sourceLength2em(line,0.06);
    const double axis=wrapper->font->params().axis_height;
    if(thickness==0){
        const auto data=wrapper->getUVQ(display);
        const double width=jsMax((data.nbox.L+data.nbox.w+data.nbox.R)*data.nbox.rscale,
                                 (data.dbox.L+data.dbox.w+data.dbox.R)*data.dbox.rscale);
        const auto numAlign=property_host::debugText(property_host::string(attributes->get("numalign")));
        const auto denAlign=property_host::debugText(property_host::string(attributes->get("denomalign")));
        const double nx=wrapper->sourceAlignX(width,data.nbox,numAlign)+wrapper->pad;
        const double dx=wrapper->sourceAlignX(width,data.dbox,denAlign)+wrapper->pad;
        auto n=childScene(numerator,builder,placements,roots);
        placeSvgNode(numerator,builder,first(n),nx,data.u,true);
        auto d=childScene(denominator,builder,placements,roots);
        placeSvgNode(denominator,builder,first(d),dx,-data.v,true);
        return roots;
    }
    const auto data=wrapper->getTUV(display,thickness);
    const double W=jsMax((nbox->L+nbox->w+nbox->R)*nbox->rscale,
                         (dbox->L+dbox->w+dbox->R)*dbox->rscale);
    const auto numAlign=property_host::debugText(property_host::string(attributes->get("numalign")));
    const auto denAlign=property_host::debugText(property_host::string(attributes->get("denomalign")));
    const double nx=wrapper->sourceAlignX(W,*nbox,numAlign)+0.1+wrapper->pad;
    const double dx=wrapper->sourceAlignX(W,*dbox,denAlign)+0.1+wrapper->pad;
    auto n=childScene(numerator,builder,placements,roots);
    placeSvgNode(numerator,builder,first(n),nx,axis+data.T+jsMax(nbox->d*nbox->rscale,data.u),true);
    auto d=childScene(denominator,builder,placements,roots);
    placeSvgNode(denominator,builder,first(d),dx,axis-data.T-jsMax(dbox->h*dbox->rscale,data.v),true);
    const auto fixed=[&](double value){return Property(wrapper->jax().fixed(value*1000,1));};
    auto rule=builder.node(u"rect",PropertyList{{"width",fixed(W+0.2)},
        {"height",fixed(thickness)},{"x",fixed(wrapper->pad)},{"y",fixed(axis-thickness/2)}});
    RectGeometry geometry{wrapper->pad,axis-thickness/2,W+0.2,thickness};
    geometry.fields={RectField::Width,RectField::Height,RectField::X,RectField::Y};
    rule->geometry=geometry;
    builder.append(roots.front(),rule);
    return roots;
}

void registerMfracPlacement(WrapperFactory& factory){factory.registerKind("mfrac",std::make_shared<MfracPlacement>());}
}
