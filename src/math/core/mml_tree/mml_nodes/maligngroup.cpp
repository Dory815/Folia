module folia.math.core.mml_tree.mml_nodes.maligngroup;
import folia.math.core.mml_tree.node_defaults;
import std;

namespace folia::math::mml {
AttributeSet MmlMaligngroup::defaults() {
    return nodeDefaultAttributes(NodeKind::maligngroup);
}

NodeKind MmlMaligngroup::kind() const noexcept {
    return NodeKind::maligngroup;
}

bool MmlMaligngroup::isSpacelike() const {
    return true;
}

void MmlMaligngroup::setChildInheritedAttributes(AttributeList const &incoming, bool display,
                                                 double level, bool prime) {
    AttributeAdditions explicitValues;
    explicitValues.reserve(attributes().ownEntries().size());
    for (const auto &[name, value] : attributes().ownEntries())
        explicitValues.emplace_back(name, value);
    auto propagated = addInheritedAttributes(incoming, explicitValues);
    AbstractMmlLayoutNode::setChildInheritedAttributes(propagated, display, level, prime);
}
} // namespace folia::math::mml
