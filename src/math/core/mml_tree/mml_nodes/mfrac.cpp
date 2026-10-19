module folia.math.core.mml_tree.mml_nodes.mfrac;
import folia.math.core.mml_tree.node_defaults;
import std;

namespace folia::math::mml {
MmlMfrac::MmlMfrac(MmlDocument &document) : AbstractMmlBaseNode(document, NodeKind::mfrac) {}

AttributeSet MmlMfrac::defaults() {
    return nodeDefaultAttributes(NodeKind::mfrac);
}

NodeKind MmlMfrac::kind() const noexcept {
    return NodeKind::mfrac;
}

double MmlMfrac::arity() const {
    return 2;
}

bool MmlMfrac::linebreakContainer() const {
    return true;
}

std::string MmlMfrac::linebreakAlign() const {
    return {};
}

MmlNode *MmlMfrac::setTeXclass(MmlNode *previous) {
    getPrevClass(previous);
    for (auto *child : children())
        child->setTeXclass(nullptr);
    return this;
}

void MmlMfrac::setChildInheritedAttributes(AttributeList const &attributes, bool display,
                                           double level, bool prime) {
    if (!display || level > 0)
        ++level;
    auto const *numerator = this->attributes().lookup(u"numalign");
    auto const *denominator = this->attributes().lookup(u"denomalign");
    if (!numerator || !denominator)
        throw std::logic_error("mfrac alignment defaults were not initialized");
    const auto numerator_attributes =
        addInheritedAttributes(attributes, {{u"numalign", *numerator},
                                            {u"indentshift", std::u16string(u"0")},
                                            {u"indentalignfirst", *numerator},
                                            {u"indentshiftfirst", std::u16string(u"0")},
                                            {u"indentalignlast", std::u16string(u"indentalign")},
                                            {u"indentshiftlast", std::u16string(u"indentshift")}});
    const auto denominator_attributes =
        addInheritedAttributes(attributes, {{u"denalign", *denominator},
                                            {u"indentshift", std::u16string(u"0")},
                                            {u"indentalignfirst", *denominator},
                                            {u"indentshiftfirst", std::u16string(u"0")},
                                            {u"indentalignlast", std::u16string(u"indentalign")},
                                            {u"indentshiftlast", std::u16string(u"indentshift")}});
    if (children().size() < 2)
        throw std::out_of_range("mfrac requires two children");
    children()[0]->setInheritedAttributes(numerator_attributes, false, level, prime);
    children()[1]->setInheritedAttributes(denominator_attributes, false, level, true);
}
} // namespace folia::math::mml
