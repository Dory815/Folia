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

    MmlNode& base = host::multiscriptChild(*this, 0, "setInheritedAttributes");
    base.setInheritedAttributes(attributes, display, level, prime);

    bool prescripts = false;
    for (std::size_t i = 1, n = 0;
         i < host::multiscriptChildren(*this)->size(); ++i) {
        MmlNode& child = host::multiscriptChild(*this, i, "isKind");
        if (child.isKind("mprescripts")) {
            if (!prescripts) {
                prescripts = true;
                if (i % 2 == 0) {
                    MmlNode* const created = host::multiscriptCreate(factory, "none");
                    host::multiscriptInsert(childNodes, i, NodeValue(created));
                    NodeValue(created).requireNodeForWrite("parent")
                        ->setParent(NodeValue(this));
                    ++i;
                }
            }
        } else {
            const Property nextLevel = host::add(level, 1);
            const Property primeStyle = host::truthy(prime)
                ? prime : Property(n % 2 == 0);
            child.setInheritedAttributes(attributes, false, nextLevel, primeStyle);
            ++n;
        }
    }

    const auto currentChildren = host::multiscriptChildren(*this);
    if (currentChildren->size() % 2 == (prescripts ? 1u : 0u)) {
        (void)appendChild(NodeValue(host::multiscriptCreate(factory, "none")));

        // Source evaluates the indexed receiver's array before re-reading the
        // current array for its length expression.
        const auto indexedChildren = host::multiscriptChildren(*this);
        const auto lengthChildren = host::multiscriptChildren(*this);
        const std::size_t length = lengthChildren->size();
        const NodeValue last = length == 0
            ? NodeValue(Missing{})
            : host::multiscriptSlot(indexedChildren, length - 1);
        MmlNode& lastChild = host::multiscriptNode(last, "setInheritedAttributes");
        const Property nextLevel = host::add(level, 1);
        lastChild.setInheritedAttributes(attributes, false, nextLevel, prime);
    }
}

void MmlMmultiscripts::verifyChildren(Property options) {
    tree::TreeCallGuard<MmlNode> guard;

    bool prescripts = false;
    if (options.missing()) host::typeError("Cannot read properties of undefined (reading 'fixMmultiscripts')");
    if (std::holds_alternative<PropertyNull>(options.value))
        host::typeError("Cannot read properties of null (reading 'fixMmultiscripts')");
    const Property fix = host::get(options, "fixMmultiscripts");
    for (std::size_t i = 0; i < host::multiscriptChildren(*this)->size(); ++i) {
        MmlNode& child = host::multiscriptChild(*this, i, "isKind");
        if (child.isKind("mprescripts")) {
            if (prescripts) {
                const std::string childKind = child.kind();
                const std::string ownerKind = kind();
                (void)child.mError(childKind + " can only appear once in " + ownerKind,
                                   options, true);
            } else {
                prescripts = true;
                if (i % 2 == 0 && !host::truthy(fix)) {
                    (void)mError("There must be an equal number of prescripts of each type",
                                 options);
                }
            }
        }
    }
    const auto currentChildren = host::multiscriptChildren(*this);
    if (currentChildren->size() % 2 == (prescripts ? 1u : 0u) &&
        !host::truthy(fix)) {
        (void)mError("There must be an equal number of scripts of each type", options);
    }
    AbstractMmlNode::verifyChildren(options);
}

void MmlMprescripts::verifyTree(Property options) {
    tree::TreeCallGuard<MmlNode> guard;
    AbstractMmlNode::verifyTree(options);
    if (getParent().isNode() && !getParent().requireNode("isKind")->isKind("mmultiscripts")) {
        (void)mError(kind() + " must be a child of mmultiscripts", options, true);
    }
}

void MmlNone::verifyTree(Property options) {
    tree::TreeCallGuard<MmlNode> guard;
    AbstractMmlNode::verifyTree(options);
    if (getParent().isNode() && !getParent().requireNode("isKind")->isKind("mmultiscripts")) {
        (void)mError(kind() + " must be a child of mmultiscripts", options, true);
    }
}

} // namespace folia::math::mml
