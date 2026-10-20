// MathJax 4.1.3 input/tex/newcommand/NewcommandItems.ts; Apache-2.0.
module folia.math.input.tex.newcommand.items;
import std;
namespace folia::math::tex {
BeginEnvItem::BeginEnvItem(StackItemFactory &factory) : BaseItem(factory, true) {}
StackKind BeginEnvItem::kind() const {
    return StackKind::begin_env;
}
bool BeginEnvItem::isOpen() const noexcept {
    return true;
}
std::optional<std::u16string_view> BeginEnvItem::getName() const noexcept {
    return name ? std::optional<std::u16string_view>(*name) : std::nullopt;
}
CheckResult BeginEnvItem::checkItem(FrameHandle const &item) {
    auto current = std::u16string(getName().value_or(u"undefined"));
    if (item->isKind(StackKind::end)) {
        if (item->getName() != getName()) {
            const std::array args{current, std::u16string(item->getName().value_or(u"undefined"))};
            throw TexError("EnvBadEnd", u"\\begin{%1} ended with \\end{%2}", args);
        }
        auto frame = factory().create(StackKind::mml, MmlItemInit{toMml()});
        return CheckResult(std::vector<ReplayEntry>{std::move(frame)});
    }
    if (item->isKind(StackKind::stop)) {
        const std::array args{current};
        throw TexError("EnvMissingEnd", u"Missing \\end{%1}", args);
    }
    return BaseItem::checkItem(item);
}
} // namespace folia::math::tex
