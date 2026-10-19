module folia.math.core.mml_tree.mml_nodes.mtr;
import folia.math.core.mml_tree.mml_document;
import folia.math.core.mml_tree.node_defaults;
import folia.math.util.string;
import std;

namespace folia::math::mml {
namespace {
std::u16string kindText(NodeKind kind) {
    std::u16string result;
    for (const char c : nodeKindName(kind))
        result.push_back(static_cast<char16_t>(c));
    return result;
}

std::vector<AttributeValue> splitValues(const AttributeValue *value) {
    if (!value)
        return {};
    const auto *text = std::get_if<std::u16string>(value);
    if (!text)
        throw std::invalid_argument("table alignment attribute must be a string");
    std::vector<AttributeValue> result;
    for (auto &item : string_util::split(*text))
        result.emplace_back(std::move(item));
    return result;
}

const AttributeValue *findValue(const AttributeList &values, std::u16string_view key) {
    for (auto it = values.rbegin(); it != values.rend(); ++it)
        if (it->name == key)
            return &it->value;
    return nullptr;
}

void setValue(AttributeList &values, std::u16string name, AttributeValue value, NodeKind origin) {
    for (auto &item : values)
        if (item.name == name) {
            item.value = std::move(value);
            item.origin = origin;
            return;
        }
    values.push_back({std::move(name), origin, std::move(value)});
}

AttributeValue shifted(std::vector<AttributeValue> &values, const AttributeValue *fallback) {
    if (!values.empty()) {
        auto value = std::move(values.front());
        values.erase(values.begin());
        if (!std::holds_alternative<AttributeNull>(value) &&
            !(std::holds_alternative<bool>(value) && !std::get<bool>(value)) &&
            !(std::holds_alternative<double>(value) &&
              (std::get<double>(value) == 0.0 || std::isnan(std::get<double>(value)))) &&
            !(std::holds_alternative<std::u16string>(value) &&
              std::get<std::u16string>(value).empty()))
            return value;
    }
    return fallback ? *fallback : AttributeValue{AttributeNull{}};
}
} // namespace

AttributeSet MmlMtr::defaults() {
    return nodeDefaultAttributes(NodeKind::mtr);
}

NodeKind MmlMtr::kind() const noexcept {
    return NodeKind::mtr;
}

bool MmlMtr::linebreakContainer() const {
    return true;
}

std::string MmlMtr::linebreakAlign() const {
    return {};
}

NodeKind MmlMlabeledtr::kind() const noexcept {
    return NodeKind::mlabeledtr;
}

double MmlMlabeledtr::arity() const {
    return 1.0;
}

void MmlMtr::setChildInheritedAttributes(AttributeList const &incoming, bool display, double level,
                                         bool prime) {
    auto &factory = document().factory();
    for (std::size_t i = 0; i < children().size(); ++i) {
        auto *child = children()[i];
        if (!child->isKind(NodeKind::mtd)) {
            auto &cell = factory.make(NodeKind::mtd);
            replaceChild(cell, *child);
            cell.appendChild(*child);
        }
    }
    auto columns = splitValues(attributes().lookup(u"columnalign"));
    auto breakAlignments = splitValues(attributes().lookup(u"data-break-align"));
    if (arity() == 1.0) {
        AttributeValue labelSide{AttributeNull{}};
        if (auto *parentNode = parent()) {
            if (const auto *side = parentNode->attributes().lookup(u"side"))
                labelSide = *side;
        }
        columns.insert(columns.begin(), std::move(labelSide));
        breakAlignments.insert(breakAlignments.begin(), std::u16string{u"top"});
    }
    AttributeAdditions additions;
    if (const auto *row = attributes().lookup(u"rowalign"))
        additions.emplace_back(u"rowalign", *row);
    additions.emplace_back(u"columnalign", std::u16string{u"center"});
    additions.emplace_back(u"data-break-align", std::u16string{u"top"});
    auto propagated = addInheritedAttributes(incoming, additions);
    for (auto *cell : children()) {
        const auto *previousColumn = findValue(propagated, u"columnalign");
        const auto column = shifted(columns, previousColumn);
        setValue(propagated, u"columnalign", column, kind());
        const auto *previousBreak = findValue(propagated, u"data-break-align");
        const auto breakValue = shifted(breakAlignments, previousBreak);
        // InheritedAttribute carries {origin, scalar value}; this is the typed
        // equivalent of Source's [rowKind, breakalign] inheritance pair.
        setValue(propagated, u"data-vertical-align", breakValue, kind());
        cell->setInheritedAttributes(propagated, display, level, prime);
    }
}

void MmlMtr::verifyChildren(VerifyOptions const &options) {
    if (auto *parentNode = parent(); parentNode && !parentNode->isKind(NodeKind::mtable)) {
        auto message = kindText(kind());
        message += u" can only be a child of an mtable";
        (void)mError(std::move(message), options, true);
        return;
    }
    auto &factory = document().factory();
    for (std::size_t i = 0; i < children().size(); ++i) {
        auto *child = children()[i];
        if (child->isKind(NodeKind::mtd))
            continue;
        auto &cell = factory.make(NodeKind::mtd);
        replaceChild(cell, *child);
        cell.appendChild(*child);
        if (!options.fixMtables) {
            auto message = std::u16string{u"Children of "} + kindText(kind()) + u" must be mtd";
            (void)child->mError(std::move(message), options);
        }
    }
    AbstractMmlNode::verifyChildren(options);
}

MmlNode *MmlMtr::setTeXclass(MmlNode *previous) {
    getPrevClass(previous);
    for (auto *child : children())
        child->setTeXclass(nullptr);
    return this;
}
} // namespace folia::math::mml
