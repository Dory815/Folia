// MathJax 4.1.3 input/tex/Configuration.ts; Apache-2.0. Declarations only.
export module folia.math.input.tex.parser:configuration;
export import :types;
import std;
export namespace folia::math::tex {
options::ObjectRef bindConfigurationPriorityList(ParserRuntime&,
    const std::shared_ptr<PrioritizedList<ConfigurationRef>>&);
class Configuration:public NativeObject {
    friend class ParserRuntime;
    ParserRuntime& runtime_;
    options::ObjectRef identity_;
    static ProcessorValue makeProcessor(ParserRuntime&,options::Value,options::Value);
    static ConfigurationRef _create(ParserRuntime&,options::Value receiver,options::Value name,options::Value config);
    explicit Configuration(ParserRuntime&);
    void initializeConfiguration(options::ObjectRef,options::Value name,options::Value handler,options::Value fallback,
        StackItemConfigValue,TagsConfigValue,options::Value options,options::Value nodes,
        ProcessorListValue preprocessors,ProcessorListValue postprocessors,
        ProcessorValue initMethod,ProcessorValue configMethod,options::Value priority,options::Value parser);
public:
    ParserRuntime& runtime()const;
    options::Value field(std::u16string_view)const;
    std::u16string name()const;
    static ConfigurationRef construct(ParserRuntime&,std::span<const options::Value>);
    static ConfigurationRef create(ParserRuntime&,std::u16string name,options::Value config=options::Undefined{});
    static ConfigurationRef local(ParserRuntime&,options::Value config=options::Undefined{});
    virtual options::Value init()const;
    virtual options::Value config()const;
};
class ConfigurationHandler {
    SourceStringMap<ConfigurationRef> maps_;
public:
    void initializeModule(ParserRuntime&);
    options::ObjectRef createContextObject(options::Context&,NativeIdentityBindings<ConfigurationRef>&);
    void set(std::u16string,ConfigurationRef);
    ParserShared<Configuration> get(std::u16string_view)const;
    SourceStringMap<ConfigurationRef>::KeyIterator keys()const;
};
class ParserConfiguration:public NativeObject,public std::enable_shared_from_this<ParserConfiguration> {
    friend class ParserRuntime;
    ParserRuntime& runtime_;
    options::ObjectRef identity_;
    options::Value field(std::u16string_view)const;
    std::shared_ptr<FunctionList> functionList(std::u16string_view)const;
    std::shared_ptr<PrioritizedList<ConfigurationRef>> configurationList()const;
    std::shared_ptr<SubHandlers> handlerList()const;
    void setNativeField(std::u16string_view,std::any);
protected:
    virtual ParserShared<Configuration> getPackage(std::u16string_view);
private:
    void addFilters(TeX&,ConfigurationRef);
    void warn(std::u16string);
public:
    explicit ParserConfiguration(ParserRuntime&); // allocation only
    virtual void append(ConfigurationRef,options::Value priority=options::Undefined{});
    void initializeParserConfiguration(options::Value packages,options::Value parsers=options::Undefined{});
    ParserRuntime& runtime()const;
    options::Value sourceField(std::u16string_view name)const{return field(name);}
    virtual void init();
    virtual void config(TeX&);
    virtual void addPackage(options::Value);
    virtual void add(std::u16string name,TeX&,options::Value options=options::Undefined{});
};
}
