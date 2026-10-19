// MathJax 4.1.3 input/tex/newcommand/NewcommandItems.ts.
export module folia.math.input.tex.newcommand.items;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
class BeginEnvItem final:public BaseItem {
public:
    std::string kind()const override;
    bool isOpen()const override;
    CheckType checkItem(BaseItem*)override;
};
struct NewcommandItems { static void initializeModule(ParserRuntime&); };
}
