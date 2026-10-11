/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native implementation of MmlNodes/mmultiscripts.ts (MathJax 4.1.3).
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 * Source SHA-256: bb8d8d2180ef483b86b0b1570b919ba424ed54db1f30ef6e0e697cfab0bf5cf7
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.mmultiscripts;
import folia.math.core.mml_tree.multiscript_host;
import std;

namespace folia::math::mml {

void MmlMmultiscripts::setChildInheritedAttributes(
    AttributeList attributes, Property display, Property level, Property prime) {
    tree::TreeCallGuard<MmlNode> guard;

    // The base call precedes every normalization operation and receives the
    // original values. Each helper access below deliberately captures the
    // current childNodes property at that source access point.
    host::multiscriptChild(*this, 0).setInheritedAttributes(
        attributes, display, level, prime);

    bool prescripts = false;
    for (std::size_t i = 1, n = 0;; ++i) {
        const Children conditionChildren = host::multiscriptChildren(*this);
        if (i >= conditionChildren->size()) break;

        const Children indexedChildren = host::multiscriptChildren(*this);
        MmlNode& child = host::multiscriptNode(
            host::multiscriptSlot(indexedChildren, i));
        if (child.isKind("mprescripts")) {
            if (!prescripts) {
                prescripts = true;
                if (i % 2 == 0) {
                    MmlNode* const none = host::multiscriptCreate(factory, "none");
                    // Factory callbacks run before resolving the current splice receiver.
                    const Children spliceChildren = host::multiscriptChildren(*this);
                    host::multiscriptInsert(spliceChildren, i, none);
                    host::multiscriptNode(none).parent = this;
                    ++i;
                }
            }
        } else {
            const Property primestyle = host::truthy(prime)
                ? prime
                : Property(n % 2 == 0);
            Property childLevel = host::add(level, Property(1.0));
            child.setInheritedAttributes(attributes, Property(false),
                                         std::move(childLevel), primestyle);
            ++n;
        }
    }

    const Children parityChildren = host::multiscriptChildren(*this);
    const std::size_t parityLength = parityChildren->size();
    if (parityLength % 2 == (prescripts ? 1U : 0U)) {
        MmlNode* const none = host::multiscriptCreate(factory, "none");
        appendChild(none);

        // The source evaluates the indexed array receiver before its fresh
        // length read; appendChild may replace or mutate childNodes.
        const Children indexedChildren = host::multiscriptChildren(*this);
        const Children lengthChildren = host::multiscriptChildren(*this);
        MmlNode* const last = lengthChildren->empty()
            ? nullptr
            : host::multiscriptSlot(indexedChildren, lengthChildren->size() - 1);
        MmlNode& appendedChild = host::multiscriptNode(last);
        Property childLevel = host::add(level, Property(1.0));
        appendedChild.setInheritedAttributes(
            attributes, Property(false), std::move(childLevel), prime);
    }
}

void MmlMmultiscripts::verifyChildren(PropertyList options) {
    tree::TreeCallGuard<MmlNode> guard;

    const Property fix = options.get("fixMmultiscripts");
    bool prescripts = false;
    for (std::size_t i = 0;; ++i) {
        const Children conditionChildren = host::multiscriptChildren(*this);
        if (i >= conditionChildren->size()) break;

        const Children indexedChildren = host::multiscriptChildren(*this);
        MmlNode& child = host::multiscriptNode(
            host::multiscriptSlot(indexedChildren, i));
        if (child.isKind("mprescripts")) {
            if (prescripts) {
                const std::string childKind = child.kind();
                const std::string multiscriptKind = kind();
                const std::string message = childKind +
                    " can only appear once in " + multiscriptKind;
                child.mError(message, ErrorOptions(options), true);
            } else {
                prescripts = true;
                if (i % 2 == 0 && !host::truthy(fix)) {
                    mError("There must be an equal number of prescripts of each type",
                           ErrorOptions(options));
                }
            }
        }
    }

    const Children parityChildren = host::multiscriptChildren(*this);
    if (parityChildren->size() % 2 == (prescripts ? 1U : 0U) &&
        !host::truthy(fix)) {
        mError("There must be an equal number of scripts of each type",
               ErrorOptions(options));
    }

    MmlMsubsup::verifyChildren(std::move(options));
}

void MmlMprescripts::verifyTree(std::optional<PropertyList> options) {
    tree::TreeCallGuard<MmlNode> guard;
    AbstractMmlNode::verifyTree(options);

    // The source checks the actual direct parent edge, not logical Parent().
    MmlNode* const currentParent = parent;
    if (currentParent && !currentParent->isKind("mmultiscripts")) {
        const std::string message = kind() + " must be a child of mmultiscripts";
        mError(message, host::multiscriptErrorOptions(options), true);
    }
}

void MmlNone::verifyTree(std::optional<PropertyList> options) {
    tree::TreeCallGuard<MmlNode> guard;
    AbstractMmlNode::verifyTree(options);

    MmlNode* const currentParent = parent;
    if (currentParent && !currentParent->isKind("mmultiscripts")) {
        const std::string message = kind() + " must be a child of mmultiscripts";
        mError(message, host::multiscriptErrorOptions(options), true);
    }
}

} // namespace folia::math::mml
