// MathJax 4.1.3 input/tex/ams/AmsItems.ts.
export module folia.math.input.tex.ams.items;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
class MultlineItem:public ArrayItem {
public:
    void initializeMultlineItem(FunctionArgs);
    std::string kind()const override;
    void EndEntry()override;void EndRow()override;void EndTable()override;
};
class FlalignItem final:public EqnArrayItem {
public:
    std::u16string name;bool numbered=false,padded=false,center=false;
    void initializeFlalignItem(FunctionArgs);
    std::string kind()const override;
    void EndEntry()override;void EndRow()override;void EndTable()override;
};
struct AmsItems { static void initializeModule(ParserRuntime&); };
}
