module folia.math.core.mml_tree.mml_nodes.math;
import folia.math.core.mml_tree.node_defaults;
import std;

namespace folia::math::mml {
namespace {
bool truthy(const AttributeValue *value) {
    if (!value || std::holds_alternative<AttributeNull>(*value))
        return false;
    if (const auto *boolean = std::get_if<bool>(value))
        return *boolean;
    if (const auto *number = std::get_if<double>(value))
        return *number != 0.0 && !std::isnan(*number);
    if (const auto *text = std::get_if<std::u16string>(value))
        return !text->empty();
    return false;
}

double numericValue(const AttributeValue *value, double fallback) {
    if (!value)
        return fallback;
    if (const auto *number = std::get_if<double>(value))
        return *number;
    if (const auto *boolean = std::get_if<bool>(value))
        return *boolean ? 1.0 : 0.0;
    if (const auto *text = std::get_if<std::u16string>(value)) {
        std::string ascii;
        for (const auto character : *text) {
            if (character > 0x7f)
                return std::numeric_limits<double>::quiet_NaN();
            ascii.push_back(static_cast<char>(character));
        }
        const auto first = ascii.find_first_not_of(" \t\r\n");
        if (first == std::string::npos)
            return 0.0;
        const auto last = ascii.find_last_not_of(" \t\r\n");
        const auto begin = ascii.data() + first;
        const auto end = ascii.data() + last + 1;
        double result{};
        const auto parsed = std::from_chars(begin, end, result);
        if (parsed.ec == std::errc{} && parsed.ptr == end)
            return result;
        return std::numeric_limits<double>::quiet_NaN();
    }
    return 0.0;
}
} // namespace

MmlMath::MmlMath(MmlDocument &document) : AbstractMmlLayoutNode(document, NodeKind::math) {}

AttributeSet MmlMath::defaults() {
    return nodeDefaultAttributes(NodeKind::math);
}

NodeKind MmlMath::kind() const noexcept {
    return NodeKind::math;
}

bool MmlMath::linebreakContainer() const {
    return true;
}

std::string MmlMath::linebreakAlign() const {
    return {};
}

void MmlMath::setChildInheritedAttributes(AttributeList const &inherited, bool, double,
                                          bool prime) {
    AttributeAdditions additions;
    additions.reserve(attributes().ownEntries().size());
    for (const auto &entry : attributes().ownEntries())
        additions.push_back(entry);
    auto values = addInheritedAttributes(inherited, additions);
    const auto mode = attributes().lookup(u"mode");
    if (mode && std::get_if<std::u16string>(mode) &&
        *std::get_if<std::u16string>(mode) == u"display") {
        attributes().setInherited(u"display", std::u16string{u"block"});
    }

    const bool displayStyle = truthy(attributes().lookup(u"displaystyle"));
    const auto *displayMode = attributes().lookup(u"display");
    const auto *displayText = displayMode ? std::get_if<std::u16string>(displayMode) : nullptr;
    const bool childDisplay =
        displayStyle || (!displayStyle && displayText && *displayText == u"block");
    attributes().setInherited(u"displaystyle", childDisplay);

    const auto *scriptLevel = attributes().lookup(u"scriptlevel");
    const auto classDefaults = defaults();
    const double defaultLevel = numericValue(classDefaults.lookup(u"scriptlevel"), 0.0);
    const double childLevel =
        truthy(scriptLevel) ? numericValue(scriptLevel, defaultLevel) : defaultLevel;
    AbstractMmlNode::setChildInheritedAttributes(values, childDisplay, childLevel, prime);
}

void MmlMath::verifyTree(std::optional<VerifyOptions> const &options) {
    AbstractMmlNode::verifyTree(options);
    if (Parent()) {
        const VerifyOptions effective = options.value_or(verifyDefaults());
        (void)mError(u"Improper nesting of math tags", effective, true);
    }
}
} // namespace folia::math::mml
