/*************************************************************
 * Native MathJax 4.1.3 output/svg/Wrappers/maction.ts owner.
 *************************************************************/
module folia.math.output.svg.maction;
import folia.math.core.mml_tree.mml_document;
import folia.math.core.mml_tree.mml_nodes.math;
import folia.math.output.common;
import folia.math.util.js_numeric_input;
import folia.math.util.js_number;
import folia.math.util.utf16;
import std;

namespace folia::math::output::svg {
namespace {
namespace mml = folia::math::mml;
using Args = common::WrapperBuildArguments;
using Base = common::CommonWrapper;
using common::CommonWrapperClassData;
using folia::math::style_json::StyleEntry;

constexpr std::u16string_view nodeSelector = u"g[data-mml-node=\"maction\"][data-toggle]";
constexpr std::u16string_view collapsedSelector = u"g[data-mjx-collapsed]";
constexpr std::u16string_view toolSelector = u"[jax=\"SVG\"] mjx-tool";
constexpr std::u16string_view positionedTipSelector = u"[jax=\"SVG\"] mjx-tool > mjx-tip";
constexpr std::u16string_view tipSelector = u"mjx-tool > mjx-tip";
constexpr std::u16string_view statusSelector = u"mjx-status";
constexpr std::u16string_view darkSelector =
    u"@media (prefers-color-scheme: dark) /* svg maction */";

StyleEntry rule(std::u16string_view selector, std::vector<StyleEntry> declarations) {
    return {std::u16string(selector), std::move(declarations)};
}
StyleEntry declaration(std::u16string_view name, std::u16string_view value) {
    return {std::u16string(name), std::u16string(value)};
}
StyleEntry declaration(std::u16string_view name, double value) {
    return {std::u16string(name), value};
}
style_json::StyleJson sourceStyles() {
    return {
        rule(toolSelector, {declaration(u"display", u"inline-block"),
                            declaration(u"position", u"relative"),
                            {u"width", 0.0},
                            {u"height", 0.0}}),
        rule(positionedTipSelector, {declaration(u"position", u"absolute"),
                                     declaration(u"top", 0.0), declaration(u"left", 0.0)}),
        rule(tipSelector,
             {declaration(u"display", u"inline-block"), declaration(u"line-height", 0.0),
              declaration(u"padding", u".2em"), declaration(u"border", u"1px solid #888"),
              declaration(u"background-color", u"#F8F8F8"), declaration(u"color", u"black"),
              declaration(u"box-shadow", u"2px 2px 5px #AAAAAA")}),
        rule(nodeSelector, {declaration(u"cursor", u"pointer")}),
        rule(statusSelector,
             {declaration(u"display", u"block"), declaration(u"position", u"fixed"),
              declaration(u"left", u"1em"), declaration(u"bottom", u"1em"),
              declaration(u"min-width", u"25%"), declaration(u"padding", u".2em .4em"),
              declaration(u"border", u"1px solid #888"), declaration(u"font-size", u"90%"),
              declaration(u"background-color", u"#F8F8F8"), declaration(u"color", u"black")}),
        rule(collapsedSelector, {declaration(u"fill", u"#55F")}),
        rule(darkSelector, {rule(tipSelector, {declaration(u"background-color", u"#303030"),
                                               declaration(u"color", u"#E0E0E0"),
                                               declaration(u"box-shadow", u"2px 2px 5px #000")}),
                            rule(statusSelector, {declaration(u"background-color", u"#303030"),
                                                  declaration(u"color", u"#E0E0E0")}),
                            rule(collapsedSelector, {declaration(u"fill", u"#88F")})}),
    };
}

bool truthy(mml::AttributeValue const *value) {
    using mml::AttributeNull;
    if (!value || std::holds_alternative<AttributeNull>(*value))
        return false;
    if (auto const *boolean = std::get_if<bool>(value))
        return *boolean;
    if (auto const *number = std::get_if<double>(value))
        return *number != 0 && !std::isnan(*number);
    return !std::get<std::u16string>(*value).empty();
}

std::string selectionText(mml::AttributeValue const *value) {
    if (!value)
        return "1";
    if (auto const *text = std::get_if<std::u16string>(value))
        return utf16::encode(*text);
    if (auto const *boolean = std::get_if<bool>(value))
        return *boolean ? "true" : "false";
    if (std::holds_alternative<mml::AttributeNull>(*value))
        return "null";
    return js_number::toString(std::get<double>(*value));
}

double numericSelection(mml::AttributeValue const *value) {
    if (!value)
        return 1.0;
    if (auto const *number = std::get_if<double>(value))
        return *number;
    if (auto const *boolean = std::get_if<bool>(value))
        return *boolean ? 1.0 : 0.0;
    if (auto const *text = std::get_if<std::u16string>(value))
        return js_input::number(*text);
    if (std::holds_alternative<mml::AttributeNull>(*value))
        return 0.0;
    return 0.0;
}

void setAttribute(scene::SemanticMetadata &metadata, std::string name, std::string value) {
    auto found =
        std::find_if(metadata.ordered_attributes.begin(), metadata.ordered_attributes.end(),
                     [&](auto const &attribute) { return attribute.name == name; });
    if (found == metadata.ordered_attributes.end())
        metadata.ordered_attributes.push_back({std::move(name), std::move(value)});
    else
        found->value = std::move(value);
}

class TipParentRestore final {
    mml::MmlDocument &document_;
    mml::MmlNode &tip_;
    mml::MmlNode &parent_;
    std::size_t slot_;

    static mml::MmlNode &requireParent(mml::MmlNode &tip) {
        auto *parent = tip.parent();
        if (!parent)
            throw std::logic_error("SvgMaction tooltip has no original MML parent");
        if (!parent->childIndex(tip))
            throw std::logic_error(
                "SvgMaction tooltip is not in its original parent's child slots");
        return *parent;
    }
    static std::size_t requireSlot(mml::MmlNode &parent, mml::MmlNode &tip) {
        const auto slot = parent.childIndex(tip);
        if (!slot)
            throw std::logic_error("SvgMaction tooltip lost its original MML child slot");
        return *slot;
    }

  public:
    TipParentRestore(mml::MmlDocument &document, mml::MmlNode &tip)
        : document_(document), tip_(tip), parent_(requireParent(tip)),
          slot_(requireSlot(parent_, tip)) {
        document_.setChildSlot(parent_, slot_, nullptr);
    }
    TipParentRestore(TipParentRestore const &) = delete;
    TipParentRestore &operator=(TipParentRestore const &) = delete;
    // Construction leaves the original parent slot allocated and null. The
    // tip is detached from the auxiliary math before this exact slot is
    // restored, so setChildSlot does not need to grow or allocate.
    ~TipParentRestore() noexcept {
        if (tip_.parent())
            document_.remove(tip_);
        document_.setChildSlot(parent_, slot_, &tip_);
    }
};

scene::Scene buildTooltip(common::CommonMaction &wrapper, common::CommonWrapper &tip) {
    auto &document = wrapper.node->document();
    TipParentRestore restore(document, *tip.node);
    const std::array<mml::MmlNode *, 1> children{tip.node};
    auto &auxiliaryMath = document.factory().make(mml::NodeKind::math, {}, children);
    auto &auxiliaryWrapper = wrapper.factory().wrap(auxiliaryMath);
    return wrapper.jax().buildScene(auxiliaryWrapper);
}
} // namespace

common::SceneNodes SvgMaction::emitScene(scene::Builder &builder,
                                         std::span<scene::NodeId const> parents) {
    if (auto embellished = toEmbellishedScene(builder, parents))
        return *embellished;

    auto containers = standardSceneNodes(builder, parents);
    auto *selected = this->selected();
    if (!selected)
        throw std::logic_error("SvgMaction has no selected child wrapper");

    // Source's dom contains only actual own groups. Preserve optional Scene
    // slots while assigning line BBoxes in the same order as those groups.
    std::size_t line = 0;
    const auto actionValue = node->attributes().lookup(u"actiontype");
    const auto *actionType = actionValue ? std::get_if<std::u16string>(actionValue) : nullptr;
    common::CommonWrapper *tip = childNodes.size() > 1 ? childNodes[1] : nullptr;
    const bool textTip = tip && tip->node->isKind(mml::NodeKind::mtext);

    const auto *selection = node->attributes().lookup(u"selection");
    const double selectionNumber = numericSelection(selection);
    const std::string selectionString = selectionText(selection);
    const auto *enriched = node->attributes().lookup(u"data-maction-id");
    const bool isEnriched = truthy(enriched);
    const auto children = node->children();
    std::vector<std::pair<scene::NodeId, std::size_t>> regions;
    regions.reserve(containers.nodes.size());

    for (const auto &slot : containers.nodes) {
        if (!slot)
            continue;
        const auto root = *slot;
        auto lineBox = selected->getLineBBox(line++);
        if (!lineBox)
            throw std::logic_error("SvgMaction selected child has no line BBox");

        if (actionType && *actionType == u"tooltip" && textTip)
            builder.add_primitive(
                root, scene::Title{utf16::encode(
                          dynamic_cast<mml::AbstractMmlTokenNode const &>(*tip->node).getText())});

        scene::Rule hitbox;
        hitbox.bounds = {line == 1 ? -sceneDx() : 0.0, -lineBox->d, lineBox->w,
                         lineBox->h + lineBox->d};
        hitbox.paint.fill = scene::NoPaint{};
        const auto regionIndex =
            builder.add_action_region(root, scene::ActionRegion{std::move(hitbox), {}});
        regions.emplace_back(root, regionIndex);
    }

    const auto childNodes = selected->emitScene(builder, containers.content);
    const auto &box = selected->getOuterBBox();
    selected->placeScene(builder, childNodes, box.L * box.rscale, 0.0);

    // Source invokes action(this, data) after selected-child emission and
    // placement. Mathematical tips are processed once per own DOM segment.
    for (const auto &[root, regionIndex] : regions) {
        if (actionType && *actionType == u"toggle") {
            auto metadata = builder.metadata(root);
            setAttribute(metadata, "data-toggle", selectionString);
            builder.set_metadata(root, std::move(metadata));
            builder.set_action(root,
                               scene::ToggleAction{selectionNumber, children.size(), isEnriched});
        } else if (actionType && *actionType == u"tooltip" && tip && !textTip) {
            auto tooltip = std::make_shared<scene::Scene const>(buildTooltip(*this, *tip));
            builder.set_action_region_tooltip(root, regionIndex, std::move(tooltip));
            builder.set_action(root, scene::MathTooltipAction{tipDx(), tipDy(), 600, 100});
        } else if (actionType && *actionType == u"statusline" && textTip) {
            const auto text = dynamic_cast<mml::AbstractMmlTokenNode const &>(*tip->node).getText();
            auto metadata = builder.metadata(root);
            setAttribute(metadata, "data-statusline", utf16::encode(text));
            builder.set_metadata(root, std::move(metadata));
            builder.set_action(root, scene::StatuslineAction{utf16::encode(text)});
        }
    }
    return containers.nodes;
}

common::CommonWrapperRegistration mactionRegistration() {
    tree::FactoryClass<Base, Args> creator;
    creator.construct = [](Args const &args) -> std::unique_ptr<Base> {
        return std::make_unique<SvgMaction>(args.factory, args.node, args.parent);
    };
    creator.isInstance = [](Base const &product) {
        return dynamic_cast<SvgMaction const *>(&product) != nullptr;
    };
    auto data = CommonWrapperClassData::commonDefaults();
    data.styles = sourceStyles();
    return {mml::NodeKind::maction, std::move(creator), std::move(data)};
}
} // namespace folia::math::output::svg
