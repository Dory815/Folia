// Numerical portion of Source CommonMfrac; wrapper orchestration remains unimplemented.
export module folia.math.output.common:mfrac;
import std;
import folia.math.util.bbox;

export namespace folia::math::output::common {
    enum class FractionAlignment { left, center, right };
    enum class FractionKind { ruled, atop, bevelled };

    struct FractionFontParameters {
        double axis_height = 0;
        double num1 = 0;
        double num2 = 0;
        double num3 = 0;
        double denom1 = 0;
        double denom2 = 0;
        double rule_thickness = 0;
        double nulldelimiterspace = 0;
    };

    struct FractionInput {
        BBox numerator;
        BBox denominator;
        FractionFontParameters font;
        bool display = false;
        bool with_delimiters = false;
        double line_thickness = 0;
        FractionAlignment numerator_alignment = FractionAlignment::center;
        FractionAlignment denominator_alignment = FractionAlignment::center;
    };

    struct FractionPoint { double x = 0; double y = 0; };
    struct FractionRule { double x = 0; double y = 0; double width = 0; double height = 0; };

    struct BevelRequest {
        double required_slash_height = 0;
        double delta = 0;
        double numerator_y = 0;
        double denominator_y = 0;
        BBox numerator;
        BBox denominator;
        FractionFontParameters font;
        bool display = false;
    };

    struct FractionLayout {
        FractionKind kind = FractionKind::ruled;
        BBox bbox;
        FractionPoint numerator_origin;
        FractionPoint denominator_origin;
        std::optional<FractionPoint> slash_origin;
        std::optional<FractionPoint> bbox_denominator_origin;
        std::optional<FractionRule> rule;
        std::optional<double> content_width;
        double numerator_wrap_width = 0;
        double denominator_wrap_width = 0;
    };

    using FractionResult = std::variant<FractionLayout, BevelRequest>;
    FractionResult measure_fraction(FractionInput const& input, FractionKind kind);
    FractionLayout finish_bevelled_fraction(BevelRequest const& request, BBox const& actual_slash_bbox);
}
