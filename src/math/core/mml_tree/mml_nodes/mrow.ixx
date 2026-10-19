// MathJax 4.1.3 MmlNodes/mrow.ts; Apache-2.0 (LICENSE-MathJax.txt).
export module folia.math.core.mml_tree.mml_nodes.mrow;
export import folia.math.core.mml_tree.mml_document;
import std;

export namespace folia::math::mml {
class MmlMrow : public AbstractMmlNode {
  protected:
    mutable MmlNode *core_ = nullptr;
    MmlMrow(MmlDocument &, NodeKind);

  public:
    explicit MmlMrow(MmlDocument &);
    static AttributeSet defaults();
    NodeKind kind() const noexcept override;
    bool isSpacelike() const override;
    bool isEmbellished() const override;
    MmlNode *core() override;
    MmlNode *coreMO() override;
    virtual std::size_t nonSpaceLength();
    virtual MmlNode *firstNonSpace();
    virtual MmlNode *lastNonSpace();
    MmlNode *setTeXclass(MmlNode *) override;
};

class MmlInferredMrow : public MmlMrow {
  public:
    explicit MmlInferredMrow(MmlDocument &);
    static AttributeSet defaults();
    NodeKind kind() const noexcept override;
    bool isInferred() const override;
    bool notParent() const override;
    std::string toString() const override;
};
} // namespace folia::math::mml
