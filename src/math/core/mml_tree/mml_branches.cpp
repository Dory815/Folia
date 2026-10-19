/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native C++ port of MmlNode.ts token, layout, and base branches.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.mml_document;
import folia.math.core.mml_tree.mml_nodes.html_node;
import std;

namespace folia::math::mml {
namespace {
MmlNode const &firstChild(AbstractMmlNode const &node) {
    auto values = node.children();
    if (values.empty())
        throw std::logic_error("missing first MML child");
    return *values.front();
}

MmlNode &firstChild(AbstractMmlNode &node) {
    auto values = node.children();
    if (values.empty())
        throw std::logic_error("missing first MML child");
    return *values.front();
}
} // namespace

AttributeSet AbstractMmlTokenNode::defaults() {
    auto result = AbstractMmlNode::defaults();
    result.setDefault(u"mathvariant", std::u16string(u"normal"));
    return result;
}

AttributeSet AbstractMmlLayoutNode::defaults() {
    return AbstractMmlNode::defaults();
}

AttributeSet AbstractMmlBaseNode::defaults() {
    return AbstractMmlNode::defaults();
}

bool AbstractMmlTokenNode::isToken() const {
    return true;
}

bool AbstractMmlTokenNode::isEmpty() const {
    for (MmlNode *child : children()) {
        auto *text = dynamic_cast<TextNode *>(child);
        if (!text || !text->getText().empty())
            return false;
    }
    return true;
}

std::u16string AbstractMmlTokenNode::getText() const {
    std::u16string result;
    for (MmlNode *child : children()) {
        if (auto *text = dynamic_cast<TextNode *>(child))
            result += text->getText();
        else if (auto *html = dynamic_cast<HtmlNode *>(child))
            result += html->textContent();
    }
    return result;
}

void AbstractMmlTokenNode::walkTree(std::function<void(MmlNode &)> const &visit) {
    visit(*this);
    for (MmlNode *child : children())
        if (dynamic_cast<AbstractMmlNode *>(child))
            child->walkTree(visit);
}

void AbstractMmlTokenNode::setChildInheritedAttributes(AttributeList const &a, bool display,
                                                       double level, bool prime) {
    for (MmlNode *child : children())
        if (dynamic_cast<AbstractMmlNode *>(child))
            child->setInheritedAttributes(a, display, level, prime);
}

bool AbstractMmlLayoutNode::isSpacelike() const {
    return firstChild(*this).isSpacelike();
}

bool AbstractMmlLayoutNode::isEmbellished() const {
    return firstChild(*this).isEmbellished();
}

double AbstractMmlLayoutNode::arity() const {
    return -1;
}

MmlNode *AbstractMmlLayoutNode::core() {
    return &firstChild(*this);
}

MmlNode *AbstractMmlLayoutNode::coreMO() {
    auto *n = core();
    return n ? n->coreMO() : nullptr;
}

MmlNode *AbstractMmlLayoutNode::setTeXclass(MmlNode *previous) {
    auto *n = core();
    if (!n)
        return previous;
    previous = n->setTeXclass(previous);
    updateTeXclass(n);
    return previous;
}

bool AbstractMmlBaseNode::isEmbellished() const {
    return firstChild(*this).isEmbellished();
}

MmlNode *AbstractMmlBaseNode::core() {
    return &firstChild(*this);
}

MmlNode *AbstractMmlBaseNode::coreMO() {
    auto *n = core();
    return n ? n->coreMO() : nullptr;
}

MmlNode *AbstractMmlBaseNode::setTeXclass(MmlNode *previous) {
    getPrevClass(previous);
    setTexClass(TexClass::ord);
    MmlNode *base = core();
    MmlNode *result = nullptr;
    if (base) {
        if (isEmbellished() || base->isKind(NodeKind::mi)) {
            result = base->setTeXclass(previous);
            updateTeXclass(core());
        } else {
            base->setTeXclass(nullptr);
            if (base->isKind(NodeKind::tex_atom))
                setTexClass(base->texClass());
        }
    }
    std::vector<MmlNode *> childrenCopy(children().begin(), children().end());
    for (std::size_t i = 1; i < childrenCopy.size(); ++i)
        childrenCopy[i]->setTeXclass(nullptr);
    return result ? result : this;
}
} // namespace folia::math::mml
