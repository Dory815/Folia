// MathJax 4.1.3 MmlNodes/mglyph.ts; Apache-2.0 (LICENSE-MathJax.txt).
// Source SHA256: 2c2d336067d3970c55f3fcdafb82b55d4339a2c11d69088c8c7eff4a6d953504.
export module folia.math.core.mml_tree.mml_nodes.mglyph;
export import folia.math.core.mml_tree.mml_document;
import std;

export namespace folia::math::mml {
class MmlMglyph : public AbstractMmlTokenNode {
  public:
    explicit MmlMglyph(MmlDocument &);
    static AttributeSet defaults();
    NodeKind kind() const noexcept override;

  protected:
    void verifyAttributes(VerifyOptions const &) override;
};
} // namespace folia::math::mml
