// Root declarations: complete MathJax 4.1.3 input/tex/MapHandler.ts port.
export module folia.math.input.tex.map_handler;
export import folia.math.input.tex.token_map_types;
export import folia.math.util.method_config;
export import folia.math.util.function_list;
export import folia.math.util.option_functions;
export import folia.math.util.ordered_map_storage;
import std;
export namespace folia::math::tex {
using HandlerConfig=PropertyList;
using FallbackConfig=MethodConfig;
using WarningSink=std::function<void(const std::u16string&)>;
struct MapHandler {
private:
    std::vector<std::pair<Property,TokenMapRef>> maps_;
    std::shared_ptr<void> lifetime_=std::make_shared<std::uint8_t>(0);
public:
    MapHandler()=default;
    MapHandler(const MapHandler&)=delete;
    MapHandler& operator=(const MapHandler&)=delete;
    MapHandler(MapHandler&&)=delete;
    MapHandler& operator=(MapHandler&&)=delete;
    ~MapHandler(){lifetime_.reset();}
    void registerMap(TokenMapRef);
    TokenMapRef getMap(const Property&)const;
    std::weak_ptr<void> lifetimeToken()const{return lifetime_;}
};
class SubHandler:public NativeObject {
public:
    using Configuration=PrioritizedList<TokenMapRef>;
    using ConfigurationRef=std::shared_ptr<Configuration>;
    using FallbackRef=std::shared_ptr<FunctionList>;
protected:
    ConfigurationRef configuration_=std::make_shared<Configuration>();
    FallbackRef fallback_=std::make_shared<FunctionList>();
private:
    WarningSink warning_;
    MapHandler* registry_;
    std::weak_ptr<void> registryLifetime_;
    void ensureRegistryAlive()const;
    void warn(std::u16string);
    void addMapIterator(const std::function<std::optional<Property>()>&,ParseMethod,double);
    void removeMapIterator(const std::function<std::optional<Property>()>&,ParseMethod);
public:
    static ParseResult FALLBACK;
    explicit SubHandler(MapHandler&,WarningSink={});
    ConfigurationRef configurationOwner()const noexcept{return configuration_;}
    void configurationOwner(ConfigurationRef owner){
        if(!owner)throw options::Exception("TypeError",u"SubHandler configuration owner is null");
        configuration_=std::move(owner);
    }
    FallbackRef fallbackOwner()const noexcept{return fallback_;}
    void fallbackOwner(FallbackRef owner){
        if(!owner)throw options::Exception("TypeError",u"SubHandler fallback owner is null");
        fallback_=std::move(owner);
    }
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
    // Source MapHandler.ts emits this TypeScript-private method on the public
    // prototype; the Context bridge delegates to the same warning path.
    void sourceWarn(std::u16string message){warn(std::move(message));}
};
using SubHandlerRef=std::shared_ptr<SubHandler>;
class SubHandlers {
public:
    struct HandlerKeyEqual {
        bool operator()(const Property&,const Property&)const;
    };
    using Storage=OrderedMapAccess<Property,SubHandlerRef>;
    using OrderedStorage=OrderedMapStorage<Property,SubHandlerRef,HandlerKeyEqual>;
private:
    std::shared_ptr<Storage> map_=std::make_shared<OrderedStorage>();
    MapHandler* registry_;
    std::weak_ptr<void> registryLifetime_;
    WarningSink warning_;
    void ensureRegistryAlive()const;
    void addHandlerNames(std::span<const std::string>,const std::function<void(SubHandler&,std::string_view)>&);
    void removeHandlerNames(std::span<const std::string>,const std::function<void(SubHandler&,std::string_view)>&);
public:
    explicit SubHandlers(MapHandler&,WarningSink={});
    virtual ~SubHandlers()=default;
    SubHandlers(const SubHandlers&)=delete;
    SubHandlers& operator=(const SubHandlers&)=delete;
    virtual void add(const HandlerConfig&,const FallbackConfig&,double priority=PrioritizedList<TokenMapRef>::DEFAULTPRIORITY);
    virtual void add(options::Context&,OptionFunctionBindings&,options::Value handlers,options::Value fallbacks,double priority=PrioritizedList<TokenMapRef>::DEFAULTPRIORITY);
    virtual void remove(const HandlerConfig&,const FallbackConfig&);
    virtual void remove(options::Context&,OptionFunctionBindings&,options::Value handlers,options::Value fallbacks);
    virtual void set(std::string,SubHandlerRef);
    void set(const char*,SubHandlerRef);
    virtual SubHandlerRef get(std::string_view);
    SubHandlerRef get(const std::string&);
    SubHandlerRef get(const char*);
    virtual void set(Property,SubHandlerRef);
    virtual SubHandlerRef get(const Property&);
    virtual TokenMapRef retrieve(const Property&);
    // Returns the actual typed store for a Context Map projection.
    std::shared_ptr<Storage> sourceMapStorage()const{return map_;}
    void rebindSourceMapStorage(std::shared_ptr<Storage> storage){
        if(!storage)throw options::Exception("TypeError",u"SubHandlers map storage is null");
        map_=std::move(storage);
    }
    // Source Map.keys iterator is live, not an eager keys snapshot.
    class KeyIterator {
        std::shared_ptr<Storage> storage_;std::size_t index_=0;bool done_=false;
    public:
        explicit KeyIterator(std::shared_ptr<Storage> storage):storage_(std::move(storage)){}
        std::optional<std::string> next();
        std::optional<Property> nextProperty();
    };
    KeyIterator keys(){return KeyIterator(map_);}
};
}
