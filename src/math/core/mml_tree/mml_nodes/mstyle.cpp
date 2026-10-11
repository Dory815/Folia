/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native port of MathJax 4.1.3 MmlNodes/Mstyle.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.mstyle;
import folia.math.core.mml_tree.leaf_host;
import std;

namespace folia::math::mml {

std::string MmlMstyle::kind() const { return "mstyle"; }

bool MmlMstyle::notParent() const {
    tree::TreeCallGuard<MmlNode> guard;
    const auto children = childNodes;
    if (!children) host::typeError("Cannot read child 0 from null mstyle children");
    if (children->empty() || !children->front()) return false;
    const auto nested = children->front()->childNodes;
    if (!nested) host::typeError("Cannot read child count from null child list");
    return nested->size() == 1;
}

void MmlMstyle::inheritAttributes(AttributeList attributes, Property display,
                                  Property level, Property prime) {
    tree::TreeCallGuard<MmlNode> guard;
    host::inheritanceDefaults(attributes, display, level, prime);
    auto currentAttributes = this->attributes();
    if (!currentAttributes) host::typeError("Cannot set inherited displaystyle on null attributes");
    currentAttributes->setInherited("displaystyle", display);
    currentAttributes = this->attributes();
    if (!currentAttributes) host::typeError("Cannot set inherited scriptlevel on null attributes");
    currentAttributes->setInherited("scriptlevel", level);
    AbstractMmlNode::inheritAttributes(attributes, display, level, prime);
}

void MmlMstyle::setChildInheritedAttributes(AttributeList attributes, Property display,
                                            Property level, Property prime) {
    tree::TreeCallGuard<MmlNode> guard;

    auto currentAttributes = this->attributes();
    if (!currentAttributes) host::typeError("Cannot read explicit scriptlevel from null attributes");
    Property scriptlevel = currentAttributes->getExplicit("scriptlevel");
    if (!host::nullish(scriptlevel)) {
        const std::u16string scriptText = host::methodString(scriptlevel);
        if (host::relativeLevel(scriptText)) {
            level = host::add(level, Property(host::integer(scriptText)));
        } else {
            level = Property(host::integer(scriptText));
        }
        prime = false;
    }

    currentAttributes = this->attributes();
    if (!currentAttributes) host::typeError("Cannot read explicit displaystyle from null attributes");
    const Property displaystyle = currentAttributes->getExplicit("displaystyle");
    if (!host::nullish(displaystyle)) {
        display = displaystyle == Property(true);
        prime = false;
    }

    currentAttributes = this->attributes();
    if (!currentAttributes) host::typeError("Cannot read explicit data-cramped from null attributes");
    const Property cramped = currentAttributes->getExplicit("data-cramped");
    if (!host::nullish(cramped)) prime = cramped;

    currentAttributes = this->attributes();
    if (!currentAttributes) host::typeError("Cannot read attributes from null mstyle attributes");
    attributes = addInheritedAttributes(std::move(attributes), currentAttributes->getAllAttributes());

    const auto children = childNodes;
    if (!children) host::typeError("Cannot read child 0 from null mstyle children");
    if (children->empty() || !children->front())
        host::typeError("Cannot set inherited attributes on missing mstyle child 0");
    children->front()->setInheritedAttributes(std::move(attributes), std::move(display),
                                              std::move(level), std::move(prime));
}

}
