/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native implementation of MathJax 4.1.3 MmlNodes/math.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.math;
import folia.math.core.mml_tree.leaf_host;
import std;

namespace folia::math::mml {
std::string MmlMath::kind() const { return "math"; }
bool MmlMath::linebreakContainer() const { return true; }
std::string MmlMath::linebreakAlign() const { return {}; }

void MmlMath::setChildInheritedAttributes(AttributeList inherited, Property display,
                                         Property level, Property prime) {
    tree::TreeCallGuard<MmlNode> guard;
    auto attrs = attributes();
    if (!attrs) host::typeError("Cannot read math mode from null attributes");
    if (attrs->get("mode") == Property("display")) {
        attrs = attributes();
        if (!attrs) host::typeError("Cannot set display mode on null attributes");
        attrs->setInherited("display", "block");
    }

    attrs = attributes();
    if (!attrs) host::typeError("Cannot augment inherited attributes from null attributes");
    inherited = addInheritedAttributes(std::move(inherited), attrs->getAllAttributes());

    attrs = attributes();
    if (!attrs) host::typeError("Cannot read displaystyle from null attributes");
    const Property displayStyleForTruth = attrs->get("displaystyle");
    bool childDisplay = host::truthy(displayStyleForTruth);
    if (!childDisplay) {
        attrs = attributes();
        if (!attrs) host::typeError("Cannot re-read displaystyle from null attributes");
        const Property displayStyleForNegation = attrs->get("displaystyle");
        if (!host::truthy(displayStyleForNegation)) {
            attrs = attributes();
            if (!attrs) host::typeError("Cannot read display from null attributes");
            const Property displayMode = attrs->get("display");
            childDisplay = displayMode == Property("block");
        }
    }

    attrs = attributes();
    if (!attrs) host::typeError("Cannot set inherited displaystyle on null attributes");
    attrs->setInherited("displaystyle", childDisplay);

    attrs = attributes();
    if (!attrs) host::typeError("Cannot read scriptlevel from null attributes");
    level = attrs->get("scriptlevel");
    if (!host::truthy(level)) level = constructorDefaults().get("scriptlevel");
    AbstractMmlNode::setChildInheritedAttributes(std::move(inherited), childDisplay,
                                                 std::move(level), std::move(prime));
}

void MmlMath::verifyTree(Property options) {
    tree::TreeCallGuard<MmlNode> guard;
    if (options.missing()) options = PropertyNull{};
    AbstractMmlNode::verifyTree(options);
    if (getParent().isNode()) {
        (void)mError("Improper nesting of math tags", options, true);
    }
}
} // namespace folia::math::mml
