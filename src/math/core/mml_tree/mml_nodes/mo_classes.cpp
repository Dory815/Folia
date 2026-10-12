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
    NodeValue parent = this;
    const auto mathClass = this->factory.getNodeClass("math");
    tree::TreeWalkBudget<MmlNode> walkBudget;
    while (parent.isNode() && parent.requireNode("isEmbellished")->isEmbellished() &&
           parent.requireNode("coreMO")->coreMO() == NodeValue(this) &&
           !host::moInstance(parent.nodeOrNull(), mathClass)) {
        walkBudget.step();
        embellished = parent.nodeOrNull();
        parent = parent.requireNode("parent")->getParent();
    }
    return embellished ? embellished : this;
}

std::u16string MmlMo::coreText(NodeValue parent) {
    tree::TreeCallGuard<MmlNode> guard;
    if (!parent.isNode()) return {};
    MmlNode* node = parent.nodeOrNull();
    if (node->isEmbellished()) {
        auto* core = node->coreMO().requireNode("getText");
        if (auto* text = dynamic_cast<TextNode*>(core)) return text->getText();
        if (auto* token = dynamic_cast<AbstractMmlTokenNode*>(core)) return token->getText();
        throw Error("TypeError",u"parent.coreMO(...).getText is not a function");
    }
    const auto isUnwrappable = [](MmlNode& candidate) {
        return candidate.isKind("mrow") || candidate.isKind("TeXAtom") ||
               candidate.isKind("mstyle") || candidate.isKind("mphantom");
    };
    tree::TreeWalkBudget<MmlNode> walkBudget;
    while (((isUnwrappable(*parent.requireNode("isKind")) && ([&] {
                if (!node->childNodes) throw Error("TypeError",u"Cannot read properties of null (reading 'length')");
                return node->childNodes->size() == 1;
            })()) ||
            node->isKind("munderover")) &&
           host::moChild(NodeValue(node), 0).isNode()) {
        walkBudget.step();
        parent = host::moChild(NodeValue(node), 0);
        node = parent.requireNode("isKind");
    }
    if (!node->isToken()) return {};
    auto* token = dynamic_cast<AbstractMmlTokenNode*>(node);
    if (!token) throw Error("TypeError",u"parent.getText is not a function");
    return token->getText();
}

Property MmlMo::isAccent() {
    tree::TreeCallGuard<MmlNode> guard;
    Property accent = false;
    MmlNode* node = NodeValue(coreParent()).requireNode("parent")->getParent().nodeOrNull();
    if (node) {
        std::string key;
        if (node->isKind("mover")) {
            auto* mover = dynamic_cast<MmlMunderover*>(node);
            if (!mover) {
                if (!node->childNodes) host::typeError("Cannot read properties of null (reading 'undefined')");
                NodeValue(Missing{}).requireNode("coreMO");
            }
            const auto index = mover->over();
            key = host::moChild(NodeValue(node), index).requireNode("coreMO")->coreMO().isNode()
                ? "accent" : "";
        } else if (node->isKind("munder")) {
            auto* munder = dynamic_cast<MmlMunderover*>(node);
            if (!munder) {
                if (!node->childNodes) host::typeError("Cannot read properties of null (reading 'undefined')");
                NodeValue(Missing{}).requireNode("coreMO");
            }
            const auto index = munder->under();
            key = host::moChild(NodeValue(node), index).requireNode("coreMO")->coreMO().isNode()
                ? "accentunder" : "";
        } else if (node->isKind("munderover")) {
            auto* multi = dynamic_cast<MmlMunderover*>(node);
            if (!multi) {
                if (!node->childNodes) host::typeError("Cannot read properties of null (reading 'undefined')");
                NodeValue(Missing{}).requireNode("coreMO");
            }
            if (NodeValue(this) == host::moChild(NodeValue(node), multi->over())
                                    .requireNode("coreMO")->coreMO()) {
                key = "accent";
            } else if (NodeValue(this) == host::moChild(NodeValue(node), multi->under())
                                           .requireNode("coreMO")->coreMO()) {
                key = "accentunder";
            }
        }
        if (!key.empty()) {
            const auto nodeAttributes = node->attributes();
            if (!nodeAttributes) host::typeError("Cannot read properties of null (reading 'getExplicit')");
            const Property value = nodeAttributes->getExplicit(key);
            if (value.missing()) {
                const auto ownAttributes = attributes();
                if (!ownAttributes) host::typeError("Cannot read properties of null (reading 'get')");
                accent = ownAttributes->get("accent");
            }
        }
    }
    return accent;
}

NodeValue MmlMo::setTeXclass(NodeValue prev) {
    tree::TreeCallGuard<MmlNode> guard;
    const auto currentAttributes = attributes();
    if (!currentAttributes) host::typeError("Cannot read properties of null (reading 'getList')");
    const auto values = currentAttributes->getList({"form", "fence"});
    const Property form = values.get("form");
    const Property fence = values.get("fence");
    if (getProperty("texClass").missing() && hasSpacingAttributes()) return PropertyNull{};
    if (host::truthy(fence) && texClass() == Property(TEXCLASS.REL)) {
        if (form == Property{std::string("prefix")}) setTexClass(TEXCLASS.OPEN);
        if (form == Property{std::string("postfix")}) setTexClass(TEXCLASS.CLOSE);
    }
    return adjustTeXclass(prev);
}

NodeValue MmlMo::adjustTeXclass(NodeValue prev) {
    return adjustOperatorTeXclass(*this, prev);
}

}
