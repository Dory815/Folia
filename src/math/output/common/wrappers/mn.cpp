module folia.math.output.common;
import :mn;
import :output_jax;
import std;

namespace folia::math::output::common {
std::vector<std::uint32_t> CommonMn::remapChars(std::span<std::uint32_t const> characters) const {
    std::vector<std::uint32_t> result(characters.begin(), characters.end());
    if (result.empty())
        return result;

    const auto &remaps = jax().face().numberRemap;
    const auto found = remaps.find(result.front());
    if (found == remaps.end() || found->second.empty())
        return result;

    const auto replacement = unicodeChars(found->second, variant);
    if (replacement.size() == 1) {
        result.front() = replacement.front();
        return result;
    }
    std::vector<std::uint32_t> expanded;
    expanded.reserve(replacement.size() + result.size() - 1);
    expanded.insert(expanded.end(), replacement.begin(), replacement.end());
    expanded.insert(expanded.end(), result.begin() + 1, result.end());
    return expanded;
}
} // namespace folia::math::output::common
