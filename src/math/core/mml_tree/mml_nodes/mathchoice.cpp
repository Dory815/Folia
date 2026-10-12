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

    double selection = 0;
    if (!host::truthy(display)) {
        const double clamped = host::mathMax(0.0, host::mathMin(host::number(level), 2.0));
        selection = clamped + 1.0;
    }

    // Keep the selected tagged slot even if factory callbacks mutate this node.
    NodeValue child = host::numericChild(this->childNodes, selection);
    if (!child.isNode()) {
        child = NodeValue(this->factory.create("mrow", tree::FactoryArgs{}));
    }

    // Source reads the raw parent only after selection/factory effects. The
    // replacement result is intentionally discarded; inheritance uses child.
    const NodeValue parent = this->getParent();
    parent.requireNode("replaceChild")->replaceChild(child, NodeValue(this));
    child.requireNode("setInheritedAttributes")
        ->setInheritedAttributes(attributes, display, level, prime);
}

}
