// MathJax 4.1.3 input/tex/braket/BraketItems.ts.
export module folia.math.input.tex.braket.items;
export import folia.math.input.tex.parser;
import folia.math.core.mml_tree.mml_node;
import folia.math.core.tree.factory;
import std;
export namespace folia::math::tex {
class BraketItem final:public BaseItem {
    std::vector<mml::NodeValue> barNodes_;
public:
    std::string kind()const override;
    bool isOpen()const override;
    CheckType checkItem(BaseItem*)override;
    mml::NodeValue toMml(Property inferred=Missing{},Property forceRow=Missing{})override;
    std::vector<mml::NodeValue>& barNodes()noexcept{return barNodes_;}
    static tree::FactoryClassRef<BaseItem> initializeModule(ParserRuntime&);
};
}
