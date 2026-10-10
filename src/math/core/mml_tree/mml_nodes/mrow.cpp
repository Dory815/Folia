/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native implementation of MathJax 4.1.3 MmlNodes/mrow.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.mrow;
import folia.math.core.mml_tree.leaf_host;
import std;

namespace folia::math::mml {
void MmlMrow::initializeClassFields() {
    core_.reset();
}

std::string MmlMrow::kind() const { return "mrow"; }

bool MmlMrow::isSpacelike() const {
    tree::TreeCallGuard<MmlNode> guard;
    const auto children = this->childNodes;
    if (!children) host::typeError("this.childNodes is not iterable");
    for (std::size_t i = 0; i < children->size(); ++i) {
        auto* child = (*children)[i];
        if (!child) host::typeError("Cannot read isSpacelike from null child");
        if (!child->isSpacelike()) return false;
    }
    return true;
}

bool MmlMrow::isEmbellished() const {
    tree::TreeCallGuard<MmlNode> guard;
    bool embellished = false;
    std::size_t index = 0;
    const auto children = this->childNodes;
    if (!children) host::typeError("this.childNodes is not iterable");
    for (; index < children->size(); ++index) {
        auto* child = (*children)[index];
        if (child) {
            if (child->isEmbellished()) {
                if (embellished) return false;
                embellished = true;
                core_ = index;
            } else if (!child->isSpacelike()) {
                return false;
            }
        }
    }
    return embellished;
}

MmlNode* MmlMrow::core() {
    tree::TreeCallGuard<MmlNode> guard;
    if (!isEmbellished() || !core_) return this;
    const auto children = this->childNodes; // Re-read after virtual predicate.
    if (!children) host::typeError("Cannot read core child from null child list");
    return *core_ < children->size() ? (*children)[*core_] : nullptr;
}

MmlNode* MmlMrow::coreMO() {
    tree::TreeCallGuard<MmlNode> guard;
    if (!isEmbellished() || !core_) return this;
    const auto children = this->childNodes; // Re-read after virtual predicate.
    if (!children) host::typeError("Cannot read core mo from null child list");
    if (*core_ >= children->size()) host::typeError("Cannot read core mo from missing child");
    auto* child = (*children)[*core_];
    if (!child) host::typeError("Cannot read core mo from null child");
    return child->coreMO();
}

std::size_t MmlMrow::nonSpaceLength() {
    tree::TreeCallGuard<MmlNode> guard;
    const auto children = this->childNodes;
    if (!children) host::typeError("this.childNodes is not iterable");
    std::size_t count = 0;
    for (std::size_t i = 0; i < children->size(); ++i) {
        auto* child = (*children)[i];
        if (child && !child->isSpacelike()) ++count;
    }
    return count;
}

MmlNode* MmlMrow::firstNonSpace() {
    tree::TreeCallGuard<MmlNode> guard;
    const auto children = this->childNodes;
    if (!children) host::typeError("this.childNodes is not iterable");
    for (std::size_t i = 0; i < children->size(); ++i) {
        auto* child = (*children)[i];
        if (child && !child->isSpacelike()) return child;
    }
    return nullptr;
}

MmlNode* MmlMrow::lastNonSpace() {
    tree::TreeCallGuard<MmlNode> guard;
    if (!this->childNodes) host::typeError("Cannot read length of null child list");
    std::ptrdiff_t index = static_cast<std::ptrdiff_t>(this->childNodes->size());
    while (--index >= 0) {
        const auto children = this->childNodes; // This source method re-reads the property.
        if (!children) host::typeError("Cannot read child from null child list");
        auto* child = static_cast<std::size_t>(index) < children->size()
            ? (*children)[static_cast<std::size_t>(index)] : nullptr;
        if (child && !child->isSpacelike()) return child;
    }
    return nullptr;
}

MmlNode* MmlMrow::setTeXclass(MmlNode* previous) {
    tree::TreeCallGuard<MmlNode> guard;
    if (!host::nullish(getProperty("open")) || !host::nullish(getProperty("close"))) {
        getPrevClass(previous);
        previous = nullptr;
        const auto children = this->childNodes;
        if (!children) host::typeError("this.childNodes is not iterable");
        for (std::size_t i = 0; i < children->size(); ++i) {
            auto* child = (*children)[i];
            if (!child) host::typeError("Cannot set TeX class on null child");
            previous = child->setTeXclass(previous);
        }
        if (host::nullish(texClass())) setTexClass(static_cast<double>(TEXCLASS.INNER));
        return this;
    }

    const auto children = this->childNodes;
    if (!children) host::typeError("this.childNodes is not iterable");
    for (std::size_t i = 0; i < children->size(); ++i) {
        auto* child = (*children)[i];
        if (!child) host::typeError("Cannot set TeX class on null child");
        previous = child->setTeXclass(previous);
    }
    const auto currentChildren = this->childNodes; // The post-traversal index read is fresh.
    if (!currentChildren) host::typeError("Cannot read first child from null child list");
    if (!currentChildren->empty() && currentChildren->front())
        updateTeXclass(currentChildren->front());
    return previous;
}

std::string MmlInferredMrow::kind() const { return "inferredMrow"; }
bool MmlInferredMrow::isInferred() const { return true; }
bool MmlInferredMrow::notParent() const { return true; }

std::string MmlInferredMrow::toString() const {
    tree::TreeCallGuard<MmlNode> guard;
    const auto children = this->childNodes;
    if (!children) host::typeError("Cannot join null inferred-row children");
    tree::TreeJoinGuard<MmlNode> joinGuard(children.get());
    if (!joinGuard.entered()) return "[]";
    const auto length = children->size(); // Array.join captures its initial length.
    std::string result = "[";
    for (std::size_t i = 0; i < length; ++i) {
        if (i) result.push_back(',');
        if (i < children->size()) {
            if (auto* child = (*children)[i]) result += child->toString();
        }
    }
    result.push_back(']');
    return result;
}
} // namespace folia::math::mml
