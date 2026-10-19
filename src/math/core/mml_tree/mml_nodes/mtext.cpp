/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native implementation of MathJax 4.1.3 MmlNodes/mtext.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.mtext;
import folia.math.core.mml_tree.node_defaults;
import folia.math.util.js_numeric_input;
import std;

namespace folia::math::mml {
namespace {
constexpr std::array<std::u16string_view, 3> nonSpacelike{u"style", u"mathbackground",
                                                          u"background"};

bool anyExplicit(AttributeSet const &attributes, std::span<const std::u16string_view> names) {
    return std::any_of(names.begin(), names.end(),
                       [&](auto name) { return attributes.own(name) != nullptr; });
}
} // namespace

MmlMtext::MmlMtext(MmlDocument &document) : AbstractMmlTokenNode(document, NodeKind::mtext) {}

AttributeSet MmlMtext::defaults() {
    return nodeDefaultAttributes(NodeKind::mtext);
}

std::span<const std::u16string_view> MmlMtext::NONSPACELIKE() {
    return nonSpacelike;
}

NodeKind MmlMtext::kind() const noexcept {
    return NodeKind::mtext;
}

bool MmlMtext::isSpacelike() const {
    const auto value = getText();
    if (!std::all_of(value.begin(), value.end(), js_input::isWhitespace))
        return false;
    return !anyExplicit(attributes(), NONSPACELIKE());
}
} // namespace folia::math::mml
