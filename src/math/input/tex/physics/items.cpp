// MathJax 4.1.3 input/tex/physics/PhysicsItems.ts; Apache-2.0.
module folia.math.input.tex.physics.items;
import folia.math.input.tex.parse_util;
import folia.math.input.tex.node_util;
import std;
namespace folia::math::tex {
AutoOpen::AutoOpen(StackItemFactory &factory) : BaseItem(factory, true) {}
StackKind AutoOpen::kind() const {
    return StackKind::auto_open;
}
bool AutoOpen::isOpen() const noexcept {
    return true;
}
std::optional<StackErrorDefinition> AutoOpen::getErrors(StackErrorKey key) const {
    if (key == StackErrorKey::stop)
        return StackErrorDefinition{"ExtraOrMissingDelims",
                                    u"Extra open or missing close delimiter"};
    return BaseItem::getErrors(key);
}
bool AutoOpen::closing(std::u16string_view fence) {
    if (fence != close)
        return false;
    auto previous = openCount--;
    return previous == 0 || std::isnan(previous);
}
CheckResult AutoOpen::checkItem(FrameHandle const &incoming) {
    if (auto close = dynamic_cast<CloseItem *>(incoming.get()); close && close->pre_autoclose)
        return CheckAction::drop;
    if (auto item = dynamic_cast<MmlItem *>(incoming.get()); item && item->autoclose) {
        if (ignore) {
            Clear();
            return CheckResult(std::vector<ReplayEntry>{});
        }
        auto &node = toMml();
        return CheckResult(
            std::vector<ReplayEntry>{factory().create(StackKind::mml, MmlItemInit{node})});
    }
    if (incoming->isKind(StackKind::mml) && incoming->Size() == 1) {
        auto &node = incoming->toMml();
        if (node.kind() == mml::NodeKind::mo && NodeUtil::getText(node) == open)
            ++openCount;
    }
    return BaseItem::checkItem(incoming);
}
mml::MmlNode &AutoOpen::toMml(bool inferred, bool forceRow) {
    if (!inferred)
        return BaseItem::toMml(inferred, forceRow);
    auto &configuration = factory().configuration();
    auto *parser = configuration.parser();
    if (!parser)
        throw std::logic_error("AutoOpen requires the active TexParser");
    if (smash) {
        auto &content = BaseItem::toMml();
        NodeDefinition definition;
        definition.attributes.set(u"height", 0.0);
        definition.attributes.set(u"depth", 0.0);
        const std::array children{&content};
        auto &padded = parser->create(mml::NodeKind::mpadded, children, std::move(definition));
        Clear();
        const std::array paddedChildren{&padded};
        auto &atom = parser->create(mml::NodeKind::tex_atom, paddedChildren);
        const std::array nodes{&atom};
        Push(nodes);
    }
    if (!right.empty()) {
        auto *node =
            TexParser(right, ParserEnvironmentSeed{parser->stack.env()}, parser->configuration)
                .mml();
        const std::array nodes{node};
        Push(nodes);
    }
    auto &result = parse_util::fenced(configuration, open, BaseItem::toMml(), close, big);
    auto &properties = result.properties().tex;
    properties.open.reset();
    properties.close.reset();
    properties.explicit_tex_class.reset();
    return result;
}
} // namespace folia::math::tex
