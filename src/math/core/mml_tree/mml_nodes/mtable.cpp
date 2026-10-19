/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native implementation of MmlMtable semantic methods from MathJax 4.1.3.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 * Source SHA256: 8c701c9e8e55565c7ea905bd2d0e23e8a433547c5ed11bfb868f74e3089a3bda
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.mtable;
import folia.math.core.mml_tree.table_host;
import std;

namespace folia::math::mml {

void MmlMtable::inheritAttributes(AttributeList attributes, Property display,
                                  Property level, Property prime) {
    tree::TreeCallGuard<MmlNode> guard;

    for (std::size_t index = 0; index < indentAttributes.size(); ++index) {
        // for-of captures the string value for this iteration before callbacks;
        // the shared vector itself remains live and may grow during the loop.
        const std::string name = indentAttributes[index];
        if (host::truthy(host::get(attributes, name))) {
            // JavaScript captures the method receiver before evaluating args.
            const auto receiver = host::arithmeticAttributes(*this);
            const Property tuple = host::get(attributes, name);
            const Property value = host::get(tuple, "1");
            receiver->setInherited(name, value);
        }
        if (host::arithmeticAttributes(*this)->hasExplicit(name)) {
            host::arithmeticAttributes(*this)->unset(name);
        }
    }

    AbstractMmlNode::inheritAttributes(
        std::move(attributes), std::move(display), std::move(level), std::move(prime));
}

void MmlMtable::setChildInheritedAttributes(AttributeList attributes, Property display,
                                             Property level, Property /*prime*/) {
    tree::TreeCallGuard<MmlNode> guard;

    const Children originalChildren = host::tableChildren(*this);
    for (std::size_t index = 0; index < originalChildren->size(); ++index) {
        const NodeValue childValue = originalChildren->get(index);
        MmlNode& child = host::tableNode(childValue, "isKind");
        if (!child.isKind("mtr")) {
            const NodeValue row = replaceChild(host::tableCreate(factoryOwner(), "mtr"), childValue);
            row.requireNode("appendChild")->appendChild(childValue);
        }
    }

    auto explicitAttributes = this->attributes();
    if (!explicitAttributes) host::typeError("Cannot read properties of null (reading 'getExplicit')");
    Property displayValue = explicitAttributes->getExplicit("displaystyle");
    if (!host::truthy(displayValue)) {
        auto defaultAttributes = this->attributes();
        if (!defaultAttributes) host::typeError("Cannot read properties of null (reading 'getDefault')");
        displayValue = defaultAttributes->getDefault("displaystyle");
    }
    display = host::truthy(displayValue);

    auto columnAttributes = this->attributes();
    if (!columnAttributes) host::typeError("Cannot read properties of null (reading 'get')");
    const Property columnAlign = columnAttributes->get("columnalign");
    auto breakAttributes = this->attributes();
    if (!breakAttributes) host::typeError("Cannot read properties of null (reading 'get')");
    const Property breakAlign = breakAttributes->get("data-break-align");
    PropertyList additions{{"columnalign", columnAlign}, {"rowalign", "center"},
                           {"data-break-align", breakAlign}};
    attributes = addInheritedAttributes(std::move(attributes), Property(additions));

    auto crampedAttributes = this->attributes();
    if (!crampedAttributes) host::typeError("Cannot read properties of null (reading 'getExplicit')");
    const Property cramped = crampedAttributes->getExplicit("data-cramped");

    auto rowAttributes = this->attributes();
    if (!rowAttributes) host::typeError("Cannot read properties of null (reading 'get')");
    std::deque<Property> rowAlignments = host::tableSplit(rowAttributes->get("rowalign"));

    const Children rows = host::tableChildren(*this);
    for (std::size_t index = 0; index < rows->size(); ++index) {
        const NodeValue childValue = rows->get(index);
        // The assignment receiver is read before evaluating shift()/fallback.
        const Property tupleReceiver = host::get(attributes, "rowalign");
        const Property shifted = host::tableShift(rowAlignments);
        Property rowAlign = shifted;
        if (!host::truthy(shifted)) {
            rowAlign = host::get(host::get(attributes, "rowalign"), "1");
        }
        host::tableSetIndex(tupleReceiver, "1", rowAlign);

        MmlNode& child = host::tableNode(childValue, "setInheritedAttributes");
        child.setInheritedAttributes(attributes, display, level, Property(host::truthy(cramped)));
    }
}

void MmlMtable::verifyChildren(Property options) {
    tree::TreeCallGuard<MmlNode> guard;
    NodeValue currentRow = PropertyNull{};
    Factory& rowFactory = this->factoryOwner();

    std::int64_t index = 0;
    while (true) {
        const Children lengthChildren = host::tableChildren(*this, "length");
        if (static_cast<double>(index) >= static_cast<double>(lengthChildren->size())) break;

        const Children slotChildren = host::tableChildren(*this, std::to_string(index));
        const NodeValue childValue = host::tableSlot(slotChildren, static_cast<std::size_t>(index));
        MmlNode& child = host::tableNode(childValue, "isKind");
        if (child.isKind("mtr")) {
            currentRow = PropertyNull{};
        } else {
            const bool isCell = child.isKind("mtd");
            if (currentRow.isNode()) {
                removeChild(childValue);
                --index;
            } else {
                currentRow = replaceChild(host::tableCreate(rowFactory, "mtr"), childValue);
            }

            MmlNode* const rowReceiver = currentRow.requireNode("appendChild");
            NodeValue cellValue = childValue;
            if (!isCell) cellValue = host::tableCreate(rowFactory, "mtd", childValue);
            rowReceiver->appendChild(cellValue);

            if (options.missing()) host::typeError("Cannot read properties of undefined (reading 'fixMtables')");
            if (std::holds_alternative<PropertyNull>(options.value))
                host::typeError("Cannot read properties of null (reading 'fixMtables')");
            if (!host::truthy(host::get(options, "fixMtables"))) {
                child.getParent().requireNode("removeChild")->removeChild(childValue);
                childValue.requireNodeForWrite("parent")->setParent(NodeValue(this));
                if (isCell) {
                    MmlNode* const emptyCellRow = currentRow.requireNode("appendChild");
                    emptyCellRow->appendChild(host::tableCreate(rowFactory, "mtd"));
                }

                const std::string message = "Children of " + kind() + " must be mtr or mlabeledtr";
                const NodeValue error(childValue.requireNode("mError")->mError(message, options, isCell));

                const Children rowChildren = currentRow.requireNode("childNodes")->childNodes;
                if (!rowChildren) host::typeError("Cannot read properties of null (reading 'length')");
                const Children indexedRowChildren = currentRow.requireNode("childNodes")->childNodes;
                if (!indexedRowChildren) host::typeError("Cannot read properties of null (reading 'length')");
                const std::size_t rowLength = indexedRowChildren->size();
                const NodeValue lastCell = rowLength == 0
                    ? NodeValue(Missing{})
                    : rowChildren->get(rowLength - 1);
                lastCell.requireNode("appendChild")->appendChild(error);
            }
        }
        ++index;
    }
    AbstractMmlNode::verifyChildren(options);
}

NodeValue MmlMtable::setTeXclass(NodeValue previous) {
    tree::TreeCallGuard<MmlNode> guard;
    getPrevClass(previous);
    const Children children = host::tableChildren(*this);
    for (std::size_t index = 0; index < children->size(); ++index) {
        children->get(index).requireNode("setTeXclass")->setTeXclass(PropertyNull{});
    }
    return this;
}

} // namespace folia::math::mml
