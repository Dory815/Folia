// MathJax 4.1.3 input/tex/newcommand/NewcommandConfiguration.ts; Apache-2.0.
module folia.math.input.tex.newcommand.configuration;
import folia.math.input.tex.newcommand.util;
import folia.math.input.tex.parse_methods;
import std;
namespace folia::math::tex {
void NewcommandConfig(ParserConfiguration &config, TeX &jax) {
    auto &options = jax.parseOptions();
    auto current = std::dynamic_pointer_cast<MacroMap>(config.maps().getMap(NEW_COMMAND));
    if (current && current->sessionOwner() == &options.packageData)
        return;
    config.maps().registerMap(std::make_shared<DelimiterMap>(SessionDelimiterMapInit{
        {std::u16string(NEW_DELIMITER), TokenMapParser{&ParseMethods::delimiter}},
        options.packageData}));
    config.maps().registerMap(std::make_shared<CommandMap>(
        SessionMacroMapInit{{std::u16string(NEW_COMMAND), std::nullopt},
                            options.packageData,
                            config.callbacks(),
                            NewcommandMethods::Macro,
                            &NewcommandMethods::MacroWithTemplate}));
    config.maps().registerMap(std::make_shared<EnvironmentMap>(
        SessionEnvironmentMapInit{{std::u16string(NEW_ENVIRONMENT), std::nullopt},
                                  options.packageData,
                                  &ParseMethods::environment,
                                  &NewcommandMethods::BeginEnv}));
    const std::array handlers{
        HandlerRegistration{HandlerKind::character, {}, nullptr, NEWCOMMAND_PRIORITY},
        HandlerRegistration{
            HandlerKind::delimiter, {std::u16string(NEW_DELIMITER)}, nullptr, NEWCOMMAND_PRIORITY},
        HandlerRegistration{HandlerKind::macro,
                            {std::u16string(NEW_DELIMITER), std::u16string(NEW_COMMAND)},
                            nullptr,
                            NEWCOMMAND_PRIORITY},
        HandlerRegistration{HandlerKind::environment,
                            {std::u16string(NEW_ENVIRONMENT)},
                            nullptr,
                            NEWCOMMAND_PRIORITY}};
    options.handlers.add(handlers);
}
ConfigurationRef NewcommandConfiguration::initializeModule(ConfigurationHandler &registry,
                                                           MapHandler &maps) {
    NewcommandMappings::initialize(maps);
    ConfigurationDefinition definition;
    definition.handler = {{HandlerKind::macro, {u"Newcommand-macros"}}};
    definition.items = {
        {StackKind::begin_env,
         StackItemClass{[](StackBuildArguments const &args) -> std::unique_ptr<BaseItem> {
                            return std::make_unique<BeginEnvItem>(args.factory);
                        },
                        [](BaseItem const &item) {
                            return dynamic_cast<BeginEnvItem const *>(&item) != nullptr;
                        }}}};
    definition.options.maxMacros = 1000;
    definition.options.protectedMacros = std::vector<std::u16string>{u"begingroupSandbox"};
    definition.config = ProcessorSpec<ConfigMethod>{&NewcommandConfig};
    return Configuration::create(registry, u"newcommand", std::move(definition));
}
} // namespace folia::math::tex
