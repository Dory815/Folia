module folia.math.core.mml_tree.mml_nodes.mo;
import folia.math.core.mml_tree.mml_nodes.mrow;
import folia.math.core.mml_tree.mml_nodes.munderover;
import folia.math.core.mml_tree.node_defaults;
import std;

namespace folia::math::mml {
namespace {
const std::vector<std::pair<char32_t, char32_t>> primeMappings{
    {U'"', 8243}, {U'\'', 8242}, {8216, 8245}, {8217, 8242}, {8218, 8242},
    {8219, 8245}, {8220, 8246},  {8221, 8243}, {8222, 8243}, {8223, 8246}};

std::u16string_view stringAttribute(AttributeSet const &attributes, std::u16string_view name) {
    const auto *value = attributes.lookup(name);
    if (const auto *text = value ? std::get_if<std::u16string>(value) : nullptr)
        return *text;
    return {};
}

bool truthy(AttributeValue const *value) {
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

std::size_t spacingIndex(TexClass value) {
    switch (value) {
    case TexClass::ord:
        return 0;
    case TexClass::op:
        return 1;
    case TexClass::bin:
        return 2;
    case TexClass::rel:
        return 3;
    case TexClass::open:
        return 4;
    case TexClass::close:
        return 5;
    case TexClass::punct:
        return 6;
    case TexClass::inner:
        return 7;
    case TexClass::none:
        return 0;
    }
    return 0;
}

} // namespace

AttributeSet MmlMo::defaults() {
    return nodeDefaultAttributes(NodeKind::mo);
}

std::span<operator_dict::OperatorEntry const> MmlMo::operatorTable() noexcept {
    return operator_dict::OPTABLE();
}

std::span<std::array<double, 2> const> MmlMo::mathSpacing() noexcept {
    return operator_dict::MMLSPACING();
}

std::vector<std::pair<char32_t, char32_t>> const &MmlMo::remapPrimes() {
    return primeMappings;
}

void MmlMo::initializeMml(AttributeSet attributes, std::span<MmlNode *const> children) {
    AbstractMmlTokenNode::initializeMml(std::move(attributes), children);
    // Source-derived class fields initialize after the superclass body.
    tex_class_mode_ = MoTexClassMode::cleared;
    state().working_tex_class.reset();
    lspace = 5.0 / 18.0;
    rspace = 5.0 / 18.0;
}

NodeKind MmlMo::kind() const noexcept {
    return NodeKind::mo;
}

bool MmlMo::isEmbellished() const {
    return true;
}

void MmlMo::setTexClass(std::optional<TexClass> value) {
    if (value) {
        tex_class_mode_ = MoTexClassMode::explicit_value;
        state().working_tex_class = *value;
    } else {
        tex_class_mode_ = MoTexClassMode::cleared;
        state().working_tex_class.reset();
    }
}

std::optional<TexClass> MmlMo::texClass() {
    if (tex_class_mode_ == MoTexClassMode::unset)
        return std::nullopt;
    if (tex_class_mode_ == MoTexClassMode::explicit_value)
        return state().working_tex_class;
    return getOperatorDef(getText()).texClass;
}

bool MmlMo::hasSpacingAttributes() {
    return attributes().isSet(u"lspace") || attributes().isSet(u"rspace");
}

std::vector<std::u16string> MmlMo::getForms() {
    MmlNode *physicalParent = parent();
    MmlNode *logicalParent = Parent();
    MmlNode *core = nullptr;
    while (logicalParent && logicalParent->isEmbellished()) {
        core = physicalParent;
        physicalParent = logicalParent->parent();
        logicalParent = logicalParent->Parent();
    }
    if (!core)
        core = this;

    if (physicalParent && physicalParent->isKind(NodeKind::mrow)) {
        auto *row = dynamic_cast<MmlMrow *>(physicalParent);
        if (!row)
            throw std::logic_error("mrow kind has incompatible family type");
        const std::size_t nonSpace = row->nonSpaceLength();
        MmlNode *const first = row->firstNonSpace();
        MmlNode *const last = row->lastNonSpace();
        if (nonSpace != 1) {
            if (first == core)
                return {u"prefix", u"infix", u"postfix"};
            if (last == core)
                return {u"postfix", u"infix", u"prefix"};
        }
    }
    return {u"infix", u"prefix", u"postfix"};
}

std::vector<std::u16string> MmlMo::handleExplicitForm(std::span<std::u16string_view const> forms) {
    const auto &attrs = attributes();
    if (!attrs.isSet(u"form"))
        return {forms.begin(), forms.end()};
    const std::u16string form(stringAttribute(attrs, u"form"));
    std::vector<std::u16string> result;
    result.reserve(forms.size() + 1);
    result.push_back(form);
    for (const auto candidate : forms)
        if (candidate != form)
            result.emplace_back(candidate);
    return result;
}

operator_dict::OperatorDef MmlMo::getOperatorDef(std::u16string_view text) {
    const auto forms = getForms();
    std::vector<std::u16string_view> candidates;
    candidates.reserve(4);
    for (const auto &form : forms)
        candidates.push_back(form);
    const auto ordered = handleExplicitForm(candidates);
    attributes().setInherited(u"form", ordered.front());

    for (const auto &form : ordered) {
        if (const auto *definition = operator_dict::findOperatorDefinition(form, text))
            return *definition;
    }

    properties().tex.no_dictionary_definition = true;
    const bool operatorName = matchMoPattern(opPattern(), text);
    const bool movable = truthy(attributes().lookup(u"movablelimits"));
    const bool operatorOrLimits = operatorName || movable;
    if (operatorOrLimits && !properties().tex.explicit_tex_class)
        return operator_dict::OPDEF(1.0, 2.0, TexClass::op);

    const auto range = operator_dict::getRange(text);
    const auto spacing = mathSpacing();
    const std::size_t index = spacingIndex(range.texClass);
    if (index >= spacing.size())
        throw std::logic_error("operator spacing class out of range");
    return operator_dict::OPDEF(spacing[index][0], spacing[index][1], range.texClass);
}

void MmlMo::checkOperatorTable(std::u16string_view text) {
    const auto definition = getOperatorDef(text);
    if (!properties().tex.explicit_tex_class)
        setTexClass(definition.texClass);
    for (const auto &[name, value] : definition.properties)
        attributes().setInherited(name, value);
    lspace = definition.left / 18.0;
    rspace = definition.right / 18.0;
}
} // namespace folia::math::mml
