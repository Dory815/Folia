module folia.math.core.mml_tree.mml_nodes.maction;
import folia.math.core.mml_tree.node_defaults;
import folia.math.util.js_case;
import folia.math.util.js_numeric_input;
import folia.math.util.js_number;
import std;

namespace folia::math::mml {
namespace {
std::u16string sourceString(const AttributeValue &value) {
    if (const auto *text = std::get_if<std::u16string>(&value))
        return *text;
    if (const auto *number = std::get_if<double>(&value)) {
        const auto ascii = js_number::toString(*number);
        return {ascii.begin(), ascii.end()};
    }
    if (const auto *boolean = std::get_if<bool>(&value))
        return *boolean ? u"true" : u"false";
    return u"null";
}

double parseIntegerPrefix(const AttributeValue *value, double fallback) {
    return value ? js_input::parseInt(sourceString(*value)) : fallback;
}
} // namespace

MmlMaction::MmlMaction(MmlDocument &document) : AbstractMmlNode(document, NodeKind::maction) {}

AttributeSet MmlMaction::defaults() {
    return nodeDefaultAttributes(NodeKind::maction);
}

NodeKind MmlMaction::kind() const noexcept {
    return NodeKind::maction;
}

double MmlMaction::arity() const {
    return 1.0;
}

MmlNode *MmlMaction::setTeXclass(MmlNode *previous) {
    const auto *action = attributes().lookup(u"actiontype");
    const auto *actionText = action ? std::get_if<std::u16string>(action) : nullptr;
    if (actionText && *actionText == u"tooltip" && children().size() > 1) {
        (void)children()[1]->setTeXclass(nullptr);
    }
    auto &child = selected();
    auto *result = child.setTeXclass(previous);
    updateTeXclass(&child);
    return result;
}

void MmlMaction::nextToggleSelection() {
    const auto parsed = parseIntegerPrefix(attributes().lookup(u"selection"), 1.0);
    auto next =
        std::isnan(parsed) ? std::numeric_limits<double>::quiet_NaN() : std::max(1.0, parsed + 1.0);
    if (next > static_cast<double>(children().size()))
        next = 1.0;
    attributes().set(u"selection", next);
}

void MmlMaction::verifyAttributes(VerifyOptions const &options) {
    AbstractMmlNode::verifyAttributes(options);
    const auto *action = attributes().lookup(u"actiontype");
    const auto *value = action ? std::get_if<std::u16string>(action) : nullptr;
    if ((!value || *value != u"toggle") && attributes().own(u"selection")) {
        attributes().erase(u"selection");
    }
}

void MmlMaction::setChildInheritedAttributes(AttributeList const &inherited, bool display,
                                             double level, bool prime) {
    const auto *action = attributes().lookup(u"actiontype");
    const auto *value = action ? std::get_if<std::u16string>(action) : nullptr;
    if (!value)
        throw std::invalid_argument("maction actiontype must be a UTF-16 string");
    if (js_case::lowerCase(*value) != u"tooltip") {
        AbstractMmlNode::setChildInheritedAttributes(inherited, display, level, prime);
        return;
    }
    if (!children().empty())
        children()[0]->setInheritedAttributes(inherited, display, level, prime);
    if (children().size() > 1)
        children()[1]->setInheritedAttributes(inherited, false, 1.0, false);
}
} // namespace folia::math::mml
