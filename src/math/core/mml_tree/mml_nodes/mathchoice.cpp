/*************************************************************
 * Copyright (c) 2018-2026 The MathJax Consortium.
 * Native implementation of MathJax 4.1.3 MmlNodes/mathchoice.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 * Source SHA256: e1f9fdf92e345c7bc09e3333c4f436ae8f2d1e4c8fa9a2280877590cf9025d76
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.mathchoice;
import folia.math.core.mml_tree.mml_node_host;
import std;

namespace folia::math::mml {

void MathChoice::inheritAttributes(AttributeList attributes, Property display,
                                  Property level, Property prime) {
    tree::TreeCallGuard<MmlNode> guard;

    // MathJax's conditional tests display before ToNumber(level); do not
    // coerce level on the display branch.
    double selection = 0;
    if (!host::truthy(display)) {
        const double rawLevel = host::number(level);
        // Math.min/Math.max propagate NaN, unlike std::max(0, NaN).
        if (std::isnan(rawLevel)) {
            selection = std::numeric_limits<double>::quiet_NaN();
        } else {
            const double capped = std::min(rawLevel, 2.0);
            selection = std::max(0.0, capped) + 1.0;
        }
    }

    // JavaScript array indexing stringifies the numeric key. The MML child
    // handle is a typed array of node pointers, so only canonical in-range
    // integer indices can name a slot; fractional, NaN and infinite keys miss.
    const Children children = this->childNodes;
    if (!children) host::typeError("Cannot read MathChoice child slot from null array");
    MmlNode* child = nullptr;
    if (std::isfinite(selection) && selection >= 0.0 &&
        std::trunc(selection) == selection &&
        selection < static_cast<double>(children->size())) {
        child = (*children)[static_cast<std::size_t>(selection)];
    }
    if (!child) child = this->factory.create("mrow", {});

    // Read the current parent only after selection/fallback creation. The
    // replacement is intentionally committed before dispatching raw arguments
    // to the captured child, matching MathChoice's partial-state behavior.
    MmlNode* const currentParent = this->parent;
    if (!currentParent) host::typeError("Cannot replace detached MathChoice");
    currentParent->replaceChild(child, this);
    if (!child) host::typeError("Cannot inherit attributes on null MathChoice child");
    child->setInheritedAttributes(std::move(attributes), std::move(display),
                                  std::move(level), std::move(prime));
}

}
