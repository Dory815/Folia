/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native port of MathJax 4.1.3 MmlNodes/munderover.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.munderover;
import folia.math.core.mml_tree.mml_node_host;
import std;

namespace folia::math::mml {
namespace {

std::string property_key(const Property& value) {
    return host::debugText(host::string(value));
}

NodeValue child_slot(const Children& nodes, std::size_t index) {
    if (!nodes) host::typeError("Cannot read properties of null (reading '"+std::to_string(index)+"')");
    return nodes->get(index);
}

MmlNode* require_child(const Children& nodes, std::size_t index,
                       std::string_view operation) {
    return child_slot(nodes,index).requireNode(operation);
}

Property truthy_or_under(const Property& left, const MmlMunderover& node,
                         std::size_t expected) {
    if (host::truthy(left)) return left;
    return Property(node.under() == expected);
}

}

std::string MmlMunderover::kind() const { return "munderover"; }
double MmlMunderover::arity() const { return 3; }
std::size_t MmlMunderover::base() const { return 0; }
std::size_t MmlMunderover::under() const { return 1; }
std::size_t MmlMunderover::over() const { return 2; }
bool MmlMunderover::linebreakContainer() const { return true; }

void MmlMunderover::setChildInheritedAttributes(AttributeList attributes,
                                               Property display,
                                               Property level,
                                               Property prime) {
    tree::TreeCallGuard<MmlNode> guard;
    const Children nodes = this->childArrayOwner();

    MmlNode* const baseNode = require_child(nodes, 0, "setInheritedAttributes");
    const bool basePrimePresent = host::truthy(prime)
        ? true
        : child_slot(nodes, this->over()).isNode();
    const Property basePrime = host::truthy(prime) ? prime : Property(basePrimePresent);
    baseNode->setInheritedAttributes(attributes, display, level, basePrime);

    bool force = false;
    if (!host::truthy(display)) {
        MmlNode* const baseForLimits = require_child(nodes, 0, "coreMO");
        const NodeValue baseCore = baseForLimits->coreMO();
        MmlNode* const coreNode = baseCore.requireNode("attributes");
        const auto coreAttributes = coreNode->attributes();
        if (!coreAttributes) host::typeError("Cannot read properties of null (reading 'get')");
        force = host::truthy(coreAttributes->get("movablelimits"));
    }

    const auto accentAt = constructorAccentReader();
    MmlNode* const underNode = require_child(nodes, 1, "setInheritedAttributes");
    const Property underAccent = accentAt(1);
    const Property underLevel = getScriptlevel(underAccent, force, level);
    const Property underPrime = host::truthy(prime) ? prime : Property(this->under() == 1);
    underNode->setInheritedAttributes(attributes, false, underLevel, underPrime);
    setInheritedAccent(1, accentAt(1), display, level, prime, force);

    if (!child_slot(nodes, 2).isNode()) return;
    MmlNode* const overNode = require_child(nodes, 2, "setInheritedAttributes");
    const Property overAccent = accentAt(2);
    const Property overLevel = getScriptlevel(overAccent, force, level);
    const Property overPrime = host::truthy(prime) ? prime : Property(this->under() == 2);
    overNode->setInheritedAttributes(attributes, false, overLevel, overPrime);
    setInheritedAccent(2, accentAt(2), display, level, prime, force);
}

Property MmlMunderover::getScriptlevel(Property accent, bool force, Property level) {
    tree::TreeCallGuard<MmlNode> guard;
    if (force) return Property(host::number(level) + 1);
    const auto currentAttributes = attributes();
    if (!currentAttributes) host::typeError("Cannot read script accent from null attributes");
    if (!host::truthy(currentAttributes->get(property_key(accent))))
        return Property(host::number(level) + 1);
    return level;
}

void MmlMunderover::setInheritedAccent(std::size_t index, Property accent,
                                      Property display, Property level,
                                      Property prime, bool force) {
    tree::TreeCallGuard<MmlNode> guard;
    const Children nodes = this->childArrayOwner();
    const NodeValue child = child_slot(nodes, index);
    const auto initialAttributes = this->attributes();
    if (!initialAttributes) host::typeError("Cannot read properties of null (reading 'hasExplicit')");
    if (initialAttributes->hasExplicit(property_key(accent)) ||
        !child.requireNode("isEmbellished")->isEmbellished()) return;

    MmlNode* const childNode = child.requireNode("coreMO");
    const NodeValue core = childNode->coreMO();
    MmlNode* const coreNode = core.requireNode("attributes");
    const auto coreAttributes = coreNode->attributes();
    if (!coreAttributes) host::typeError("Cannot read properties of null (reading 'get')");
    const Property value = coreAttributes->get("accent");

    auto currentAttributes = this->attributes();
    if (!currentAttributes) host::typeError("Cannot read properties of null (reading 'setInherited')");
    currentAttributes->setInherited(property_key(accent), value);

    currentAttributes = this->attributes();
    if (!currentAttributes) host::typeError("Cannot read properties of null (reading 'getDefault')");
    if (!(value == currentAttributes->getDefault(property_key(accent)))) {
        child.requireNode("setInheritedAttributes")->setInheritedAttributes(
            Property(PropertyList{}), display, getScriptlevel(accent, force, level), prime);
    }
}

std::string MmlMunder::kind() const { return "munder"; }
double MmlMunder::arity() const { return 2; }

std::string MmlMover::kind() const { return "mover"; }
double MmlMover::arity() const { return 2; }
std::size_t MmlMover::over() const { return 1; }
std::size_t MmlMover::under() const { return 2; }

}
