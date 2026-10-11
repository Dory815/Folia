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
    if (!children) throw Error("TypeError",u"object null is not iterable (cannot read property Symbol(Symbol.iterator))");
    for (std::size_t i = 0; i < children->size(); ++i) {
        const NodeValue value = children->get(i);
        if (!value.requireNode("isSpacelike")->isSpacelike()) return false;
    }
    return true;
}

bool MmlMrow::isEmbellished() const {
    tree::TreeCallGuard<MmlNode> guard;
    const auto children = this->childNodes;
    if (!children) throw Error("TypeError",u"object null is not iterable (cannot read property Symbol(Symbol.iterator))");
    bool embellished = false;
    for (std::size_t i = 0; i < children->size(); ++i) {
        const NodeValue value = children->get(i);
        MmlNode* const child = value.nodeOrNull();
        if (!child) continue;
        if (child->isEmbellished()) {
            if (embellished) return false;
            embellished = true;
            core_ = i;
        } else if (!child->isSpacelike()) {
            return false;
        }
    }
    return embellished;
}

NodeValue MmlMrow::core() {
    tree::TreeCallGuard<MmlNode> guard;
    if (!this->isEmbellished()) return this;
    if (!core_) return this;
    const std::size_t index = *core_;
    const auto children = this->childNodes;
    if (!children) {
        std::string message = "Cannot read properties of null (reading '" + std::to_string(index) + "')";
        throw Error("TypeError",std::u16string(message.begin(),message.end()));
    }
    return children->get(index);
}

NodeValue MmlMrow::coreMO() {
    tree::TreeCallGuard<MmlNode> guard;
    if (!this->isEmbellished()) return this;
    if (!core_) return this;
    const std::size_t index = *core_;
    const auto children = this->childNodes;
    if (!children) {
        std::string message = "Cannot read properties of null (reading '" + std::to_string(index) + "')";
        throw Error("TypeError",std::u16string(message.begin(),message.end()));
    }
    const NodeValue child = children->get(index);
    return child.requireNode("coreMO")->coreMO();
}

std::size_t MmlMrow::nonSpaceLength() {
    // P23f Root stub: MmlMrow::nonSpaceLength.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

MmlNode* MmlMrow::firstNonSpace() {
    // P23f Root stub: MmlMrow::firstNonSpace.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

MmlNode* MmlMrow::lastNonSpace() {
    // P23f Root stub: MmlMrow::lastNonSpace.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

NodeValue MmlMrow::setTeXclass(NodeValue previous) {
    // P23f Root stub: MmlMrow::setTeXclass.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

std::string MmlInferredMrow::kind() const { return "inferredMrow"; }
bool MmlInferredMrow::isInferred() const { return true; }
Property MmlInferredMrow::notParent() const { return true; }

std::string MmlInferredMrow::toString() const {
    // P23f Root stub: MmlInferredMrow::toString.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}
} // namespace folia::math::mml
