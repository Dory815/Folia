/*************************************************************
 * MathJax 4.1.3 output/svg/Wrappers/TeXAtom.ts placement.
 *************************************************************/
module folia.math.output.scene;
import :tex_atom;
import folia.math.core.mml_tree.mml_nodes.tex_atom;
import folia.math.util.property_host;
import std;

namespace folia::math::output::scene {
void TeXAtomPlacement::placeContent(common::CommonWrapper& base,Builder& builder,
    WrapperFactory& placements,std::span<const NodeRef> parents){
    auto* wrapper=dynamic_cast<common::CommonTeXAtom*>(&base);
    if(!wrapper)throw options::Exception("TypeError",u"TeXAtom placement received a non-TeXAtom wrapper");
    addSvgBaseChildren(*wrapper,builder,placements,parents);
}

std::vector<NodeRef> TeXAtomPlacement::build(common::CommonWrapper& base,Builder& builder,
    WrapperFactory& placements,std::span<const NodeRef> parents){
    auto* wrapper=dynamic_cast<common::CommonTeXAtom*>(&base);
    if(!wrapper)throw options::Exception("TypeError",u"TeXAtom placement received a non-TeXAtom wrapper");
    auto roots=SvgWrapperPlacement::build(base,builder,placements,parents);
    if(!roots.empty()){
        auto* node=dynamic_cast<mml::MmlTeXAtom*>(wrapper->node);
        if(!node)throw options::Exception("TypeError",u"TeXAtom placement has an incompatible MML node");
        const auto value=node->texClass();
        const double raw=property_host::number(value);
        const auto index=std::isfinite(raw)&&raw>=0&&raw<static_cast<double>(mml::TEXCLASSNAMES.size())
            ?static_cast<std::size_t>(raw):mml::TEXCLASSNAMES.size();
        const Property name=index<mml::TEXCLASSNAMES.size()
            ?Property(std::string(mml::TEXCLASSNAMES[index])):Property(folia::math::Missing{});
        for(const auto& root:roots)if(root)
            root->attributes.set("data-mjx-texclass",Property(property_host::debugText(property_host::string(name))));
    }
    return roots;
}

void registerTeXAtomPlacement(WrapperFactory& factory){
    factory.registerKind("TeXAtom",std::make_shared<TeXAtomPlacement>());
}
}
