/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native port of MathJax 4.1.3 MmlNodes/mo.ts class methods.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.mo;
import folia.math.core.mml_tree.mo_host;
import folia.math.core.mml_tree.operator_class;
import folia.math.core.mml_tree.mml_nodes.math;
import folia.math.core.mml_tree.mml_nodes.munderover;
import std;

namespace folia::math::mml {

MmlNode* MmlMo::coreParent() {
    tree::TreeCallGuard<MmlNode> guard;
    MmlNode* embellished = nullptr;
    MmlNode* parent = this;
    const auto math = factory.getNodeClass("math");
    while (parent && parent->isEmbellished() && parent->coreMO() == this &&
           !host::moInstance(parent, math)) {
        embellished = parent;
        parent = parent->parent;
    }
    return embellished ? embellished : this;
}

std::u16string MmlMo::coreText(MmlNode* parent) {
    tree::TreeCallGuard<MmlNode> guard;
    if (!parent) return {};
    if (parent->isEmbellished()) {
        MmlNode* const core = parent->coreMO();
        if (!core) host::typeError("Cannot read getText from null core operator");
        auto* const token = dynamic_cast<AbstractMmlTokenNode*>(core);
        if (!token) host::typeError("Embellished core operator is not a token");
        return token->getText();
    }
    while (((parent->isKind("mrow") || parent->isKind("TeXAtom") ||
             parent->isKind("mstyle") || parent->isKind("mphantom")) &&
            ([&]() {
                if (!parent->childNodes) host::typeError("Cannot read length of null childNodes");
                return parent->childNodes->size() == 1;
            }()) ||
            parent->isKind("munderover")) &&
           host::moChild(parent, 0)) {
        parent = host::moChild(parent, 0);
    }
    if (!parent->isToken()) return {};
    auto* const token = dynamic_cast<AbstractMmlTokenNode*>(parent);
    if (!token) host::typeError("Token node is not bound to the token interface");
    return token->getText();
}

Property MmlMo::isAccent() {
    tree::TreeCallGuard<MmlNode> guard;
    Property accent = false;
    MmlNode* const coreParentNode = coreParent();
    MmlNode* const node = host::moRequired(coreParentNode).parent;
    if (node) {
        std::string key;
        if (node->isKind("mover")) {
            auto* const mover = dynamic_cast<MmlMover*>(node);
            if (!mover) host::typeError("mover node has no over accessor");
            const std::size_t index = mover->over();
            MmlNode* const child = host::moChild(node, index);
            if (!child) host::typeError("Cannot read coreMO from missing mover child");
            if (child->coreMO()) key = "accent";
        } else if (node->isKind("munder")) {
            auto* const munder = dynamic_cast<MmlMunder*>(node);
            if (!munder) host::typeError("munder node has no under accessor");
            const std::size_t index = munder->under();
            MmlNode* const child = host::moChild(node, index);
            if (!child) host::typeError("Cannot read coreMO from missing munder child");
            if (child->coreMO()) key = "accentunder";
        } else if (node->isKind("munderover")) {
            auto* const munderover = dynamic_cast<MmlMunderover*>(node);
            if (!munderover) host::typeError("munderover node has no script accessors");
            const std::size_t over = munderover->over();
            MmlNode* const overChild = host::moChild(node, over);
            if (!overChild) host::typeError("Cannot read coreMO from missing over child");
            if (this == overChild->coreMO()) {
                key = "accent";
            } else {
                const std::size_t under = munderover->under();
                MmlNode* const underChild = host::moChild(node, under);
                if (!underChild) host::typeError("Cannot read coreMO from missing under child");
                if (this == underChild->coreMO()) key = "accentunder";
            }
        }
        if (!key.empty()) {
            const auto nodeAttributes = host::moAttributes(*node);
            const Property value = nodeAttributes->getExplicit(key);
            if (value.missing()) {
                const auto ownAttributes = host::moAttributes(*this);
                accent = ownAttributes->get("accent");
            }
        }
    }
    return accent;
}

MmlNode* MmlMo::setTeXclass(MmlNode* prev) {
    tree::TreeCallGuard<MmlNode> guard;
    const auto attributes = host::moAttributes(*this);
    const PropertyList values = attributes->getList({"form", "fence"});
    const Property form = values.get("form");
    const Property fence = values.get("fence");
    const Property property = getProperty("texClass");
    if (property.missing() && hasSpacingAttributes()) return nullptr;
    if (host::truthy(fence) && texClass() == Property(TEXCLASS.REL)) {
        if (form == Property("prefix")) setTexClass(TEXCLASS.OPEN);
        if (form == Property("postfix")) setTexClass(TEXCLASS.CLOSE);
    }
    return adjustTeXclass(prev);
}

MmlNode* MmlMo::adjustTeXclass(MmlNode* prev) {
    return adjustOperatorTeXclass(*this, prev);
}

}
