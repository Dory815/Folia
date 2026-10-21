module folia.math;
import folia.math.adaptors.lite_adaptor;
import folia.math.cancellation;
import folia.math.core.math_item;
import folia.math.fonts.newcm_font_module_provider;
import folia.math.handlers.html.html_dom_strings;
import folia.math.input.tex.parser;
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
import std;

namespace folia::math {
namespace common = output::common;
namespace scene = output::scene;
namespace svg = output::svg;

namespace {
struct Warnings final : tex::WarningSink {
    void warn(std::u16string_view text) override {
        std::cerr << utf16::encode(text) << '\n';
    }
};

std::unique_ptr<common::Linebreaks> makeLinebreakVisitor(common::CommonWrapperFactory &factory) {
    return std::make_unique<common::LinebreakVisitor>(factory);
}

std::vector<tex::PackageSelection> rendererPackages() {
    return {{u"base", {}},         {u"action", {}},     {u"ams", {}},        {u"amscd", {}},
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

class LiteMeasurement final : public common::MeasurementTarget {
    adaptors::lite::LiteAdaptor &adaptor_;

  public:
    explicit LiteMeasurement(adaptors::lite::LiteAdaptor &adaptor) noexcept : adaptor_(adaptor) {}

    common::ExtendedMetrics measureMetrics(common::MetricRequest const &) override {
        throw std::logic_error("pinned NewCM requests must use the supplied font metrics");
    }

    common::UnknownBBox measureUnknownText(common::UnknownTextRequest const &request) override {
        const std::array<DOMNodeRef, 1> children{adaptor_.text(request.text)};
        const auto text = adaptor_.node(u"text", {}, children, u"http://www.w3.org/2000/svg");
        const auto [width, height] = adaptor_.nodeSize(text, 1000, true);
        (void)height;
        return {width, .75, .2};
    }
};

class SceneOutput final : public core::MathOutputJaxService {
    common::FontRepository const &fonts_;
    RenderOptions options_;
    LiteMeasurement measurement_;
    DOMDocumentRef htmlDocument_;
    common::HtmlMeasurement htmlMeasurement_;

    common::CommonOutputJaxConfig config() const;

  public:
    SceneOutput(common::FontRepository const &, adaptors::lite::LiteAdaptor &, RenderOptions);
    scene::Scene typeset(core::MathItem &, mml::MmlDocument &) override;
    scene::Scene escaped(core::MathItem &, mml::MmlDocument &) override;
    BBox getBBox(core::MathItem &, mml::MmlDocument &) override;
};
} // namespace

struct Renderer::State {
    std::thread::id owner = std::this_thread::get_id();
    bool unusable = false;
    fonts::NewcmFontDataBundle fontData = fonts::makeNewcmFontData();
    adaptors::lite::LiteAdaptor adaptor;
    tex::ConfigurationHandler configurations;
    tex::MapHandler maps;
    tex::MacroCallbacks callbacks{{}, {}};
    tex::TexOptionValues texOptions;
    std::shared_ptr<handlers::html::HTMLDomStringsOptions> htmlOptions =
        std::make_shared<handlers::html::HTMLDomStringsOptions>();
    Warnings warnings;
    std::unique_ptr<tex::TeX> input;

    RenderResult render(std::u16string_view, std::stop_token, RenderOptions);
    scene::Scene renderScene(std::u16string_view, std::stop_token, RenderOptions);
    std::u16string decode(std::string_view, std::stop_token, RenderOptions) const;
    void validateOptions(RenderOptions const &) const;

    State() {
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
        tex::TexHtmlConfiguration::initializeModule(configurations, maps, htmlOptions);
        tex::TextcompConfiguration::initializeModule(configurations, maps);
        tex::TextMacrosConfiguration::initializeModule(configurations, maps);
        tex::UnicodeConfiguration::initializeModule(configurations, maps);
        tex::UnitsConfiguration::initializeModule(configurations, maps);
        tex::UpgreekConfiguration::initializeModule(configurations, maps);
        tex::VerbConfiguration::initializeModule(configurations, maps);

        texOptions.packages = rendererPackages();
        texOptions.tagSide = u"right";
        input = std::make_unique<tex::TeX>(configurations, maps, callbacks, warnings, texOptions);
    }

    void checkThread() const {
        if (owner != std::this_thread::get_id())
            throw RenderError("WrongThread", "Renderer used from a non-owning thread");
    }

    output::scene::Scene buildScene(std::u16string_view source, RenderOptions options) {
        cancellation::checkpoint(cancellation::CheckpointStage::TeX);
        mml::MmlDocument document;
        tex::TeXRequest request(*input, document.factory());
        SceneOutput output(fontData.fonts, adaptor, options);
        core::AbstractMathItem expression(std::u16string(source), input.get(),
                                          options.display ? core::MathDisplay::display_math
                                                          : core::MathDisplay::inline_math);
        expression.setMetrics(options.em, options.em * .5, options.containerWidth, 1);
        expression.compile(document, {&output, &adaptor});
        if (!expression.root)
            throw std::logic_error("TeX input produced no MathItem root");
        document.setRoot(*expression.root);
        cancellation::checkpoint(cancellation::CheckpointStage::TeX);
        expression.typeset(output, document);
        cancellation::checkpoint(cancellation::CheckpointStage::Scene);
        if (!expression.typesetRoot)
            throw std::logic_error("Common output did not produce an owned Scene");
        return std::move(*expression.typesetRoot);
    }
};

namespace {
SceneOutput::SceneOutput(common::FontRepository const &fonts, adaptors::lite::LiteAdaptor &adaptor,
                         RenderOptions options)
    : fonts_(fonts), options_(options), measurement_(adaptor),
      htmlDocument_(adaptor.parse(u"", u"text/html")),
      htmlMeasurement_{
          adaptor, adaptor.parent(adaptor.body(htmlDocument_)),
          adaptor.node(u"mjx-container",
                       std::array<AttributeData, 2>{{{u"class", u"MathJax"}, {u"jax", u"SVG"}}})} {}

common::CommonOutputJaxConfig SceneOutput::config() const {
    common::CommonOptions commonOptions;
    commonOptions.displayOverflow = "linebreak";
    commonOptions.linebreaks.inlineBreaks = true;
    common::ExtendedMetrics metrics{options_.em, options_.em * .5, options_.containerWidth, 1,
                                    {},          options_.display};
    common::CommonOutputJaxConfig result{metrics, std::move(commonOptions), fonts_, "MathJaxNewcm"};
    result.inlineBreakPolicy = common::InlineBreakPolicy::forced;
    result.htmlMeasurement.emplace(htmlMeasurement_);
    return result;
}

scene::Scene SceneOutput::typeset(core::MathItem &math, mml::MmlDocument &document) {
    if (!math.root)
        throw std::logic_error("Common-to-Scene typeset requires a compiled MathItem root");
    if (document.root() != math.root)
        document.setRoot(*math.root);
    cancellation::checkpoint(cancellation::CheckpointStage::Layout);
    common::LayoutJob job(document, config(), measurement_, svg::SvgWrapperFactory::defaultNodes(),
                          makeLinebreakVisitor);
    auto result = job.buildScene();
    cancellation::checkpoint(cancellation::CheckpointStage::Scene);
    return result;
}

scene::Scene SceneOutput::escaped(core::MathItem &, mml::MmlDocument &) {
    throw svg::UnsupportedError("escaped math output is outside the TeX renderer contract");
}

BBox SceneOutput::getBBox(core::MathItem &math, mml::MmlDocument &document) {
    if (!math.root)
        throw std::logic_error("Common BBox requires a compiled MathItem root");
    return common::CommonOutputJax::getBBox(document, *math.root, config(), measurement_,
                                            svg::SvgWrapperFactory::defaultNodes(),
                                            makeLinebreakVisitor);
}

} // namespace

RenderResult Renderer::State::render(std::u16string_view source, std::stop_token stop,
                                     RenderOptions options) {
    cancellation::Request request(stop, options.deadline);
    checkThread();
    if (unusable)
        throw RenderError("SessionUnusable", "Renderer session is unusable after cancellation");
    try {
        request.checkpoint();
    } catch (cancellation::Cancelled const &) {
        throw RenderError("Cancelled", "Math rendering cancelled before the request started");
    }
    validateOptions(options);
    cancellation::Scope scope(request);
    try {
        auto completedScene = buildScene(source, options);
        cancellation::checkpoint(cancellation::CheckpointStage::Serialization);
        svg::SvgWriteOptions writeOptions;
        writeOptions.font_cache = svg::FontCacheMode::none;
        writeOptions.font_resources = &fontData.svg_resources;
        auto svgText = svg::write(completedScene, writeOptions);
        request.checkpoint();
        return {std::move(svgText), std::move(completedScene)};
    } catch (cancellation::Cancelled const &) {
        unusable = true;
        throw RenderError("Cancelled", "Math rendering cancelled; renderer session is unusable");
    }
}

scene::Scene Renderer::State::renderScene(std::u16string_view source, std::stop_token stop,
                                          RenderOptions options) {
    cancellation::Request request(stop, options.deadline);
    checkThread();
    if (unusable)
        throw RenderError("SessionUnusable", "Renderer session is unusable after cancellation");
    try {
        request.checkpoint();
    } catch (cancellation::Cancelled const &) {
        throw RenderError("Cancelled", "Math rendering cancelled before the request started");
    }
    validateOptions(options);
    cancellation::Scope scope(request);
    try {
        auto result = buildScene(source, options);
        request.checkpoint();
        return result;
    } catch (cancellation::Cancelled const &) {
        unusable = true;
        throw RenderError("Cancelled", "Math rendering cancelled; renderer session is unusable");
    }
}

void Renderer::State::validateOptions(RenderOptions const &options) const {
    if (!std::isfinite(options.em) || options.em <= 0)
        throw RenderError("InvalidArgument", "Renderer em must be finite and positive");
    if (!std::isfinite(options.containerWidth) || options.containerWidth <= 0)
        throw RenderError("InvalidArgument",
                          "Renderer container width must be finite and positive");
}

std::u16string Renderer::State::decode(std::string_view text, std::stop_token stop,
                                       RenderOptions options) const {
    cancellation::Request request(stop, options.deadline);
    checkThread();
    if (unusable)
        throw RenderError("SessionUnusable", "Renderer session is unusable after cancellation");
    try {
        request.checkpoint();
    } catch (cancellation::Cancelled const &) {
        throw RenderError("Cancelled", "Math rendering cancelled before the request started");
    }
    validateOptions(options);
    cancellation::Scope scope(request);
    try {
        return utf16::decode(text).text;
    } catch (cancellation::Cancelled const &) {
        throw RenderError("Cancelled", "Math rendering cancelled before the request started");
    }
}

Renderer::Renderer() : state_(std::make_unique<State>()) {}
Renderer::~Renderer() = default;

RenderResult Renderer::render(std::u16string_view source, RenderOptions options) {
    return render(source, std::stop_token{}, options);
}
RenderResult Renderer::render(std::string_view source, RenderOptions options) {
    return render(source, std::stop_token{}, options);
}
RenderResult Renderer::render(std::u16string_view source, std::stop_token stop,
                              RenderOptions options) {
    return state_->render(source, stop, options);
}
RenderResult Renderer::render(std::string_view source, std::stop_token stop,
                              RenderOptions options) {
    auto decoded = state_->decode(source, stop, options);
    return render(decoded, stop, options);
}

scene::Scene Renderer::renderScene(std::u16string_view source, RenderOptions options) {
    return renderScene(source, std::stop_token{}, options);
}
scene::Scene Renderer::renderScene(std::string_view source, RenderOptions options) {
    return renderScene(source, std::stop_token{}, options);
}
scene::Scene Renderer::renderScene(std::u16string_view source, std::stop_token stop,
                                   RenderOptions options) {
    return state_->renderScene(source, stop, options);
}
scene::Scene Renderer::renderScene(std::string_view source, std::stop_token stop,
                                   RenderOptions options) {
    auto decoded = state_->decode(source, stop, options);
    return renderScene(decoded, stop, options);
}

std::string Renderer::toSvg(std::u16string_view source, RenderOptions options) {
    return render(source, options).svg;
}
std::string Renderer::toSvg(std::string_view source, RenderOptions options) {
    return render(source, options).svg;
}
std::string Renderer::toSvg(std::u16string_view source, std::stop_token stop,
                            RenderOptions options) {
    return render(source, stop, options).svg;
}
std::string Renderer::toSvg(std::string_view source, std::stop_token stop, RenderOptions options) {
    return render(source, stop, options).svg;
}
} // namespace folia::math
