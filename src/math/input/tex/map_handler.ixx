// Root declarations: complete MathJax 4.1.3 input/tex/MapHandler.ts port.
export module folia.math.input.tex.map_handler;
export import folia.math.input.tex.token_map_types;
export import folia.math.util.method_config;
export import folia.math.util.function_list;
export import folia.math.util.option_functions;
import std;
export namespace folia::math::tex {
using HandlerConfig=PropertyList;
using FallbackConfig=MethodConfig;
using WarningSink=std::function<void(const std::u16string&)>;
struct MapHandler {
    static void registerMap(TokenMapRef);
    static TokenMapRef getMap(const Property&);
};
class SubHandler:public NativeObject {
protected:
    PrioritizedList<TokenMapRef> configuration_;
    FunctionList fallback_;
private:
    WarningSink warning_;
    void warn(std::u16string);
    void addMapIterator(const std::function<std::optional<Property>()>&,ParseMethod,double);
    void removeMapIterator(const std::function<std::optional<Property>()>&,ParseMethod);
public:
    static ParseResult FALLBACK;
    explicit SubHandler(WarningSink={});
    virtual ~SubHandler()=default;
    virtual void add(const Property& maps,ParseMethod fallback=Property(Missing{}),double priority=PrioritizedList<TokenMapRef>::DEFAULTPRIORITY);
    virtual void add(options::Context&,options::Value maps,ParseMethod fallback=Property(Missing{}),double priority=PrioritizedList<TokenMapRef>::DEFAULTPRIORITY);
    virtual void remove(const Property& maps,ParseMethod fallback=Property(PropertyNull{}));
    virtual void remove(options::Context&,options::Value maps,ParseMethod fallback=Property(PropertyNull{}));
    virtual ParseResult parse(const ParseInput&);
    virtual TokenLookup lookup(const Property&);
    virtual bool contains(const Property&);
    virtual std::string toString();
    virtual TokenMapRef applicable(const Property&);
    virtual TokenMapRef retrieve(const Property&);
};
using SubHandlerRef=std::shared_ptr<SubHandler>;
class SubHandlers {
    using MapStorage=std::vector<std::pair<std::string,SubHandlerRef>>;
    std::shared_ptr<MapStorage> map_=std::make_shared<MapStorage>();
    WarningSink warning_;
    void addHandlerNames(std::span<const std::string>,const std::function<void(SubHandler&,std::string_view)>&);
    void removeHandlerNames(std::span<const std::string>,const std::function<void(SubHandler&,std::string_view)>&);
public:
    explicit SubHandlers(WarningSink={});
    virtual ~SubHandlers()=default;
    SubHandlers(const SubHandlers&)=delete;
    SubHandlers& operator=(const SubHandlers&)=delete;
    virtual void add(const HandlerConfig&,const FallbackConfig&,double priority=PrioritizedList<TokenMapRef>::DEFAULTPRIORITY);
    virtual void add(options::Context&,OptionFunctionBindings&,options::Value handlers,options::Value fallbacks,double priority=PrioritizedList<TokenMapRef>::DEFAULTPRIORITY);
    virtual void remove(const HandlerConfig&,const FallbackConfig&);
    virtual void remove(options::Context&,OptionFunctionBindings&,options::Value handlers,options::Value fallbacks);
    virtual void set(std::string,SubHandlerRef);
    virtual SubHandlerRef get(std::string_view);
    virtual TokenMapRef retrieve(const Property&);
    // Source Map.keys iterator is live, not an eager keys snapshot.
    class KeyIterator {
        std::shared_ptr<MapStorage> storage_;std::size_t index_=0;bool done_=false;
    public:
        explicit KeyIterator(std::shared_ptr<MapStorage> storage):storage_(std::move(storage)){}
        std::optional<std::string> next();
    };
    KeyIterator keys(){return KeyIterator(map_);}
};
}
