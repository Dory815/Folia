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

Property accent_at(const std::shared_ptr<std::vector<Property>>& accents,
                   std::size_t index) {
    if (!accents) host::typeError("Cannot read ACCENTS from null");
    return index < accents->size() ? (*accents)[index] : Property(Missing{});
}

MmlNode* child_at_or_null(const Children& nodes, std::size_t index) {
    if (!nodes) host::typeError("Cannot read child from null script children");
    return index < nodes->size() ? (*nodes)[index] : nullptr;
}

MmlNode* require_child(const Children& nodes, std::size_t index,
                       std::string_view operation) {
    MmlNode* const child = child_at_or_null(nodes, index);
    if (!child) host::typeError(operation);
    return child;
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
    const Children nodes = childNodes;

    MmlNode* const baseNode = require_child(
        nodes, 0, "Cannot set inherited attributes on missing base child");
    // Preserve JavaScript's || short circuit: do not evaluate the virtual
    // `over` getter when prime is already truthy.
    Property basePrime = prime;
    if (!host::truthy(prime))
        basePrime = Property(child_at_or_null(nodes, over()) != nullptr);
    baseNode->setInheritedAttributes(attributes, display, level, std::move(basePrime));

    bool force = false;
    if (!host::truthy(display)) {
        MmlNode* const currentBase = require_child(
            nodes, 0, "Cannot read core mo from missing base child");
        MmlNode* const core = currentBase->coreMO();
        if (!core) host::typeError("Cannot read attributes from missing base core mo");
        const auto coreAttributes = core->attributes();
        if (!coreAttributes) host::typeError("Cannot read movablelimits from null core attributes");
        force = host::truthy(coreAttributes->get("movablelimits"));
    }

    // This is read after the base setter and force calculation. The captured
    // shared vector survives replacement of the constructor's static binding.
    const auto accents = constructorAccents();

    MmlNode* const underNode = require_child(
        nodes, 1, "Cannot set inherited attributes on missing under child");
    const Property underAccent = accent_at(accents, 1);
    const Property underLevel = getScriptlevel(underAccent, force, level);
    const Property underPrime = truthy_or_under(prime, *this, 1);
    underNode->setInheritedAttributes(attributes, Property(false), underLevel, underPrime);

    const Property underAccentForInheritance = accent_at(accents, 1);
    setInheritedAccent(1, underAccentForInheritance, display, level, prime, force);

    if (!child_at_or_null(nodes, 2)) return;
    MmlNode* const overNode = require_child(
        nodes, 2, "Cannot set inherited attributes on missing over child");
    const Property overAccent = accent_at(accents, 2);
    const Property overLevel = getScriptlevel(overAccent, force, level);
    const Property overPrime = truthy_or_under(prime, *this, 2);
    overNode->setInheritedAttributes(attributes, Property(false), overLevel, overPrime);

    const Property overAccentForInheritance = accent_at(accents, 2);
    setInheritedAccent(2, overAccentForInheritance, display, level, prime, force);
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
    const Children currentChildren = childNodes;
    MmlNode* const node = child_at_or_null(currentChildren, index);

    auto currentAttributes = attributes();
    if (!currentAttributes) host::typeError("Cannot read explicit accent from null attributes");
    if (currentAttributes->hasExplicit(property_key(accent))) return;
    if (!node) host::typeError("Cannot read embellishment from missing accent child");
    if (!node->isEmbellished()) return;

    MmlNode* const core = node->coreMO();
    if (!core) host::typeError("Cannot read accent from missing core mo");
    const auto coreAttributes = core->attributes();
    if (!coreAttributes) host::typeError("Cannot read accent from null core attributes");
    const Property value = coreAttributes->get("accent");

    currentAttributes = attributes();
    if (!currentAttributes) host::typeError("Cannot set inherited accent on null attributes");
    currentAttributes->setInherited(property_key(accent), value);

    currentAttributes = attributes();
    if (!currentAttributes) host::typeError("Cannot read default accent from null attributes");
    const Property defaultValue = currentAttributes->getDefault(property_key(accent));
    if (value == defaultValue) return;

    // The source supplies a new {} on each re-inheritance. Keep its identity
    // distinct and preserve the original display/prime values.
    AttributeList emptyAttributes;
    const Property reinheritedLevel = getScriptlevel(accent, force, level);
    node->setInheritedAttributes(std::move(emptyAttributes), std::move(display),
                                 reinheritedLevel, std::move(prime));
}

std::string MmlMunder::kind() const { return "munder"; }
double MmlMunder::arity() const { return 2; }

std::string MmlMover::kind() const { return "mover"; }
double MmlMover::arity() const { return 2; }
std::size_t MmlMover::over() const { return 1; }
std::size_t MmlMover::under() const { return 2; }

}
