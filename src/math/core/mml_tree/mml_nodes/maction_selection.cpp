/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native selected-child projections for MathJax 4.1.3 maction.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 * TS SHA256: fa173b7e2b25b90e0f83ac201098357a8abf28462fa86fa69d92b65906527895
 * CJS SHA256: eddcdd2111b4b7cf5ad72937b0a4ff7eb555b98308c6771a013a18121af8ce7
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.maction;
import folia.math.core.mml_tree.action_host;
import std;

namespace folia::math::mml {

MmlNode* MmlMaction::selected() const {
    const auto attributes = host::arithmeticAttributes(*this);
    const Property selection = attributes->get("selection");

    // The source reads childNodes.length before Math.min coerces selection.
    const Children lengthSource = this->childNodes;
    if (!lengthSource) host::typeError("Cannot read length of null maction children");
    const double childLength = static_cast<double>(lengthSource->size());
    const double selectedNumber = host::number(selection);

    // Math.min/Math.max propagate NaN. Keep fractional values fractional so
    // they miss the typed node-array's integer slots instead of being rounded.
    double minimum = std::numeric_limits<double>::quiet_NaN();
    if (!std::isnan(selectedNumber)) minimum = std::min(childLength, selectedNumber);
    double index = std::numeric_limits<double>::quiet_NaN();
    if (!std::isnan(minimum)) index = std::max(1.0, minimum) - 1.0;

    // The source rereads this.childNodes after selection coercion; do not use
    // the array handle whose length was read above.
    const Children currentChildren = this->childNodes;
    if (!currentChildren) host::typeError("Cannot index null maction children");
    MmlNode* child = nullptr;
    if (std::isfinite(index) && index >= 0.0 && std::trunc(index) == index &&
        index < static_cast<double>(currentChildren->size())) {
        child = (*currentChildren)[static_cast<std::size_t>(index)];
    }
    if (!child) child = this->factory.create("mrow", {});
    return child;
}

bool MmlMaction::isEmbellished() const {
    tree::TreeCallGuard<MmlNode> guard;
    MmlNode* const child = selected();
    if (!child) host::typeError("Cannot read isEmbellished from null maction selection");
    return child->isEmbellished();
}

bool MmlMaction::isSpacelike() const {
    tree::TreeCallGuard<MmlNode> guard;
    MmlNode* const child = selected();
    if (!child) host::typeError("Cannot read isSpacelike from null maction selection");
    return child->isSpacelike();
}

MmlNode* MmlMaction::core() {
    tree::TreeCallGuard<MmlNode> guard;
    MmlNode* const child = selected();
    if (!child) host::typeError("Cannot read core from null maction selection");
    return child->core();
}

MmlNode* MmlMaction::coreMO() {
    tree::TreeCallGuard<MmlNode> guard;
    MmlNode* const child = selected();
    if (!child) host::typeError("Cannot read coreMO from null maction selection");
    return child->coreMO();
}

}
