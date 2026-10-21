/*************************************************************
 * Source output/svg/Wrappers/semantics.ts and HtmlNode.ts.
 *************************************************************/
module folia.math.output.svg.xml_wrapper;
import folia.math.output.scene.html_snapshot;
import folia.math.util.utf16;
import std;

namespace folia::math::output::svg {
namespace {
using folia::math::output::common::CommonWrapperClassData;
using folia::math::output::common::WrapperBuildArguments;
using folia::math::output::scene::ForeignContent;
using folia::math::output::scene::ForeignObjectGeometry;
using folia::math::style_json::StyleEntry;

std::string cssText(folia::math::styles::StyleList const &styles) {
    folia::math::styles::Styles sourceStyles;
    for (auto const &[name, value] : styles)
        sourceStyles.set(utf16::decode(name).text, folia::math::styles::StyleValue(value));
    return utf16::encode(sourceStyles.cssText());
}

void setStyle(folia::math::styles::StyleList &styles, std::string name, std::u16string value) {
    const auto found = std::find_if(styles.begin(), styles.end(),
                                    [&](auto const &entry) { return entry.first == name; });
    if (found == styles.end())
        styles.emplace_back(std::move(name), std::move(value));
    else
        found->second = std::move(value);
}

void eraseStyle(folia::math::styles::StyleList &styles, std::string_view name) {
    std::erase_if(styles, [&](auto const &entry) { return entry.first == name; });
}

folia::math::output::common::CommonWrapperRegistration registration(folia::math::mml::NodeKind kind,
                                                                    bool html) {
    using Args = WrapperBuildArguments;
    using Base = folia::math::output::common::CommonWrapper;
    tree::FactoryClass<Base, Args> creator;
    creator.construct = [html](Args const &args) -> std::unique_ptr<Base> {
        if (html)
            return std::make_unique<SvgHtmlNode>(args.factory, args.node, args.parent);
        return std::make_unique<SvgXmlNode>(args.factory, args.node, args.parent);
    };
    creator.isInstance = [html](Base const &product) {
        return html ? dynamic_cast<SvgHtmlNode const *>(&product) != nullptr
                    : dynamic_cast<SvgXmlNode const *>(&product) != nullptr;
    };

    CommonWrapperClassData data = CommonWrapperClassData::commonDefaults();
    data.styles = {
        {u"foreignObject[data-mjx-html]",
         std::vector<StyleEntry>{{u"overflow", std::u16string(u"visible")}}},
        {u"mjx-measure-xml", std::vector<StyleEntry>{{u"position", std::u16string(u"absolute")},
                                                     {u"left", 0.0},
                                                     {u"top", 0.0},
                                                     {u"display", std::u16string(u"inline-block")},
                                                     {u"line-height", std::u16string(u"normal")},
                                                     {u"white-space", std::u16string(u"normal")}}},
        {u"mjx-html", std::vector<StyleEntry>{{u"display", std::u16string(u"inline-block")},
                                              {u"line-height", std::u16string(u"normal")},
                                              {u"text-align", std::u16string(u"initial")},
                                              {u"white-space", std::u16string(u"initial")}}},
        {u"mjx-html-holder", std::vector<StyleEntry>{{u"display", std::u16string(u"block")},
                                                     {u"position", std::u16string(u"absolute")},
                                                     {u"top", 0.0},
                                                     {u"left", 0.0},
                                                     {u"bottom", 0.0},
                                                     {u"right", 0.0}}},
    };
    return {kind, std::move(creator), std::move(data)};
}
} // namespace

DOMNodeRef SvgXmlNode::addHDW(DOMNodeRef html, folia::math::styles::StyleList &styles) {
    auto &adaptor = jax().htmlMeasurement().adaptor;
    const auto styleBeforeLayout = cssText(styles);
    const std::array<folia::math::AttributeData, 1> initialAttributes{
        {{u"style", utf16::decode(styleBeforeLayout).text}}};
    const std::array<DOMNodeRef, 1> children{std::move(html)};
    auto holder = adaptor.node(u"mjx-html-holder", initialAttributes, children);

    // Keep Source ordering: holder construction snapshots the incoming style
    // record; updates below apply to the outer mjx-html style record only.
    const auto &box = getBBox();
    const double scale = metrics().scale;
    setStyle(styles, "height", utf16::decode(em((box.h + box.d) * scale)).text);
    setStyle(styles, "width", utf16::decode(em(box.w * scale)).text);
    setStyle(styles, "vertical-align", utf16::decode(em(-box.d * scale)).text);
    eraseStyle(styles, "font-size");
    eraseStyle(styles, "font-family");
    return holder;
}

common::SceneNodes SvgXmlNode::emitScene(scene::Builder &builder,
                                         std::span<scene::NodeId const> parents) {
    if (parents.empty())
        throw std::logic_error("SvgXmlNode requires a parent Scene group");
    const auto &metricsValue = metrics();
    const auto &box = getBBox();
    const double em = metricsValue.em * metricsValue.scale * rscale;
    if (!(em > 0) || !std::isfinite(em))
        throw std::logic_error("SvgXmlNode requires a finite positive em scale");

    auto &adaptor = jax().htmlMeasurement().adaptor;
    const auto html = getHTML();
    auto snapshot = scene::snapshotHtmlNode(adaptor, html, u"http://www.w3.org/1999/xhtml");
    ForeignContent foreign{
        std::move(snapshot),
        ForeignObjectGeometry{0, -box.h * em, box.w * em, (box.h + box.d) * em, 1 / em},
        {}};
    const auto group =
        builder.add_group(parents.front(), {}, {}, {}, {}, {},
                          scene::ElementRole::foreign_content_viewport, {}, {}, std::move(foreign));
    return {group};
}

common::CommonWrapperRegistration xmlRegistration() {
    return registration(folia::math::mml::NodeKind::xml, false);
}

common::CommonWrapperRegistration htmlRegistration() {
    return registration(folia::math::mml::NodeKind::html, true);
}

common::CommonWrapperRegistration annotationXmlRegistration() {
    using Args = WrapperBuildArguments;
    using Base = common::CommonWrapper;
    tree::FactoryClass<Base, Args> creator;
    creator.construct = [](Args const &args) -> std::unique_ptr<Base> {
        return std::make_unique<SvgAnnotationXML>(args.factory, args.node, args.parent);
    };
    creator.isInstance = [](Base const &product) {
        return dynamic_cast<SvgAnnotationXML const *>(&product) != nullptr;
    };

    auto data = CommonWrapperClassData::commonDefaults();
    data.styles = {
        {u"foreignObject[data-mjx-xml]",
         std::vector<StyleEntry>{{u"font-family", std::u16string(u"initial")},
                                 {u"line-height", std::u16string(u"normal")},
                                 {u"overflow", std::u16string(u"visible")}}},
    };
    return {folia::math::mml::NodeKind::annotation_xml, std::move(creator), std::move(data)};
}
} // namespace folia::math::output::svg
