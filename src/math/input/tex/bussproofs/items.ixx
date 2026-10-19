// MathJax 4.1.3 input/tex/bussproofs/BussproofsItems.ts.
export module folia.math.input.tex.bussproofs.items;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
class ProofTreeItem final : public BaseItem {
    using InnerStackValue=std::variant<std::monostate,StackRef,options::Value>;
    InnerStackValue innerStack_;
    Stack& innerStack(std::string_view member="Push");
public:
    StackRef innerStackOwner(){(void)innerStack();return std::get<StackRef>(innerStack_);}
    options::Value innerStackSourceValue(ParserRuntime&);
    void setInnerStackSourceValue(ParserRuntime&,options::Value);
    options::Value leftLabel=options::Null{};
    options::Value rigthLabel=options::Null{};
    std::string kind() const override;
    CheckType checkItem(BaseItem*) override;
    mml::NodeValue toMml(Property inferred=Missing{},Property forceRow=Missing{}) override;
    static tree::FactoryClassRef<BaseItem> initializeModule(ParserRuntime&);
};
struct BussproofsItems { static tree::FactoryClassRef<BaseItem> initializeModule(ParserRuntime&); };
}
