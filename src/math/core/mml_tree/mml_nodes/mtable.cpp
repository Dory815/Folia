module folia.math.core.mml_tree.mml_nodes.mtable;
import folia.math.core.mml_tree.mml_document;
import folia.math.core.mml_tree.node_defaults;
import folia.math.util.string;
import std;

namespace folia::math::mml {
namespace {
std::u16string widen(std::string_view value) {
    std::u16string result;
    result.reserve(value.size());
    for (unsigned char c : value)
        result.push_back(static_cast<char16_t>(c));
    return result;
}

bool truthy(const AttributeValue &value) {
    if (std::holds_alternative<AttributeNull>(value))
        return false;
    if (const auto *b = std::get_if<bool>(&value))
        return *b;
    if (const auto *n = std::get_if<double>(&value))
        return *n != 0.0 && !std::isnan(*n);
    return !std::get<std::u16string>(value).empty();
}

const InheritedAttribute *findEntry(const AttributeList &values, std::u16string_view name) {
    for (auto it = values.rbegin(); it != values.rend(); ++it)
        if (it->name == name)
            return &*it;
    return nullptr;
}

void setValue(AttributeList &values, std::u16string name, AttributeValue value, NodeKind origin) {
    for (auto &entry : values)
        if (entry.name == name) {
            entry.value = std::move(value);
            entry.origin = origin;
            return;
        }
    values.push_back({std::move(name), origin, std::move(value)});
}

std::u16string stringValue(const AttributeValue *value, std::u16string_view key) {
    if (!value)
        return {};
    if (const auto *text = std::get_if<std::u16string>(value))
        return *text;
    (void)key;
    throw std::invalid_argument("mtable alignment attribute must be a UTF-16 string");
}
} // namespace

AttributeSet MmlMtable::defaults() {
    return nodeDefaultAttributes(NodeKind::mtable);
}

NodeKind MmlMtable::kind() const noexcept {
    return NodeKind::mtable;
}

bool MmlMtable::linebreakContainer() const {
    return true;
}

std::string MmlMtable::linebreakAlign() const {
    return "";
}

void MmlMtable::inheritAttributes(AttributeList const &incoming, bool display, double level,
                                  bool prime) {
    for (const auto name : indentAttributes) {
        const auto key = widen(name);
        if (const auto *entry = findEntry(incoming, key))
            attributes().setInherited(key, entry->value);
        attributes().erase(key);
    }
    AbstractMmlNode::inheritAttributes(incoming, display, level, prime);
}

void MmlMtable::setChildInheritedAttributes(AttributeList const &incoming, bool display,
                                            double level, bool prime) {
    auto &factory = document().factory();
    for (std::size_t i = 0; i < children().size(); ++i) {
        MmlNode *child = children()[i];
        if (!child->isKind(NodeKind::mtr) && !child->isKind(NodeKind::mlabeledtr)) {
            auto &row = factory.make(NodeKind::mtr);
            replaceChild(row, *child);
            row.appendChild(*child);
        }
    }

    if (const auto *explicitDisplay = attributes().own(u"displaystyle");
        explicitDisplay && truthy(*explicitDisplay))
        display = truthy(*explicitDisplay);
    else if (const auto *defaultDisplay = attributes().effectiveDefault(u"displaystyle"))
        display = truthy(*defaultDisplay);

    AttributeAdditions additions;
    if (const auto *v = attributes().lookup(u"columnalign"))
        additions.emplace_back(u"columnalign", *v);
    additions.emplace_back(u"rowalign", std::u16string{u"center"});
    if (const auto *v = attributes().lookup(u"data-break-align"))
        additions.emplace_back(u"data-break-align", *v);
    auto propagated = addInheritedAttributes(incoming, additions);
    const bool cramped =
        attributes().own(u"data-cramped") && truthy(*attributes().own(u"data-cramped"));
    auto alignments =
        string_util::split(stringValue(attributes().lookup(u"rowalign"), u"rowalign"));

    for (auto *row : children()) {
        const auto *previousEntry = findEntry(propagated, u"rowalign");
        const auto *previous = previousEntry ? &previousEntry->value : nullptr;
        AttributeValue rowAlign = alignments.empty()
                                      ? (previous ? *previous : AttributeValue{AttributeNull{}})
                                      : AttributeValue{std::move(alignments.front())};
        if (!alignments.empty())
            alignments.erase(alignments.begin());
        if (!truthy(rowAlign) && previous)
            rowAlign = *previous;
        setValue(propagated, u"rowalign", std::move(rowAlign), kind());
        row->setInheritedAttributes(propagated, display, level, cramped);
    }
}

void MmlMtable::verifyChildren(VerifyOptions const &options) {
    auto &factory = document().factory();
    MmlNode *currentRow = nullptr;
    std::size_t i = 0;
    while (i < children().size()) {
        MmlNode *child = children()[i];
        if (child->isKind(NodeKind::mtr) || child->isKind(NodeKind::mlabeledtr)) {
            currentRow = nullptr;
            ++i;
            continue;
        }
        const bool isCell = child->isKind(NodeKind::mtd);
        if (currentRow) {
            removeChild(*child);
            --i;
        } else {
            currentRow = &factory.make(NodeKind::mtr);
            replaceChild(*currentRow, *child);
        }
        MmlNode *rowChild = child;
        if (!isCell) {
            std::array<MmlNode *, 1> wrapped{child};
            rowChild = &factory.make(NodeKind::mtd, {}, wrapped);
        }
        currentRow->appendChild(*rowChild);
        if (!options.fixMtables) {
            // Source detaches the invalid node and gives it a transient parent
            // reference to the table before calling mError. Keep that relationship
            // in the document's auxiliary-parent channel; it is never a second
            // membership in the table's normal children.
            document().remove(*child);
            document().attachAuxiliary(*this, *child);
            if (isCell) {
                auto &emptyCell = factory.make(NodeKind::mtd);
                currentRow->appendChild(emptyCell);
            }
            const auto message = std::u16string{u"Children of mtable must be mtr or mlabeledtr"};
            auto *error = child->mError(message, options, isCell);
            if (error && !currentRow->children().empty())
                currentRow->children().back()->appendChild(*error);
        }
        ++i;
    }
    AbstractMmlNode::verifyChildren(options);
}

MmlNode *MmlMtable::setTeXclass(MmlNode *previous) {
    getPrevClass(previous);
    for (auto *child : children())
        child->setTeXclass(nullptr);
    return this;
}
} // namespace folia::math::mml
