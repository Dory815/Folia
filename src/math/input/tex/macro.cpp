// Native compiled callback bindings for Token.ts::Macro. Apache-2.0.
module folia.math.input.tex.parser;
import std;

namespace folia::math::tex {
MacroCallbacks::MacroCallbacks(std::span<MacroHandler const> macros,
                               std::span<TokenInputHandler const> tokens)
    : macros_(macros.begin(), macros.end()), tokens_(tokens.begin(), tokens.end()) {
    if (macros_.size() > std::numeric_limits<std::uint32_t>::max() ||
        tokens_.size() > std::numeric_limits<std::uint32_t>::max() ||
        std::ranges::find(macros_, nullptr) != macros_.end() ||
        std::ranges::find(tokens_, nullptr) != tokens_.end())
        throw std::invalid_argument("Invalid compiled TeX callback table");
}
MacroHandler MacroCallbacks::macroHandler(MacroHandlerId id) const {
    if (!id.value || id.value > macros_.size())
        throw std::out_of_range("TeX macro callback is not bound in this profile");
    return macros_[id.value - 1];
}
TokenInputHandler MacroCallbacks::tokenParser(TokenParserId id) const {
    if (!id.value || id.value > tokens_.size())
        throw std::out_of_range("TeX token callback is not bound in this profile");
    return tokens_[id.value - 1];
}
MacroHandlerId MacroCallbacks::macroBinding(MacroHandler function) const noexcept {
    auto entry = std::ranges::find(macros_, function);
    return {entry == macros_.end() ? 0 : static_cast<std::uint32_t>(entry - macros_.begin() + 1)};
}
TokenParserId MacroCallbacks::tokenBinding(TokenInputHandler function) const noexcept {
    auto entry = std::ranges::find(tokens_, function);
    return {entry == tokens_.end() ? 0 : static_cast<std::uint32_t>(entry - tokens_.begin() + 1)};
}
}
