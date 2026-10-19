// MathJax 4.1.3 input/tex.ts; Apache-2.0.
export module folia.math.input.tex.parser:tex_jax;
export import :types;
import folia.math.util.native_identities;
import std;
export namespace folia::math::tex {
class StackItemFactory;
class TeX:public core::AbstractInputJax {
    ParserRuntime& runtime_;
protected:
    ParserShared<ParseOptions> parseOptions_=Missing{};
    StackItemFactory* requestStackItemFactory_=nullptr;
    std::size_t requestStackItemMark_=0;
    std::uint64_t requestStackItemGeneration_=0;
    bool requestOwnerScopeActive_=false;
    std::optional<RequestIdentityCapture> requestParserCapture_;
    std::optional<RequestIdentityCapture> requestStackCapture_;
    std::optional<RequestIdentityCapture> requestStackItemCapture_;
    std::optional<RequestIdentityCapture> requestStackPropertyCapture_;
    bool requestReferencesDetached_=false,requestParserIdentitiesRetired_=false,
         requestStackIdentitiesRetired_=false,requestStackItemsRetired_=false,
         requestStackPropertiesRetired_=false,requestStackProductsRetired_=false;
    std::vector<options::ObjectRef> requestContextRecords_;
    bool requestContextRecordsCaptured_=false;
    bool requestTransientRecordsRetired_=false;
public:
    static void initializeModule(ParserRuntime&);
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
    void beginRequestOwnerScope();
    // Private Renderer request teardown: unlink only the last compilation's
    // references while preserving ParseOptions/package/tag/module state.
    void releaseRequestReferences();
    void reset(FunctionArgs args={})override;
    mml::NodeValue compile(core::MathItem&,core::MathDocumentReference)override;
    core::ProtoMathItems findMath(std::any strings={},options::Value ignored=options::Undefined{})override;
    virtual mml::NodeValue formatError(const TexError&);
};
}
