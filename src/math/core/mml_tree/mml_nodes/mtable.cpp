/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native implementation of MmlMtable semantic methods from MathJax 4.1.3.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 * Source SHA256: 8c701c9e8e55565c7ea905bd2d0e23e8a433547c5ed11bfb868f74e3089a3bda
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.mtable;
import folia.math.core.mml_tree.table_host;
import std;

namespace folia::math::mml {

void MmlMtable::inheritAttributes(AttributeList attributes, Property display,
                                  Property level, Property prime) {
    tree::TreeCallGuard<MmlNode> guard;

    for (std::size_t index = 0; index < indentAttributes.size(); ++index) {
        // for-of captures the string value for this iteration before callbacks;
        // the shared vector itself remains live and may grow during the loop.
        const std::string name = indentAttributes[index];
        if (host::truthy(host::get(attributes, name))) {
            // JavaScript captures the method receiver before evaluating args.
            const auto receiver = host::arithmeticAttributes(*this);
            const Property tuple = host::get(attributes, name);
            const Property value = host::get(tuple, "1");
            receiver->setInherited(name, value);
        }
        if (host::arithmeticAttributes(*this)->hasExplicit(name)) {
            host::arithmeticAttributes(*this)->unset(name);
        }
    }

    AbstractMmlNode::inheritAttributes(
        std::move(attributes), std::move(display), std::move(level), std::move(prime));
}

void MmlMtable::setChildInheritedAttributes(AttributeList attributes, Property display,
                                             Property level, Property /*prime*/) {
    // P23f Root stub: MmlMtable::setChildInheritedAttributes.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

void MmlMtable::verifyChildren(PropertyList options) {
    // P23f Root stub: MmlMtable::verifyChildren.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

NodeValue MmlMtable::setTeXclass(NodeValue previous) {
    // P23f Root stub: MmlMtable::setTeXclass.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

} // namespace folia::math::mml
