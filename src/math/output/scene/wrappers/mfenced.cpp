/*************************************************************
 * Source placement for SVG mfenced: its synthetic inferred mrow is rendered
 * by the ordinary registered Mrow placement, with the Source parent swap.
 *************************************************************/
module folia.math.output.scene;
import :mfenced;
import std;

namespace folia::math::output::scene {
void MfencedPlacement::placeContent(common::CommonWrapper& base,Builder& builder,
    WrapperFactory& placements,std::span<const NodeRef> roots){
    auto* wrapper=dynamic_cast<common::CommonMfenced*>(&base);
    if(!wrapper)throw options::Exception("TypeError",u"Mfenced placement received a non-Mfenced wrapper");
    if(!wrapper->mrow)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'toSVG')");
    std::vector<std::pair<common::CommonWrapper*,common::CommonWrapper*>> previous;
    if(wrapper->mrow->childNodes&&*wrapper->mrow->childNodes){
        const auto children=*wrapper->mrow->childNodes;
        previous.reserve(children->size());
        for(std::size_t i=0;i<children->size();++i){
            auto* child=dynamic_cast<common::CommonWrapper*>(children->get(i).requireNode("toSVG"));
            previous.emplace_back(child,child->parent);
            child->parent=wrapper->mrow;
        }
    }
    (void)placements.build(*wrapper->mrow,builder,roots);
    for(auto [child,parent]:previous)child->parent=parent;
}
void registerMfencedPlacement(WrapperFactory& factory){
    factory.registerKind("mfenced",std::make_shared<MfencedPlacement>());
}
}
