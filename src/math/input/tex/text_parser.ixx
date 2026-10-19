// MathJax 4.1.3 input/tex/textmacros/TextParser.ts; Apache-2.0.
export module folia.math.input.tex.parser:text_parser;
export import :tex_parser;
export import :base_items;
import folia.math.input.tex.tex_parser_context_fields;
import std;
export namespace folia::math::tex {
class TextParser:public TexParser {
    std::u16string text_;
    std::shared_ptr<TextParserEnvStackOwner> envStackOwner_;
    Property level_=Missing{};
    mml::Children nodes_=std::make_shared<tree::ChildArray<mml::MmlNode>>();
public:
    static void initializeModule(ParserRuntime&);
    static std::shared_ptr<TextParser> make(ParserRuntime&,std::u16string,Property,ParseOptions&,Property level=Missing{});
    static options::Value internalMathFunction(ParserRuntime&);
    std::u16string& text(){return text_;}
    const std::u16string& text()const{return text_;}
    void pushEnvStack(Property value){envStackOwner_->push(std::move(value));}
    bool envStackEmpty()const{return envStackOwner_->empty();}
    Property envStackBack()const{return envStackOwner_->back();}
    Property popEnvStack(){return envStackOwner_->pop();}
    options::Value envStackContextValue()const{return envStackOwner_?options::Value(envStackOwner_->identity()):options::Value(options::Undefined{});}
    void replaceEnvStack(options::Value value){envStackOwner_->replace(std::move(value));}
    options::ObjectRef resetEnvStack(options::Context& context,ContextPropertyBindings& properties,
        std::shared_ptr<void> owner,std::weak_ptr<std::atomic_bool> lease){
        envStackOwner_=std::make_shared<TextParserEnvStackOwner>(context,properties,std::move(owner),std::move(lease));
        return envStackOwner_->identity();
    }
    Property level()const{return level_;}
    mml::Children nodes()const{return nodes_;}
    options::Value texParserValue()const;
    virtual TexParser& texParser()const;
    ParserShared<Tags> tags()const override;
    mml::NodeValue mml()override;
    void copyLists();
    void Parse()override;
    void saveText();
    void Push(StackEntry)override;
    void PushMath(mml::NodeValue);
    void addAttributes(mml::NodeValue);
    mml::NodeValue ParseTextArg(std::u16string,Property);
    mml::NodeValue ParseArg(std::u16string)override;
    [[noreturn]] void Error(std::u16string,std::u16string,std::span<const Property> args={});
};
}
