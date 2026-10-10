/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native C++ port of the AbstractMmlTokenNode, AbstractMmlLayoutNode,
 * and AbstractMmlBaseNode behavior from MathJax 4.1.3 MmlNode.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.mml_node;
import folia.math.core.mml_tree.mml_node_host;
import std;

namespace folia::math::mml {
namespace {
MmlNode* child_zero(AbstractMmlNode& node) {
    if (!node.childNodes) host::typeError("Cannot read child list");
    return node.childNodes->empty() ? nullptr : node.childNodes->front();
}

MmlNode& required_child_zero(const AbstractMmlNode& node, std::string_view member) {
    if (!node.childNodes) host::typeError("Cannot read properties of null (reading '0')");
    if (node.childNodes->empty()) {
        const std::string message = "Cannot read properties of undefined (reading '" +
                                    std::string(member) + "')";
        host::typeError(message);
    }
    MmlNode* const child = node.childNodes->front();
    if (!child) {
        const std::string message = "Cannot read properties of null (reading '" +
                                    std::string(member) + "')";
        host::typeError(message);
    }
    return *child;
}

void require_child_list(AbstractMmlNode& node) {
    if (!node.childNodes) host::typeError("this.childNodes is not iterable");
}
}

bool AbstractMmlTokenNode::isToken() const {
    return true;
}

bool AbstractMmlTokenNode::isEmpty() const {
    tree::TreeCallGuard<MmlNode> guard;
    if (!childNodes) host::typeError("this.childNodes is not iterable");
    const auto children = childNodes;
    for (std::size_t index = 0; index < children->size(); ++index) {
        MmlNode* const child = (*children)[index];
        auto* const text = dynamic_cast<TextNode*>(child);
        if (!text || !text->getText().empty()) return false;
    }
    return true;
}

std::u16string AbstractMmlTokenNode::getText() const {
    tree::TreeCallGuard<MmlNode> guard;
    if (!childNodes) host::typeError("this.childNodes is not iterable");
    std::u16string text;
    const auto children = childNodes;
    for (std::size_t index = 0; index < children->size(); ++index) {
        MmlNode* const child = (*children)[index];
        if (auto* const textNode = dynamic_cast<TextNode*>(child)) {
            text += textNode->getText();
        } else {
            if (!child) host::typeError("Cannot use 'in' operator to search for 'textContent' in null");
            if (auto* const binding = dynamic_cast<TextContentBinding*>(child)) {
                if (!binding->textContent) continue;
                if (!*binding->textContent) host::typeError("node.textContent is not a function");
                text += (*binding->textContent)();
            }
        }
    }
    return text;
}

void AbstractMmlTokenNode::setChildInheritedAttributes(
    AttributeList attributes, Property display, Property level, Property prime) {
    require_child_list(*this);
    const auto children = childNodes;
    for (std::size_t index = 0; index < children->size(); ++index) {
        if (auto* const child = dynamic_cast<AbstractMmlNode*>((*children)[index])) {
            child->setInheritedAttributes(attributes, display, level, prime);
        }
    }
}

std::any AbstractMmlTokenNode::walkTree(
    const tree::TreeVisitor<MmlNode>& visit, const std::any& data) {
    tree::TreeCallGuard<MmlNode> guard;
    visit(this, data);
    require_child_list(*this);
    const auto children = childNodes;
    for (std::size_t index = 0; index < children->size(); ++index) {
        if (auto* const child = dynamic_cast<AbstractMmlNode*>((*children)[index])) {
            child->walkTree(visit, data);
        }
    }
    return data;
}

bool AbstractMmlLayoutNode::isSpacelike() const {
    tree::TreeCallGuard<MmlNode> guard;
    return required_child_zero(*this, "isSpacelike").isSpacelike();
}

bool AbstractMmlLayoutNode::isEmbellished() const {
    tree::TreeCallGuard<MmlNode> guard;
    return required_child_zero(*this, "isEmbellished").isEmbellished();
}

double AbstractMmlLayoutNode::arity() const {
    return -1;
}

MmlNode* AbstractMmlLayoutNode::core() {
    return child_zero(*this);
}

MmlNode* AbstractMmlLayoutNode::coreMO() {
    tree::TreeCallGuard<MmlNode> guard;
    return required_child_zero(*this, "coreMO").coreMO();
}

MmlNode* AbstractMmlLayoutNode::setTeXclass(MmlNode* previous) {
    tree::TreeCallGuard<MmlNode> guard;
    MmlNode& child = required_child_zero(*this, "setTeXclass");
    previous = child.setTeXclass(previous);
    updateTeXclass(child_zero(*this));
    return previous;
}

bool AbstractMmlBaseNode::isEmbellished() const {
    tree::TreeCallGuard<MmlNode> guard;
    return required_child_zero(*this, "isEmbellished").isEmbellished();
}

MmlNode* AbstractMmlBaseNode::core() {
    return child_zero(*this);
}

MmlNode* AbstractMmlBaseNode::coreMO() {
    tree::TreeCallGuard<MmlNode> guard;
    return required_child_zero(*this, "coreMO").coreMO();
}

MmlNode* AbstractMmlBaseNode::setTeXclass(MmlNode* previous) {
    tree::TreeCallGuard<MmlNode> guard;
    getPrevClass(previous);
    setTexClass(static_cast<double>(TEXCLASS.ORD));
    MmlNode* const base = child_zero(*this);
    MmlNode* result = nullptr;
    if (base) {
        if (isEmbellished() || base->isKind("mi")) {
            result = base->setTeXclass(previous);
            updateTeXclass(core());
        } else {
            base->setTeXclass(nullptr);
            if (base->isKind("TeXAtom")) setTexClass(base->texClass());
        }
    }
    if (!childNodes) host::typeError("Cannot read child list");
    // Array.prototype.slice(1) snapshots the script list after base handling.
    const auto children = childNodes;
    std::vector<MmlNode*> scripts;
    if (children->size() > 1) {
        scripts.assign(children->begin() + 1, children->end());
    }
    for (MmlNode* const child : scripts) {
        if (child) child->setTeXclass(nullptr);
    }
    return result ? result : this;
}

} // namespace folia::math::mml
