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
NodeValue child_zero(AbstractMmlNode& node) {
    const auto children = node.childNodes;
    if (!children) throw Error("TypeError",u"Cannot read properties of null (reading '0')");
    return children->get(0);
}

MmlNode& required_child_zero(const AbstractMmlNode& node, std::string_view member) {
    const auto children = node.childNodes;
    if (!children) throw Error("TypeError",u"Cannot read properties of null (reading '0')");
    return *children->get(0).requireNode(member);
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
    const auto children = this->childNodes;
    if (!children) throw Error("TypeError",u"object null is not iterable (cannot read property Symbol(Symbol.iterator))");
    for (std::size_t i = 0; i < children->size(); ++i) {
        const NodeValue child = children->get(i);
        MmlNode* const node = child.nodeOrNull();
        auto* const text = node ? dynamic_cast<TextNode*>(node) : nullptr;
        if (!text || !text->getText().empty()) return false;
    }
    return true;
}

std::u16string AbstractMmlTokenNode::getText() const {
    // P23f Root stub: AbstractMmlTokenNode::getText.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

void AbstractMmlTokenNode::setChildInheritedAttributes(
    AttributeList attributes, Property display, Property level, Property prime) {
    // P23f Root stub: AbstractMmlTokenNode::setChildInheritedAttributes.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

std::any AbstractMmlTokenNode::walkTree(
    const tree::TreeVisitor<MmlNode>& visit, const std::any& data) {
    // P23f Root stub: AbstractMmlTokenNode::walkTree.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
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

NodeValue AbstractMmlLayoutNode::core() {
    return child_zero(*this);
}

NodeValue AbstractMmlLayoutNode::coreMO() {
    tree::TreeCallGuard<MmlNode> guard;
    return child_zero(*this).requireNode("coreMO")->coreMO();
}

NodeValue AbstractMmlLayoutNode::setTeXclass(NodeValue previous) {
    // P23f Root stub: AbstractMmlLayoutNode::setTeXclass.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

bool AbstractMmlBaseNode::isEmbellished() const {
    tree::TreeCallGuard<MmlNode> guard;
    return required_child_zero(*this, "isEmbellished").isEmbellished();
}

NodeValue AbstractMmlBaseNode::core() {
    return child_zero(*this);
}

NodeValue AbstractMmlBaseNode::coreMO() {
    tree::TreeCallGuard<MmlNode> guard;
    return child_zero(*this).requireNode("coreMO")->coreMO();
}

NodeValue AbstractMmlBaseNode::setTeXclass(NodeValue previous) {
    // P23f Root stub: AbstractMmlBaseNode::setTeXclass.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

} // namespace folia::math::mml
