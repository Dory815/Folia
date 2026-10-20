// MathJax 4.1.3 input/tex/Configuration.ts; Apache-2.0.
export module folia.math.input.tex.parser:configuration;
export import :map_handler;
export import :tags;
import std;

export namespace folia::math::tex {
using InitMethod = void (*)(ParserConfiguration&);
using ConfigMethod = void (*)(ParserConfiguration&, TeX&);
using StackItemConfig = tree::FactoryClassMap<BaseItem, StackKind, StackBuildArguments>;
using TagsConfig = std::vector<TagsCreatorEntry>;
struct HandlerMaps { HandlerKind kind; std::vector<std::u16string> maps; };
using HandlerConfig = std::vector<HandlerMaps>;
using FallbackConfig = std::vector<std::pair<HandlerKind, FallbackHandler>>;

template<class Method> struct ProcessorSpec {
    Method method;
    std::optional<double> priority;
    ProcessorSpec(Method value, std::optional<double> rank = {}) : method(value), priority(rank) {}
};
template<class Method> struct Processor { Method method; double priority; };
struct ConfigurationDefinition {
    HandlerConfig handler;
    FallbackConfig fallback;
    StackItemConfig items;
    TagsConfig tags;
    TexOptionValues options;
    NodeCreators nodes;
    std::vector<ProcessorSpec<FilterList::Callback>> preprocessors, postprocessors;
    std::optional<ProcessorSpec<InitMethod>> init;
    std::optional<ProcessorSpec<ConfigMethod>> config;
    std::optional<double> priority;
    std::u16string parser{u"tex"};
};
using ConfigurationRef = std::shared_ptr<Configuration const>;

class Configuration {
    Configuration(std::u16string, ConfigurationDefinition);
public:
    std::u16string name;
    HandlerConfig handler;
    FallbackConfig fallback;
    StackItemConfig items;
    TagsConfig tags;
    TexOptionValues options;
    NodeCreators nodes;
    std::vector<Processor<FilterList::Callback>> preprocessors, postprocessors;
    std::optional<Processor<InitMethod>> initMethod;
    std::optional<Processor<ConfigMethod>> configMethod;
    double priority;
    std::u16string parser;
    static ConfigurationRef create(ConfigurationHandler&, std::u16string name, ConfigurationDefinition = {});
    static ConfigurationRef local(ConfigurationDefinition = {});
    InitMethod init() const noexcept;
    ConfigMethod config() const noexcept;
};

class ConfigurationHandler final {
    std::vector<std::pair<std::u16string, ConfigurationRef>> maps_;
public:
    void set(std::u16string, ConfigurationRef);
    ConfigurationRef get(std::u16string_view) const;
    std::vector<std::u16string> keys() const;
};

class ParserConfiguration {
    ConfigurationHandler& registry_;
    MapHandler& maps_;
    TexSessionState& session_;
    MacroCallbacks const& callbacks_;
    WarningSink& warnings_;
    std::vector<std::u16string> parsers_;
    FunctionList<ParserConfiguration&> initMethods_;
    FunctionList<ParserConfiguration&, TeX&> configMethods_;
    PrioritizedList<ConfigurationRef> configurations_;
    ConfigurationRef getPackage(std::u16string_view);
    void addFilters(TeX&, Configuration const&);
public:
    ParserConfiguration(ConfigurationHandler&, MapHandler&, TexSessionState&,
        MacroCallbacks const&, WarningSink&, std::span<PackageSelection const>,
        std::vector<std::u16string> parsers = {u"tex"});
    ParserConfiguration(ParserConfiguration const&) = delete;
    ParserConfiguration& operator=(ParserConfiguration const&) = delete;
    SubHandlers handlers;
    StackItemConfig items;
    TagsConfig tags;
    TexOptionValues options;
    NodeCreators nodes;
    MapHandler& maps() const noexcept { return maps_; }
    std::u16string defaultTags{u"none"};
    TexSessionState& session() const noexcept { return session_; }
    MacroCallbacks const& callbacks() const noexcept { return callbacks_; }
    void append(ConfigurationRef, std::optional<double> priority = {});
    void addPackage(PackageSelection const&);
    void init();
    // Source input-jax phases require the real native TeX/ParseOptions services.
    // These remain unimplemented until those owners are ready, with no fallback.
    void config(TeX&);
    void add(std::u16string_view name, TeX&, TexOptionValues const& = {});
};
}
