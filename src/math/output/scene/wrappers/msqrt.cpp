/*************************************************************
 * Source placement for SVG msqrt/mroot wrappers.  All dimensions remain
 * full precision in scene geometry; serialization alone rounds them.
 *************************************************************/
module folia.math.output.scene;
import :msqrt;
import :svg_paint;
import folia.math.util.property_host;
import std;

namespace folia::math::output::scene {
namespace {
common::CommonWrapper& childAt(common::CommonMsqrt& wrapper,std::size_t index){
    if(!wrapper.childNodes)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'getOuterBBox')");
    auto children=*wrapper.childNodes;
    if(!children)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'getOuterBBox')");
    return *dynamic_cast<common::CommonWrapper*>(children->get(index).requireNode("getOuterBBox"));
}
NodeRef firstRoot(std::vector<NodeRef>& nodes){
    if(nodes.empty()||!nodes.front())throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'transform')");
    return nodes.front();
}
std::vector<NodeRef> placeChild(common::CommonWrapper& wrapper,Builder& builder,
                               WrapperFactory& factory,const NodeRef& parent){
    const std::array<NodeRef,1> parents{parent};
    return factory.build(wrapper,builder,parents);
}
}

std::vector<NodeRef> buildMsqrtContent(common::CommonMsqrt& value,Builder& builder,
    WrapperFactory& placements,std::span<const NodeRef> parents){
    auto* wrapper=&value;
    auto& content=childAt(*wrapper,wrapper->base());
    auto surdBox=wrapper->surd->getBBox();
    auto contentBox=content.getOuterBBox();
    const double q=wrapper->getPQ(*surdBox)[1];
    const double t=wrapper->font->params().surd_height*wrapper->bbox->scale;
    const double H=contentBox->h+q+t;
    auto roots=createSvgRoots(*wrapper,builder,parents);
    decorateSvgWrapper(*wrapper,builder,roots);
    if(roots.empty())throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'append')");
    const auto surdRoots=placements.build(*wrapper->surd,builder,roots);

    double dx=0;
    if(const auto index=wrapper->root()){
        auto& root=childAt(*wrapper,*index);
        const auto rootRoots=placements.build(root,builder,roots);
        const auto dimensions=wrapper->getRootDimens(*surdBox,H);
        dx=dimensions[0];
        const double rootScale=root.getOuterBBox()->rscale;
        for(const auto& rootNode:rootRoots)
            if(rootNode)placeSvgNode(root,builder,rootNode,dimensions[2]*rootScale,dimensions[1],true);
    }

    auto group=builder.node(u"g");
    builder.append(roots.front(),group);
    const std::array<NodeRef,1> contentParent{group};
    auto contentRoots=placements.build(content,builder,contentParent);
    auto contentRoot=firstRoot(contentRoots);
    for(std::size_t i=0;i<roots.size();++i){
        if(!roots[i])continue;
        if(i<surdRoots.size()&&surdRoots[i])
            placeSvgNode(*wrapper->surd,builder,surdRoots[i],dx,H-surdBox->h,true);
    }
    placeSvgNode(content,builder,contentRoot,dx+surdBox->w,0,true);
    const auto fixed=[&](double value){return Property(wrapper->jax().fixed(value*1000,1));};
    auto rule=builder.node(u"rect",PropertyList{{"width",fixed(contentBox->w)},
        {"height",fixed(t)},{"x",fixed(dx+surdBox->w)},{"y",fixed(H-t)}});
    RectGeometry geometry{dx+surdBox->w,H-t,contentBox->w,t};
    geometry.fields={RectField::Width,RectField::Height,RectField::X,RectField::Y};
    rule->geometry=geometry;
    builder.append(roots.back(),rule);
    return roots;
}

std::vector<NodeRef> MsqrtPlacement::build(common::CommonWrapper& base,Builder& builder,
    WrapperFactory& placements,std::span<const NodeRef> parents){
    auto* wrapper=dynamic_cast<common::CommonMsqrt*>(&base);
    if(!wrapper)throw options::Exception("TypeError",u"Msqrt placement received a non-Msqrt wrapper");
    return buildMsqrtContent(*wrapper,builder,placements,parents);
}

void registerMsqrtPlacement(WrapperFactory& factory){
    factory.registerKind("msqrt",std::make_shared<MsqrtPlacement>());
}
}
