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
    // P23f Root stub: MmlMrow::isSpacelike.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

bool MmlMrow::isEmbellished() const {
    // P23f Root stub: MmlMrow::isEmbellished.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

NodeValue MmlMrow::core() {
    // P23f Root stub: MmlMrow::core.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

NodeValue MmlMrow::coreMO() {
    // P23f Root stub: MmlMrow::coreMO.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
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
