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
    const auto children = node.childArrayOwner();
    if (!children) throw Error("TypeError",u"Cannot read properties of null (reading '0')");
    return children->get(0);
}

MmlNode& required_child_zero(const AbstractMmlNode& node, std::string_view member) {
    const auto children = node.childArrayOwner();
    if (!children) throw Error("TypeError",u"Cannot read properties of null (reading '0')");
    return *children->get(0).requireNode(member);
}

void require_child_list(AbstractMmlNode& node) {
    if (!node.childArrayOwner()) host::typeError("this.childNodes is not iterable");
}
}

bool AbstractMmlTokenNode::isToken() const {
    return true;
}

bool AbstractMmlTokenNode::isEmpty() const {
    tree::TreeCallGuard<MmlNode> guard;
    const auto children = this->childArrayOwner();
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
    tree::TreeCallGuard<MmlNode> guard;
    const Children children = this->childArrayOwner();
    if (!children) host::typeError("object null is not iterable (cannot read property Symbol(Symbol.iterator))");

    std::u16string text;
    for (std::size_t index = 0; index < children->size(); ++index) {
        const NodeValue child = children->get(index);
        MmlNode* const node = child.nodeOrNull();
        if (auto* const textNode = node ? dynamic_cast<TextNode*>(node) : nullptr) {
            text += textNode->getText();
        } else if (!node) {
            const std::string value = child.kind() == tree::ChildKind::Null ? "null" : "undefined";
            host::typeError("Cannot use 'in' operator to search for 'textContent' in " + value);
        } else if (auto* const binding = dynamic_cast<TextContentBinding*>(node);
                   binding && binding->textContent.has_value()) {
            if (!*binding->textContent) host::typeError("child.textContent is not a function");
            text += (*binding->textContent)();
        }
    }
    return text;
}

void AbstractMmlTokenNode::setChildInheritedAttributes(
    AttributeList attributes, Property display, Property level, Property prime) {
    const Children children = this->childArrayOwner();
    if (!children) host::typeError("object null is not iterable (cannot read property Symbol(Symbol.iterator))");
    for (std::size_t index = 0; index < children->size(); ++index) {
        MmlNode* const node = children->get(index).nodeOrNull();
        if (auto* const mmlNode = node ? dynamic_cast<AbstractMmlNode*>(node) : nullptr) {
            mmlNode->setInheritedAttributes(attributes, display, level, prime);
        }
    }
}

std::any AbstractMmlTokenNode::walkTree(
    const tree::TreeVisitor<MmlNode>& visit, const std::any& data) {
    walkTreeRaw([&visit](NodeValue child,const std::any& state){visit(child.requireNode("walkTree"),state);},data);
    return data;
}

std::any AbstractMmlTokenNode::walkTreeRaw(const tree::TreeRawVisitor<MmlNode>& visit,
    const std::any& data,const std::any& visitorIdentity) {
    if(const auto access=sourceTreeAccess())
        return access->walkTreeRaw(visit,data,visitorIdentity);
    tree::TreeNodeAlgorithms<MmlNode>::walkTreeFiltered(treeReceiver(),visit,data,visitorIdentity,
        [](NodeValue child){return dynamic_cast<AbstractMmlNode*>(child.nodeOrNull())!=nullptr;});
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

NodeValue AbstractMmlLayoutNode::core() {
    return child_zero(*this);
}

NodeValue AbstractMmlLayoutNode::coreMO() {
    tree::TreeCallGuard<MmlNode> guard;
    return child_zero(*this).requireNode("coreMO")->coreMO();
}

NodeValue AbstractMmlLayoutNode::setTeXclass(NodeValue previous) {
    tree::TreeCallGuard<MmlNode> guard;
    previous = required_child_zero(*this, "setTeXclass").setTeXclass(previous);
    updateTeXclass(child_zero(*this));
    return previous;
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
    tree::TreeCallGuard<MmlNode> guard;
    getPrevClass(previous);
    setTexClass(static_cast<double>(TEXCLASS.ORD));

    const NodeValue base = child_zero(*this);
    NodeValue result(PropertyNull{});
    if (auto* const baseNode = base.nodeOrNull()) {
        if (isEmbellished() || baseNode->isKind("mi")) {
            result = baseNode->setTeXclass(previous);
            updateTeXclass(core());
        } else {
            baseNode->setTeXclass(NodeValue(PropertyNull{}));
            if (baseNode->isKind("TeXAtom")) {
                setTexClass(baseNode->texClass());
            }
        }
    }

    const auto currentChildren = childArrayOwner();
    if (!currentChildren) {
        throw Error("TypeError",u"Cannot read properties of null (reading 'slice')");
    }
    const auto tail = currentChildren->slice(1);
    for (std::size_t index = 0; index < tail->size(); ++index) {
        const NodeValue child = tail->get(index);
        if (child.isNode()) {
            child.requireNode("setTeXclass")->setTeXclass(NodeValue(PropertyNull{}));
        }
    }

    return result.isNode() ? result : NodeValue(this);
}

} // namespace folia::math::mml
