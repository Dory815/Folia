// MathJax 4.1.3 input/tex/Configuration.ts; Apache-2.0. Declarations only.
export module folia.math.input.tex.parser:configuration;
export import :types;
import std;
export namespace folia::math::tex {
class Configuration:public NativeObject {
    ParserRuntime& runtime_;
    static ProcessorValue makeProcessor(ParserRuntime&,options::Value,options::Value);
    static ConfigurationRef _create(ParserRuntime&,std::u16string,options::Value);
    explicit Configuration(ParserRuntime&);
    void initializeConfiguration(std::u16string,options::Value handler,options::Value fallback,
        StackItemConfigValue,TagsConfigValue,options::Value options,options::Value nodes,
        ProcessorListValue preprocessors,ProcessorListValue postprocessors,
        ProcessorValue initMethod,ProcessorValue configMethod,options::Value priority,options::Value parser);
public:
    std::u16string name;
    options::Value handler=options::Undefined{},fallback=options::Undefined{};
    StackItemConfigValue items=options::Undefined{};
    TagsConfigValue tags=options::Undefined{};
    options::Value options=options::Undefined{},nodes=options::Undefined{};
    ProcessorListValue preprocessors=options::Undefined{},postprocessors=options::Undefined{};
    ProcessorValue initMethod=options::Undefined{},configMethod=options::Undefined{};
    options::Value priority=options::Undefined{},parser=options::Undefined{};
    ParserRuntime& runtime()const;
    static ConfigurationRef create(ParserRuntime&,std::u16string name,options::Value config=options::Undefined{});
    static ConfigurationRef local(ParserRuntime&,options::Value config=options::Undefined{});
    virtual options::Value init()const;
    virtual options::Value config()const;
};
class ConfigurationHandler {
    SourceStringMap<ConfigurationRef> maps_;
public:
    void set(std::u16string,ConfigurationRef);
    ParserShared<Configuration> get(std::u16string_view)const;
    SourceStringMap<ConfigurationRef>::KeyIterator keys()const;
};
class ParserConfiguration:public NativeObject {
    ParserRuntime& runtime_;
protected:
    ParserShared<FunctionList> initMethod=Missing{},configMethod=Missing{};
    ParserShared<PrioritizedList<ConfigurationRef>> configurations=Missing{};
    options::Value parsers=options::Undefined{};
    virtual ParserShared<Configuration> getPackage(std::u16string_view);
    virtual void append(ConfigurationRef,options::Value priority=options::Undefined{});
private:
    void addFilters(TeX&,ConfigurationRef);
    void warn(std::u16string);
public:
    ParserShared<SubHandlers> handlers=Missing{};
    StackItemConfigValue items=options::Undefined{};
    TagsConfigValue tags=options::Undefined{};
    options::Value options=options::Undefined{},nodes=options::Undefined{};
    explicit ParserConfiguration(ParserRuntime&); // allocation only
    void initializeParserConfiguration(options::Value packages,options::Value parsers=options::Undefined{});
    ParserRuntime& runtime()const;
    virtual void init();
    virtual void config(TeX&);
    virtual void addPackage(options::Value);
    virtual void add(std::u16string name,TeX&,options::Value options=options::Undefined{});
};
}
