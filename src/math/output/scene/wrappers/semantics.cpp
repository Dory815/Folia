/*************************************************************
 * MathJax 4.1.3 output/svg/Wrappers/semantics.ts placement.
 *************************************************************/
module folia.math.output.scene;
import :semantics;
import folia.math.util.property_host;
import std;

namespace folia::math::output::scene {
void SemanticsPlacement::placeContent(common::CommonWrapper& base,Builder& builder,
    WrapperFactory& placements,std::span<const NodeRef> parents){
    auto* wrapper=dynamic_cast<common::CommonSemantics*>(&base);
    if(!wrapper)throw options::Exception("TypeError",u"Semantics placement received a non-Semantics wrapper");
    if(!wrapper->childNodes)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'length')");
    if(!*wrapper->childNodes)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'length')");
    if((*wrapper->childNodes)->size()){
        auto* child=dynamic_cast<common::CommonWrapper*>((*wrapper->childNodes)->get(0).requireNode("toSVG"));
        (void)placements.build(*child,builder,parents);
    }
}

void registerSemanticsPlacement(WrapperFactory& factory){
    factory.registerKind("semantics",std::make_shared<SemanticsPlacement>());
}
}
