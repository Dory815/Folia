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

void MmlMtable::setInheritedAttributes(AttributeList attributes, Property display,
                                       Property level, Property prime) {
    tree::TreeCallGuard<MmlNode> guard;

    for (std::size_t index = 0; index < indentAttributes.size(); ++index) {
        // for-of captures the string value for this iteration before callbacks;
        // the shared vector itself remains live and may grow during the loop.
        const std::string name = indentAttributes[index];
        if (host::truthy(host::get(Property(attributes), name))) {
            // JavaScript captures the method receiver before evaluating args.
            const auto receiver = host::arithmeticAttributes(*this);
            const Property tuple = host::get(Property(attributes), name);
            const Property value = host::get(tuple, "1");
            receiver->setInherited(name, value);
        }
        if (host::arithmeticAttributes(*this)->hasExplicit(name)) {
            host::arithmeticAttributes(*this)->unset(name);
        }
    }

    AbstractMmlNode::setInheritedAttributes(
        std::move(attributes), std::move(display), std::move(level), std::move(prime));
}

void MmlMtable::setChildInheritedAttributes(AttributeList attributes, Property display,
                                             Property level, Property /*prime*/) {
    tree::TreeCallGuard<MmlNode> guard;

    const Children rowsToRepair = host::tableChildren(*this);
    for (std::size_t index = 0; index < rowsToRepair->size(); ++index) {
        MmlNode& child = host::tableChild(rowsToRepair, index);
        if (!child.isKind("mtr")) {
            MmlNode& row = host::tableNode(
                replaceChild(&host::tableCreate(factory, "mtr"), &child));
            row.appendChild(&child);
        }
    }

    const Property explicitDisplay = host::arithmeticAttributes(*this)->getExplicit("displaystyle");
    Property displayValue = explicitDisplay;
    if (!host::truthy(explicitDisplay)) {
        displayValue = host::arithmeticAttributes(*this)->getDefault("displaystyle");
    }
    display = Property(host::truthy(displayValue));

    PropertyList additions;
    additions.setOwn("columnalign",
                     host::arithmeticAttributes(*this)->get("columnalign"));
    additions.setOwn("rowalign", "center");
    additions.setOwn("data-break-align",
                     host::arithmeticAttributes(*this)->get("data-break-align"));
    attributes = addInheritedAttributes(std::move(attributes), std::move(additions));

    const Property cramped = host::arithmeticAttributes(*this)->getExplicit("data-cramped");
    std::deque<Property> rowAlignments = host::tableSplit(
        host::arithmeticAttributes(*this)->get("rowalign"));

    const Children rows = host::tableChildren(*this);
    for (std::size_t index = 0; index < rows->size(); ++index) {
        MmlNode& child = host::tableChild(rows, index);

        // Capture the assignment reference before evaluating the shifting RHS.
        const Property rowalignTuple = host::get(Property(attributes), "rowalign");
        Property rowalignValue = host::tableShift(rowAlignments);
        if (!host::truthy(rowalignValue)) {
            rowalignValue = host::get(host::get(Property(attributes), "rowalign"), "1");
        }
        host::tableSetIndex(rowalignTuple, "1", std::move(rowalignValue));

        child.setInheritedAttributes(attributes, display, level,
                                     Property(host::truthy(cramped)));
    }
}

void MmlMtable::verifyChildren(PropertyList options) {
    tree::TreeCallGuard<MmlNode> guard;
    MmlNode* currentRow = nullptr;
    Factory& capturedFactory = factory;

    // Unlike for-of, this source loop re-reads the current childNodes property
    // on each condition and indexed access; removal decrements the signed index.
    for (std::ptrdiff_t index = 0;; ++index) {
        const Children conditionChildren = host::tableChildren(*this);
        if (static_cast<std::size_t>(index) >= conditionChildren->size()) break;

        const Children indexedChildren = host::tableChildren(*this);
        MmlNode& child = host::tableChild(indexedChildren,
                                         static_cast<std::size_t>(index));
        if (child.isKind("mtr")) {
            currentRow = nullptr;
        } else {
            const bool isCell = child.isKind("mtd");
            if (currentRow) {
                removeChild(&child);
                --index;
            } else {
                currentRow = replaceChild(&host::tableCreate(capturedFactory, "mtr"), &child);
            }

            MmlNode& row = host::tableNode(currentRow);
            MmlNode* appended = &child;
            if (!isCell) appended = &host::tableCreate(capturedFactory, "mtd", &child);
            row.appendChild(appended);

            if (!host::truthy(options.get("fixMtables"))) {
                MmlNode& oldParent = host::tableNode(child.parent);
                oldParent.removeChild(&child);
                child.parent = this;

                if (isCell) row.appendChild(&host::tableCreate(capturedFactory, "mtd"));

                // The child is the mError receiver before its message evaluates this.kind.
                MmlNode* const error = child.mError(
                    "Children of " + kind() + " must be mtr or mlabeledtr",
                    ErrorOptions(options), isCell);

                // Source indexes a captured childNodes base using a fresh
                // childNodes.length read, then appends through that selected cell.
                const Children cellArray = host::tableChildren(row);
                const Children lengthArray = host::tableChildren(row);
                if (lengthArray->empty()) host::typeError("Cannot read appendChild of undefined table cell");
                MmlNode& cell = host::tableChild(cellArray, lengthArray->size() - 1);
                cell.appendChild(error);
            }
        }
    }

    AbstractMmlNode::verifyChildren(std::move(options));
}

MmlNode* MmlMtable::setTeXclass(MmlNode* previous) {
    tree::TreeCallGuard<MmlNode> guard;
    getPrevClass(previous);

    const Children children = host::tableChildren(*this);
    for (std::size_t index = 0; index < children->size(); ++index) {
        host::tableChild(children, index).setTeXclass(nullptr);
    }
    return this;
}

} // namespace folia::math::mml
