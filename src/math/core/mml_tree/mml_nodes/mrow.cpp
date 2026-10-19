/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native implementation of MathJax 4.1.3 MmlNodes/mrow.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.mrow;
import folia.math.core.mml_tree.node_defaults;
import std;

namespace folia::math::mml {
MmlMrow::MmlMrow(MmlDocument &document) : MmlMrow(document, NodeKind::mrow) {}

MmlMrow::MmlMrow(MmlDocument &document, NodeKind kind) : AbstractMmlNode(document, kind) {}

void MmlMrow::copyNativeStateTo(MmlNode &destination, MmlClone &) const {
    auto *clonedRow = dynamic_cast<MmlMrow *>(&destination);
    if (!clonedRow)
        throw std::logic_error("mrow exact clone has an incompatible destination type");
    clonedRow->core_ = nullptr;
}

AttributeSet MmlMrow::defaults() {
    return nodeDefaultAttributes(NodeKind::mrow);
}

NodeKind MmlMrow::kind() const noexcept {
    return NodeKind::mrow;
}

bool MmlMrow::isSpacelike() const {
    for (auto *child : children())
        if (!child->isSpacelike())
            return false;
    return true;
}

bool MmlMrow::isEmbellished() const {
    bool embellished = false;
    for (auto *child : children()) {
        if (!child)
            continue;
        if (child->isEmbellished()) {
            if (embellished)
                return false;
            embellished = true;
            core_ = child;
        } else if (!child->isSpacelike()) {
            return false;
        }
    }
    return embellished;
}

MmlNode *MmlMrow::core() {
    if (!isEmbellished() || !core_)
        return this;
    return core_;
}

MmlNode *MmlMrow::coreMO() {
    if (!isEmbellished() || !core_)
        return this;
    return core_->coreMO();
}

std::size_t MmlMrow::nonSpaceLength() {
    std::size_t count = 0;
    for (auto *child : children())
        if (child && !child->isSpacelike())
            ++count;
    return count;
}

MmlNode *MmlMrow::firstNonSpace() {
    for (auto *child : children())
        if (child && !child->isSpacelike())
            return child;
    return nullptr;
}

MmlNode *MmlMrow::lastNonSpace() {
    const auto values = children();
    for (std::size_t index = values.size(); index > 0; --index) {
        auto *child = values[index - 1];
        if (child && !child->isSpacelike())
            return child;
    }
    return nullptr;
}

MmlNode *MmlMrow::setTeXclass(MmlNode *previous) {
    const auto &properties = this->properties().tex;
    if (properties.open || properties.close) {
        getPrevClass(previous);
        previous = nullptr;
        for (auto *child : children())
            previous = child->setTeXclass(previous);
        if (!texClass())
            setTexClass(TexClass::inner);
        return this;
    }
    for (auto *child : children())
        previous = child->setTeXclass(previous);
    const auto values = children();
    if (!values.empty() && values.front())
        updateTeXclass(values.front());
    return previous;
}

MmlInferredMrow::MmlInferredMrow(MmlDocument &document)
    : MmlMrow(document, NodeKind::inferred_mrow) {}

AttributeSet MmlInferredMrow::defaults() {
    return MmlMrow::defaults();
}

NodeKind MmlInferredMrow::kind() const noexcept {
    return NodeKind::inferred_mrow;
}

bool MmlInferredMrow::isInferred() const {
    return true;
}

bool MmlInferredMrow::notParent() const {
    return true;
}

std::string MmlInferredMrow::toString() const {
    std::string result = "[";
    bool first = true;
    for (auto *child : children()) {
        if (!first)
            result.push_back(',');
        first = false;
        if (child)
            result += child->toString();
    }
    result.push_back(']');
    return result;
}
} // namespace folia::math::mml
