// MathJax 4.1.3 input/tex.ts; Apache-2.0.
export module folia.math.input.tex.parser:tex_jax;
export import :types;
import std;
export namespace folia::math::tex {
class TeX:public core::AbstractInputJax {
    ParserRuntime& runtime_;
protected:
    ParserShared<ParseOptions> parseOptions_=Missing{};
public:
    ParserShared<FindTeX> findTeX=Missing{};
    ParserShared<ParserConfiguration> configuration=Missing{};
    Property latex=Missing{};
    mml::NodeValue mathNode=Missing{};
    TeX(ParserRuntime&,OptionFunctionBindings&,options::ObjectRef); // allocation only
    static TeXRef make(ParserRuntime&,options::Value options=options::Undefined{});
    void initializeTeX(options::Value options=options::Undefined{});
    // Static NAME/OPTIONS are actual constructor fields owned by ParserRuntime.
    static ParserConfigurationRef configure(ParserRuntime&,options::Value packages);
    static void tags(ParseOptions&,ParserConfiguration&);
    ParserRuntime& runtime()const;
    void setMmlFactory(core::InputMmlFactoryReference)override;
    virtual ParserShared<ParseOptions> parseOptions()const;
    void reset(FunctionArgs args={})override;
    mml::NodeValue compile(core::MathItem&,core::MathDocumentReference)override;
    core::ProtoMathItems findMath(std::any strings={},options::Value ignored=options::Undefined{})override;
    virtual mml::NodeValue formatError(const TexError&);
};
}
