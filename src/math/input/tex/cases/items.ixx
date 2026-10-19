// MathJax 4.1.3 input/tex/cases/CasesConfiguration.ts.
export module folia.math.input.tex.cases.items;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
class CasesBeginItem final : public BeginItem {
public:
    std::string kind() const override;
    CheckType checkItem(BaseItem*) override;
    static tree::FactoryClassRef<BaseItem> initializeModule(ParserRuntime&);
};
}
