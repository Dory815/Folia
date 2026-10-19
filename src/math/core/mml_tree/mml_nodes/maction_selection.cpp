/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native selected-child projections for MathJax 4.1.3 maction.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 * TS SHA256: fa173b7e2b25b90e0f83ac201098357a8abf28462fa86fa69d92b65906527895
 * CJS SHA256: eddcdd2111b4b7cf5ad72937b0a4ff7eb555b98308c6771a013a18121af8ce7
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.maction;
import folia.math.core.mml_tree.action_host;
import std;

namespace folia::math::mml {

MmlNode* MmlMaction::selected() const {
    tree::TreeCallGuard<MmlNode> guard;
    const Property selection = host::arithmeticAttributes(*this)->get("selection");
    const auto lengthNodes = childNodes;
    if (!lengthNodes) host::typeError("Cannot read properties of null (reading 'length')");
    const double length = static_cast<double>(lengthNodes->size());
    const double index = host::mathMax(1.0, host::mathMin(length, host::number(selection))) - 1.0;
    const NodeValue child = host::numericChild(childNodes, index);
    if (child.isNode()) return child.nodeOrNull();
    return this->factoryOwner().create("mrow", {});
}

bool MmlMaction::isEmbellished() const {
    tree::TreeCallGuard<MmlNode> guard;
    MmlNode* const child = selected();
    if (!child) host::typeError("Cannot read isEmbellished from null maction selection");
    return child->isEmbellished();
}

bool MmlMaction::isSpacelike() const {
    tree::TreeCallGuard<MmlNode> guard;
    MmlNode* const child = selected();
    if (!child) host::typeError("Cannot read isSpacelike from null maction selection");
    return child->isSpacelike();
}

NodeValue MmlMaction::core() {
    tree::TreeCallGuard<MmlNode> guard;
    const NodeValue child(selected());
    return child.requireNode("core")->core();
}

NodeValue MmlMaction::coreMO() {
    tree::TreeCallGuard<MmlNode> guard;
    const NodeValue child(selected());
    return child.requireNode("coreMO")->coreMO();
}

}
