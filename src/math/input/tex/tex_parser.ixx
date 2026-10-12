// MathJax 4.1.3 input/tex/TexParser.ts; Apache-2.0.
export module folia.math.input.tex.parser:tex_parser;
export import :types;
import std;
export namespace folia::math::tex {
class TexParser:public ParserContext,public NativeObject {
    // Native allocation handle, bound once by make before Source initialization.
    std::optional<options::ObjectRef> identity_;
    std::u16string string_;
    Property currentCS_=""; // sole Source currentCS, accessed by bridge
    double saveI=0;
    void updateResult(std::u16string,double old);
    void composeLatex(mml::NodeValue,std::u16string,double pos1,double pos2);
    void composeBraces(mml::NodeValue);
    std::u16string composeBracedContent(mml::NodeValue);
protected:
    virtual std::u16string trimTex(std::u16string);
public:
    double macroCount=0,i=0;
    ParserShared<Stack> stack=Missing{};
    ParserBorrowed<ParseOptions> configuration=Missing{};
    TexParser()=default; // allocation only; initialize after retaining identity
    static TexParserRef make(std::u16string string,Property env,ParseOptions&);
    void initializeTexParser(std::u16string string,Property env,ParseOptions&);
    options::ObjectRef instanceIdentity()const {
        if(!identity_)throw options::Exception("Unimplemented",u"TexParser has no canonical native instance binding");
        return *identity_;
    }
    virtual options::Value options()const;
    virtual ParserShared<StackItemFactory> itemFactory()const;
    virtual ParserShared<Tags> tags()const;
    virtual void setString(std::u16string);
    virtual std::u16string string()const;
    Property currentCS()const override;
    void setCurrentCS(Property)override;
    virtual ParseResult parse(Property kind,const ParseInput&);
    virtual TokenLookup lookup(Property kind,std::u16string);
    virtual bool contains(Property kind,std::u16string);
    virtual std::u16string toString()const;
    virtual void Parse();
    virtual void Push(StackEntry);
    virtual void PushAll(CheckList);
    virtual mml::NodeValue mml();
    virtual Property convertDelimiter(std::u16string); // string | null
    virtual std::u16string getCodePoint();
    virtual bool nextIsSpace();
    virtual std::u16string GetNext();
    virtual std::u16string GetCS();
    virtual Property GetArgument(std::u16string,bool noneOK=false); // string | null
    virtual Property GetBrackets(std::u16string,Property def=Missing{},bool matchBrackets=false);
    virtual Property GetDelimiter(std::u16string,bool braceOK=false); // string | null
    virtual std::u16string GetDimen(std::u16string);
    virtual std::u16string GetUpTo(std::u16string,std::u16string);
    virtual mml::NodeValue ParseArg(std::u16string);
    virtual mml::NodeValue ParseUpTo(std::u16string,std::u16string);
    virtual Property GetDelimiterArg(std::u16string); // string | null
    virtual bool GetStar();
    virtual mml::NodeValue create(std::u16string,FunctionArgs rest={});
};
}
