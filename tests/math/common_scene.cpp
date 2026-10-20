#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include "support/folia_test.hpp"
#include "fixtures/common_scene.inc"
import folia.math.core.math_item;
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

std::unique_ptr<common::Linebreaks> makeLinebreakVisitor(common::CommonWrapperFactory &factory) {
    return std::make_unique<common::LinebreakVisitor>(factory);
}

struct InputOwner {
    folia::math::fonts::NewcmFontDataBundle fonts{folia::math::fonts::makeNewcmFontData()};
    tex::ConfigurationHandler configurations;
    tex::MapHandler maps;
    tex::MacroCallbacks callbacks{{}, {}};
    tex::TexOptionValues texOptions;
    std::shared_ptr<folia::math::handlers::html::HTMLDomStringsOptions> texHtmlOptions =
        std::make_shared<folia::math::handlers::html::HTMLDomStringsOptions>();
    struct Warnings final : tex::WarningSink {
        void warn(std::u16string_view) override {}
    } warnings;
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
                                    InputOwner &owner, NoUnexpectedMeasurement &measurement) {
    common::LayoutJob job(document, commonConfig(item, owner), measurement,
                          common::CommonWrapperFactory::standardNodes(), makeLinebreakVisitor);
    return job.buildScene();
}

class NativeOutput final : public folia::math::core::MathOutputJaxService {
    CommonSceneCase const &item_;
    InputOwner &owner_;
    NoUnexpectedMeasurement &measurement_;

  public:
    NativeOutput(CommonSceneCase const &item, InputOwner &owner,
                 NoUnexpectedMeasurement &measurement) noexcept
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
            common::CommonWrapperFactory::standardNodes(), makeLinebreakVisitor);
    }
};

scene::Scene buildScene(CommonSceneCase const &item, InputOwner &owner,
                        NoUnexpectedMeasurement &measurement) {
    mml::MmlDocument document;
    tex::TeX input(owner.configurations, owner.maps, owner.callbacks, owner.warnings,
                   document.factory(), owner.texOptions);
    NativeOutput output(item, owner, measurement);
    folia::math::core::AbstractMathItem expression(
        folia::math::utf16::decode(item.tex).text, &input,
        item.display ? folia::math::core::MathDisplay::display_math
                     : folia::math::core::MathDisplay::inline_math);
    expression.setMetrics(item.em, item.ex, item.width, 1);
    expression.compile(document, {&output, nullptr});
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
    std::size_t text_runs{};
    std::size_t typed_fonts{};
    std::size_t typed_paints{};
    std::size_t phantom_groups{};
    std::size_t phantom_glyphs{};
    std::size_t phantom_entries{};
    std::size_t vertical_extender_roles{};
    std::size_t vertical_extender_viewports{};
    std::size_t horizontal_extender_roles{};
    std::size_t horizontal_extender_viewports{};
};

void inspect(scene::Group const &group, SceneStats &stats, bool insidePhantom = false) {
    ++stats.groups;
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
                    ++stats.typed_paints;
                } else if constexpr (std::same_as<T, scene::Rule>) {
                    ++stats.rules;
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
        // Stage coverage is deliberately limited to focused currently emitted
        // Common families plus isolated script-family probes. Other pinned
        // Source cases remain outstanding until their families emit Scene
        // nodes. This is not the required 75 Common / 150 byte-exact full-profile gate.
        constexpr std::array<std::string_view, 35> selected{"plain",
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
                                                            "scene-padding-negative-dw-right"};
        for (auto name : selected) {
            try {
                const auto &source = sourceCase(name);
                const bool sourceOk = source.error_name.empty();
                expect(bool(sourceOk)) << "Source conversion failed for selected case " << name;
                if (!sourceOk) {
                    std::cerr << "Common-to-Scene case=" << name
                              << " result=mismatch source-error=" << source.error_name << ": "
                              << source.error_message << '\n';
                    continue;
                }
                auto completed = buildScene(source, owner, measurement);
                SceneStats stats;
                inspect(completed.root(), stats);
                const bool sourceHasGlyphs = source.svg.find("data-c=") != std::string_view::npos;
                const bool sourceHasDrawing = sourceHasGlyphs ||
                                              source.svg.find("<rect") != std::string_view::npos ||
                                              source.svg.find("<line") != std::string_view::npos ||
                                              source.svg.find("<polygon") != std::string_view::npos;
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
                    (!sourceHasGlyphs || (stats.glyphs > 0 && stats.typed_fonts > 0)) &&
                    (!sourceHasDrawing || stats.typed_paints > 0);
                bool phantomEmissionOk = true;
                if (name == "scene-simple-phantom") {
                    phantomEmissionOk = !sourceHasGlyphs && stats.phantom_groups > 0 &&
                                        stats.phantom_glyphs == 0 && stats.phantom_entries == 0 &&
                                        metrics.width > 0 && metrics.height + metrics.depth > 0;
                }
                expect(bool(stats.groups > 0)) << "no real Scene groups for " << name;
                expect(bool(completedMetrics)) << "non-finite completed Scene metrics for " << name;
                expect(bool(frameMetrics))
                    << "invalid completed frame or fragment metrics for " << name;
                expect(bool(frameFragmentsMatchSource))
                    << "Scene frame fragment count differs from Source SVG segments for " << name;
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
                std::cerr << "Common-to-Scene case=" << name << " result="
                          << (typedSceneOk && phantomEmissionOk && svgMatches ? "success"
                                                                              : "mismatch")
                          << " viewport_roles(vertical=" << stats.vertical_extender_roles
                          << ",horizontal=" << stats.horizontal_extender_roles << ')'
                          << " phantom(groups=" << stats.phantom_groups
                          << ",entries=" << stats.phantom_entries
                          << ",glyphs=" << stats.phantom_glyphs << ')' << '\n';
            } catch (std::exception const &error) {
                std::cerr << "Common-to-Scene case=" << name
                          << " result=exception: " << error.what() << '\n';
                expect(false) << "Common-to-Scene case " << name << " threw: " << error.what();
            } catch (...) {
                std::cerr << "Common-to-Scene case=" << name
                          << " result=exception: non-standard exception\n";
                expect(false) << "Common-to-Scene case " << name
                              << " threw a non-standard exception";
            }
        }
    };

    "frozen_Scene_serializes_after_job_and_document_retirement_and_repeat_request"_test = [] {
        InputOwner owner;
        NoUnexpectedMeasurement measurement;
        constexpr std::array<std::string_view, 4> repeatedCases{
            "fraction-inline", "plain", "scene-inline-break-before", "scene-inline-break-after"};
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
