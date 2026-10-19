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
    const auto initialChildren = host::tableChildren(*this);
    for (std::size_t index = 0; index < initialChildren->size(); ++index) {
        const NodeValue child = initialChildren->get(index);
        if (!host::tableNode(child, "isKind").isKind("mtd")) {
            const NodeValue replacement = host::tableCreate(this->factoryOwner(), "mtd");
            const NodeValue replaced = this->replaceChild(replacement, child);
            host::tableNode(replaced, "appendChild").appendChild(child);
        }
    }

    auto currentAttributes = this->attributes();
    if (!currentAttributes) host::typeError("Cannot read properties of null (reading 'get')");
    std::deque<Property> calign = host::tableSplit(currentAttributes->get("columnalign"));
    currentAttributes = this->attributes();
    if (!currentAttributes) host::typeError("Cannot read properties of null (reading 'get')");
    std::deque<Property> balign = host::tableSplit(currentAttributes->get("data-break-align"));
    if (this->arity() == 1) {
        const NodeValue parent = this->getParent();
        MmlNode& parentNode = host::tableNode(parent, "attributes");
        const auto parentAttributes = parentNode.attributes();
        if (!parentAttributes) host::typeError("Cannot read properties of null (reading 'get')");
        calign.emplace_front(parentAttributes->get("side"));
        balign.emplace_front(std::string("top"));
    }

    currentAttributes = this->attributes();
    if (!currentAttributes) host::typeError("Cannot read properties of null (reading 'get')");
    PropertyList additions{
        {"rowalign", currentAttributes->get("rowalign")},
        {"columnalign", std::string("center")},
        {"data-break-align", std::string("top")}
    };
    attributes = addInheritedAttributes(std::move(attributes), Property(additions));

    const auto children = host::tableChildren(*this);
    for (std::size_t index = 0; index < children->size(); ++index) {
        const NodeValue child = children->get(index);
        Property columnalignReceiver = host::get(attributes, "columnalign");
        const Property shiftedColumnalign = host::tableShift(calign);
        Property columnalignValue = shiftedColumnalign;
        if (!host::truthy(columnalignValue)) {
            columnalignValue = host::get(host::get(attributes, "columnalign"), "1");
        }
        host::tableSetIndex(columnalignReceiver, "1", std::move(columnalignValue));

        const std::string rowKind = this->kind();
        const Property shiftedBreakalign = host::tableShift(balign);
        Property breakalignValue = shiftedBreakalign;
        if (!host::truthy(breakalignValue)) {
            breakalignValue = host::get(host::get(attributes, "data-break-align"), "1");
        }
        PropertyArray verticalAlign{Property(rowKind), std::move(breakalignValue)};
        host::tableSetIndex(attributes, "data-vertical-align", Property(verticalAlign));

        child.requireNode("setInheritedAttributes")
            ->setInheritedAttributes(attributes, display, level, prime);
    }
}

void MmlMtr::verifyChildren(Property options) {
    tree::TreeCallGuard<MmlNode> guard;
    if (this->getParent().nodeOrNull()) {
        const NodeValue parent = this->getParent();
        if (!host::tableNode(parent, "isKind").isKind("mtable")) {
            mError(this->kind() + " can only be a child of an mtable", options, true);
            return;
        }
    }
    const auto children = host::tableChildren(*this);
    for (std::size_t index = 0; index < children->size(); ++index) {
        const NodeValue child = children->get(index);
        MmlNode& childNode = host::tableNode(child, "isKind");
        if (!childNode.isKind("mtd")) {
            const NodeValue replacement = host::tableCreate(this->factoryOwner(), "mtd");
            const NodeValue replaced = this->replaceChild(replacement, child);
            host::tableNode(replaced, "appendChild").appendChild(child);
            if (options.missing()) host::typeError("Cannot read properties of undefined (reading 'fixMtables')");
            if (std::holds_alternative<PropertyNull>(options.value))
                host::typeError("Cannot read properties of null (reading 'fixMtables')");
            if (!host::truthy(host::get(options, "fixMtables"))) {
                childNode.mError("Children of " + this->kind() + " must be mtd", options);
            }
        }
    }
    AbstractMmlNode::verifyChildren(options);
}

NodeValue MmlMtr::setTeXclass(NodeValue previous) {
    tree::TreeCallGuard<MmlNode> guard;
    getPrevClass(previous);
    const auto children = host::tableChildren(*this);
    for (std::size_t index = 0; index < children->size(); ++index) {
        host::tableChild(children, index, "setTeXclass").setTeXclass(PropertyNull{});
    }
    return this;
}

}
