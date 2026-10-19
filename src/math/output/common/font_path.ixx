export module folia.math.output.common.font_path;
import folia.math.output.scene.types;
import std;
export namespace folia::math::output::common {
enum class FontOutlineErrorReason : std::uint8_t {
    missing_command,
    unsupported_command,
    invalid_number,
    incomplete_segment
};

struct FontOutlineError {
    std::size_t byte_offset{};
    char command{};
    FontOutlineErrorReason reason{};
    friend bool operator==(FontOutlineError const&, FontOutlineError const&) = default;
};

struct FontOutlineDecodeResult {
    output::scene::GlyphOutline outline;
    std::optional<FontOutlineError> error;
    bool complete() const noexcept { return !error.has_value(); }
};

FontOutlineDecodeResult decodeFontOutlinePrefix(std::string_view path_data,
                                                 double units_per_em = 1000.0);
output::scene::GlyphOutline decodeFontOutline(std::string_view path_data,
                                               double units_per_em = 1000.0);
}
