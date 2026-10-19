export module folia.math.output.common:notation;
export import folia.math.util.bbox;
export import folia.math.output.scene.types;
import std;

export namespace folia::math::output::common {
using PaddingData = std::array<double, 4>; // Source order: top,right,bottom,left.

// Each value names a finite MathJax 4 SVG-profile notation. No executable
// renderer, callback, JavaScript object or output-backend value is retained.
enum class NotationKind : std::uint8_t {
    top,
    right,
    bottom,
    left,
    actuarial,
    madruwb,
    up_diagonal_strike,
    down_diagonal_strike,
    horizontal_strike,
    vertical_strike,
    box,
    rounded_box,
    circle,
    phasor_angle,
    up_arrow,
    down_arrow,
    left_arrow,
    right_arrow,
    up_down_arrow,
    left_right_arrow,
    up_diagonal_arrow,
    northeast_arrow,
    southeast_arrow,
    northwest_arrow,
    southwest_arrow,
    northeast_southwest_arrow,
    northwest_southeast_arrow,
    long_div,
    radical
};
enum class BBoxRecipe : std::uint8_t {
    full,
    full_padding,
    one_side,
    two_sides,
    diagonal_strike,
    horizontal_strike,
    vertical_strike,
    arrow,
    diagonal_arrow,
    phasor_angle,
    long_div,
    radical
};
enum class BorderRecipe : std::uint8_t { none, one_side, two_sides, full, phasor_bottom };
enum class PaintRecipe : std::uint8_t {
    side_line,
    diagonal_line,
    horizontal_line,
    vertical_line,
    box,
    rounded_box,
    circle,
    phasor_angle,
    arrow,
    diagonal_arrow,
    long_div,
    radical
};

struct NotationSpec {
    NotationKind kind;
    std::string_view source_name;
    BBoxRecipe bbox;
    BorderRecipe border;
    PaintRecipe paint;
    bool render_child{};
    std::span<std::string_view const> removes{};
};

struct Arrowhead {
    double x = 4, y = 2, dx = 1;
};

struct ArrowMetrics {
    double angle{}, width{}, x{}, y{};
};
enum class LineName : std::uint8_t { top, right, bottom, left, vertical, horizontal, up, down };

struct ArrowGeometry {
    scene::PathGeometry path;
    scene::Transform transform;
};

struct NotationGeometry {
    scene::PathGeometry path;
    scene::Transform transform;
    scene::Paint paint;
};

[[nodiscard]] std::optional<NotationKind> findNotation(std::span<NotationSpec const>,
                                                       std::u16string_view source_name);
[[nodiscard]] double arrowHead(double padding, double thickness, Arrowhead) noexcept;
[[nodiscard]] PaddingData notationBBox(NotationSpec const &, double padding, double thickness,
                                       Arrowhead, BBox const *content, PaddingData radical = {});
[[nodiscard]] PaddingData notationBorder(NotationSpec const &, double thickness);
[[nodiscard]] ArrowMetrics arrowData(double padding, double thickness, Arrowhead,
                                     BBox const &child) noexcept;
[[nodiscard]] std::pair<double, double> arrowAW(PaddingData trbl, BBox const &child) noexcept;
[[nodiscard]] std::pair<double, double> getArgMod(double width, double height) noexcept;
[[nodiscard]] double getOffset(PaddingData trbl, std::string_view direction) noexcept;
[[nodiscard]] scene::PathGeometry lineGeometry(LineName, BBox const &, double thickness);
[[nodiscard]] std::vector<NotationGeometry> notationGeometry(NotationSpec const &, BBox const &,
                                                             double thickness, double padding,
                                                             Arrowhead, PaddingData trbl,
                                                             BBox const *content = nullptr);
[[nodiscard]] ArrowGeometry arrowGeometry(double width, double angle, bool double_headed,
                                          std::string_view offset, double distance,
                                          double thickness, Arrowhead, BBox const &content);
} // namespace folia::math::output::common
