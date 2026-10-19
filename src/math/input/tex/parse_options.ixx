// MathJax 4.1.3 input/tex/ParseOptions.ts; Apache-2.0.
export module folia.math.input.tex.parser:parse_options;
export import :column_parser;
import std;
export namespace folia::math::tex {
class ParseOptions:public NativeObject,public NodeFactoryConfiguration,public StackItemFactoryConfiguration,
                   public std::enable_shared_from_this<ParseOptions> {
    ParserRuntime& runtime_;
    bool inTree(mml::NodeValue)const;
public:
    static void initializeModule(ParserRuntime&);
    static ParseOptionsRef make(ParserRuntime&,ParserConfiguration&,options::Value definitions=options::Undefined{});
    // The static Source Map is owned once by ParserRuntime's class statics.
    static SourceStringMap<options::Value>& getVariant(ParserRuntime&);
    ParserShared<SubHandlers> handlers=Missing{};
    options::Value options=options::Undefined{};
    ParserShared<StackItemFactory> itemFactory=Missing{};
    options::Value nodeFactory=options::Undefined{};
    ParserShared<Tags> tags=Missing{};
    options::Value mathStyle=options::Undefined{};
    options::Value columnParser=options::Undefined{};
    // One actual Map per ParseOptions. Payloads are actual Context objects;
    // heterogenous package classes retain their canonical native bindings.
    ParserShared<SourceStringMap<options::Value>> packageData=Missing{};
    // The actual Source Array identity; parser operations use its live methods.
    options::Value parsers=options::Undefined{};
    core::MathItemReference mathItem=Missing{};
    mml::NodeValue root=PropertyNull{};
    options::Value nodeLists=options::Undefined{};
    bool error=false;
    explicit ParseOptions(ParserRuntime&); // allocation only
    void initializeParseOptions(ParserConfiguration&,options::Value definitions=options::Undefined{});
    ParserRuntime& runtime()const;
    ColumnParser& columnParserOwner()const;
    NodeFactory& nodeFactoryOwner()const;
    std::shared_ptr<NodeFactory> nodeFactoryOwnerRef()const;
    core::MathItemReference mathItemOwner()const;
    void setMathItem(core::MathItem*);
    void setTags(TagsRef);
    virtual void pushParser(TexParserRef);
    virtual void popParser();
    virtual ParserShared<TexParser> parser()const;
    virtual void clear();
    void addNode(Property,mml::NodeValue)override;
    virtual mml::Children getList(std::u16string);
    virtual void removeFromList(std::u16string,mml::Children);
    TeXNodeFactoryReference nodeFactoryReference()const override;
};
options::ObjectRef bindNodeArray(ParserRuntime&,mml::Children);
}
