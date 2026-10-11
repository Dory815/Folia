/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native implementation of MathJax 4.1.3 MmlNodes/msubsup.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.msubsup;
import folia.math.core.mml_tree.leaf_host;
import std;

namespace folia::math::mml {

std::string MmlMsubsup::kind() const { return "msubsup"; }
double MmlMsubsup::arity() const { return 3; }
std::size_t MmlMsubsup::base() const { return 0; }
std::size_t MmlMsubsup::sub() const { return 1; }
std::size_t MmlMsubsup::sup() const { return 2; }

void MmlMsubsup::setChildInheritedAttributes(
    AttributeList attributes, Property display, Property level, Property prime) {
    // P23f Root stub: MmlMsubsup::setChildInheritedAttributes.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

std::string MmlMsub::kind() const { return "msub"; }
double MmlMsub::arity() const { return 2; }

std::string MmlMsup::kind() const { return "msup"; }
double MmlMsup::arity() const { return 2; }
std::size_t MmlMsup::sup() const { return 1; }
std::size_t MmlMsup::sub() const { return 2; }

} // namespace folia::math::mml
