/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native implementation of MathJax 4.1.3 MmlNodes/mspace.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.mspace;
import folia.math.core.mml_tree.leaf_host;
import std;

namespace folia::math::mml {
void MmlMspace::initializeClassFields() {
    initializeTexClass(static_cast<double>(TEXCLASS.NONE));
}

std::string MmlMspace::kind() const {
    return "mspace";
}

double MmlMspace::arity() const {
    return 0;
}

bool MmlMspace::isSpacelike() const {
    tree::TreeCallGuard<MmlNode> guard;
    const auto nodeAttributes = attributes();
    if (!nodeAttributes) host::typeError("Cannot read hasExplicit on null MML attributes");
    return !nodeAttributes->hasExplicit("linebreak") && canBreak();
}

bool MmlMspace::hasNewline() const {
    tree::TreeCallGuard<MmlNode> guard;
    const auto nodeAttributes = attributes();
    if (!nodeAttributes) host::typeError("Cannot read linebreak on null MML attributes");
    const Property linebreak = nodeAttributes->get("linebreak");
    if (!canBreak()) return false;
    return linebreak == Property(std::string("newline")) ||
           linebreak == Property(std::string("indentingnewline"));
}

bool MmlMspace::canBreak() const {
    tree::TreeCallGuard<MmlNode> guard;
    const auto presenceAttributes = attributes();
    if (!presenceAttributes) host::typeError("Cannot read hasOneOf on null MML attributes");
    if (presenceAttributes->hasOneOf(NONSPACELIKE())) return false;
    const auto widthAttributes = attributes();
    if (!widthAttributes) host::typeError("Cannot read width on null MML attributes");
    const std::u16string width = host::string(widthAttributes->get("width"));
    const std::u16string_view trimmed = host::trim(width);
    return trimmed.empty() || trimmed.front() != u'-';
}

MmlNode* MmlMspace::setTeXclass(MmlNode* previous) {
    return previous;
}
}
