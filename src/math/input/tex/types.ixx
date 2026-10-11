// Root native declaration seam for pinned input/tex/Types.ts. Apache-2.0.
export module folia.math.input.tex.types;
export import folia.math.input.tex.token;
export import folia.math.util.function_types;
import std;
export namespace folia::math::tex {
using Args=Property;
using Attributes=Property;
using Environment=Property;
class ParserContext {
public:
    virtual ~ParserContext()=default;
    virtual Property currentCS() const=0;
    virtual void setCurrentCS(Property)=0;
};
class StackItemContext {public: virtual ~StackItemContext()=default;};
using StackItemRef=std::shared_ptr<StackItemContext>;
struct ParseSymbol {std::u16string description;};
using ParseSymbolRef=std::shared_ptr<const ParseSymbol>;
using ParseResult=std::variant<Property,StackItemRef,ParseSymbolRef>;
using ParseAtom=std::variant<Property,TokenRef,StackItemRef>;
struct ParseInput {ParserContext& parser;Property token;};
using ParseRest=std::span<const std::any>;
using ParseCallable=Function;
using ParseCallableRef=FunctionRef;
using ParseMethod=FunctionValue;
using ParseBody=std::function<ParseResult(const FunctionReceiver&,ParserContext&,const ParseAtom&,ParseRest)>;
ParseCallableRef makeParseCallable(ParseBody);
ParseResult invokeParseMethod(const ParseMethod&,const FunctionReceiver&,ParserContext&,const ParseAtom&,ParseRest={});
}
