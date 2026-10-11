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
    tree::TreeCallGuard<MmlNode> guard;
    const auto nodes = this->childNodes;
    if (!nodes) host::typeError("Cannot read properties of null (reading '0')");

    nodes->get(0).requireNode("setInheritedAttributes")->setInheritedAttributes(
        attributes, display, level, prime);

    MmlNode& subscript = *nodes->get(1).requireNode("setInheritedAttributes");
    const Property subscriptLevel = host::add(level, Property(1));
    Property subscriptPrime = prime;
    if (!host::truthy(prime)) subscriptPrime = Property(this->sub() == 1);
    subscript.setInheritedAttributes(attributes, false, subscriptLevel, subscriptPrime);

    const NodeValue superscriptValue = nodes->get(2);
    if (!superscriptValue.isNode()) return;
    MmlNode& superscript = *superscriptValue.requireNode("setInheritedAttributes");
    const Property superscriptLevel = host::add(level, Property(1));
    Property superscriptPrime = prime;
    if (!host::truthy(prime)) superscriptPrime = Property(this->sub() == 2);
    superscript.setInheritedAttributes(attributes, false, superscriptLevel, superscriptPrime);
}

std::string MmlMsub::kind() const { return "msub"; }
double MmlMsub::arity() const { return 2; }

std::string MmlMsup::kind() const { return "msup"; }
double MmlMsup::arity() const { return 2; }
std::size_t MmlMsup::sup() const { return 1; }
std::size_t MmlMsup::sub() const { return 2; }

} // namespace folia::math::mml
