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

Property MmlMstyle::notParent() const {
    const auto firstArray = this->childNodes;
    if (!firstArray) throw Error("TypeError",u"Cannot read properties of null (reading '0')");
    const NodeValue first = firstArray->get(0);
    if (!first.isNode()) {
        return first.kind() == tree::ChildKind::Null ? Property(PropertyNull{}) : Property(Missing{});
    }

    const auto currentArray = this->childNodes;
    if (!currentArray) throw Error("TypeError",u"Cannot read properties of null (reading '0')");
    const NodeValue current = currentArray->get(0);
    MmlNode* const child = current.requireNode("childNodes");
    const auto grandchildren = child->childNodes;
    if (!grandchildren) throw Error("TypeError",u"Cannot read properties of null (reading 'length')");
    return Property(grandchildren->size() == 1);
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
    if (!currentAttributes) host::typeError("Cannot read properties of null (reading 'getExplicit')");
    Property scriptlevel = currentAttributes->getExplicit("scriptlevel");
    if (!host::nullish(scriptlevel)) {
        const std::u16string text = host::methodString(scriptlevel);
        if (host::relativeLevel(text)) {
            level = host::add(level, Property(host::integer(text)));
        } else {
            level = host::integer(text);
        }
        prime = false;
    }

    currentAttributes = this->attributes();
    if (!currentAttributes) host::typeError("Cannot read properties of null (reading 'getExplicit')");
    const Property displaystyle = currentAttributes->getExplicit("displaystyle");
    if (!host::nullish(displaystyle)) {
        display = displaystyle == Property(true);
        prime = false;
    }

    currentAttributes = this->attributes();
    if (!currentAttributes) host::typeError("Cannot read properties of null (reading 'getExplicit')");
    const Property cramped = currentAttributes->getExplicit("data-cramped");
    if (!host::nullish(cramped)) prime = cramped;

    currentAttributes = this->attributes();
    if (!currentAttributes) host::typeError("Cannot read properties of null (reading 'getAllAttributes')");
    attributes = addInheritedAttributes(std::move(attributes), Property(currentAttributes->getAllAttributes()));

    const auto children = this->childNodes;
    if (!children) host::typeError("Cannot read properties of null (reading '0')");
    children->get(0).requireNode("setInheritedAttributes")
        ->setInheritedAttributes(std::move(attributes), display, level, prime);
}

}
