// MathJax 4.1.3 input/tex/ams/AmsItems.ts; Apache-2.0.
module folia.math.input.tex.ams.items;
import folia.math.input.tex.parse_util;
import folia.math.util.utf16;
import std;

namespace folia::math::tex {
namespace {
bool hasAlignment(mml::MmlNode const &cell) {
    auto value = cell.attributes().lookup(u"columnalign");
    if (!value)
        return false;
    return std::visit(
        [](auto const &alignment) -> bool {
            using T = std::remove_cvref_t<decltype(alignment)>;
            if constexpr (std::same_as<T, mml::AttributeNull>)
                return false;
            else if constexpr (std::same_as<T, std::u16string>)
                return !alignment.empty();
            else if constexpr (std::same_as<T, double>)
                return alignment != 0 && !std::isnan(alignment);
            else
                return alignment;
        },
        *value);
}
} // namespace
MultlineItem::MultlineItem(StackItemFactory &factory, bool numbered) : ArrayItem(factory) {
    factory.configuration().tags->start("multline", true, numbered);
}
StackKind MultlineItem::kind() const {
    return StackKind::multline;
}
void MultlineItem::EndEntry() {
    if (!table.empty())
        parse_util::fixInitialMO(factory().configuration(), nodes());
    mml::AttributeSet attributes;
    if (shove && !shove->empty())
        attributes.set(u"columnalign", *shove);
    auto &cell = create(mml::NodeKind::mtd, nodes(), std::move(attributes));
    shove.reset();
    row.push_back(&cell);
    Clear();
}
void MultlineItem::EndRow() {
    if (row.size() != 1) {
        const std::array<std::u16string, 1> arguments{u"multline"};
        throw TexError("MultlineRowsOneCol",
                       u"The rows within the %1 environment must have exactly one column",
                       arguments);
    }
    table.push_back(&create(mml::NodeKind::mtr, row));
    row.clear();
}
void MultlineItem::EndTable() {
    ArrayItem::EndTable();
    if (!table.empty()) {
        auto &first = *table.front()->children()[0];
        if (!hasAlignment(first))
            first.attributes().set(u"columnalign", u"left");
        auto &last = *table.back()->children()[0];
        if (!hasAlignment(last))
            last.attributes().set(u"columnalign", u"right");
        if (auto *tag = factory().configuration().tags->getTag()) {
            auto side = arraydef.own(u"side");
            const auto *text = side ? std::get_if<std::u16string>(side) : nullptr;
            const auto index = text && *text == u"left" ? 0 : table.size() - 1;
            auto &original = *table[index];
            std::vector<mml::MmlNode *> cells{tag};
            cells.insert(cells.end(), original.children().begin(), original.children().end());
            auto &labeled = create(mml::NodeKind::mlabeledtr, cells);
            NodeUtil::copyAttributes(original, labeled);
            table[index] = &labeled;
        }
    }
    factory().configuration().tags->end();
}
FlalignItem::FlalignItem(StackItemFactory &factory, std::u16string environment, bool number,
                         bool pad, bool centered)
    : EqnArrayItem(factory, u"", false, false), padded(pad), center(centered) {
    // The original parent constructor starts an unnamed tag scope before the
    // derived constructor starts the actual flalign environment.
    name = std::move(environment);
    numbered = number;
    factory.configuration().tags->start(utf16::encode(name), numbered, numbered);
}
StackKind FlalignItem::kind() const {
    return StackKind::flalign;
}
void FlalignItem::EndEntry() {
    EqnArrayItem::EndEntry();
    const auto count = xalignat.value_or(0);
    if (count == 0 || std::isnan(count))
        return;
    if (static_cast<double>(row.size()) > count) {
        const std::array<std::u16string, 2> arguments{u"&", name};
        throw TexError("XalignOverflow", u"Extra %1 in row of %2", arguments);
    }
}
void FlalignItem::EndRow() {
    const auto count = xalignat.value_or(0);
    while (static_cast<double>(row.size()) < count)
        row.push_back(&create(mml::NodeKind::mtd));
    auto original = std::move(row);
    row.clear();
    if (padded)
        row.push_back(&create(mml::NodeKind::mtd));
    std::size_t index{};
    while (index < original.size()) {
        auto *cell = original[index++];
        if (!cell)
            break;
        row.push_back(cell);
        if (index < original.size()) {
            if (auto *second = original[index++])
                row.push_back(second);
        }
        if (index < original.size() || padded)
            row.push_back(&create(mml::NodeKind::mtd));
    }
    maxrow = std::max(maxrow, row.size());
    EqnArrayItem::EndRow();
    auto &last = *table.back();
    if (zeroWidthLabel && last.isKind(mml::NodeKind::mlabeledtr)) {
        auto &cell = *last.children()[0];
        mml::AttributeSet attributes;
        attributes.set(u"width", 0.0);
        if (factory().configuration().options.tagSide.value() == u"right")
            attributes.set(u"lspace", u"-1width");
        const auto contents =
            std::vector<mml::MmlNode *>(cell.children().begin(), cell.children().end());
        auto &padded = create(mml::NodeKind::mpadded, contents, std::move(attributes));
        const std::array children{&padded};
        cell.setChildren(children);
    }
}
void FlalignItem::EndTable() {
    EqnArrayItem::EndTable();
    if (center && maxrow <= 2) {
        arraydef.erase(u"width");
        global().indent_align.reset();
    }
}
} // namespace folia::math::tex
