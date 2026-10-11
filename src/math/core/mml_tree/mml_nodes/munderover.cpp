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

NodeValue child_slot(const Children& nodes, std::size_t index) {
    // P23f Root stub: child_slot.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

MmlNode* require_child(const Children& nodes, std::size_t index,
                       std::string_view operation) {
    // P23f Root stub: require_child.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
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
    // P23f Root stub: MmlMunderover::setChildInheritedAttributes.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
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
    // P23f Root stub: MmlMunderover::setInheritedAccent.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

std::string MmlMunder::kind() const { return "munder"; }
double MmlMunder::arity() const { return 2; }

std::string MmlMover::kind() const { return "mover"; }
double MmlMover::arity() const { return 2; }
std::size_t MmlMover::over() const { return 1; }
std::size_t MmlMover::under() const { return 2; }

}
