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

    // MathJax first captures the array object, then performs live indexed
    // reads on it. Replacing this->childNodes during a child callback must not
    // retarget the remaining reads, while mutations to this vector stay live.
    const auto nodes = this->childNodes;
    if (!nodes) host::typeError("Cannot read childNodes from null");

    // JS evaluates the member receiver before its call arguments. Consequently
    // a missing/null base or subscript fails before level addition or a virtual
    // sub getter can run.
    if (nodes->empty() || !(*nodes)[0])
        host::typeError("Cannot read setInheritedAttributes from missing base");
    auto* baseNode = (*nodes)[0];
    baseNode->setInheritedAttributes(attributes, display, level, prime);

    if (nodes->size() < 2 || !(*nodes)[1])
        host::typeError("Cannot read setInheritedAttributes from missing subscript");
    auto* subNode = (*nodes)[1];

    // Preserve the source's left-to-right argument evaluation: level + 1 is
    // JS primitive addition (so strings append), followed by prime || sub===1.
    Property subLevel = host::add(level, Property(1.0));
    Property subPrime;
    if (host::truthy(prime)) {
        subPrime = prime;
    } else {
        const auto subIndex = this->sub();
        subPrime = Property(subIndex == 1);
    }
    subNode->setInheritedAttributes(attributes, Property(false),
                                    std::move(subLevel), std::move(subPrime));

    // An absent/null third child is falsey in the source. Prior base/sub
    // writes remain observable and no superscript arguments are evaluated.
    if (nodes->size() < 3 || !(*nodes)[2]) return;
    auto* supNode = (*nodes)[2];

    Property supLevel = host::add(level, Property(1.0));
    Property supPrime;
    if (host::truthy(prime)) {
        supPrime = prime;
    } else {
        const auto subIndex = this->sub();
        supPrime = Property(subIndex == 2);
    }
    supNode->setInheritedAttributes(attributes, Property(false),
                                    std::move(supLevel), std::move(supPrime));
}

std::string MmlMsub::kind() const { return "msub"; }
double MmlMsub::arity() const { return 2; }

std::string MmlMsup::kind() const { return "msup"; }
double MmlMsup::arity() const { return 2; }
std::size_t MmlMsup::sup() const { return 1; }
std::size_t MmlMsup::sub() const { return 2; }

} // namespace folia::math::mml
