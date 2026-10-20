// MathJax 4.1.3 input/tex/bussproofs/BussproofsItems.ts; Apache-2.0.
export module folia.math.input.tex.bussproofs.items;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
class ProofTreeItem final : public BaseItem {
    Stack innerStack_;

  public:
    std::optional<std::u16string> name;
    std::u16string line, currentLine;
    bool rootAtTop{};
    mml::MmlNode *left{}, *right{}; // borrowed labels in the active MmlDocument
    explicit ProofTreeItem(StackItemFactory &);
    StackKind kind() const override;
    std::optional<std::u16string_view> getName() const noexcept override;
    CheckResult checkItem(FrameHandle const &) override;
    mml::MmlNode &toMml(bool inferred = true, bool forceRow = false) override;
};
} // namespace folia::math::tex
