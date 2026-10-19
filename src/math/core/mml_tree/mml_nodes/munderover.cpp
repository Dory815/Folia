// Native port of MathJax 4.1.3 MmlNodes/munderover.ts; licensed under Apache-2.0.
module folia.math.core.mml_tree.mml_nodes.munderover;
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

MmlMunderover::MmlMunderover(MmlDocument &document)
    : MmlMunderover(document, NodeKind::munderover) {}

MmlMunderover::MmlMunderover(MmlDocument &document, NodeKind kind)
    : AbstractMmlBaseNode(document, kind) {}

AttributeSet MmlMunderover::defaults() {
    return nodeDefaultAttributes(NodeKind::munderover);
}

NodeKind MmlMunderover::kind() const noexcept {
    return NodeKind::munderover;
}

double MmlMunderover::arity() const {
    return 3;
}

std::size_t MmlMunderover::base() const {
    return 0;
}

std::size_t MmlMunderover::under() const {
    return 1;
}

std::size_t MmlMunderover::over() const {
    return 2;
}

bool MmlMunderover::linebreakContainer() const {
    return true;
}

std::u16string_view MmlMunderover::accentAttribute(std::size_t index) const noexcept {
    constexpr std::array<std::u16string_view, 3> accents{u"", u"accentunder", u"accent"};
    return index < accents.size() ? accents[index] : std::u16string_view{};
}

void MmlMunderover::setChildInheritedAttributes(AttributeList const &attributes, bool display,
                                                double level, bool prime) {
    const auto nodes = children();
    if (nodes.size() < 2)
        throw std::logic_error("munderover inheritance requires base and script children");
    MmlNode *const baseNode = nodes[0];
    baseNode->setInheritedAttributes(attributes, display, level,
                                     prime || (over() < nodes.size() && nodes[over()] != nullptr));

    bool force = false;
    if (!display) {
        MmlNode *const core = baseNode->coreMO();
        if (!core)
            throw std::logic_error("munderover base has no core operator");
        force = truthy(core->attributes().lookup(u"movablelimits"));
    }

    // Source processes physical positions 1 and 2. Mover's under()/over()
    // indices describe their roles, while its accent table maps these slots.
    MmlNode *const underNode = nodes[1];
    const auto underAccent = accentAttribute(1);
    underNode->setInheritedAttributes(attributes, false, getScriptlevel(underAccent, force, level),
                                      prime || under() == 1);
    setInheritedAccent(1, underAccent, display, level, prime, force);

    if (nodes.size() < 3 || !nodes[2])
        return;
    MmlNode *const overNode = nodes[2];
    const auto overAccent = accentAttribute(2);
    overNode->setInheritedAttributes(attributes, false, getScriptlevel(overAccent, force, level),
                                     prime || under() == 2);
    setInheritedAccent(2, overAccent, display, level, prime, force);
}

double MmlMunderover::getScriptlevel(std::u16string_view accent, bool force, double level) const {
    if (force || !truthy(attributes().lookup(accent)))
        return level + 1;
    return level;
}

void MmlMunderover::setInheritedAccent(std::size_t index, std::u16string_view accent, bool display,
                                       double level, bool prime, bool force) {
    const auto nodes = children();
    if (index >= nodes.size() || !nodes[index] || attributes().own(accent) ||
        !nodes[index]->isEmbellished())
        return;
    MmlNode *const core = nodes[index]->coreMO();
    if (!core)
        throw std::logic_error("embellished script has no core operator");
    const auto *value = core->attributes().lookup(u"accent");
    const AttributeValue inherited = value ? *value : AttributeValue{AttributeNull{}};
    attributes().setInherited(std::u16string(accent), inherited);
    const auto *defaultValue = attributes().effectiveDefault(accent);
    if (!defaultValue || inherited != *defaultValue) {
        nodes[index]->setInheritedAttributes(AttributeList{}, display,
                                             getScriptlevel(accent, force, level), prime);
    }
}

MmlMunder::MmlMunder(MmlDocument &document) : MmlMunderover(document, NodeKind::munder) {}

AttributeSet MmlMunder::defaults() {
    return nodeDefaultAttributes(NodeKind::munder);
}

NodeKind MmlMunder::kind() const noexcept {
    return NodeKind::munder;
}

double MmlMunder::arity() const {
    return 2;
}

MmlMover::MmlMover(MmlDocument &document) : MmlMunderover(document, NodeKind::mover) {}

AttributeSet MmlMover::defaults() {
    return nodeDefaultAttributes(NodeKind::mover);
}

NodeKind MmlMover::kind() const noexcept {
    return NodeKind::mover;
}

double MmlMover::arity() const {
    return 2;
}

std::size_t MmlMover::over() const {
    return 1;
}

std::size_t MmlMover::under() const {
    return 2;
}

std::u16string_view MmlMover::accentAttribute(std::size_t index) const noexcept {
    constexpr std::array<std::u16string_view, 3> accents{u"", u"accent", u"accentunder"};
    return index < accents.size() ? accents[index] : std::u16string_view{};
}
} // namespace folia::math::mml
