/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native port of MathJax 4.1.3 MmlNodes/maction.ts behavior methods.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 * Source SHA256: fa173b7e2b25b90e0f83ac201098357a8abf28462fa86fa69d92b65906527895
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.maction;
import folia.math.core.mml_tree.action_host;
import std;

namespace folia::math::mml {

void MmlMaction::verifyAttributes(PropertyList options) {
    tree::TreeCallGuard<MmlNode> guard;
    AbstractMmlNode::verifyAttributes(std::move(options));

    const Property actiontype = host::arithmeticAttributes(*this)->get("actiontype");
    if (actiontype != Property(std::string("toggle")) &&
        host::arithmeticAttributes(*this)->hasExplicit("selection")) {
        host::arithmeticAttributes(*this)->unset("selection");
    }
}

NodeValue MmlMaction::setTeXclass(NodeValue previous) {
    // P23f Root stub: MmlMaction::setTeXclass.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

void MmlMaction::nextToggleSelection() {
    // P23f Root stub: MmlMaction::nextToggleSelection.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

void MmlMaction::setChildInheritedAttributes(AttributeList attributes,
                                              Property display,
                                              Property level,
                                              Property prime) {
    // P23f Root stub: MmlMaction::setChildInheritedAttributes.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

} // namespace folia::math::mml
