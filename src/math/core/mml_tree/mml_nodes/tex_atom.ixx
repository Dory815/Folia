// Typed native TeXAtom API based on MathJax 4.1.3 MmlNodes/TeXAtom.ts.
export module folia.math.core.mml_tree.mml_nodes.tex_atom;
export import folia.math.core.mml_tree.mml_document;
import std;

export namespace folia::math::mml {
class MmlTeXAtom : public AbstractMmlBaseNode {
  public:
    explicit MmlTeXAtom(MmlDocument &document);
    static AttributeSet defaults();
    NodeKind kind() const noexcept override;
    double arity() const override;
    bool notParent() const override;
    MmlNode *setTeXclass(MmlNode *previous) override;
};
} // namespace folia::math::mml
