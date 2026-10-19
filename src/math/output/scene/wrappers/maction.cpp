/*************************************************************
 * MathJax 4.1.3 output/svg/Wrappers/maction.ts placement.
 *************************************************************/
module folia.math.output.scene;
import :maction;
import :svg_paint;
import folia.math.output.common.maction_source;
import folia.math.util.property_host;
import std;

namespace folia::math::output::scene {
void MactionPlacement::placeContent(common::CommonWrapper& base,Builder& builder,
    WrapperFactory& placements,std::span<const NodeRef> roots){
    auto* wrapper=dynamic_cast<common::CommonMaction*>(&base);
    if(!wrapper)throw options::Exception("TypeError",u"Maction placement received a non-Maction wrapper");
    auto* selected=wrapper->selected();
    if(!selected)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'getLineBBox')");
    std::size_t index=0;
    for(const auto& root:roots){
        if(!root)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'append')");
        const auto line=selected->getLineBBox(index++);
        const double dx=wrapper->styleData
            ?wrapper->styleData->padding[3]+(wrapper->styleData->border?wrapper->styleData->border->width[3]:0)
            :0;
        const auto fixed=[&](double value){return wrapper->jax().fixed(value*1000,1);};
        PropertyList attributes{{"width",Property(fixed(line->w))},
            {"height",Property(fixed(line->h+line->d))},
            {"x",index==1?Property(fixed(-dx)):Property(0)},
            {"y",Property(fixed(-line->d))},{"fill",Property("none")},
            {"pointer-events",Property("all")}};
        auto hitbox=builder.node(u"rect",std::move(attributes));
        hitbox->geometry=RectGeometry{index==1?-dx:0,-line->d,line->w,line->h+line->d};
        builder.append(root,hitbox);
    }
    const auto childRoots=placements.build(*selected,builder,roots);
    const auto box=selected->getOuterBBox();
    for(std::size_t i=0;i<childRoots.size();++i)
        if(childRoots[i])placeSvgNode(*selected,builder,childRoots[i],box->L*box->rscale,0,true);
    placements.runMactionAction(*wrapper,builder,roots);
}

void registerMactionPlacement(WrapperFactory& factory){
    factory.registerKind("maction",std::make_shared<MactionPlacement>());
}
}
