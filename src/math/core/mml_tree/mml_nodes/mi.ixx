// MathJax 4.1.3 MmlNodes/mi.ts; Apache-2.0 (LICENSE-MathJax.txt).
export module folia.math.core.mml_tree.mml_nodes.mi;
export import folia.math.core.mml_tree.mml_document;
import std;

export namespace folia::math::mml {
class MmlMi : public AbstractMmlTokenNode {
  public:
    explicit MmlMi(MmlDocument &);
    static AttributeSet defaults();
    static bool operatorName(std::u16string_view);
    static bool singleCharacter(std::u16string_view);
    NodeKind kind() const noexcept override;

  protected:
    void inheritAttributes(AttributeList const &, bool, double, bool) override;

  public:
    MmlNode *setTeXclass(MmlNode *) override;
};
} // namespace folia::math::mml
