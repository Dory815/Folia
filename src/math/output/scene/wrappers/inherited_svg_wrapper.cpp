/*************************************************************
 * Source-mapped inherited SVG wrappers.
 *************************************************************/
module folia.math.output.scene;
import :inherited_svg_wrapper;
import std;

namespace folia::math::output::scene {
void InheritedSvgWrapperPlacement::placeContent(common::CommonWrapper& wrapper,
    Builder& builder,WrapperFactory& placements,std::span<const NodeRef> roots){
    addSvgBaseChildren(wrapper,builder,placements,roots);
}

void registerInheritedSvgWrappers(WrapperFactory& factory){
    for(const auto* kind:{"mi","mn","ms","mspace","mtext","unknown"})
        factory.registerKind(kind,std::make_shared<InheritedSvgWrapperPlacement>());
}
}
