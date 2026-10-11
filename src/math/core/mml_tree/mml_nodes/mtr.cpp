/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native port of MathJax 4.1.3 MmlNodes/mtr.ts methods.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.mtr;
import folia.math.core.mml_tree.table_host;
import std;

namespace folia::math::mml {

void MmlMtr::setChildInheritedAttributes(AttributeList attributes, Property display,
                                         Property level, Property prime) {
    tree::TreeCallGuard<MmlNode> guard;

    const Children initialChildren = host::tableChildren(*this);
    for (std::size_t index = 0; index < initialChildren->size(); ++index) {
        MmlNode& child = host::tableChild(initialChildren, index);
        if (!child.isKind("mtd")) {
            MmlNode& replacement = host::tableCreate(factory, "mtd");
            MmlNode* const replaced = replaceChild(&replacement, &child);
            host::tableNode(replaced).appendChild(&child);
        }
    }

    auto currentAttributes = host::arithmeticAttributes(*this);
    std::deque<Property> columnAlign = host::tableSplit(currentAttributes->get("columnalign"));
    currentAttributes = host::arithmeticAttributes(*this);
    std::deque<Property> breakAlign = host::tableSplit(currentAttributes->get("data-break-align"));

    if (arity() == 1.0) {
        MmlNode& parent = host::tableNode(this->parent);
        const auto parentAttributes = host::arithmeticAttributes(parent);
        Property side = parentAttributes->get("side");
        columnAlign.push_front(std::move(side));
        breakAlign.push_front("top");
    }

    currentAttributes = host::arithmeticAttributes(*this);
    Property rowAlign = currentAttributes->get("rowalign");
    PropertyList additions;
    additions.set("rowalign", std::move(rowAlign));
    additions.set("columnalign", "center");
    additions.set("data-break-align", "top");
    attributes = addInheritedAttributes(std::move(attributes), std::move(additions));

    const Children inheritedChildren = host::tableChildren(*this);
    for (std::size_t index = 0; index < inheritedChildren->size(); ++index) {
        MmlNode& child = host::tableChild(inheritedChildren, index);

        // JavaScript evaluates the indexed assignment receiver before shift().
        Property columnReceiver = attributes.get("columnalign");
        Property columnValue = host::tableShift(columnAlign);
        if (!host::truthy(columnValue))
            columnValue = host::get(attributes.get("columnalign"), "1");
        host::tableSetIndex(columnReceiver, "1", std::move(columnValue));

        // The kind getter is evaluated before balign.shift() and its fallback.
        Property verticalKind = this->kind();
        Property verticalAlign = host::tableShift(breakAlign);
        if (!host::truthy(verticalAlign))
            verticalAlign = host::get(attributes.get("data-break-align"), "1");
        attributes.set("data-vertical-align", PropertyArray{std::move(verticalKind),
                                                             std::move(verticalAlign)});

        child.setInheritedAttributes(attributes, display, level, prime);
    }
}

void MmlMtr::verifyChildren(PropertyList options) {
    tree::TreeCallGuard<MmlNode> guard;

    if (this->parent && !this->parent->isKind("mtable")) {
        const std::string message = this->kind() + " can only be a child of an mtable";
        mError(message, ErrorOptions(options), true);
        return;
    }

    const Children initialChildren = host::tableChildren(*this);
    for (std::size_t index = 0; index < initialChildren->size(); ++index) {
        MmlNode& child = host::tableChild(initialChildren, index);
        if (!child.isKind("mtd")) {
            MmlNode& replacement = host::tableCreate(factory, "mtd");
            MmlNode* const replaced = replaceChild(&replacement, &child);
            host::tableNode(replaced).appendChild(&child);
            if (!host::truthy(options.get("fixMtables"))) {
                const std::string message = "Children of " + this->kind() + " must be mtd";
                child.mError(message, ErrorOptions(options));
            }
        }
    }

    AbstractMmlNode::verifyChildren(std::move(options));
}

MmlNode* MmlMtr::setTeXclass(MmlNode* previous) {
    tree::TreeCallGuard<MmlNode> guard;
    getPrevClass(previous);
    const Children children = host::tableChildren(*this);
    for (std::size_t index = 0; index < children->size(); ++index) {
        MmlNode& child = host::tableChild(children, index);
        child.setTeXclass(nullptr);
    }
    return this;
}

}
