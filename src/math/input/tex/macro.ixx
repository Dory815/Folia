// MathJax 4.1.3 Token.ts::Macro; Apache-2.0.
export module folia.math.input.tex.parser:macro;
export import :types;
import std;

export namespace folia::math::tex {
struct Macro {
    std::u16string token;
    MacroMethod function;
    std::vector<TexArgument> arguments;
    Macro(std::u16string name, MacroMethod method, std::vector<TexArgument> args = {})
        : token(std::move(name)), function(method), arguments(std::move(args)) {}
};

// The input-jax profile supplies two fixed typed function tables. Definitions
// store their stable one-based indices; zero is absent. There is no registration,
// name lookup, erased argument call, captured receiver or per-request allocation.
class MacroCallbacks final {
    std::vector<MacroHandler> macros_;
    std::vector<TokenInputHandler> tokens_;
public:
    MacroCallbacks(std::span<MacroHandler const>, std::span<TokenInputHandler const>);
    MacroCallbacks(MacroCallbacks const&) = delete;
    MacroCallbacks& operator=(MacroCallbacks const&) = delete;
    MacroHandler macroHandler(MacroHandlerId) const;
    TokenInputHandler tokenParser(TokenParserId) const;
    MacroHandlerId macroBinding(MacroHandler) const noexcept;
    TokenParserId tokenBinding(TokenInputHandler) const noexcept;
};
}
