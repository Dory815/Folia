// MathJax 4.1.3 Configuration.ts package selection and collation. Apache-2.0.
module folia.math.input.tex.parser;
import std;

namespace folia::math::tex {
namespace {
template<class Method> bool callable(Method method) {
    if constexpr (std::is_pointer_v<Method>) return method != nullptr;
    else return std::visit([](auto pointer) { return pointer != nullptr; }, method);
}
template<class Method>
std::optional<Processor<Method>> normalize(std::optional<ProcessorSpec<Method>> const& spec, double priority) {
    if (!spec || !callable(spec->method)) return std::nullopt;
    return Processor<Method>{spec->method, spec->priority.value_or(priority)};
}
template<class Method>
std::vector<Processor<Method>> normalize(std::vector<ProcessorSpec<Method>> const& specs, double priority) {
    std::vector<Processor<Method>> result;
    result.reserve(specs.size());
    for (auto const& spec : specs) {
        if (!callable(spec.method)) throw std::invalid_argument("A package processor must be callable");
        result.push_back({spec.method, spec.priority.value_or(priority)});
    }
    return result;
}
template<class Kind, class Value>
void assign(std::vector<std::pair<Kind, Value>>& destination, std::vector<std::pair<Kind, Value>> const& source) {
    for (auto const& entry : source) {
        auto previous = std::ranges::find(destination, entry.first, &std::pair<Kind, Value>::first);
        if (previous == destination.end()) destination.push_back(entry);
        else previous->second = entry.second;
    }
}
}

Configuration::Configuration(std::u16string value, ConfigurationDefinition definition)
    : name(std::move(value)), handler{{HandlerKind::character, {}}, {HandlerKind::delimiter, {}},
        {HandlerKind::macro, {}}, {HandlerKind::environment, {}}},
      fallback(std::move(definition.fallback)), items(std::move(definition.items)),
      tags(std::move(definition.tags)), options(std::move(definition.options)), nodes(definition.nodes),
      priority(definition.priority.value_or(5)), parser(definition.parser.empty() ? u"tex" : std::move(definition.parser)) {
    for (auto& entry : definition.handler) {
        auto existing = std::ranges::find(handler, entry.kind, &HandlerMaps::kind);
        if (existing == handler.end()) throw std::invalid_argument("Unknown native TeX handler domain");
        existing->maps = std::move(entry.maps);
    }
    preprocessors = normalize(definition.preprocessors, priority);
    postprocessors = normalize(definition.postprocessors, priority);
    initMethod = normalize(definition.init, priority);
    configMethod = normalize(definition.config, priority);
}
ConfigurationRef Configuration::create(ConfigurationHandler& registry, std::u16string name,
                                       ConfigurationDefinition definition) {
    auto configuration = ConfigurationRef(new Configuration(name, std::move(definition)));
    registry.set(std::move(name), configuration);
    return configuration;
}
ConfigurationRef Configuration::local(ConfigurationDefinition definition) {
    return ConfigurationRef(new Configuration({}, std::move(definition)));
}
InitMethod Configuration::init() const noexcept { return initMethod ? initMethod->method : nullptr; }
ConfigMethod Configuration::config() const noexcept { return configMethod ? configMethod->method : nullptr; }

void ConfigurationHandler::set(std::u16string name, ConfigurationRef configuration) {
    if (!configuration) throw std::invalid_argument("A package registry entry needs its configuration");
    auto previous = std::ranges::find(maps_, name, &decltype(maps_)::value_type::first);
    if (previous == maps_.end()) maps_.emplace_back(std::move(name), std::move(configuration));
    else previous->second = std::move(configuration);
}
ConfigurationRef ConfigurationHandler::get(std::u16string_view name) const {
    auto entry = std::ranges::find_if(maps_, [&](auto const& item) { return item.first == name; });
    return entry == maps_.end() ? nullptr : entry->second;
}
std::vector<std::u16string> ConfigurationHandler::keys() const {
    std::vector<std::u16string> names;
    names.reserve(maps_.size());
    for (auto const& entry : maps_) names.push_back(entry.first);
    return names;
}

ParserConfiguration::ParserConfiguration(ConfigurationHandler& registry, MapHandler& maps,
    TexSessionState& session, MacroCallbacks const& callbacks, WarningSink& warnings,
    std::span<PackageSelection const> packages, std::vector<std::u16string> parsers)
    : registry_(registry), maps_(maps), session_(session), callbacks_(callbacks), warnings_(warnings),
      parsers_(std::move(parsers)), handlers(maps, warnings) {
    for (auto package = packages.rbegin(); package != packages.rend(); ++package) addPackage(*package);
    for (auto entry : configurations_) append(entry.item, entry.priority);
}
ConfigurationRef ParserConfiguration::getPackage(std::u16string_view name) {
    auto configuration = registry_.get(name);
    if (configuration && std::ranges::find(parsers_, configuration->parser) == parsers_.end()) {
        std::u16string argument(name);
        TexError message("PackageParser", u"Package '%1' doesn't target the proper parser", {&argument, 1});
        throw std::runtime_error(message.what());
    }
    if (!configuration) warnings_.warn(u"MathJax Warning: Package '" + std::u16string(name) + u"' not found.  Omitted.");
    return configuration;
}
void ParserConfiguration::addPackage(PackageSelection const& package) {
    if (auto configuration = getPackage(package.name))
        configurations_.add(configuration, package.priority.value_or(configuration->priority));
}
void ParserConfiguration::append(ConfigurationRef configuration, std::optional<double> supplied) {
    if (!configuration) throw std::invalid_argument("Cannot append an absent package configuration");
    // Source append uses ||, while addPackage uses the supplied tuple priority.
    const double priority = supplied && *supplied != 0 && !std::isnan(*supplied) ? *supplied : configuration->priority;
    if (configuration->initMethod)
        initMethods_.add(configuration->initMethod->method, configuration->initMethod->priority);
    if (configuration->configMethod)
        configMethods_.add(configuration->configMethod->method, configuration->configMethod->priority);
    std::vector<HandlerRegistration> registrations;
    for (auto const& handler : configuration->handler) {
        FallbackHandler fallback{};
        for (auto const& entry : configuration->fallback) if (entry.first == handler.kind) fallback = entry.second;
        registrations.push_back({handler.kind, handler.maps, fallback, priority});
    }
    handlers.add(registrations);
    assign(items, configuration->items);
    for (auto const& tag : configuration->tags) {
        auto previous = std::ranges::find(tags, tag.name, &TagsCreatorEntry::name);
        if (previous == tags.end()) tags.push_back(tag);
        else *previous = tag;
    }
    options.mergeDefaults(configuration->options);
    nodes.merge(configuration->nodes);
}
void ParserConfiguration::init() { (void)initMethods_.execute(*this); }
void ParserConfiguration::addFilters(TeX& jax, Configuration const& configuration) {
    for (auto const& processor : configuration.preprocessors)
        jax.preFilters.add(processor.method, processor.priority);
    for (auto const& processor : configuration.postprocessors)
        jax.postFilters.add(processor.method, processor.priority);
}
void ParserConfiguration::config(TeX& jax) {
    (void)configMethods_.execute(*this, jax);
    for (auto const& entry : configurations_) addFilters(jax, *entry.item);
}
void ParserConfiguration::add(std::u16string_view name, TeX& jax,
                              TexOptionValues const& supplied) {
    auto configuration = getPackage(name);
    append(configuration);
    configurations_.add(configuration, configuration->priority);
    init();
    auto& parser = jax.parseOptions();
    parser.setNodeCreators(configuration->nodes);
    for (auto const& [kind, constructor] : configuration->items)
        parser.itemFactory->setNodeClass(kind, constructor);
    TagsFactory::addTags(*this, configuration->tags);
    parser.options.mergeDefaults(configuration->options);
    parser.options.mergeDefaults(supplied);
    addFilters(jax, *configuration);
    if (auto configure = configuration->config()) configure(*this, jax);
}
}
