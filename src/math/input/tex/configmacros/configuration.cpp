// MathJax 4.1.3 input/tex/configmacros/ConfigMacrosConfiguration.ts; Apache-2.0.
module folia.math.input.tex.configmacros.configuration;
import folia.math.input.tex.newcommand.items;
import folia.math.input.tex.newcommand.methods;
import folia.math.input.tex.parse_methods;
import folia.math.util.js_number;
import std;

namespace folia::math::tex {
namespace {
constexpr std::u16string_view MACROSMAP = u"configmacros-map";
constexpr std::u16string_view ACTIVEMAP = u"configmacros-active-map";
constexpr std::u16string_view ENVIRONMENTMAP = u"configmacros-env-map";

TexArgument argumentCount(double count, bool substitute) {
    if (!substitute)
        return count;
    auto value = js_number::toString(count);
    return std::u16string(value.begin(), value.end());
}
void configmacrosInit(ParserConfiguration &configuration) {
    configuration.maps().registerMap(std::make_shared<MacroMap>(
        MacroMapInit{{std::u16string(ACTIVEMAP), std::nullopt}, {}}));
    configuration.maps().registerMap(std::make_shared<CommandMap>(
        CommandMapInit{MacroMapInit{{std::u16string(MACROSMAP), std::nullopt}, {}}}));
    configuration.maps().registerMap(std::make_shared<EnvironmentMap>(EnvironmentMapInit{
        MacroMapInit{{std::u16string(ENVIRONMENTMAP), std::nullopt}, {}},
        &ParseMethods::environment}));
    ConfigurationDefinition local;
    local.handler = {{HandlerKind::character, {std::u16string(ACTIVEMAP)}},
                     {HandlerKind::macro, {std::u16string(MACROSMAP)}},
                     {HandlerKind::environment, {std::u16string(ENVIRONMENTMAP)}}};
    local.priority = 3;
    configuration.append(Configuration::local(std::move(local)));
}
void setMacros(std::u16string_view name,
               std::optional<std::unordered_map<std::u16string, MacroDefinition>> const &macros,
               TeX &jax) {
    auto handler =
        std::dynamic_pointer_cast<MacroMap>(jax.parseOptions().handlers.retrieve(name));
    if (!handler)
        throw std::logic_error("The configured macro map is missing");
    if (!macros)
        return;
    for (auto const &[command, definition] : *macros) {
        std::vector<TexArgument> arguments{definition.replacement_source,
                                          argumentCount(definition.arity,
                                                        definition.substitute_parameters)};
        MacroHandler method = NewcommandMethods::Macro;
        if (definition.parameter_template) {
            method = &NewcommandMethods::MacroWithTemplate;
            arguments.emplace_back(definition.parameter_prefix);
            for (auto const &delimiter : definition.parameter_delimiters)
                arguments.emplace_back(delimiter);
        } else if (definition.default_first_argument) {
            arguments.emplace_back(*definition.default_first_argument);
        }
        handler->add(command, Macro(command, method, std::move(arguments)));
    }
}
void configEnvironments(TeX &jax) {
    auto handler = std::dynamic_pointer_cast<EnvironmentMap>(
        jax.parseOptions().handlers.retrieve(ENVIRONMENTMAP));
    if (!handler)
        throw std::logic_error("The configured environment map is missing");
    auto const &environments = jax.parseOptions().options.environments;
    if (!environments)
        return;
    for (auto const &[name, definition] : *environments) {
        std::vector<TexArgument> arguments{true, definition.begin_source, definition.end_source,
                                          argumentCount(definition.arity,
                                                        definition.substitute_parameters)};
        if (definition.default_first_argument)
            arguments.emplace_back(*definition.default_first_argument);
        handler->add(name, Macro(name, &NewcommandMethods::BeginEnv, std::move(arguments)));
    }
}
void configmacrosConfig(ParserConfiguration &, TeX &jax) {
    setMacros(ACTIVEMAP, jax.parseOptions().options.active, jax);
    setMacros(MACROSMAP, jax.parseOptions().options.macros, jax);
    configEnvironments(jax);
}
}
ConfigurationRef ConfigMacrosConfiguration::initializeModule(ConfigurationHandler &registry,
                                                           MapHandler &) {
    ConfigurationDefinition definition;
    definition.init = ProcessorSpec<InitMethod>{&configmacrosInit};
    definition.config = ProcessorSpec<ConfigMethod>{&configmacrosConfig};
    definition.items = {
        {StackKind::begin_env,
         StackItemClass{[](StackBuildArguments const &args) -> std::unique_ptr<BaseItem> {
                            return std::make_unique<BeginEnvItem>(args.factory);
                        },
                        [](BaseItem const &item) {
                            return dynamic_cast<BeginEnvItem const *>(&item) != nullptr;
                        }}}};
    definition.options.active.emplace();
    definition.options.macros.emplace();
    definition.options.environments.emplace();
    return Configuration::create(registry, u"configmacros", std::move(definition));
}
}
