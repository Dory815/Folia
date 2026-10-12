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

void MmlMaction::verifyAttributes(Property options) {
    tree::TreeCallGuard<MmlNode> guard;
    AbstractMmlNode::verifyAttributes(std::move(options));

    const Property actiontype = host::arithmeticAttributes(*this)->get("actiontype");
    if (actiontype != Property(std::string("toggle")) &&
        host::arithmeticAttributes(*this)->hasExplicit("selection")) {
        host::arithmeticAttributes(*this)->unset("selection");
    }
}

NodeValue MmlMaction::setTeXclass(NodeValue previous) {
    tree::TreeCallGuard<MmlNode> guard;
    if (host::arithmeticAttributes(*this)->get("actiontype") == Property(std::string("tooltip")) &&
        host::numericChild(childNodes, 1).isNode()) {
        host::numericChild(childNodes, 1).requireNode("setTeXclass")->setTeXclass(PropertyNull{});
    }
    const NodeValue selectedNode(selected());
    const NodeValue result = selectedNode.requireNode("setTeXclass")->setTeXclass(previous);
    updateTeXclass(selectedNode);
    return result;
}

void MmlMaction::nextToggleSelection() {
    tree::TreeCallGuard<MmlNode> guard;
    const Property rawSelection = host::arithmeticAttributes(*this)->get("selection");
    double selection = host::mathMax(1.0, host::integer(host::string(rawSelection)) + 1.0);
    const auto children = childNodes;
    if (!children) host::typeError("Cannot read properties of null (reading 'length')");
    if (selection > static_cast<double>(children->size())) selection = 1.0;
    host::arithmeticAttributes(*this)->set("selection", selection);
}

void MmlMaction::setChildInheritedAttributes(AttributeList attributes,
                                              Property display,
                                              Property level,
                                              Property prime) {
    tree::TreeCallGuard<MmlNode> guard;
    const Property actiontype = host::arithmeticAttributes(*this)->get("actiontype");
    if (host::lowerString(actiontype) != u"tooltip") {
        AbstractMmlNode::setChildInheritedAttributes(attributes, display, level, prime);
        return;
    }
    const NodeValue base = host::numericChild(childNodes, 0);
    if (base.isNode()) base.requireNode("setInheritedAttributes")->setInheritedAttributes(attributes, display, level, prime);
    const NodeValue tip = host::numericChild(childNodes, 1);
    if (tip.isNode()) tip.requireNode("setInheritedAttributes")->setInheritedAttributes(attributes, false, 1, false);
}

} // namespace folia::math::mml
