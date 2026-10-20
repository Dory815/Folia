// MathJax 4.1.3 input/tex/bussproofs/BussproofsItems.ts; Apache-2.0.
module folia.math.input.tex.bussproofs.items;
import std;
namespace folia::math::tex {
ProofTreeItem::ProofTreeItem(StackItemFactory &factory)
    : BaseItem(factory, false), innerStack_(factory, {}, true) {}
StackKind ProofTreeItem::kind() const {
    return StackKind::proof_tree;
}
std::optional<std::u16string_view> ProofTreeItem::getName() const noexcept {
    return name ? std::optional<std::u16string_view>(*name) : std::nullopt;
}
CheckResult ProofTreeItem::checkItem(FrameHandle const &incoming) {
    if (incoming->isKind(StackKind::end) && incoming->getName() == u"prooftree") {
        auto &node = toMml();
        node.properties().proof.proof = true;
        return CheckResult(std::vector<ReplayEntry>{
            factory().create(StackKind::mml, MmlItemInit{node}), ReuseIncoming{}});
    }
    if (incoming->isKind(StackKind::stop)) {
        const std::array<std::u16string, 1> args{std::u16string(getName().value_or(u""))};
        throw TexError("EnvMissingEnd", u"Missing \\end{%1}", args);
    }
    innerStack_.Push(incoming);
    return CheckAction::drop;
}
mml::MmlNode &ProofTreeItem::toMml(bool, bool) {
    // Source's zero-argument override always calls super.toMml() with defaults.
    auto &tree = BaseItem::toMml();
    auto *start = innerStack_.Top();
    if (start->isKind(StackKind::start) && !start->Size())
        return tree;
    innerStack_.Push(factory().create(StackKind::stop));
    auto &prefix = innerStack_.Top()->toMml();
    const std::array children{&prefix, &tree};
    return create(mml::NodeKind::mrow, children);
}
} // namespace folia::math::tex
