// MathJax 4.1.3 input/tex/physics/PhysicsItems.ts.
export module folia.math.input.tex.physics.items;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
class AutoOpen final:public BaseItem {
    double openCount_=0;
public:
    std::string kind()const override;
    bool isOpen()const override;
    bool closing(std::u16string_view fence);
    CheckType checkItem(BaseItem*)override;
    mml::NodeValue toMml(Property inferred=Missing{},Property forceRow=Missing{})override;
    static tree::FactoryClassRef<BaseItem> initializeModule(ParserRuntime&);
};
struct PhysicsItems { static options::Value initializeModule(ParserRuntime&); };
}
