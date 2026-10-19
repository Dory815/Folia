// Root-frozen declarations for complete MathJax 4.1.3 TokenMap.ts.
// Apache-2.0; source SHA256 0209895b13569008b2129b61c88050e7c70e70ce9477d234b9a6e303a86a738e.
export module folia.math.input.tex.token_map;
export import folia.math.input.tex.token_map_types;
export import folia.math.input.tex.map_handler;
export import folia.math.util.ordered_map_storage;
import std;

export namespace folia::math::tex {
class TokenMapHost;
class TokenMapAllocation {
    TokenMapAllocation()=default;
    friend class TokenMapHost;
};

// Engine dependency: identity-bearing live test service. Controlled services
// prove map dispatch only; no engine is accepted by this declaration.
class RegExpContext:public NativeObject {
public:
    virtual ~RegExpContext()=default;
    virtual bool test(const Property&)=0;
};
using RegExpRef=std::shared_ptr<RegExpContext>;

struct TokenPropertyEqual {
    bool operator()(const Property&,const Property&)const;
};
using TokenLookupStorage=OrderedMapAccess<Property,TokenLookup>;
using TokenLookupOrderedStorage=OrderedMapStorage<Property,TokenLookup,TokenPropertyEqual>;

ParseResult parseResult(ParseResult);

class AbstractTokenMap:public TokenMap,public std::enable_shared_from_this<AbstractTokenMap> {
    Property name_=Missing{};
    ParseMethod parser_=Property(Missing{});
protected:
    explicit AbstractTokenMap(TokenMapAllocation){}
    void initializeSource(MapHandler&,Property,ParseMethod);
    friend class TokenMapHost;
public:
    AbstractTokenMap(const AbstractTokenMap&)=delete;
    AbstractTokenMap& operator=(const AbstractTokenMap&)=delete;
    AbstractTokenMap(AbstractTokenMap&&)=delete;
    AbstractTokenMap& operator=(AbstractTokenMap&&)=delete;
    Property name() const override;
    ParseMethod parser() const override;
    void parser(ParseMethod) override;
    // Raw Source own-field accessors. These do not dispatch through virtual
    // TokenMap methods and therefore preserve field semantics in subclasses.
    Property sourceNameField()const{return name_;}
    void sourceNameField(Property value){name_=std::move(value);}
    ParseMethod sourceParserField()const{return parser_;}
    void sourceParserField(ParseMethod value){parser_=std::move(value);}
    ParseMethod parserFor(const Property&) override;
    ParseResult parse(const ParseInput&) override;
};

class RegExpMap:public AbstractTokenMap {
protected:
    RegExpRef regexp_;
    void initializeSource(MapHandler&,Property,ParseMethod,RegExpRef);
    friend class TokenMapHost;
public:
    explicit RegExpMap(TokenMapAllocation tag):AbstractTokenMap(tag){}
    RegExpRef sourceRegExpField()const{return regexp_;}
    void sourceRegExpField(RegExpRef value){regexp_=std::move(value);}
    bool contains(const Property&) override;
    TokenLookup lookup(const Property&) override;
};

class AbstractParseMap:public AbstractTokenMap {
protected:
    // Source field initializer happens after base registration, not allocation.
    std::optional<std::shared_ptr<TokenLookupStorage>> map_;
    void initializeSource(MapHandler&,Property,ParseMethod);
    friend class TokenMapHost;
public:
    explicit AbstractParseMap(TokenMapAllocation tag):AbstractTokenMap(tag){}
    TokenLookup lookup(const Property&) override;
    bool contains(const Property&) override;
    virtual void add(Property,TokenLookup);
    virtual void remove(const Property&);
    // Aliases the actual map member and retains this owner for the Context
    // projection lifetime. The returned handle contains no copied entries.
    std::shared_ptr<TokenLookupStorage> mapStorage()const;
    void rebindMapStorage(std::shared_ptr<TokenLookupStorage>);
};

class CharacterMap:public AbstractParseMap,public CharacterMapIdentity {
protected:
    void initializeSource(MapHandler&,Property,ParseMethod,Property);
    friend class TokenMapHost;
public:
    explicit CharacterMap(TokenMapAllocation tag):AbstractParseMap(tag){}
};

class DelimiterMap:public CharacterMap {
public:
    explicit DelimiterMap(TokenMapAllocation tag):CharacterMap(tag){}
    ParseResult parse(const ParseInput&) override;
};

class MacroMap:public AbstractParseMap {
protected:
    void initializeSource(MapHandler&,Property,Property,Property=Missing{});
    friend class TokenMapHost;
public:
    explicit MacroMap(TokenMapAllocation tag):AbstractParseMap(tag){}
    ParseMethod parserFor(const Property&) override;
    ParseResult parse(const ParseInput&) override;
};

class CommandMap:public MacroMap {
public:
    explicit CommandMap(TokenMapAllocation tag):MacroMap(tag){}
    ParseResult parse(const ParseInput&) override;
};

class EnvironmentMap:public MacroMap {
protected:
    void initializeSource(MapHandler&,Property,ParseMethod,Property,Property=Missing{});
    friend class TokenMapHost;
public:
    explicit EnvironmentMap(TokenMapAllocation tag):MacroMap(tag){}
    ParseResult parse(const ParseInput&) override;
};

class TokenMapHost {
public:
    template<class T,class...Args>
    static std::shared_ptr<T> create(MapHandler& registry,Args&&...args){
        static_assert(std::derived_from<T,AbstractTokenMap>);
        // One final allocation, then original base-to-derived initializer order.
        auto object=std::make_shared<T>(TokenMapAllocation{});
        object->initializeSource(registry,std::forward<Args>(args)...);
        return object;
    }
};
}
