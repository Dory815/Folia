// Native immutable lookup for MathJax 4.1.3 input/tex/color/ColorConstants.ts.
module folia.math.input.tex.color.constants;
import std;

namespace folia::math::tex {
std::optional<std::string_view> ColorConstants::find(std::u16string_view name) noexcept {
    for (auto const& [key,value] : ColorConstantValues) {
        if (name.size() != key.size()) continue;
        bool equal = true;
        for (std::size_t i=0; i<name.size(); ++i) {
            if (name[i] != static_cast<unsigned char>(key[i])) {
                equal = false;
                break;
            }
        }
        if (equal) return value;
    }
    return std::nullopt;
}
}
