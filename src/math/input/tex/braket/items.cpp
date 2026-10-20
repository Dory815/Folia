// MathJax 4.1.3 input/tex/braket/BraketItems.ts; Apache-2.0.
module folia.math.input.tex.braket.items;
import folia.math.input.tex.parse_util;
import folia.math.util.lengths;
import std;

namespace folia::math::tex {
BraketItem::BraketItem(StackItemFactory &factory) : BaseItem(factory, true) {}
StackKind BraketItem::kind() const {
    return StackKind::braket;
}
bool BraketItem::isOpen() const noexcept {
    return true;
}
CheckResult BraketItem::checkItem(FrameHandle const &item) {
    if (item->isKind(StackKind::close)) {
        if (dynamic_cast<CloseItem &>(*item).braket_bar) {
            auto &saved = BaseItem::toMml(true, true);
            barNodes.insert(barNodes.end(), saved.children().begin(), saved.children().end());
            Clear();
            return CheckAction::drop;
        }
        return CheckResult(
            std::vector<ReplayEntry>{factory().create(StackKind::mml, MmlItemInit{toMml()})});
    }
    if (item->isKind(StackKind::mml)) {
        auto *node = &item->toMml();
        Push({&node, 1});
        if (single)
            return CheckResult(std::vector<ReplayEntry>{&toMml()});
        return CheckAction::drop;
    }
    return BaseItem::checkItem(item);
}
mml::MmlNode &BraketItem::toMml(bool inferred, bool forceRow) {
    auto *inner = &BaseItem::toMml(inferred, forceRow);
    if (!inferred)
        return *inner;
    if (!barNodes.empty()) {
        auto children = barNodes;
        children.push_back(inner);
        inner = &create(mml::NodeKind::inferred_mrow, children);
    }
    if (stretchy) {
        if (space) {
            const auto thin = lengths::em(3.0 / 18.0);
            mml::AttributeSet attributes;
            attributes.set(u"width", std::u16string(thin.begin(), thin.end()));
            auto &left = createToken(mml::NodeKind::mspace, u"", attributes);
            auto &right = createToken(mml::NodeKind::mspace, u"", std::move(attributes));
            const std::array children{&left, inner, &right};
            inner = &create(mml::NodeKind::inferred_mrow, children);
        }
        return parse_util::fenced(factory().configuration(), open, *inner, close);
    }
    NodeDefinition definition;
    definition.attributes.set(u"fence", true);
    definition.attributes.set(u"stretchy", false);
    definition.attributes.set(u"symmetric", true);
    definition.properties.tex.explicit_tex_class = mml::TexClass::open;
    auto &nodes = factory().configuration().nodeFactory;
    auto &openNode = nodes.makeToken(mml::NodeKind::mo, open, definition);
    definition.properties.tex.explicit_tex_class = mml::TexClass::close;
    auto &closeNode = nodes.makeToken(mml::NodeKind::mo, close, std::move(definition));
    mml::AttributeSet attributes;
    attributes.set(u"open", open);
    attributes.set(u"close", close);
    const std::array children{&openNode, inner, &closeNode};
    return create(mml::NodeKind::mrow, children, std::move(attributes));
}
} // namespace folia::math::tex
