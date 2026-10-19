// Native port of MathJax 4.1.3 MmlNodes/mmultiscripts.ts; licensed under Apache-2.0.
module folia.math.core.mml_tree.mml_nodes.mmultiscripts;
import folia.math.core.mml_tree.mml_document;
import folia.math.core.mml_tree.node_defaults;
import std;

namespace folia::math::mml {
namespace {
bool truthy(AttributeValue const *value) {
    if (!value)
        return false;
    return std::visit(
        [](auto const &item) {
            using T = std::decay_t<decltype(item)>;
            if constexpr (std::is_same_v<T, AttributeNull>)
                return false;
            else if constexpr (std::is_same_v<T, bool>)
                return item;
            else if constexpr (std::is_same_v<T, double>)
                return item != 0 && !std::isnan(item);
            else
                return !item.empty();
        },
        *value);
}
} // namespace

MmlMmultiscripts::MmlMmultiscripts(MmlDocument &document)
    : MmlMsubsup(document, NodeKind::mmultiscripts) {}

AttributeSet MmlMmultiscripts::defaults() {
    return nodeDefaultAttributes(NodeKind::mmultiscripts);
}

NodeKind MmlMmultiscripts::kind() const noexcept {
    return NodeKind::mmultiscripts;
}

double MmlMmultiscripts::arity() const {
    return 1;
}

void MmlMmultiscripts::setChildInheritedAttributes(AttributeList const &attributes, bool display,
                                                   double level, bool prime) {
    auto nodes = children();
    nodes[0]->setInheritedAttributes(attributes, display, level, prime);

    bool prescripts = false;
    for (std::size_t i = 1, n = 0; i < children().size(); ++i) {
        MmlNode *const child = children()[i];
        if (child->isKind(NodeKind::mprescripts)) {
            if (!prescripts) {
                prescripts = true;
                if (i % 2 == 0) {
                    MmlNode &none = document().factory().make(NodeKind::none);
                    document().insert(*this, i, none);
                    ++i;
                }
            }
        } else {
            child->setInheritedAttributes(attributes, false, level + 1, prime || n % 2 == 0);
            ++n;
        }
    }

    nodes = children();
    if (nodes.size() % 2 == (prescripts ? 1u : 0u)) {
        MmlNode &none = document().factory().make(NodeKind::none);
        appendChild(none);
        const auto current = children();
        current.back()->setInheritedAttributes(attributes, false, level + 1, prime);
    }
}

void MmlMmultiscripts::verifyChildren(VerifyOptions const &options) {
    bool prescripts = false;
    const auto nodes = children();
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        MmlNode &child = *nodes[i];
        if (child.isKind(NodeKind::mprescripts)) {
            if (prescripts) {
                (void)child.mError(u"mprescripts can only appear once in mmultiscripts", options,
                                   true);
            } else {
                prescripts = true;
                if (i % 2 == 0 && !options.fixMmultiscripts) {
                    (void)mError(u"There must be an equal number of prescripts of each type",
                                 options);
                }
            }
        }
    }
    if (nodes.size() % 2 == (prescripts ? 1u : 0u) && !options.fixMmultiscripts) {
        (void)mError(u"There must be an equal number of scripts of each type", options);
    }
    AbstractMmlNode::verifyChildren(options);
}

MmlMprescripts::MmlMprescripts(MmlDocument &document)
    : AbstractMmlNode(document, NodeKind::mprescripts) {}

AttributeSet MmlMprescripts::defaults() {
    return nodeDefaultAttributes(NodeKind::mprescripts);
}

NodeKind MmlMprescripts::kind() const noexcept {
    return NodeKind::mprescripts;
}

double MmlMprescripts::arity() const {
    return 0;
}

void MmlMprescripts::verifyTree(std::optional<VerifyOptions> const &options) {
    AbstractMmlNode::verifyTree(options);
    MmlNode *const parent = Parent();
    if (options && parent && !parent->isKind(NodeKind::mmultiscripts)) {
        (void)mError(u"mprescripts must be a child of mmultiscripts", *options, true);
    }
}

MmlNone::MmlNone(MmlDocument &document) : AbstractMmlNode(document, NodeKind::none) {}

AttributeSet MmlNone::defaults() {
    return nodeDefaultAttributes(NodeKind::none);
}

NodeKind MmlNone::kind() const noexcept {
    return NodeKind::none;
}

double MmlNone::arity() const {
    return 0;
}

void MmlNone::verifyTree(std::optional<VerifyOptions> const &options) {
    AbstractMmlNode::verifyTree(options);
    MmlNode *const parent = Parent();
    if (options && parent && !parent->isKind(NodeKind::mmultiscripts)) {
        (void)mError(u"none must be a child of mmultiscripts", *options, true);
    }
}
} // namespace folia::math::mml
