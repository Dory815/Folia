// MathJax 4.1.3 input/tex/textmacros/TextMacrosConfiguration.ts; Apache-2.0.
module folia.math.input.tex.textmacros.configuration;
import std;
namespace folia::math::tex {
namespace {
void textCharacter(TexParser &parser, std::u16string_view character) {
    dynamic_cast<TextParser &>(parser).text() += character;
}
bool replacementMacro(TokenLookup const &lookup, MacroCallbacks const &callbacks) {
    if (auto macro = std::get_if<Macro const *>(&lookup)) {
        auto method = std::get_if<MacroHandler>(&(*macro)->function);
        return method && *method == TextMacrosMethods::Macro;
    }
    if (auto command = std::get_if<CommandDefinition const *>(&lookup)) {
        if (std::holds_alternative<MacroDefinition>((*command)->body))
            return true;
        if (auto alias = std::get_if<NativeMacroAlias>(&(*command)->body))
            return callbacks.macroHandler(alias->handler) == TextMacrosMethods::Macro;
    }
    return false;
}
void textMacro(TexParser &parser, std::u16string_view name) {
    auto &text = dynamic_cast<TextParser &>(parser);
    auto &original = text.texParser();
    const auto macro = original.lookup(HandlerKind::macro, name);
    if (!std::holds_alternative<std::monostate>(macro) &&
        !replacementMacro(macro, original.configuration.callbacks())) {
        const std::u16string command = u"\\" + std::u16string(name);
        text.Error("MathMacro", u"%1 is only supported in math mode", {&command, 1});
    }
    (void)original.parse(HandlerKind::macro, ParseInput{text, std::u16string(name)});
}
std::vector<mml::MmlNode *> internalMath(TexParser &parser, std::u16string_view source,
                                         std::optional<ScriptLevel> level,
                                         std::optional<std::u16string_view> font) {
    auto &data = *parser.configuration.textmacros;
    struct Restore {
        TextMacrosData &data;
        TexParser *previous;
        ~Restore() {
            data.texParser = previous;
        }
    } restore{data, data.texParser};
    if (!dynamic_cast<TextParser *>(&parser))
        data.texParser = &parser;
    data.parseOptions->clear();
    ParserEnvironmentSeed environment;
    if (font && !font->empty())
        environment.environment.math_variant = std::u16string(*font);
    return {TextParser(std::u16string(source), std::move(environment), *data.parseOptions,
                       std::move(level))
                .mml()};
}
void configureTextMacros(ParserConfiguration &configuration, TeX &jax) {
    auto data = std::make_unique<TextMacrosData>();
    data->textConf = std::make_unique<ParserConfiguration>(
        configuration.registry(), configuration.maps(), jax.session(), configuration.callbacks(),
        configuration.warnings(), *jax.options.textmacros->packages,
        std::vector<std::u16string>{u"tex", u"text"});
    data->textConf->init();
    data->parseOptions = std::make_unique<ParseOptions>(
        *data->textConf, SharedTexOptions{jax.options}, jax.session(),
        jax.parseOptions().nodeFactory.mmlFactory(),
        ParseOptions::getVariant(data->textConf->options.mathStyle.value_or(u"TeX")));
    data->textConf->config(jax);
    data->parseOptions->tags = TagsFactory::getDefault(
        *data->parseOptions, data->textConf->defaultTags, data->textConf->tags);
    jax.installTextMacros(std::move(data));
    jax.options.internalMath = &internalMath;
}
void prepareTextMacros(TexFilterData &data) {
    auto &inner = *data.data.textmacros->parseOptions;
    inner.nodeFactory.setMmlFactory(data.data.nodeFactory.mmlFactory());
    inner.clear();
}
} // namespace
ConfigurationRef TextMacrosConfiguration::initializeModule(ConfigurationHandler &registry,
                                                           MapHandler &maps) {
    TextMacrosMappings::initializeModule(maps);
    ConfigurationDefinition base;
    base.parser = u"text";
    base.priority = 1;
    base.handler = {{HandlerKind::character, {u"command", u"text-special"}},
                    {HandlerKind::macro, {u"text-macros"}}};
    base.fallback = {{HandlerKind::character, &textCharacter}, {HandlerKind::macro, &textMacro}};
    const auto items = baseItemClasses();
    for (auto kind : {StackKind::start, StackKind::stop, StackKind::mml, StackKind::style}) {
        const auto found = std::ranges::find(items, kind, &StackItemCatalog::value_type::first);
        base.items.push_back(*found);
    }
    (void)Configuration::create(registry, u"text-base", std::move(base));
    ConfigurationDefinition definition;
    definition.priority = 1;
    definition.config = ProcessorSpec<ConfigMethod>{&configureTextMacros};
    definition.preprocessors.emplace_back(FilterList::Callback{&prepareTextMacros});
    definition.options.textmacros =
        TextMacrosOptions{.packages = std::vector<PackageSelection>{{u"text-base", {}}}};
    return Configuration::create(registry, u"textmacros", std::move(definition));
}
} // namespace folia::math::tex
