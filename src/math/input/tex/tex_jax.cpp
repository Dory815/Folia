// MathJax 4.1.3 input/tex.ts; Apache-2.0.
module folia.math.input.tex.parser;
import folia.math.input.tex.base_configuration;
import folia.math.input.tex.filter_util;
import std;

namespace folia::math::tex {
namespace {
TexOptionValues inputOptions(TexOptionValues const &supplied) {
    auto options = TeX::defaults();
    TexOptionValues core;
    core.preFilters = supplied.preFilters;
    core.postFilters = supplied.postFilters;
    core.findTeX = supplied.findTeX;
    core.packages = supplied.packages;
    core.maxBuffer = supplied.maxBuffer;
    core.maxTemplateSubtitutions = supplied.maxTemplateSubtitutions;
    core.mathStyle = supplied.mathStyle;
    core.formatError = supplied.formatError;
    options.mergeDefaults(core);
    return options;
}
TexOptionValues packageOptions(TexOptionValues supplied) {
    supplied.preFilters.reset();
    supplied.postFilters.reset();
    supplied.findTeX.reset();
    supplied.packages.reset();
    supplied.maxBuffer.reset();
    supplied.maxTemplateSubtitutions.reset();
    supplied.mathStyle.reset();
    supplied.formatError.reset();
    return supplied;
}
mml::MmlNode &formatDefaultError(TeX &jax, TexError const &error) {
    return jax.formatError(error);
}
} // namespace

TexOptionValues TeX::defaults() {
    TexOptionValues values;
    values.preFilters.emplace();
    values.postFilters.emplace();
    values.packages = std::vector<PackageSelection>{{u"base", {}}};
    values.maxBuffer = 5 * 1024;
    values.maxTemplateSubtitutions = 10000;
    values.mathStyle = u"TeX";
    values.formatError = formatDefaultError;
    return values;
}
TeX::TeX(ConfigurationHandler &registry, MapHandler &maps, MacroCallbacks const &callbacks,
         WarningSink &warnings, mml::MmlFactory &factory, TexOptionValues supplied)
    : AbstractInputJax(factory, inputOptions(supplied)) {
    if (!registry.get(u"base"))
        BasePackage::initialize(registry, maps);
    configuration_ = std::make_unique<ParserConfiguration>(registry, maps, session_, callbacks,
                                                           warnings, *options.packages);
    configuration_->init();
    options.mergeDefaults(TagsFactory::defaults());
    auto configured = options;
    configured.mergeDefaults(configuration_->options);
    parseOptions_ = std::make_unique<ParseOptions>(
        *configuration_, options, session_, factory,
        ParseOptions::getVariant(configured.mathStyle.value_or(u"TeX")));
    options.mergeDefaults(packageOptions(std::move(supplied)));
    configuration_->config(*this);
    TagsFactory::setDefault(*configuration_, options.tags.value_or(u"none"));
    parseOptions_->tags =
        TagsFactory::getDefault(*parseOptions_, configuration_->defaultTags, configuration_->tags);
    postFilters.add(FilterUtil::cleanSubSup, -7);
    postFilters.add(FilterUtil::setInherited, -6);
    postFilters.add(FilterUtil::checkScriptlevel, -5);
    postFilters.add(FilterUtil::moveLimits, -4);
    postFilters.add(FilterUtil::cleanStretchy, -3);
    postFilters.add(FilterUtil::cleanAttributes, -2);
    postFilters.add(FilterUtil::combineRelations, -1);
}
TeX::~TeX() = default;
std::string_view TeX::name() const noexcept {
    return "TeX";
}
ParseOptions &TeX::parseOptions() const noexcept {
    return *parseOptions_;
}
ParserConfiguration &TeX::configuration() const noexcept {
    return *configuration_;
}
TexSessionState &TeX::session() noexcept {
    return session_;
}
void TeX::setMmlFactory(mml::MmlFactory &factory) {
    AbstractInputJax::setMmlFactory(factory);
    parseOptions_->nodeFactory.setMmlFactory(factory);
}
void TeX::reset(std::int64_t offset) {
    parseOptions_->clear();
    parseOptions_->tags->reset(offset);
}
mml::MmlNode &TeX::compile(core::MathItem &math, mml::MmlDocument &document) {
    setMmlFactory(document.factory());
    auto &parsed = *parseOptions_;
    parsed.clear();
    parsed.mathItem = &math;
    executeFilters(preFilters, math, document, parsed);
    latex_ = math.math;
    parsed.tags->startEquation(math);
    std::unique_ptr<TexParser> parser;
    mml::MmlNode *node{};
    try {
        ParserEnvironmentSeed seed;
        seed.environment.display = math.isDisplay();
        seed.is_inner = false;
        parser = std::make_unique<TexParser>(latex_, std::move(seed), parsed);
        node = parser->mml();
    } catch (TexError const &error) {
        parsed.error = true;
        node = &(*options.formatError)(*this, error);
    }
    const std::array children{node};
    auto &root = parsed.nodeFactory.makeNode(mml::NodeKind::math, children);
    root.attributes().set(u"data-latex", latex_);
    if (math.isDisplay())
        root.attributes().set(u"display", u"block");
    parsed.tags->finishEquation(math);
    parsed.root = &root;
    executeFilters(postFilters, math, document, parsed);
    if (parser) {
        auto const &width = parser->stack.env().horizontal_size;
        if (width && !width->empty()) {
            root.attributes().set(u"maxwidth", *width);
            root.attributes().set(u"overflow", u"linebreak");
        }
    }
    mathNode_ = parsed.root;
    return *mathNode_;
}
mml::MmlNode &TeX::formatError(TexError const &error) {
    std::u16string message(error.message());
    // Source uses /\n.*/ without the global or dotAll flags: remove only
    // the first LF and the remainder of that line, retaining later lines.
    if (auto begin = message.find(u'\n'); begin != message.npos) {
        auto end = message.find_first_of(u"\n\r\u2028\u2029", begin + 1);
        message.erase(begin, end == message.npos ? message.npos : end - begin);
    }
    return parseOptions_->nodeFactory.makeError(message);
}
} // namespace folia::math::tex
