// Borrowed view implemented by the one actual TexParser. No parser state.
// This owner is above Stack; pure Types/StackItem never import parser context.
export module folia.math.input.tex.tex_parser_context;
export import folia.math.input.tex.types;
export import folia.math.input.tex.stack;
import std;
export namespace folia::math::tex {
class ParserContext {
public:
    virtual ~ParserContext()=default;
    virtual Property currentCS()const=0;
    virtual void setCurrentCS(Property)=0;
};
struct ParseInput {ParserContext& parser;Property token;};
using ParseBody=std::function<ParseResult(const FunctionReceiver&,ParserContext&,const ParseAtom&,ParseRest)>;
ParseCallableRef makeParseCallable(ParseBody);
ParseResult invokeParseMethod(const ParseMethod&,const FunctionReceiver&,ParserContext&,const ParseAtom&,ParseRest={});
}
