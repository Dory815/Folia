/*************************************************************
 * MathJax output/svg/Wrappers/mpadded.ts.
 * CommonMpadded owns dimension parsing and BBox changes; this owner only
 * transfers its nine Source dimensions into Scene placement.
 *************************************************************/
module folia.math.output.scene;
import :mpadded;
import :svg_paint;
import folia.math.util.property_host;
import std;

namespace folia::math::output::scene {
std::vector<NodeRef> MpaddedPlacement::build(common::CommonWrapper& base,
    Builder& builder,WrapperFactory& placements,std::span<const NodeRef> parents){
    auto* wrapper=dynamic_cast<common::CommonMpadded*>(&base);
    if(!wrapper)throw options::Exception("TypeError",u"Mpadded placement received a non-Mpadded wrapper");
    if(auto split=tryEmbellishedSvg(*wrapper,builder,placements,parents))return *split;

    auto roots=createSvgRoots(*wrapper,builder,parents);
    decorateSvgWrapper(*wrapper,builder,roots);
    if(roots.empty())return roots;
    const auto dimensions=wrapper->getDimens();
    const double dw=dimensions[5],x=dimensions[6],y=dimensions[7],dx=dimensions[8];
    const auto alignValue=wrapper->node->attributes()->get("data-align");
    const Property align=property_host::truthy(alignValue)?alignValue:Property("left");
    const bool left=align==Property("left");
    const bool center=align==Property("center");
    const double adjustedWidth=dw<0&&!left?(center?dw/2:dw):0;
    const double X=x+dx-adjustedWidth;
    NodeRef contentRoot=roots.front();
    if(X||y){
        contentRoot=builder.node(u"g");
        builder.append(roots.front(),contentRoot);
        placeSvgNode(*wrapper,builder,contentRoot,X,y,false);
    }
    addSvgBaseChildren(*wrapper,builder,placements,
        std::span<const NodeRef>(&contentRoot,1));
    return roots;
}

void registerMpaddedPlacement(WrapperFactory& factory){
    factory.registerKind("mpadded",std::make_shared<MpaddedPlacement>());
}
}
