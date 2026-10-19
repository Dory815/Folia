// Canonical Context identities for MathJax 4.1.3 TokenMap.ts / Token.ts.
export module folia.math.input.tex.token_map_context_bindings;
export import folia.math.input.tex.token_map;
export import folia.math.util.map_bindings;
export import folia.math.util.context_property_bindings;
export import folia.math.util.option_functions;
import std;

export namespace folia::math::tex {
enum class TokenMapClass : std::uint8_t {
    AbstractTokenMap, RegExpMap, AbstractParseMap, CharacterMap,
    DelimiterMap, MacroMap, CommandMap, EnvironmentMap
};

// Concrete C++ carrier for the emitted JavaScript AbstractTokenMap class.
// Its absent abstract methods raise Source-equivalent TypeErrors if inherited
// parsing attempts to call them.
TokenMapRef makeAbstractTokenMap(MapHandler&,Property,ParseMethod);

// Runtime-specific crossings are explicit. The callbacks must delegate to the
// actual TokenMapHost/ParserRuntime graph and existing ParseMethod bodies.
struct TokenMapContextHooks {
    std::function<TokenMapRef(TokenMapClass, options::Context&,
                              std::span<const options::Value>)> constructMap;
    std::function<TokenRef(options::Context&,std::span<const options::Value>)> constructToken;
    std::function<MacroRef(options::Context&,std::span<const options::Value>)> constructMacro;
    std::function<ParseMethod(options::Context&,options::Value)> parseMethodFromValue;
    std::function<options::Value(options::Context&,const ParseMethod&)> parseMethodToValue;
    std::function<RegExpRef(options::Context&,options::Value)> regexpFromValue;
    std::function<options::Value(options::Context&,const RegExpRef&)> regexpToValue;
    std::function<ParseInput(options::Context&,options::Value)> parseInputFromValue;
    std::function<options::Value(options::Context&,ParseResult)> parseResultToValue;
};

class TokenMapContextBindings {
    struct State;
    std::shared_ptr<State> state_;
public:
    TokenMapContextBindings(options::Context&,OptionFunctionBindings&,
                            ContextPropertyBindings&,options::MapContextBindings&,
                            TokenMapContextHooks,std::weak_ptr<std::atomic_bool> executionLease);
    ~TokenMapContextBindings();
    TokenMapContextBindings(const TokenMapContextBindings&)=delete;
    TokenMapContextBindings& operator=(const TokenMapContextBindings&)=delete;

    options::ObjectRef constructor(TokenMapClass)const;
    options::ObjectRef prototype(TokenMapClass)const;
    options::ObjectRef tokenConstructor()const;
    options::ObjectRef tokenPrototype()const;
    options::ObjectRef macroConstructor()const;
    options::ObjectRef macroPrototype()const;

    options::ObjectRef bind(const TokenMapRef&);
    options::ObjectRef bind(const TokenRef&);
    options::ObjectRef bind(const MacroRef&);
    TokenMapRef resolveTokenMap(options::ObjectRef)const;
    TokenMapRef resolve(options::ObjectRef)const;
    TokenRef resolveToken(options::ObjectRef)const;
    MacroRef resolveMacro(options::ObjectRef)const;
    TokenLookup tokenLookupFromValue(options::Value);
    options::Value tokenLookupToValue(const TokenLookup&);
    Property fromValue(options::Value)const;
    options::Value toValue(const Property&)const;
    ParseMethod parseMethodFromValue(options::Value)const;
    options::Value parseMethodToValue(const ParseMethod&)const;
    ParseInput parseInputFromValue(options::Value)const;
    options::Value parseResultToValue(ParseResult)const;
    options::Context& context()const;
};
}
