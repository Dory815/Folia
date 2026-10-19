module folia.math.core.mml_tree.mml_nodes.mstyle;
import folia.math.core.mml_tree.node_defaults;
import folia.math.util.js_numeric_input;
import folia.math.util.js_number;
import std;

namespace folia::math::mml {
namespace {
std::u16string source_string(AttributeValue const &value) {
    if (auto const *text = std::get_if<std::u16string>(&value))
        return *text;
    if (auto const *number = std::get_if<double>(&value)) {
        const auto ascii = js_number::toString(*number);
        return {ascii.begin(), ascii.end()};
    }
    if (auto const *boolean = std::get_if<bool>(&value))
        return *boolean ? u"true" : u"false";
    return u"null";
}

double parse_script_level(AttributeValue const &value, double &level) {
    const auto text = source_string(value);
    std::size_t index = 0;
    while (index < text.size() && js_input::isWhitespace(text[index]))
        ++index;
    const bool relative = index < text.size() && (text[index] == u'+' || text[index] == u'-');
    const double parsed = js_input::parseInt(text);
    return relative ? level + parsed : parsed;
}

bool explicitly_null(AttributeValue const *value) {
    return !value || std::holds_alternative<AttributeNull>(*value);
}
} // namespace

MmlMstyle::MmlMstyle(MmlDocument &document) : AbstractMmlLayoutNode(document, NodeKind::mstyle) {}

AttributeSet MmlMstyle::defaults() {
    return nodeDefaultAttributes(NodeKind::mstyle);
}

NodeKind MmlMstyle::kind() const noexcept {
    return NodeKind::mstyle;
}

bool MmlMstyle::notParent() const {
    return !children().empty() && children()[0]->children().size() == 1;
}

void MmlMstyle::inheritAttributes(AttributeList const &attributes, bool display, double level,
                                  bool prime) {
    this->attributes().setInherited(u"displaystyle", display);
    this->attributes().setInherited(u"scriptlevel", level);
    AbstractMmlNode::inheritAttributes(attributes, display, level, prime);
}

void MmlMstyle::setChildInheritedAttributes(AttributeList const &attributes, bool display,
                                            double level, bool prime) {
    auto const *script_level = this->attributes().own(u"scriptlevel");
    if (!explicitly_null(script_level)) {
        level = parse_script_level(*script_level, level);
        prime = false;
    }

    auto const *display_style = this->attributes().own(u"displaystyle");
    if (!explicitly_null(display_style)) {
        const auto *value = std::get_if<bool>(display_style);
        display = value && *value;
        prime = false;
    }
    auto const *cramped = this->attributes().own(u"data-cramped");
    if (!explicitly_null(cramped)) {
        const auto *value = std::get_if<bool>(cramped);
        prime = value && *value;
    }

    AttributeAdditions additions;
    additions.reserve(this->attributes().ownEntries().size());
    for (auto const &[name, value] : this->attributes().ownEntries())
        additions.emplace_back(name, value);
    auto inherited = addInheritedAttributes(attributes, additions);
    if (children().empty())
        throw std::out_of_range("mstyle requires its inferred row");
    children()[0]->setInheritedAttributes(inherited, display, level, prime);
}
} // namespace folia::math::mml
