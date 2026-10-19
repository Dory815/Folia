export module folia.math.output.scene.types;
import std;

export namespace folia::math::output::scene {
// All geometry is in ems. The common layout pass supplies these values;
// consumers serialize/draw them without recomputing mathematical placement.
using Unit = double;
using NodeId = std::uint64_t;

struct Point { Unit x{}, y{}; friend bool operator==(Point const&, Point const&) = default; };
struct Size { Unit width{}, height{}; friend bool operator==(Size const&, Size const&) = default; };
struct Rect { Unit x{}, y{}, width{}, height{}; friend bool operator==(Rect const&, Rect const&) = default; };
// Column-vector affine transform: (x',y')=(a*x+c*y+e,b*x+d*y+f).
// Local child transform is applied first, then its parent's: world=P*L.
struct Affine {
    Unit a{1}, b{}, c{}, d{1}, e{}, f{};
    friend bool operator==(Affine const&, Affine const&) = default;
};
[[nodiscard]] constexpr Affine compose(Affine const& parent, Affine const& local) noexcept {
    return {
        parent.a*local.a + parent.c*local.b,
        parent.b*local.a + parent.d*local.b,
        parent.a*local.c + parent.c*local.d,
        parent.b*local.c + parent.d*local.d,
        parent.a*local.e + parent.c*local.f + parent.e,
        parent.b*local.e + parent.d*local.f + parent.f
    };
}

// Baseline is y=0, x increases right, y increases up. Signed values preserve
// completed common-layout BBox metrics, including negative extents.
struct Metrics { Unit width{}, height{}, depth{}; friend bool operator==(Metrics const&, Metrics const&) = default; };
struct FontMetrics { Unit em{1}, ex{}, x_height{}, cap_height{}; friend bool operator==(FontMetrics const&, FontMetrics const&) = default; };
struct Color { std::uint8_t red{}, green{}, blue{}, alpha{255}; friend bool operator==(Color const&, Color const&) = default; };
enum class FillRule { nonzero, evenodd };
struct InheritedPaint { friend bool operator==(InheritedPaint, InheritedPaint) = default; };
struct NoPaint { friend bool operator==(NoPaint, NoPaint) = default; };
struct CurrentColorPaint { friend bool operator==(CurrentColorPaint, CurrentColorPaint) = default; };
using PaintColor = std::variant<InheritedPaint, NoPaint, CurrentColorPaint, Color>;
struct Paint {
    // Closed CSS/SVG paint alternatives preserve inheritance, explicit none,
    // currentColor, and literal color as distinct semantics.
    PaintColor fill{InheritedPaint{}};
    PaintColor stroke{InheritedPaint{}};
    Unit stroke_width{};
    Unit opacity{1};
    FillRule fill_rule{FillRule::nonzero};
    friend bool operator==(Paint const&, Paint const&) = default;
};

struct MoveTo { Point to; friend bool operator==(MoveTo const&, MoveTo const&) = default; };
struct LineTo { Point to; friend bool operator==(LineTo const&, LineTo const&) = default; };
struct QuadTo { Point control, to; friend bool operator==(QuadTo const&, QuadTo const&) = default; };
struct CubicTo { Point control1, control2, to; friend bool operator==(CubicTo const&, CubicTo const&) = default; };
struct ArcTo {
    Unit radius_x{}, radius_y{}, rotation{};
    bool large_arc{}, sweep{};
    Point to;
    friend bool operator==(ArcTo const&, ArcTo const&) = default;
};
struct ClosePath { friend bool operator==(ClosePath const&, ClosePath const&) = default; };
using PathSegment = std::variant<MoveTo, LineTo, QuadTo, CubicTo, ArcTo, ClosePath>;
struct PathGeometry { std::vector<PathSegment> segments; };

struct GlyphOutline { PathGeometry outline; };
class GlyphResource final {
    std::string font_id_;
    std::shared_ptr<GlyphOutline const> outline_;
    GlyphResource(std::string font_id, GlyphOutline outline)
        : font_id_(std::move(font_id)),
          outline_(std::make_shared<GlyphOutline const>(std::move(outline))) {}
public:
    GlyphResource(GlyphResource const&) = delete;
    GlyphResource& operator=(GlyphResource const&) = delete;
    [[nodiscard]] static std::shared_ptr<GlyphResource const> make(
        std::string font_id, GlyphOutline outline) {
        return std::shared_ptr<GlyphResource const>(
            new GlyphResource(std::move(font_id), std::move(outline)));
    }
    [[nodiscard]] std::string_view font_id() const noexcept { return font_id_; }
    [[nodiscard]] GlyphOutline const& outline() const noexcept { return *outline_; }
};
struct Glyph {
    std::shared_ptr<GlyphResource const> resource;
    std::uint32_t glyph_id{};
    Point origin{};
    Unit advance{};
    Paint paint{};
};
struct GlyphRun { std::vector<Glyph> glyphs; };
struct Rule { Rect bounds; Paint paint{}; };
struct Path { PathGeometry geometry; Paint paint{}; };
struct TextRun {
    std::string utf8;
    Point origin{};
    Metrics metrics{};
    GlyphRun measured_glyphs;
    Paint paint{};
};
struct Clip { PathGeometry geometry; FillRule fill_rule{FillRule::nonzero}; };
struct Link { std::string href; std::string target; std::string rel; };
struct Tooltip { std::string text; };
struct SemanticAttribute { std::string name, value; };
struct SemanticMetadata {
    std::string source_tag;
    std::string math_role;
    std::string annotation_encoding;
    std::string annotation;
    // Ordered spelling needed by Source output and accessibility consumers.
    std::vector<SemanticAttribute> ordered_attributes;
};
struct ForeignRun { TextRun run; };
// HTML/foreign content stays owned and retains already measured runs. `content`
// is foreign HTML payload, never an SVG/XML DOM or a layout instruction.
struct ForeignContent {
    std::string content;
    Point origin{};
    std::vector<ForeignRun> measured_runs;
    Metrics metrics{};
    SemanticMetadata metadata{};
};
using Primitive = std::variant<GlyphRun, Rule, Path, TextRun, ForeignContent>;
}
