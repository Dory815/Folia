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
}
