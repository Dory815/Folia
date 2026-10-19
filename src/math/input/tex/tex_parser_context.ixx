// Borrowed view implemented by the one actual TexParser. No parser state.
// This owner is above Stack; pure Types/StackItem never import parser context.
export module folia.math.input.tex.tex_parser_context;
export import folia.math.input.tex.types;
export import folia.math.input.tex.stack;
import std;
export namespace folia::math::tex {
struct ParserContextIdentity {options::Context* context=nullptr;options::ObjectRef object{};};
class ParserContext {
public:
    virtual ~ParserContext()=default;
    // The parser argument exposed to Source callbacks must be its canonical
    // Context instance, never a payload-only carrier around this C++ view.
    virtual ParserContextIdentity contextIdentity()const {
        throw options::Exception("TypeError",u"Parser has no canonical Context identity");
    }
    virtual Property currentCS()const=0;
    virtual void setCurrentCS(Property)=0;
};
struct ParseInput {ParserContext& parser;Property token;};
using ParseBody=std::function<ParseResult(const FunctionReceiver&,ParserContext&,const ParseAtom&,ParseRest)>;
ParseCallableRef makeParseCallable(ParseBody);
ParseResult invokeParseMethod(const ParseMethod&,const FunctionReceiver&,ParserContext&,const ParseAtom&,ParseRest={});
}
