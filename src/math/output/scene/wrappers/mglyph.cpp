/*************************************************************
 * MathJax 4.1.3 output/svg/Wrappers/mglyph.ts placement.
 *************************************************************/
module folia.math.output.scene;
import :mglyph;
import folia.math.util.property_host;
import std;

namespace folia::math::output::scene {
std::vector<NodeRef> MglyphPlacement::build(common::CommonWrapper& base,Builder& builder,
    WrapperFactory& placements,std::span<const NodeRef> parents){
    auto* wrapper=dynamic_cast<common::CommonMglyph*>(&base);
    if(!wrapper)throw options::Exception("TypeError",u"mglyph placement received a non-mglyph wrapper");
    auto roots=createSvgRoots(*wrapper,builder,parents);
    decorateSvgWrapper(*wrapper,builder,roots);
    if(auto* chars=wrapper->charWrapper()){
        (void)placements.build(*chars,builder,roots);
        return roots;
    }
    if(roots.empty())return roots;
    const auto attributes=wrapper->node->attributes();
    const auto values=attributes->getList({"src","alt"});
    const auto src=values.get("src"),alt=values.get("alt");
    const double height=wrapper->height,width=wrapper->width;
    const double y=wrapper->height+wrapper->valign;
    for(const auto& root:roots){
        if(!root)continue;
        auto image=builder.node(u"image",PropertyList{
            {"width",Property(property_host::debugText(property_host::string(Property(width))))},
            {"height",Property(property_host::debugText(property_host::string(Property(height))))},
            {"preserveAspectRatio",Property("none")},
            {"aria-label",alt},{"href",src}});
        image->semantic=ImagePrimitive{property_host::string(src)};
        image->geometry=RectGeometry{0,0,width,height};
        image->transform=Transform{0,y,1,-1};
        builder.append(root,image);
    }
    return roots;
}

void registerMglyphPlacement(WrapperFactory& factory){
    factory.registerKind("mglyph",std::make_shared<MglyphPlacement>());
}
}
