// MathJax 4.1.3 input/tex/physics/PhysicsItems.ts; Apache-2.0.
export module folia.math.input.tex.physics.items;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
class AutoOpen : public BaseItem {
  protected:
    std::optional<StackErrorDefinition> getErrors(StackErrorKey) const override;

  public:
    std::u16string open, close, big, right;
    bool smash{}, ignore{};
    double openCount{};
    explicit AutoOpen(StackItemFactory &);
    StackKind kind() const override;
    bool isOpen() const noexcept override;
    bool closing(std::u16string_view fence);
    CheckResult checkItem(FrameHandle const &) override;
    mml::MmlNode &toMml(bool inferred = true, bool forceRow = false) override;
};
} // namespace folia::math::tex
