/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium
 * Native constructor/default selection for MmlFactory.ts (4.1.3).
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.mml_factory;
import std;

namespace folia::math::mml {
namespace {
const tree::FactoryClassMap<MmlNode>* selectedClassDefaults(
    const std::optional<tree::FactoryClassMap<MmlNode>>& nodes,
    const tree::FactoryClassMap<MmlNode>* injectedDefaults) {
    if (nodes || injectedDefaults) return injectedDefaults;
    const auto& defaults = MmlFactory::defaultNodes;
    if (!defaults) {
        throw tree::FactoryError("TypeError", u"MmlFactory.defaultNodes is null");
    }
    return defaults.get();
}
}

MmlFactory::MmlFactory(
    std::optional<tree::FactoryClassMap<MmlNode>> nodes,
    const tree::FactoryClassMap<MmlNode>* classDefaults)
    : Base(std::move(nodes), selectedClassDefaults(nodes, classDefaults)) {}

tree::FactoryCreatorView<MmlNode> MmlFactory::MML() const {
    return creatorView();
}
MmlNode* MmlFactory::adopt(std::unique_ptr<MmlNode> product){
    MmlNode* const result=Base::adopt(std::move(product));
    if(identityBinder_){
        const auto owner=weak_from_this().lock();
        if(!owner)throw tree::FactoryError("Unimplemented",u"MmlFactory Context identity requires an actual shared factory owner");
        identityBinder_(*result,owner);
    }
    return result;
}
MmlNode* MmlFactory::create(std::string_view kind,tree::FactoryArgs args){
    // Every registered class creator calls the virtual FactoryServices::adopt
    // with its actual product; publishing there covers creatorView calls and
    // both typed and FactoryArgs create overloads exactly once.
    std::array<std::any,2> normalized;
    normalized[0]=args.size()>0&&args[0].has_value()?args[0]:std::any(PropertyList{});
    normalized[1]=args.size()>1&&args[1].has_value()?args[1]:
        std::any(tree::FactoryChildren<MmlNode>(std::make_shared<tree::ChildArray<MmlNode>>()));
    return Base::createWithArguments(kind,[normalized]{
        return std::vector<std::any>(normalized.begin(),normalized.end());
    });
}
MmlNode* MmlFactory::create(std::string_view kind,PropertyList properties,
                            tree::FactoryChildren<MmlNode> children){
    if(!children)children=std::make_shared<tree::ChildArray<MmlNode>>();
    const std::array<std::any,2> arguments{std::any(std::move(properties)),std::any(std::move(children))};
    return Base::createWithArguments(kind,[arguments]{
        return std::vector<std::any>(arguments.begin(),arguments.end());
    });
}
}
