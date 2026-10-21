/*************************************************************
 * MathJax 4.1.3 output/svg/Wrappers/merror.ts.
 *************************************************************/
module folia.math.output.svg.merror;
import folia.math.core.mml_tree.mml_document;
import folia.math.util.utf16;
import std;

namespace folia::math::output::svg {
namespace {
using folia::math::output::common::CommonWrapperClassData;
using folia::math::output::common::WrapperBuildArguments;
using folia::math::output::scene::CssColor;
using folia::math::output::scene::ElementRole;
using folia::math::output::scene::NoPaint;
using folia::math::output::scene::PaintColors;
using folia::math::output::scene::Rule;
using folia::math::output::scene::SemanticMetadata;
using folia::math::output::scene::Title;
using folia::math::style_json::StyleEntry;

constexpr std::u16string_view contentSelector = u"g[data-mml-node=\"merror\"] > g";
constexpr std::u16string_view backgroundSelector =
    u"g[data-mml-node=\"merror\"] > rect[data-background]";
constexpr std::u16string_view red = u"red";
constexpr std::u16string_view yellow = u"yellow";
constexpr std::u16string_view none = u"none";

bool truthy(folia::math::mml::AttributeValue const *value) {
    using folia::math::mml::AttributeNull;
    if (!value || std::holds_alternative<AttributeNull>(*value))
        return false;
    if (auto const *boolean = std::get_if<bool>(value))
        return *boolean;
    if (auto const *number = std::get_if<double>(value))
        return *number != 0 && !std::isnan(*number);
    return !std::get<std::u16string>(*value).empty();
}

std::string sourceTitle(folia::math::mml::AttributeValue const &value) {
    if (auto const *text = std::get_if<std::u16string>(&value))
        return folia::math::utf16::encode(*text);
    throw std::logic_error("SvgMerror title attribute is not a string");
}

style_json::StyleJson sourceStyles() {
    return {
        {std::u16string(contentSelector),
         std::vector<StyleEntry>{{u"fill", std::u16string(red)}, {u"stroke", std::u16string(red)}}},
        {std::u16string(backgroundSelector),
         std::vector<StyleEntry>{{u"fill", std::u16string(yellow)},
                                 {u"stroke", std::u16string(none)}}},
    };
}

scene::PaintColors sourcePaint(std::u16string_view fill, std::u16string_view stroke) {
    const auto paint = [](std::u16string_view value) -> scene::PaintColor {
        if (value == none)
            return NoPaint{};
        return CssColor{utf16::encode(value)};
    };
    return {paint(fill), paint(stroke)};
}
} // namespace

common::SceneNodes SvgMerror::emitScene(scene::Builder &builder,
                                        std::span<scene::NodeId const> parents) {
    auto containers = standardSceneNodes(builder, parents);
    const auto &box = getBBox();
    if (containers.nodes.empty() || !containers.nodes.front())
        throw std::logic_error("SvgMerror requires its Source own Scene root at slot zero");
    const auto root = *containers.nodes.front();

    auto backgroundPaint = scene::Paint{};
    backgroundPaint.stylesheet = sourcePaint(yellow, none);
    scene::SemanticMetadata backgroundMetadata;
    builder.add_primitive(root, Rule{{0, -box.d, box.w, box.h + box.d},
                                     std::move(backgroundPaint),
                                     std::move(backgroundMetadata),
                                     ElementRole::error_background});

    const auto *title = node->attributes().lookup(u"title");
    if (truthy(title))
        builder.add_primitive(root, Title{sourceTitle(*title)});

    addSceneChildren(builder, containers.content);
    const auto childRoots = builder.child_groups(root);
    for (const auto child : childRoots) {
        if (builder.link(child) || builder.viewport(child) || builder.has_foreign_content(child))
            continue;
        auto paint = builder.paint(child);
        paint.stylesheet = sourcePaint(red, red);
        builder.set_paint(child, std::move(paint));
    }
    return containers.nodes;
}

common::CommonWrapperRegistration merrorRegistration() {
    using Args = WrapperBuildArguments;
    tree::FactoryClass<common::CommonWrapper, Args> creator;
    creator.construct = [](Args const &args) -> std::unique_ptr<common::CommonWrapper> {
        return std::make_unique<SvgMerror>(args.factory, args.node, args.parent);
    };
    creator.isInstance = [](common::CommonWrapper const &product) {
        return dynamic_cast<SvgMerror const *>(&product) != nullptr;
    };

    auto data = CommonWrapperClassData::commonDefaults();
    data.styles = sourceStyles();
    return {folia::math::mml::NodeKind::merror, std::move(creator), std::move(data)};
}
} // namespace folia::math::output::svg
