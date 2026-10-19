// MathJax 4.1.3 input/tex/mathtools/MathtoolsItems.ts.
export module folia.math.input.tex.mathtools.items;
export import folia.math.input.tex.ams.items;
import std;
export namespace folia::math::tex {
class MultlinedItem final : public MultlineItem {
public:
    void initializeMultlinedItem(FunctionArgs);
    std::string kind() const override;
    void EndTable() override;
    static StackItemClassRef initializeModule(ParserRuntime&);
};
}
