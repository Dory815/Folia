// MathJax 4.1.3 input/tex/begingroup/BegingroupStack.ts; Apache-2.0.
export module folia.math.input.tex.begingroup.stack;
export import folia.math.input.tex.session;
export import folia.math.input.tex.tex_error;
import std;

export namespace folia::math::tex {
class BegingroupStack final {
    TexDefinitionState& state_;
    DefinitionLayer& globalTarget();
public:
    static constexpr std::int32_t newcommandPriority = TexDefinitionState::newcommand_priority;
    explicit BegingroupStack(TexDefinitionState& state) noexcept : state_(state) {}
    BegingroupStack(BegingroupStack const&) = delete;
    BegingroupStack& operator=(BegingroupStack const&) = delete;
    std::int32_t index() const noexcept;
    std::int32_t top() const noexcept;
    std::int32_t base() const noexcept;
    DefinitionLayer& currentLayer() noexcept;
    // Source's terminal marker removes the old target entry as well as shadows.
    DefinitionLayer& checkGlobal(std::span<DefinitionKey const>);
    void push();
    void pop();
    void finish() noexcept;
    void remove();
    void reset();
    void sandbox();
};
}
