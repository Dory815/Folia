/*************************************************************
 * MathJax 4.1.3 output/svg/Wrappers/mroot.ts placement.
 *************************************************************/
module folia.math.output.scene;
import :mroot;
import std;

namespace folia::math::output::scene {
std::vector<NodeRef> MrootPlacement::build(common::CommonWrapper& base,Builder& builder,
    WrapperFactory& placements,std::span<const NodeRef> parents){
    auto* wrapper=dynamic_cast<common::CommonMroot*>(&base);
    if(!wrapper)throw options::Exception("TypeError",u"Mroot placement received a non-Mroot wrapper");
    return buildMsqrtContent(*wrapper,builder,placements,parents);
}

void registerMrootPlacement(WrapperFactory& factory){
    factory.registerKind("mroot",std::make_shared<MrootPlacement>());
}
}
