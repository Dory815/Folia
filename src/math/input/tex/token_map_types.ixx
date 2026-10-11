// Root native protocol for pinned TokenMap.ts and MapHandler.ts dependencies.
// Apache-2.0; the concrete TokenMap classes remain a separate required port.
export module folia.math.input.tex.token_map_types;
export import folia.math.input.tex.types;
export import folia.math.input.tex.macro;
export import folia.math.util.object_types;
import std;
export namespace folia::math::tex {
using TokenLookup=std::variant<Property,TokenRef,MacroRef>;
class TokenMap:public NativeObject {
public:
    virtual ~TokenMap()=default;
    virtual Property name() const=0;
    virtual ParseMethod parser() const=0;
    virtual void parser(ParseMethod)=0;
    virtual bool contains(const Property&)=0;
    virtual ParseMethod parserFor(const Property&)=0;
    virtual ParseResult parse(const ParseInput&)=0;
    virtual TokenLookup lookup(const Property&)=0;
};
using TokenMapRef=std::shared_ptr<TokenMap>;
// RTTI family identity only, corresponding to instanceof CharacterMap.
// The future concrete CharacterMap and its subclasses own this marker.
class CharacterMapIdentity {public:virtual ~CharacterMapIdentity()=default;};
}
