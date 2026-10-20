// MathJax 4.1.3 input/tex/mathtools/MathtoolsItems.ts; Apache-2.0.
module folia.math.input.tex.mathtools.items;
import std;
namespace folia::math::tex {
MultlinedItem::MultlinedItem(StackItemFactory &factory) : MultlineItem(factory, true) {}
StackKind MultlinedItem::kind() const {
    return StackKind::multlined;
}
void MultlinedItem::EndTable() {
    if (Size() || !row.empty()) {
        EndEntry();
        EndRow();
    }
    if (table.size() > 1) {
        const auto &options = factory().configuration().options.mathtools.value();
        const auto gap = options.multlinedGap.value();
        auto firstSkip = options.firstlineAfterskip.value();
        auto lastSkip = options.lastlinePreskip.value();
        if (firstSkip.empty())
            firstSkip = gap;
        if (lastSkip.empty())
            lastSkip = gap;
        auto firstChild = [](mml::MmlNode &node) -> mml::MmlNode & {
            if (node.children().empty() || !node.children()[0])
                throw std::out_of_range("Multlined row has no content child");
            return *node.children()[0];
        };
        auto &first = firstChild(*table.front());
        auto alignment = [](mml::MmlNode &node, std::u16string_view expected) {
            auto value = node.attributes().lookup(u"columnalign");
            return value && std::get_if<std::u16string>(value) &&
                   std::get<std::u16string>(*value) == expected;
        };
        if (!alignment(first, u"right")) {
            mml::AttributeSet attributes;
            attributes.set(u"width", firstSkip);
            first.appendChild(create(mml::NodeKind::mspace, {}, std::move(attributes)));
        }
        auto &last = firstChild(*table.back());
        if (!alignment(last, u"left")) {
            auto &top = firstChild(last);
            std::vector<mml::MmlNode *> children(top.children().begin(), top.children().end());
            mml::AttributeSet attributes;
            attributes.set(u"width", lastSkip);
            auto &space = create(mml::NodeKind::mspace, {}, std::move(attributes));
            children.insert(children.begin(), &space);
            top.setChildren(children);
        }
    }
    MultlineItem::EndTable();
}
} // namespace folia::math::tex
