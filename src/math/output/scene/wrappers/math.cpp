/*************************************************************
 * Source SvgMath.toSVG: common wrapper construction first, then display.
 *************************************************************/
module folia.math.output.scene;
import :math;
import folia.math.core.mml_tree.mml_nodes.math;
import folia.math.util.property_host;
import std;

namespace folia::math::output::scene {
void MathPlacement::placeContent(common::CommonWrapper& base,Builder& builder,
    WrapperFactory& placements,std::span<const NodeRef> parents){
    addSvgBaseChildren(base,builder,placements,parents);
}

std::vector<NodeRef> MathPlacement::build(common::CommonWrapper& base,Builder& builder,
    WrapperFactory& placements,std::span<const NodeRef> parents){
    auto* wrapper=dynamic_cast<common::CommonMath*>(&base);
    if(!wrapper)throw options::Exception("TypeError",u"Math placement received a non-Math wrapper");
    auto roots=SvgWrapperPlacement::build(base,builder,placements,parents);
    auto* node=dynamic_cast<mml::MmlMath*>(wrapper->node);
    if(!node)throw options::Exception("TypeError",u"Math placement has an incompatible MML node");
    const auto attributes=node->attributes();
    if(!attributes)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'get')");
    if(attributes->get("display")==Property("block"))placements.handleDisplay(*wrapper);
    return roots;
}

void registerMathPlacement(WrapperFactory& factory){
    factory.registerKind("math",std::make_shared<MathPlacement>());
}
}
