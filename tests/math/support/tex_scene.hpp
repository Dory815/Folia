#pragma once

#include <array>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

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

namespace tex_scene_test {
namespace mml = folia::math::mml;
namespace tex = folia::math::tex;
namespace common = folia::math::output::common;
namespace scene = folia::math::output::scene;
namespace svg = folia::math::output::svg;

struct TexSceneInput {
    std::string_view tex;
    bool display{};
    double em{}, ex{}, width{};
    std::string_view tag_side;
};

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

inline std::unique_ptr<common::Linebreaks>
makeLinebreakVisitor(common::CommonWrapperFactory &factory) {
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

inline common::CommonOutputJaxConfig commonConfig(TexSceneInput const &item,
                                                  InputOwner const &owner) {
    common::CommonOptions options;
    options.displayOverflow = "linebreak";
    options.linebreaks.inlineBreaks = true;
    common::ExtendedMetrics metrics{item.em, item.ex, item.width, 1, {}, item.display};
    common::CommonOutputJaxConfig config{metrics, std::move(options), owner.fonts.fonts,
                                         "MathJaxNewcm"};
    // Source SVG::forceInlineBreaks returns the enabled inline-break option.
    config.inlineBreakPolicy = common::InlineBreakPolicy::forced;
    return config;
}

inline common::HtmlMeasurement htmlMeasurement(InputOwner &owner) {
    // Source convert() starts at body; XML measurement attaches its container
    // to that node's actual parent (html), not to a fabricated font metrics host.
    const auto document = owner.adaptor.parse(u"", u"text/html");
    const auto parent = owner.adaptor.parent(owner.adaptor.body(document));
    const std::array<folia::math::AttributeData, 2> attributes{
        {{u"class", u"MathJax"}, {u"jax", u"SVG"}}};
    return {owner.adaptor, parent, owner.adaptor.node(u"mjx-container", attributes)};
}

inline scene::Scene buildSceneFromDocument(mml::MmlDocument &document, TexSceneInput const &item,
                                           InputOwner &owner,
                                           common::MeasurementTarget &measurement) {
    auto config = commonConfig(item, owner);
    config.htmlMeasurement.emplace(htmlMeasurement(owner));
    common::LayoutJob job(document, std::move(config), measurement,
                          svg::SvgWrapperFactory::defaultNodes(), makeLinebreakVisitor);
    return job.buildScene();
}

class NativeOutput final : public folia::math::core::MathOutputJaxService {
    TexSceneInput const &item_;
    InputOwner &owner_;
    common::MeasurementTarget &measurement_;

  public:
    NativeOutput(TexSceneInput const &item, InputOwner &owner,
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
        config.htmlMeasurement.emplace(htmlMeasurement(owner_));
        return common::CommonOutputJax::getBBox(
            document, *math.root, std::move(config), measurement_,
            svg::SvgWrapperFactory::defaultNodes(), makeLinebreakVisitor);
    }
};

inline scene::Scene buildScene(TexSceneInput const &item, InputOwner &owner,
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
                                           owner.warnings, owner.texOptions);
        input = owner.canonicalInput.get();
    } else {
        transientInput = std::make_unique<tex::TeX>(
            owner.configurations, owner.maps, owner.callbacks, owner.warnings, owner.texOptions);
        input = transientInput.get();
    }
    // The canonical Source capture reuses one input jax but supplies each
    // request's configured tag side. The input's parse options refer to this
    // same live option record.
    input->options.tagSide = owner.texOptions.tagSide;
    std::optional<scene::Scene> result;
    {
        // The request owns every expression-time parser/factory borrow through
        // compile, Common layout, and the final immutable Scene snapshot.
        [[maybe_unused]] tex::TeXRequest request(*input, document.factory());
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
        result.emplace(std::move(*expression.typesetRoot));
    }
    if (input->parseOptions().hasRequest())
        throw std::logic_error("TeX request remained bound after scene construction");
    return std::move(*result);
}

} // namespace tex_scene_test
