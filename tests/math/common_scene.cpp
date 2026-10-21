#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <string>
#include <typeinfo>
#include "support/folia_test.hpp"
#include "fixtures/common_scene.inc"
import folia.math.core.math_item;
import folia.math.adaptors.lite_adaptor;
import folia.math.fonts.newcm_font_module_provider;
import folia.math.handlers.html.html_dom_strings;
import folia.math.input.tex.action.configuration;
import folia.math.input.tex.ams.configuration;
import folia.math.input.tex.amscd.configuration;
import folia.math.input.tex.base_configuration;
import folia.math.input.tex.bbm.configuration;
import folia.math.input.tex.bboldx.configuration;
import folia.math.input.tex.bbox.configuration;
import folia.math.input.tex.begingroup.configuration;
import folia.math.input.tex.boldsymbol.configuration;
import folia.math.input.tex.braket.configuration;
import folia.math.input.tex.bussproofs.configuration;
import folia.math.input.tex.cancel.configuration;
import folia.math.input.tex.cases.configuration;
import folia.math.input.tex.centernot.configuration;
import folia.math.input.tex.color.configuration;
import folia.math.input.tex.colortbl.configuration;
import folia.math.input.tex.configmacros.configuration;
import folia.math.input.tex.dsfont.configuration;
import folia.math.input.tex.empheq.configuration;
import folia.math.input.tex.enclose.configuration;
import folia.math.input.tex.extpfeil.configuration;
import folia.math.input.tex.fontsizev3.configuration;
import folia.math.input.tex.gensymb.configuration;
import folia.math.input.tex.html.configuration;
import folia.math.input.tex.mathtools.configuration;
import folia.math.input.tex.mhchem.configuration;
import folia.math.input.tex.newcommand.configuration;
import folia.math.input.tex.noundefined.configuration;
import folia.math.input.tex.parser;
import folia.math.input.tex.physics.configuration;
import folia.math.input.tex.setoptions.set_options_configuration;
import folia.math.input.tex.tagformat.tag_format_configuration;
import folia.math.input.tex.texhtml.tex_html_configuration;
import folia.math.input.tex.textcomp.configuration;
import folia.math.input.tex.textmacros.configuration;
import folia.math.input.tex.unicode.configuration;
import folia.math.input.tex.units.configuration;
import folia.math.input.tex.upgreek.configuration;
import folia.math.input.tex.verb.configuration;
import folia.math.output.common;
import folia.math.output.scene;
import folia.math.output.svg.wrapper_factory;
import folia.math.output.svg.writer;
import folia.math.util.utf16;

using namespace boost::ut;
namespace mml = folia::math::mml;
namespace tex = folia::math::tex;
namespace common = folia::math::output::common;
namespace scene = folia::math::output::scene;
namespace svg = folia::math::output::svg;

namespace {
struct NoUnexpectedMeasurement final : common::MeasurementTarget {
    common::ExtendedMetrics measureMetrics(common::MetricRequest const &) override {
        throw std::logic_error("focused Common-to-Scene inputs must use pinned NewCM metrics");
    }
    common::UnknownBBox measureUnknownText(common::UnknownTextRequest const &) override {
        throw std::logic_error(
            "focused Common-to-Scene inputs unexpectedly require browser text metrics");
    }
};

class LiteUnknownTextMeasurement final : public common::MeasurementTarget {
    folia::math::adaptors::lite::LiteAdaptor &adaptor_;

  public:
    explicit LiteUnknownTextMeasurement(folia::math::adaptors::lite::LiteAdaptor &adaptor) noexcept
        : adaptor_(adaptor) {}

    common::ExtendedMetrics measureMetrics(common::MetricRequest const &) override {
        throw std::logic_error(
            "canonical Common-to-Scene cases must use the pinned NewCM font metrics");
    }

    common::UnknownBBox measureUnknownText(common::UnknownTextRequest const &request) override {
        const std::array<folia::math::DOMNodeRef, 1> children{adaptor_.text(request.text)};
        const auto textElement = adaptor_.node(
            u"text", std::span<folia::math::AttributeData const>{},
            std::span<folia::math::DOMNodeRef const>{children}, u"http://www.w3.org/2000/svg");
        const auto [width, height] = adaptor_.nodeSize(textElement, 1000, true);
        (void)height;
        return {width, .75, .2};
    }
};

std::unique_ptr<common::Linebreaks> makeLinebreakVisitor(common::CommonWrapperFactory &factory) {
    return std::make_unique<common::LinebreakVisitor>(factory);
}

struct InputOwner {
    folia::math::fonts::NewcmFontDataBundle fonts{folia::math::fonts::makeNewcmFontData()};
    folia::math::adaptors::lite::LiteAdaptor adaptor;
    tex::ConfigurationHandler configurations;
    tex::MapHandler maps;
    tex::MacroCallbacks callbacks{{}, {}};
    tex::TexOptionValues texOptions;
    std::shared_ptr<folia::math::handlers::html::HTMLDomStringsOptions> texHtmlOptions =
        std::make_shared<folia::math::handlers::html::HTMLDomStringsOptions>();
    struct Warnings final : tex::WarningSink {
        void warn(std::u16string_view) override {}
    } warnings;
    std::unique_ptr<tex::TeX> canonicalInput;
    InputOwner() {
        // Match the native configuration registration sequence in the pinned
        // tools/mathjax-bundle/entry.cjs, including the colliding font packages.
        tex::BasePackage::initialize(configurations, maps);
        tex::ActionConfiguration::initializeModule(configurations, maps);
        tex::AmsConfiguration::initializeModule(configurations, maps);
        tex::AmsCdConfiguration::initializeModule(configurations, maps);
        tex::BbmConfiguration::initializeModule(configurations, maps);
        tex::BboldxConfiguration::initializeModule(configurations, maps);
        tex::BboxConfiguration::initializeModule(configurations, maps);
        tex::BegingroupConfiguration::initializeModule(configurations, maps);
        tex::BoldsymbolConfiguration::initializeModule(configurations, maps);
        tex::BraketConfiguration::initializeModule(configurations, maps);
        tex::BussproofsConfiguration::initializeModule(configurations, maps);
        tex::CancelConfiguration::initializeModule(configurations, maps);
        tex::CasesConfiguration::initializeModule(configurations, maps);
        tex::CenternotConfiguration::initializeModule(configurations, maps);
        tex::ColorConfiguration::initializeModule(configurations, maps);
        tex::ColortblConfiguration::initializeModule(configurations, maps);
        tex::ConfigMacrosConfiguration::initializeModule(configurations, maps);
        tex::DsfontConfiguration::initializeModule(configurations, maps);
        tex::EmpheqConfiguration::initializeModule(configurations, maps);
        tex::EncloseConfiguration::initializeModule(configurations, maps);
        tex::ExtpfeilConfiguration::initializeModule(configurations, maps);
        tex::FontSizeV3Configuration::initializeModule(configurations, maps);
        tex::GensymbConfiguration::initializeModule(configurations, maps);
        tex::HtmlConfiguration::initializeModule(configurations, maps);
        tex::MathtoolsConfiguration::initializeModule(configurations, maps);
        tex::MhchemConfiguration::initializeModule(configurations, maps);
        tex::NewcommandConfiguration::initializeModule(configurations, maps);
        tex::NoUndefinedConfiguration::initializeModule(configurations);
        tex::PhysicsConfiguration::initializeModule(configurations, maps);
        tex::SetOptionsConfiguration::initializeModule(configurations);
        tex::TagFormatConfiguration::initializeModule(configurations);
        tex::TexHtmlConfiguration::initializeModule(configurations, maps, texHtmlOptions);
        tex::TextcompConfiguration::initializeModule(configurations, maps);
        tex::TextMacrosConfiguration::initializeModule(configurations, maps);
        tex::UnicodeConfiguration::initializeModule(configurations, maps);
        tex::UnitsConfiguration::initializeModule(configurations, maps);
        tex::UpgreekConfiguration::initializeModule(configurations, maps);
        tex::VerbConfiguration::initializeModule(configurations, maps);

        texOptions.packages = std::vector<tex::PackageSelection>{
            {u"base", {}},         {u"action", {}},     {u"ams", {}},        {u"amscd", {}},
            {u"bbm", {}},          {u"bboldx", {}},     {u"bbox", {}},       {u"begingroup", {}},
            {u"boldsymbol", {}},   {u"braket", {}},     {u"bussproofs", {}}, {u"cancel", {}},
            {u"cases", {}},        {u"centernot", {}},  {u"color", {}},      {u"colortbl", {}},
            {u"configmacros", {}}, {u"dsfont", {}},     {u"empheq", {}},     {u"enclose", {}},
            {u"extpfeil", {}},     {u"fontsizev3", {}}, {u"gensymb", {}},    {u"html", {}},
            {u"mathtools", {}},    {u"mhchem", {}},     {u"newcommand", {}}, {u"noundefined", {}},
            {u"physics", {}},      {u"setoptions", {}}, {u"tagformat", {}},  {u"texhtml", {}},
            {u"textcomp", {}},     {u"textmacros", {}}, {u"unicode", {}},    {u"units", {}},
            {u"upgreek", {}},      {u"verb", {}}};
    }
};

common::CommonOutputJaxConfig commonConfig(CommonSceneCase const &item, InputOwner const &owner) {
    common::CommonOptions options;
    options.displayOverflow = "linebreak";
    options.linebreaks.inlineBreaks = true;
    common::ExtendedMetrics metrics{item.em, item.ex,        static_cast<double>(item.width),
                                    1,       "MathJaxNewcm", item.display};
    common::CommonOutputJaxConfig config{metrics, std::move(options), owner.fonts.fonts,
                                         "MathJaxNewcm"};
    // Source SVG::forceInlineBreaks returns the enabled inline-break option.
    config.inlineBreakPolicy = common::InlineBreakPolicy::forced;
    return config;
}

scene::Scene buildSceneFromDocument(mml::MmlDocument &document, CommonSceneCase const &item,
                                    InputOwner &owner, common::MeasurementTarget &measurement) {
    common::LayoutJob job(document, commonConfig(item, owner), measurement,
                          svg::SvgWrapperFactory::defaultNodes(), makeLinebreakVisitor);
    return job.buildScene();
}

class NativeOutput final : public folia::math::core::MathOutputJaxService {
    CommonSceneCase const &item_;
    InputOwner &owner_;
    common::MeasurementTarget &measurement_;

  public:
    NativeOutput(CommonSceneCase const &item, InputOwner &owner,
                 common::MeasurementTarget &measurement) noexcept
        : item_(item), owner_(owner), measurement_(measurement) {}

    scene::Scene typeset(folia::math::core::MathItem &math, mml::MmlDocument &document) override {
        if (!math.root)
            throw std::logic_error("Common-to-Scene typeset requires compiled MathItem root");
        if (document.root() != math.root)
            document.setRoot(*math.root);
        return buildSceneFromDocument(document, item_, owner_, measurement_);
    }

    scene::Scene escaped(folia::math::core::MathItem &, mml::MmlDocument &) override {
        throw svg::UnsupportedError(
            "escaped math output is outside this Common-to-Scene TeX fixture");
    }

    folia::math::BBox getBBox(folia::math::core::MathItem &math,
                              mml::MmlDocument &document) override {
        if (!math.root)
            throw std::logic_error(
                "Common-to-Scene proof measurement requires compiled MathItem root");
        auto config = commonConfig(item_, owner_);
        return common::CommonOutputJax::getBBox(
            document, *math.root, std::move(config), measurement_,
            svg::SvgWrapperFactory::defaultNodes(), makeLinebreakVisitor);
    }
};

scene::Scene buildScene(CommonSceneCase const &item, InputOwner &owner,
                        common::MeasurementTarget &measurement,
                        bool persistentInputSession = false) {
    mml::MmlDocument document;
    owner.texOptions.tagSide =
        item.tag_side.empty()
            ? std::optional<std::u16string>{u"right"}
            : std::optional<std::u16string>{folia::math::utf16::decode(item.tag_side).text};
    std::unique_ptr<tex::TeX> transientInput;
    tex::TeX *input = nullptr;
    if (persistentInputSession) {
        if (!owner.canonicalInput)
            owner.canonicalInput =
                std::make_unique<tex::TeX>(owner.configurations, owner.maps, owner.callbacks,
                                           owner.warnings, document.factory(), owner.texOptions);
        input = owner.canonicalInput.get();
    } else {
        transientInput =
            std::make_unique<tex::TeX>(owner.configurations, owner.maps, owner.callbacks,
                                       owner.warnings, document.factory(), owner.texOptions);
        input = transientInput.get();
    }
    // The canonical Source capture reuses one input jax but supplies each
    // request's configured tag side. The input's parse options refer to this
    // same live option record.
    input->options.tagSide = owner.texOptions.tagSide;
    NativeOutput output(item, owner, measurement);
    folia::math::core::AbstractMathItem expression(
        folia::math::utf16::decode(item.tex).text, input,
        item.display ? folia::math::core::MathDisplay::display_math
                     : folia::math::core::MathDisplay::inline_math);
    expression.setMetrics(item.em, item.ex, item.width, 1);
    expression.compile(document, {&output, &owner.adaptor});
    if (!expression.root)
        throw std::logic_error("TeX input produced no MathItem root");
    document.setRoot(*expression.root);
    expression.typeset(output, document);
    if (!expression.typesetRoot)
        throw std::logic_error("NativeOutput did not retain its owned frozen Scene value");
    return std::move(*expression.typesetRoot);
}

std::string serialize(scene::Scene const &value, InputOwner const &owner) {
    svg::SvgWriteOptions options;
    options.font_cache = svg::FontCacheMode::none;
    options.font_resources = &owner.fonts.svg_resources;
    return svg::write(value, options);
}

void reportFirstDifference(std::string_view name, std::string_view expected,
                           std::string_view actual) {
    if (expected == actual)
        return;
    if (const char *dumpDirectory = std::getenv("FOLIA_MATH_SCENE_DUMP_DIR")) {
        const auto base = std::filesystem::path{dumpDirectory} / std::string{name};
        const auto writeSvg = [&](std::filesystem::path const &path, std::string_view value) {
            std::ofstream output(path, std::ios::binary | std::ios::trunc);
            if (!output) {
                std::cerr << "Common-to-Scene could not open SVG dump " << path.string() << '\n';
                return;
            }
            output.write(value.data(), static_cast<std::streamsize>(value.size()));
            if (!output)
                std::cerr << "Common-to-Scene failed writing SVG dump " << path.string() << '\n';
        };
        writeSvg(base.string() + ".expected.svg", expected);
        writeSvg(base.string() + ".actual.svg", actual);
    }
    std::size_t at = 0;
    while (at < expected.size() && at < actual.size() && expected[at] == actual[at])
        ++at;
    const auto begin = at > 48 ? at - 48 : 0;
    std::cerr << "Common-to-Scene SVG mismatch case=" << name << " first_byte=" << at
              << " expected_size=" << expected.size() << " actual_size=" << actual.size()
              << "\nexpected: " << expected.substr(begin, 112)
              << "\nactual:   " << actual.substr(begin, 112) << '\n';
}

struct SceneStats {
    std::size_t groups{};
    std::size_t glyphs{};
    std::size_t paths{};
    std::size_t rules{};
    std::size_t ellipses{};
    std::size_t invalid_ellipses{};
    std::size_t text_runs{};
    std::size_t typed_fonts{};
    std::size_t typed_paints{};
    std::size_t phantom_groups{};
    std::size_t phantom_glyphs{};
    std::size_t phantom_entries{};
    std::size_t table_nodes{};
    std::size_t table_frame_roles{};
    std::size_t table_vertical_roles{};
    std::size_t table_horizontal_roles{};
    std::size_t table_viewports{};
    std::size_t table_unclipped_viewports{};
    std::size_t table_invalid_viewports{};
    std::size_t table_rules{};
    std::size_t table_invalid_paints{};
    std::size_t table_line_attributes{};
    std::size_t table_frame_attributes{};
    std::size_t table_label_attributes{};
    std::size_t table_class_attributes{};
    std::size_t table_background_rules{};
    std::size_t table_invalid_backgrounds{};
    std::size_t vertical_extender_roles{};
    std::size_t vertical_extender_viewports{};
    std::size_t horizontal_extender_roles{};
    std::size_t horizontal_extender_viewports{};
};

bool isTableRole(scene::ElementRole role) {
    return role == scene::ElementRole::table_frame ||
           role == scene::ElementRole::table_vertical_line ||
           role == scene::ElementRole::table_horizontal_line;
}

bool isTableViewportRole(scene::ElementRole role) {
    return role == scene::ElementRole::table_content_viewport ||
           role == scene::ElementRole::table_label_frame;
}

void inspectTableMetadata(scene::SemanticMetadata const &metadata, SceneStats &stats) {
    for (const auto &attribute : metadata.ordered_attributes) {
        if (attribute.name == "data-line")
            ++stats.table_line_attributes;
        else if (attribute.name == "data-frame")
            ++stats.table_frame_attributes;
        else if (attribute.name == "data-labels")
            ++stats.table_label_attributes;
        else if (attribute.name == "class")
            ++stats.table_class_attributes;
    }
}

void inspectTableRole(scene::ElementRole role, SceneStats &stats) {
    if (!isTableRole(role))
        return;
    if (role == scene::ElementRole::table_frame)
        ++stats.table_frame_roles;
    else if (role == scene::ElementRole::table_vertical_line)
        ++stats.table_vertical_roles;
    else
        ++stats.table_horizontal_roles;
}

void inspectTableRule(scene::ElementRole role, scene::Paint const &paint, SceneStats &stats) {
    if (!isTableRole(role))
        return;
    ++stats.table_rules;
    if (!std::holds_alternative<scene::NoPaint>(paint.fill) ||
        !std::holds_alternative<scene::InheritedPaint>(paint.stroke) ||
        !std::isfinite(paint.stroke_width) || std::abs(paint.stroke_width - 0.07) > 1e-9)
        ++stats.table_invalid_paints;
    if (paint.dash_array) {
        const auto &dash = *paint.dash_array;
        const bool dashed = dash.size() == 1 && std::abs(dash[0] - 0.14) <= 1e-9;
        const bool dotted =
            dash.size() == 2 && std::abs(dash[0]) <= 1e-9 && std::abs(dash[1] - 0.14) <= 1e-9;
        if (!dashed && !dotted)
            ++stats.table_invalid_paints;
        if (dotted && paint.line_cap != scene::StrokeCap::round)
            ++stats.table_invalid_paints;
    }
}

void inspect(scene::Group const &group, SceneStats &stats, bool insidePhantom = false) {
    ++stats.groups;
    if (group.metadata.source_tag == "mtable")
        ++stats.table_nodes;
    inspectTableMetadata(group.metadata, stats);
    inspectTableRole(group.role, stats);
    if (isTableViewportRole(group.role) && group.viewport) {
        ++stats.table_viewports;
        if (!group.viewport->clip_content)
            ++stats.table_unclipped_viewports;
        const auto &bounds = group.viewport->bounds;
        const auto &viewBox = group.viewport->view_box;
        if (!std::isfinite(bounds.x) || !std::isfinite(bounds.y) || !std::isfinite(bounds.width) ||
            !std::isfinite(bounds.height) || !std::isfinite(viewBox.x) ||
            !std::isfinite(viewBox.y) || !std::isfinite(viewBox.width) ||
            !std::isfinite(viewBox.height) || bounds.width <= 0 || bounds.height <= 0 ||
            viewBox.width <= 0 || viewBox.height <= 0)
            ++stats.table_invalid_viewports;
    }
    const bool isPhantom = insidePhantom || group.metadata.source_tag == "mphantom";
    if (group.metadata.source_tag == "mphantom")
        ++stats.phantom_groups;
    if (isPhantom)
        stats.phantom_entries += group.entries.size();
    if (group.role == scene::ElementRole::vertical_extender_viewport) {
        ++stats.vertical_extender_roles;
        stats.vertical_extender_viewports += group.viewport.has_value();
    } else if (group.role == scene::ElementRole::horizontal_extender_viewport) {
        ++stats.horizontal_extender_roles;
        stats.horizontal_extender_viewports += group.viewport.has_value();
    }
    for (auto const &entry : group.entries) {
        if (auto const *child = std::get_if<std::shared_ptr<scene::Group const>>(&entry)) {
            if (*child)
                inspect(**child, stats, isPhantom);
            continue;
        }
        auto const &primitive = std::get<scene::Primitive>(entry);
        std::visit(
            [&](auto const &value) {
                using T = std::remove_cvref_t<decltype(value)>;
                if constexpr (std::same_as<T, scene::GlyphRun>) {
                    for (auto const &glyph : value.glyphs) {
                        ++stats.glyphs;
                        if (isPhantom)
                            ++stats.phantom_glyphs;
                        if (glyph.resource && !glyph.key.variant.empty())
                            ++stats.typed_fonts;
                        ++stats.typed_paints;
                    }
                } else if constexpr (std::same_as<T, scene::Path>) {
                    ++stats.paths;
                    inspectTableMetadata(value.metadata, stats);
                    inspectTableRole(value.role, stats);
                    inspectTableRule(value.role, value.paint, stats);
                    ++stats.typed_paints;
                } else if constexpr (std::same_as<T, scene::Rule>) {
                    ++stats.rules;
                    inspectTableMetadata(value.metadata, stats);
                    inspectTableRole(value.role, stats);
                    inspectTableRule(value.role, value.paint, stats);
                    const bool tableBackground =
                        value.role == scene::ElementRole::background_box ||
                        std::any_of(
                            value.metadata.ordered_attributes.begin(),
                            value.metadata.ordered_attributes.end(),
                            [](const auto &attribute) { return attribute.name == "data-bgcolor"; });
                    if (tableBackground) {
                        ++stats.table_background_rules;
                        const bool validBounds = std::isfinite(value.bounds.x) &&
                                                 std::isfinite(value.bounds.y) &&
                                                 std::isfinite(value.bounds.width) &&
                                                 std::isfinite(value.bounds.height) &&
                                                 value.bounds.width > 0 && value.bounds.height > 0;
                        const bool colorFill =
                            std::holds_alternative<scene::CssColor>(value.paint.fill) ||
                            std::holds_alternative<scene::Color>(value.paint.fill);
                        const bool noExplicitStroke =
                            std::holds_alternative<scene::NoPaint>(value.paint.stroke) ||
                            std::holds_alternative<scene::InheritedPaint>(value.paint.stroke);
                        if (!validBounds || !colorFill || !noExplicitStroke)
                            ++stats.table_invalid_backgrounds;
                    }
                    ++stats.typed_paints;
                } else if constexpr (std::same_as<T, scene::Ellipse>) {
                    ++stats.ellipses;
                    inspectTableMetadata(value.metadata, stats);
                    inspectTableRole(value.role, stats);
                    const bool validGeometry =
                        std::isfinite(value.center.x) && std::isfinite(value.center.y) &&
                        std::isfinite(value.radius_x) && std::isfinite(value.radius_y) &&
                        value.radius_x >= 0 && value.radius_y >= 0;
                    const bool validPaint = std::isfinite(value.paint.stroke_width) &&
                                            std::isfinite(value.paint.opacity) &&
                                            value.paint.stroke_width >= 0 &&
                                            value.paint.opacity >= 0 && value.paint.opacity <= 1;
                    if (!validGeometry || !validPaint)
                        ++stats.invalid_ellipses;
                    ++stats.typed_paints;
                } else if constexpr (std::same_as<T, scene::TextRun>) {
                    ++stats.text_runs;
                    if (value.font && !value.font->family.empty())
                        ++stats.typed_fonts;
                    ++stats.typed_paints;
                }
            },
            primitive);
    }
}

CommonSceneCase const &sourceCase(std::string_view name) {
    auto found = std::find_if(commonSceneCases.begin(), commonSceneCases.end(),
                              [&](auto const &item) { return item.name == name; });
    if (found == commonSceneCases.end())
        throw std::logic_error("missing pinned Common-to-Scene Source case");
    return *found;
}

std::size_t sourceMathSvgCount(std::string_view svg) {
    std::size_t count = 0;
    std::size_t cursor = 0;
    while ((cursor = svg.find("<svg", cursor)) != std::string_view::npos) {
        const auto end = svg.find('>', cursor);
        if (end == std::string_view::npos)
            break;
        const auto tag = svg.substr(cursor, end - cursor);
        if (tag.find("role=\"img\"") != std::string_view::npos)
            ++count;
        cursor = end + 1;
    }
    return count;
}

std::size_t sourceMmlTableCount(std::string_view svg) {
    constexpr std::string_view needle = "data-mml-node=\"mtable\"";
    std::size_t count = 0, cursor = 0;
    while ((cursor = svg.find(needle, cursor)) != std::string_view::npos) {
        ++count;
        cursor += needle.size();
    }
    return count;
}

bool sourceHasNestedTableViewport(std::string_view svg) {
    for (auto position = svg.find("<svg"); position != std::string_view::npos;
         position = svg.find("<svg", position + 4)) {
        const auto end = svg.find('>', position);
        if (end == std::string_view::npos)
            break;
        const auto tag = svg.substr(position, end - position);
        if (tag.find("data-table=\"true\"") != std::string_view::npos ||
            tag.find("data-labels=\"true\"") != std::string_view::npos)
            return true;
    }
    return false;
}

std::size_t sourceNestedAssemblyPieceCount(std::string_view svg, bool vertical) {
    std::size_t count = 0;
    std::size_t cursor = 0;
    while ((cursor = svg.find("<svg", cursor)) != std::string_view::npos) {
        const auto tagEnd = svg.find('>', cursor);
        if (tagEnd == std::string_view::npos)
            break;
        const auto tag = svg.substr(cursor, tagEnd - cursor);
        cursor = tagEnd + 1;
        if (tag.find("role=\"img\"") != std::string_view::npos)
            continue;
        const auto close = svg.find("</svg>", tagEnd);
        if (close == std::string_view::npos)
            continue;
        const auto piece = svg.substr(cursor, close - cursor);
        if (piece.find("<path data-c=") == std::string_view::npos)
            continue;
        if (vertical) {
            if (piece.find("transform=\"scale(1,") != std::string_view::npos)
                ++count;
        } else {
            const auto scale = piece.find("transform=\"scale(");
            if (scale == std::string_view::npos)
                continue;
            const auto comma = piece.find(',', scale);
            const auto closeParen = piece.find(')', comma);
            if (comma != std::string_view::npos && closeParen != std::string_view::npos &&
                piece.substr(comma + 1, closeParen - comma - 1) == "1")
                ++count;
        }
    }
    return count;
}
} // namespace

suite common_scene_source = [] {
    "real_TeX_to_MML_to_standard_Common_to_frozen_Scene_matches_pinned_Source_SVG"_test = [] {
        InputOwner owner;
        NoUnexpectedMeasurement measurement;
        LiteUnknownTextMeasurement canonicalMeasurement(owner.adaptor);
        // Stage coverage is deliberately limited to focused currently emitted
        // Common families, table branches, and isolated script-family probes.
        // Other pinned Source cases remain outstanding until their families
        // emit Scene nodes. This is not the required 75/150 full-profile gate.
        constexpr std::array<std::string_view, 86> selected{
            "plain",
            "mathbb",
            "fraction-inline",
            "fraction-display",
            "root",
            "indexed-root",
            "scene-identifiers-numbers",
            "scene-fixed-operator",
            "scene-assembled-operator",
            "scene-script-sub",
            "scene-script-sup",
            "scene-script-subsup",
            "scene-script-nested",
            "scene-sum-limits-inline",
            "scene-sum-limits-display",
            "scene-overline-single",
            "scene-widehat-single",
            "scene-prescript-sup-slot",
            "scene-prescript-sub-slot",
            "scene-prescript-both-slots",
            "text",
            "inline-linebreak-narrow",
            "scene-inline-break-before",
            "scene-inline-break-after",
            "scene-text-wordbreak",
            "scene-zero-size-frame",
            "scene-vertical-extender-assembly",
            "scene-horizontal-extender-assembly",
            "bbox",
            "phantom",
            "base-smash-raise",
            "base-phantom-sides",
            "scene-simple-phantom",
            "scene-padding-negative-dw-center",
            "scene-padding-negative-dw-right",
            "matrix",
            "colortbl-row-cell",
            "display-linebreak-table",
            "scene-table-array-basic",
            "scene-table-array-column-rules",
            "scene-table-array-row-rules",
            "scene-table-array-cell-padding",
            "scene-table-array-paragraph-column",
            "scene-table-row-dashed",
            "scene-table-column-dashed",
            "scene-table-label-right",
            "scene-table-label-left-metric",
            "scene-table-nested-label",
            "scene-enclose-all-lines",
            "scene-enclose-paired-sides",
            "scene-enclose-radical-top",
            "cancel-lines",
            "enclose-notations",
            "scene-enclose-notation-top",
            "scene-enclose-notation-right",
            "scene-enclose-notation-bottom",
            "scene-enclose-notation-left",
            "scene-enclose-notation-actuarial",
            "scene-enclose-notation-madruwb",
            "scene-enclose-notation-updiagonalstrike",
            "scene-enclose-notation-downdiagonalstrike",
            "scene-enclose-notation-horizontalstrike",
            "scene-enclose-notation-verticalstrike",
            "scene-enclose-notation-box",
            "scene-enclose-notation-roundedbox",
            "scene-enclose-notation-circle",
            "scene-enclose-notation-phasorangle",
            "scene-enclose-notation-uparrow",
            "scene-enclose-notation-downarrow",
            "scene-enclose-notation-leftarrow",
            "scene-enclose-notation-rightarrow",
            "scene-enclose-notation-updownarrow",
            "scene-enclose-notation-leftrightarrow",
            "scene-enclose-notation-updiagonalarrow",
            "scene-enclose-notation-northeastarrow",
            "scene-enclose-notation-southeastarrow",
            "scene-enclose-notation-northwestarrow",
            "scene-enclose-notation-southwestarrow",
            "scene-enclose-notation-northeastsouthwestarrow",
            "scene-enclose-notation-northwestsoutheastarrow",
            "scene-enclose-notation-longdiv",
            "scene-enclose-notation-radical",
            "scene-enclose-boundary-roundedbox-empty",
            "scene-enclose-boundary-uparrow-left",
            "scene-enclose-boundary-rightarrow-top",
            "scene-enclose-boundary-arrow-custom-parameters"};
        const char *profileEnvironment = std::getenv("FOLIA_MATH_SCENE_PROFILE");
        bool canonicalProfile = false;
        if (profileEnvironment)
            canonicalProfile = std::string_view(profileEnvironment) == "canonical";
        if (profileEnvironment && !canonicalProfile) {
            std::cerr << "Common-to-Scene profile=unsupported category=unsupported_profile value="
                      << std::quoted(profileEnvironment) << '\n';
            expect(false) << "unsupported FOLIA_MATH_SCENE_PROFILE value";
            return;
        }
        std::vector<CommonSceneCase const *> executionCases;
        if (canonicalProfile) {
            if (commonSceneCases.size() < 75) {
                std::cerr << "Common-to-Scene profile=canonical category=fixture_too_short count="
                          << commonSceneCases.size() << '\n';
                expect(false) << "canonical Source fixture must contain the original 75 cases";
                return;
            }
            for (std::size_t index = 0; index < 75; ++index)
                executionCases.push_back(&commonSceneCases[index]);
        } else {
            for (auto name : selected)
                executionCases.push_back(&sourceCase(name));
        }
        for (auto const *sourceCaseValue : executionCases) {
            const auto &source = *sourceCaseValue;
            const auto name = source.name;
            try {
                const bool sourceOk = source.error_name.empty();
                if (!sourceOk) {
                    expect(false) << "Source conversion failed for case " << name << ": "
                                  << source.error_name << ": " << source.error_message;
                    if (!canonicalProfile) {
                        std::cerr << "Common-to-Scene case=" << name
                                  << " result=mismatch source-error=" << source.error_name << ": "
                                  << source.error_message << '\n';
                        continue;
                    }
                }
                auto &caseMeasurement =
                    canonicalProfile
                        ? static_cast<common::MeasurementTarget &>(canonicalMeasurement)
                        : static_cast<common::MeasurementTarget &>(measurement);
                auto completed = buildScene(source, owner, caseMeasurement, canonicalProfile);
                if (!sourceOk) {
                    const auto actual = serialize(completed, owner);
                    (void)actual;
                    std::cerr
                        << "Common-to-Scene profile=canonical case=" << name
                        << " category=source_error_native_success result=failed source_error_name="
                        << std::quoted(std::string{source.error_name}) << " source_error_message="
                        << std::quoted(std::string{source.error_message})
                        << " native_outcome=scene_returned_without_source_svg\n";
                    continue;
                }
                SceneStats stats;
                inspect(completed.root(), stats);
                const bool sourceHasGlyphs = source.svg.find("data-c=") != std::string_view::npos;
                const bool sourceHasDrawing =
                    sourceHasGlyphs || source.svg.find("<path") != std::string_view::npos ||
                    source.svg.find("<rect") != std::string_view::npos ||
                    source.svg.find("<line") != std::string_view::npos ||
                    source.svg.find("<ellipse") != std::string_view::npos ||
                    source.svg.find("<polygon") != std::string_view::npos;
                const bool sourceHasTable =
                    source.svg.find("data-mml-node=\"mtable\"") != std::string_view::npos;
                const bool sourceHasVerticalTableLine =
                    source.svg.find("data-line=\"v\"") != std::string_view::npos;
                const bool sourceHasHorizontalTableLine =
                    source.svg.find("data-line=\"h\"") != std::string_view::npos;
                const bool sourceHasDashedTableLine =
                    source.svg.find("stroke-dasharray=") != std::string_view::npos;
                bool sourceHasTableFrameMetadata = false;
                for (auto attribute = source.svg.find("data-frame=\"true\"");
                     attribute != std::string_view::npos;
                     attribute = source.svg.find("data-frame=\"true\"", attribute + 18)) {
                    const auto tagStart = source.svg.rfind('<', attribute);
                    if (tagStart == std::string_view::npos ||
                        source.svg.substr(tagStart, attribute - tagStart).starts_with("<rect"))
                        continue;
                    sourceHasTableFrameMetadata = true;
                    break;
                }
                bool sourceHasTableFrameRect = false;
                for (auto rect = source.svg.find("<rect"); rect != std::string_view::npos;
                     rect = source.svg.find("<rect", rect + 5)) {
                    const auto end = source.svg.find('>', rect);
                    if (end == std::string_view::npos)
                        break;
                    if (source.svg.substr(rect, end - rect).find("data-frame=\"true\"") !=
                        std::string_view::npos) {
                        sourceHasTableFrameRect = true;
                        break;
                    }
                }
                const bool sourceHasTableFrame =
                    sourceHasTableFrameRect || sourceHasTableFrameMetadata;
                const bool sourceHasTableLabels =
                    source.svg.find("data-labels=\"true\"") != std::string_view::npos;
                const bool sourceHasTableLineMetadata =
                    source.svg.find("data-line=\"") != std::string_view::npos;
                const bool sourceHasTableClassMetadata =
                    source.svg.find("class=\"mjx-") != std::string_view::npos;
                const bool sourceHasTableBackground =
                    source.svg.find("data-bgcolor=\"true\"") != std::string_view::npos;
                const bool sourceHasPercentageTable =
                    source.svg.find("width=\"100%\"") != std::string_view::npos &&
                    source.svg.find("data-mjx-viewBox=") != std::string_view::npos;
                const bool requiresTableViewport = sourceHasNestedTableViewport(source.svg);
                const auto &metrics = completed.metrics();
                const bool completedMetrics = std::isfinite(metrics.width) &&
                                              std::isfinite(metrics.height) &&
                                              std::isfinite(metrics.depth);
                const auto &frame = completed.frame_layout();
                bool frameMetrics = std::isfinite(frame.minimum_extent) && frame.minimum_extent > 0;
                const auto sourceFragmentCount = sourceMathSvgCount(source.svg);
                const bool frameFragmentsMatchSource =
                    sourceFragmentCount > 1 ? frame.inline_fragments.size() == sourceFragmentCount
                                            : frame.inline_fragments.empty();
                bool fragmentRootsOwned = true;
                for (auto const &fragment : frame.inline_fragments) {
                    frameMetrics = frameMetrics && std::isfinite(fragment.metrics.width) &&
                                   std::isfinite(fragment.metrics.height) &&
                                   std::isfinite(fragment.metrics.depth);
                    bool ownedByRoot = false;
                    for (auto const &entry : completed.root().entries) {
                        if (auto const *group =
                                std::get_if<std::shared_ptr<scene::Group const>>(&entry))
                            ownedByRoot = ownedByRoot || (*group && (*group)->id == fragment.root);
                    }
                    fragmentRootsOwned = fragmentRootsOwned && ownedByRoot;
                }
                bool extenderRolesMatchSource = true;
                if (name == "scene-vertical-extender-assembly") {
                    const auto sourcePieces = sourceNestedAssemblyPieceCount(source.svg, true);
                    extenderRolesMatchSource = sourcePieces > 0 &&
                        stats.vertical_extender_roles == sourcePieces &&
                        stats.vertical_extender_viewports == sourcePieces;
                } else if (name == "scene-horizontal-extender-assembly") {
                    const auto sourcePieces = sourceNestedAssemblyPieceCount(source.svg, false);
                    extenderRolesMatchSource = sourcePieces > 0 &&
                        stats.horizontal_extender_roles == sourcePieces &&
                        stats.horizontal_extender_viewports == sourcePieces;
                }
                const bool typedSceneOk =
                    stats.groups > 0 && completedMetrics && frameMetrics && fragmentRootsOwned &&
                    frameFragmentsMatchSource && extenderRolesMatchSource &&
                    stats.invalid_ellipses == 0 &&
                    (!sourceHasGlyphs || (stats.glyphs > 0 && stats.typed_fonts > 0)) &&
                    (!sourceHasDrawing || stats.typed_paints > 0);
                bool phantomEmissionOk = true;
                if (name == "scene-simple-phantom") {
                    phantomEmissionOk = !sourceHasGlyphs && stats.phantom_groups > 0 &&
                                        stats.phantom_glyphs == 0 && stats.phantom_entries == 0 &&
                                        metrics.width > 0 && metrics.height + metrics.depth > 0;
                }
                bool tableStructureOk = true;
                if (sourceHasTable) {
                    tableStructureOk = stats.table_nodes > 0;
                    const auto sourceTableCount = sourceMmlTableCount(source.svg);
                    if (sourceTableCount > 1)
                        tableStructureOk =
                            tableStructureOk && stats.table_nodes >= sourceTableCount;
                    if (sourceHasVerticalTableLine)
                        tableStructureOk = tableStructureOk && stats.table_vertical_roles > 0;
                    if (sourceHasHorizontalTableLine)
                        tableStructureOk = tableStructureOk && stats.table_horizontal_roles > 0;
                    if (sourceHasDashedTableLine || sourceHasTableFrame)
                        tableStructureOk = tableStructureOk && stats.table_rules > 0;
                    if (sourceHasTableLabels)
                        tableStructureOk = tableStructureOk && stats.table_label_attributes > 0;
                    if (sourceHasTableLineMetadata)
                        tableStructureOk = tableStructureOk && stats.table_line_attributes > 0;
                    if (sourceHasTableFrameMetadata)
                        tableStructureOk = tableStructureOk && stats.table_frame_attributes > 0;
                    if (sourceHasTableClassMetadata)
                        tableStructureOk = tableStructureOk && stats.table_class_attributes > 0;
                    if (sourceHasTableBackground)
                        tableStructureOk = tableStructureOk && stats.table_background_rules > 0 &&
                                           stats.table_invalid_backgrounds == 0;
                    if (sourceHasPercentageTable) {
                        tableStructureOk = tableStructureOk && frame.percentage.has_value();
                    }
                    if (requiresTableViewport) {
                        tableStructureOk =
                            tableStructureOk && stats.table_viewports > 0 &&
                            stats.table_unclipped_viewports == stats.table_viewports &&
                            stats.table_invalid_viewports == 0;
                    }
                    tableStructureOk = tableStructureOk && stats.table_invalid_paints == 0;
                }
                expect(bool(stats.groups > 0)) << "no real Scene groups for " << name;
                expect(bool(completedMetrics)) << "non-finite completed Scene metrics for " << name;
                expect(bool(frameMetrics))
                    << "invalid completed frame or fragment metrics for " << name;
                expect(bool(frameFragmentsMatchSource))
                    << "Scene frame fragment count differs from Source SVG segments for " << name;
                expect(bool(stats.invalid_ellipses == 0))
                    << "Scene ellipse has invalid geometry or paint for " << name;
                expect(bool(fragmentRootsOwned))
                    << "inline fragments are not owned by the Scene root for " << name;
                expect(bool(extenderRolesMatchSource))
                    << "Source extender-piece count differs from typed Scene viewport roles for "
                    << name << " (vertical=" << stats.vertical_extender_roles << "/"
                    << stats.vertical_extender_viewports << ", horizontal="
                    << stats.horizontal_extender_roles << "/"
                    << stats.horizontal_extender_viewports << ")";
                if (name == "scene-simple-phantom")
                    expect(bool(phantomEmissionOk)) << "phantom must keep nonzero measured "
                                                       "dimensions but emit no hidden drawing for "
                                                    << name << " (groups=" << stats.phantom_groups
                                                    << ", entries=" << stats.phantom_entries
                                                    << ", glyphs=" << stats.phantom_glyphs << ')';
                expect(bool(tableStructureOk))
                    << "Source table attrs/lines/labels differ from typed Scene table roles for "
                    << name << " (nodes=" << stats.table_nodes
                    << ", frame=" << stats.table_frame_roles
                    << ", vertical=" << stats.table_vertical_roles
                    << ", horizontal=" << stats.table_horizontal_roles
                    << ", viewport=" << stats.table_viewports
                    << ", unclipped=" << stats.table_unclipped_viewports
                    << ", rules=" << stats.table_rules
                    << ", invalid_paint=" << stats.table_invalid_paints
                    << ", line_attrs=" << stats.table_line_attributes
                    << ", frame_attrs=" << stats.table_frame_attributes
                    << ", label_attrs=" << stats.table_label_attributes
                    << ", backgrounds=" << stats.table_background_rules
                    << ", invalid_backgrounds=" << stats.table_invalid_backgrounds << ')';
                if (sourceHasGlyphs) {
                    expect(bool(stats.glyphs > 0))
                        << "Source has glyphs but Scene has none for " << name;
                    expect(bool(stats.typed_fonts > 0))
                        << "Source has glyphs but Scene has no typed font resources for " << name;
                }
                if (sourceHasDrawing)
                    expect(bool(stats.typed_paints > 0))
                        << "Source has drawing but Scene has no typed paint for " << name;
                const auto actual = serialize(completed, owner);
                reportFirstDifference(name, source.svg, actual);
                const bool svgMatches = actual == source.svg;
                expect(bool(svgMatches)) << "Source SVG bytes differ for " << name;
                const bool casePassed =
                    typedSceneOk && phantomEmissionOk && tableStructureOk && svgMatches;
                if (canonicalProfile)
                    std::cerr << "Common-to-Scene profile=canonical case=" << name << " category="
                              << (casePassed   ? "source_success_svg_equal"
                                  : svgMatches ? "scene_validation_mismatch"
                                               : "source_success_svg_mismatch")
                              << " result=" << (casePassed ? "success" : "failed");
                else
                    std::cerr << "Common-to-Scene case=" << name
                              << " result=" << (casePassed ? "success" : "mismatch");
                std::cerr << " viewport_roles(vertical=" << stats.vertical_extender_roles
                          << ",horizontal=" << stats.horizontal_extender_roles << ')'
                          << " phantom(groups=" << stats.phantom_groups
                          << ",entries=" << stats.phantom_entries
                          << ",glyphs=" << stats.phantom_glyphs << ')'
                          << " table(nodes=" << stats.table_nodes
                          << ",frame=" << stats.table_frame_roles
                          << ",vertical=" << stats.table_vertical_roles
                          << ",horizontal=" << stats.table_horizontal_roles
                          << ",viewport=" << stats.table_viewports
                          << ",class_attrs=" << stats.table_class_attributes
                          << ",backgrounds=" << stats.table_background_rules << ')' << '\n';
            } catch (std::exception const &error) {
                if (canonicalProfile) {
                    std::cerr << "Common-to-Scene profile=canonical case=" << name << " category="
                              << (source.error_name.empty() ? "source_success_native_exception"
                                                            : "source_error_native_exception")
                              << " result=failed source_error_name="
                              << std::quoted(std::string{source.error_name})
                              << " source_error_message="
                              << std::quoted(std::string{source.error_message})
                              << " native_exception_type=" << std::quoted(typeid(error).name())
                              << " native_exception_message=" << std::quoted(error.what()) << '\n';
                } else {
                    std::cerr << "Common-to-Scene case=" << name
                              << " result=exception: " << error.what() << '\n';
                }
                expect(false) << "Common-to-Scene case " << name << " threw: " << error.what();
            } catch (...) {
                if (canonicalProfile) {
                    std::cerr << "Common-to-Scene profile=canonical case=" << name << " category="
                              << (source.error_name.empty() ? "source_success_native_exception"
                                                            : "source_error_native_exception")
                              << " result=failed source_error_name="
                              << std::quoted(std::string{source.error_name})
                              << " source_error_message="
                              << std::quoted(std::string{source.error_message})
                              << " native_exception_type=non-standard "
                                 "native_exception_message=unavailable\n";
                } else {
                    std::cerr << "Common-to-Scene case=" << name
                              << " result=exception: non-standard exception\n";
                }
                expect(false) << "Common-to-Scene case " << name
                              << " threw a non-standard exception";
            }
        }
    };

    "frozen_Scene_serializes_after_job_and_document_retirement_and_repeat_request"_test = [] {
        InputOwner owner;
        NoUnexpectedMeasurement measurement;
        constexpr std::array<std::string_view, 12> repeatedCases{
            "fraction-inline",
            "plain",
            "scene-inline-break-before",
            "scene-inline-break-after",
            "scene-table-array-row-rules",
            "scene-table-label-right",
            "scene-table-label-left-metric",
            "scene-enclose-radical-top",
            "scene-enclose-boundary-roundedbox-empty",
            "scene-enclose-notation-circle",
            "scene-enclose-notation-longdiv",
            "scene-enclose-notation-northeastsouthwestarrow"};
        for (auto name : repeatedCases) {
            const auto &source = sourceCase(name);
            auto first = buildScene(source, owner, measurement);
            const auto firstSvg = serialize(first, owner);
            auto second = buildScene(source, owner, measurement);
            const auto secondSvg = serialize(second, owner);
            const bool repeatedMatches = firstSvg == secondSvg;
            const bool sourceMatches = firstSvg == source.svg;
            expect(bool(repeatedMatches))
                << "repeated request changed the completed Scene output for " << name;
            expect(bool(sourceMatches))
                << "retired-request Scene differs from independent Source SVG for " << name;
            reportFirstDifference(name, source.svg, firstSvg);
            std::cerr << "Common-to-Scene case=" << name << "-retirement result="
                      << (repeatedMatches && sourceMatches ? "success" : "mismatch") << '\n';
        }
    };
};
