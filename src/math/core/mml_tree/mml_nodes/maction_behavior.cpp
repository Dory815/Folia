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

MmlNode* MmlMaction::setTeXclass(MmlNode* previous) {
    tree::TreeCallGuard<MmlNode> guard;

    const Property actiontype = host::arithmeticAttributes(*this)->get("actiontype");
    if (actiontype == Property(std::string("tooltip"))) {
        const Children checkedChildren = childNodes;
        if (!checkedChildren) host::typeError("Cannot read child 1 from null maction children");
        if (checkedChildren->size() > 1) {
            if ((*checkedChildren)[1]) {
                // Keep the receiver read at the second source indexing site.
                const Children currentChildren = childNodes;
                if (!currentChildren || currentChildren->size() <= 1 || !(*currentChildren)[1])
                    host::typeError("Cannot read setTeXclass from null tooltip child");
                (*currentChildren)[1]->setTeXclass(nullptr);
            }
        }
    }

    MmlNode* const selectedNode = selected();
    if (!selectedNode) host::typeError("Cannot read setTeXclass of null maction selection");
    MmlNode* const result = selectedNode->setTeXclass(previous);
    updateTeXclass(selectedNode);
    return result;
}

void MmlMaction::nextToggleSelection() {
    tree::TreeCallGuard<MmlNode> guard;

    const Property rawSelection = host::arithmeticAttributes(*this)->get("selection");
    const double parsed = host::integer(host::string(rawSelection));
    double selection = parsed + 1.0;
    if (!std::isnan(selection) && selection < 1.0) selection = 1.0;

    // JavaScript evaluates childNodes.length even when selection is NaN.
    const Children children = childNodes;
    if (!children) host::typeError("Cannot read length of null maction children");
    if (selection > static_cast<double>(children->size())) {
        selection = 1.0;
    }

    host::arithmeticAttributes(*this)->set("selection", Property(selection));
}

void MmlMaction::setChildInheritedAttributes(AttributeList attributes,
                                              Property display,
                                              Property level,
                                              Property prime) {
    tree::TreeCallGuard<MmlNode> guard;

    const Property actiontype = host::arithmeticAttributes(*this)->get("actiontype");
    if (host::lowerString(actiontype) != u"tooltip") {
        AbstractMmlNode::setChildInheritedAttributes(
            std::move(attributes), std::move(display), std::move(level), std::move(prime));
        return;
    }

    // Optional chaining skips only a nullish indexed child; the child array
    // itself is read again for slot 1 after the first virtual callback.
    const Children firstChildren = childNodes;
    if (!firstChildren) host::typeError("Cannot read child 0 from null maction children");
    if (!firstChildren->empty()) {
        if (MmlNode* const first = (*firstChildren)[0]) {
            first->setInheritedAttributes(attributes, display, level, prime);
        }
    }

    const Children secondChildren = childNodes;
    if (!secondChildren) host::typeError("Cannot read child 1 from null maction children");
    if (secondChildren->size() > 1) {
        if (MmlNode* const second = (*secondChildren)[1]) {
            second->setInheritedAttributes(
                attributes, Property(false), Property(1.0), Property(false));
        }
    }
}

} // namespace folia::math::mml
