/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native implementation of MathJax 4.1.3 MmlNodes/mspace.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.mspace;
import folia.math.core.mml_tree.node_defaults;
import folia.math.util.js_numeric_input;
import std;

namespace folia::math::mml {
namespace {
constexpr std::array<std::u16string_view, 5> nonSpacelike{u"height", u"depth", u"style",
                                                          u"mathbackground", u"background"};

bool anyExplicit(AttributeSet const &attributes, std::span<const std::u16string_view> names) {
    return std::any_of(names.begin(), names.end(),
                       [&](auto name) { return attributes.own(name) != nullptr; });
}

bool hasNegativePrefix(const AttributeValue *width) {
    if (!width)
        return false;
    if (const auto *text = std::get_if<std::u16string>(width)) {
        auto first = text->begin();
        while (first != text->end() && js_input::isWhitespace(*first))
            ++first;
        return first != text->end() && *first == u'-';
    }
    if (const auto *number = std::get_if<double>(width))
        return *number < 0;
    return false;
}
} // namespace

MmlMspace::MmlMspace(MmlDocument &document) : AbstractMmlTokenNode(document, NodeKind::mspace) {}

AttributeSet MmlMspace::defaults() {
    return nodeDefaultAttributes(NodeKind::mspace);
}

std::span<const std::u16string_view> MmlMspace::NONSPACELIKE() {
    return nonSpacelike;
}

NodeKind MmlMspace::kind() const noexcept {
    return NodeKind::mspace;
}

double MmlMspace::arity() const {
    return 0;
}

bool MmlMspace::isSpacelike() const {
    return !attributes().own(u"linebreak") && canBreak();
}

bool MmlMspace::hasNewline() const {
    if (!canBreak())
        return false;
    const auto *linebreak = attributes().lookup(u"linebreak");
    const auto *text = linebreak ? std::get_if<std::u16string>(linebreak) : nullptr;
    return text && (*text == u"newline" || *text == u"indentingnewline");
}

bool MmlMspace::canBreak() const {
    if (anyExplicit(attributes(), NONSPACELIKE()))
        return false;
    return !hasNegativePrefix(attributes().lookup(u"width"));
}

MmlNode *MmlMspace::setTeXclass(MmlNode *previous) {
    return previous;
}
} // namespace folia::math::mml
