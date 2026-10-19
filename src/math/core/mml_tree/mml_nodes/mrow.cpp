/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native implementation of MathJax 4.1.3 MmlNodes/mrow.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.mrow;
import folia.math.core.mml_tree.leaf_host;
import folia.math.util.property_host;
import std;

namespace folia::math::mml {
void MmlMrow::initializeClassFields() {
    core_=PropertyNull{};
}

std::string MmlMrow::kind() const { return "mrow"; }

bool MmlMrow::isSpacelike() const {
    tree::TreeCallGuard<MmlNode> guard;
    const auto children = this->childArrayOwner();
    if (!children) throw Error("TypeError",u"object null is not iterable (cannot read property Symbol(Symbol.iterator))");
    for (std::size_t i = 0; i < children->size(); ++i) {
        const NodeValue value = children->get(i);
        if (!value.requireNode("isSpacelike")->isSpacelike()) return false;
    }
    return true;
}

bool MmlMrow::isEmbellished() const {
    tree::TreeCallGuard<MmlNode> guard;
    const auto children = this->childArrayOwner();
    if (!children) throw Error("TypeError",u"object null is not iterable (cannot read property Symbol(Symbol.iterator))");
    bool embellished = false;
    for (std::size_t i = 0; i < children->size(); ++i) {
        const NodeValue value = children->get(i);
        MmlNode* const child = value.nodeOrNull();
        if (!child) continue;
        if (child->isEmbellished()) {
            if (embellished) return false;
            embellished = true;
            core_ = static_cast<double>(i);
        } else if (!child->isSpacelike()) {
            return false;
        }
    }
    return embellished;
}

NodeValue MmlMrow::core() {
    tree::TreeCallGuard<MmlNode> guard;
    if (!this->isEmbellished()) return this;
    if (host::nullish(core_)) return this;
    const std::string index=property_host::debugText(host::string(core_));
    const auto children = this->childArrayOwner();
    if (!children) {
        std::string message = "Cannot read properties of null (reading '" + index + "')";
        throw Error("TypeError",std::u16string(message.begin(),message.end()));
    }
    return children->getProperty(index);
}

NodeValue MmlMrow::coreMO() {
    tree::TreeCallGuard<MmlNode> guard;
    if (!this->isEmbellished()) return this;
    if (host::nullish(core_)) return this;
    const std::string index=property_host::debugText(host::string(core_));
    const auto children = this->childArrayOwner();
    if (!children) {
        std::string message = "Cannot read properties of null (reading '" + index + "')";
        throw Error("TypeError",std::u16string(message.begin(),message.end()));
    }
    const NodeValue child = children->getProperty(index);
    return child.requireNode("coreMO")->coreMO();
}

std::size_t MmlMrow::nonSpaceLength() {
    tree::TreeCallGuard<MmlNode> guard;
    const auto children = this->childArrayOwner();
    if (!children) throw Error("TypeError",u"object null is not iterable (cannot read property Symbol(Symbol.iterator))");
    std::size_t count = 0;
    for (std::size_t i = 0; i < children->size(); ++i) {
        const NodeValue value = children->get(i);
        MmlNode* const child = value.nodeOrNull();
        if (child && !child->isSpacelike()) ++count;
    }
    return count;
}

MmlNode* MmlMrow::firstNonSpace() {
    tree::TreeCallGuard<MmlNode> guard;
    const auto children = this->childArrayOwner();
    if (!children) throw Error("TypeError",u"object null is not iterable (cannot read property Symbol(Symbol.iterator))");
    for (std::size_t i = 0; i < children->size(); ++i) {
        const NodeValue value = children->get(i);
        MmlNode* const child = value.nodeOrNull();
        if (child && !child->isSpacelike()) return child;
    }
    return nullptr;
}

MmlNode* MmlMrow::lastNonSpace() {
    tree::TreeCallGuard<MmlNode> guard;
    const auto initialChildren = this->childArrayOwner();
    if (!initialChildren) {
        throw Error("TypeError",u"Cannot read properties of null (reading 'length')");
    }
    std::size_t i = initialChildren->size();
    while (i > 0) {
        --i;
        const auto children = this->childArrayOwner();
        if (!children) {
            const std::string message = "Cannot read properties of null (reading '" + std::to_string(i) + "')";
            throw Error("TypeError",std::u16string(message.begin(),message.end()));
        }
        const NodeValue value = children->get(i);
        MmlNode* const child = value.nodeOrNull();
        if (child && !child->isSpacelike()) return child;
    }
    return nullptr;
}

NodeValue MmlMrow::setTeXclass(NodeValue previous) {
    tree::TreeCallGuard<MmlNode> guard;
    if (!host::nullish(getProperty("open")) || !host::nullish(getProperty("close"))) {
        getPrevClass(previous);
        previous = PropertyNull{};
        const auto children = this->childArrayOwner();
        if (!children) throw Error("TypeError",u"object null is not iterable (cannot read property Symbol(Symbol.iterator))");
        for (std::size_t i = 0; i < children->size(); ++i) {
            previous = children->get(i).requireNode("setTeXclass")->setTeXclass(previous);
        }
        if (host::nullish(texClass())) setTexClass(static_cast<double>(TEXCLASS.INNER));
        return this;
    }
    const auto children = this->childArrayOwner();
    if (!children) throw Error("TypeError",u"object null is not iterable (cannot read property Symbol(Symbol.iterator))");
    for (std::size_t i = 0; i < children->size(); ++i) {
        previous = children->get(i).requireNode("setTeXclass")->setTeXclass(previous);
    }
    const auto currentChildren = this->childArrayOwner();
    if (!currentChildren) throw Error("TypeError",u"Cannot read properties of null (reading '0')");
    const NodeValue first = currentChildren->get(0);
    if (first.nodeOrNull()) {
        const auto latestChildren = this->childArrayOwner();
        if (!latestChildren) throw Error("TypeError",u"Cannot read properties of null (reading '0')");
        updateTeXclass(latestChildren->get(0));
    }
    return previous;
}

std::string MmlInferredMrow::kind() const { return "inferredMrow"; }
bool MmlInferredMrow::isInferred() const { return true; }
Property MmlInferredMrow::notParent() const { return true; }

std::string MmlInferredMrow::toString() const {
    tree::TreeCallGuard<MmlNode> callGuard;
    const auto children = this->childArrayOwner();
    if (!children) throw Error("TypeError",u"Cannot read properties of null (reading 'join')");
    tree::TreeJoinGuard<MmlNode> joinGuard(children.get());
    if (!joinGuard.entered()) return "[]";
    const std::size_t length = children->size();
    std::string result = "[";
    for (std::size_t i = 0; i < length; ++i) {
        if (i != 0) result.push_back(',');
        const NodeValue value = children->get(i);
        if (MmlNode* const child = value.nodeOrNull()) result += child->toString();
    }
    result.push_back(']');
    return result;
}
} // namespace folia::math::mml
