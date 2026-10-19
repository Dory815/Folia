module folia.math.output.common.placement.fraction;
import std;
import folia.math.util.bbox;

namespace folia::math::output::common::placement {
namespace {
    constexpr double kRulePadding = 0.2;
    constexpr double kRuleHalfPadding = 0.1;

    double source_max(double left, double right) {
        if (std::isnan(left) || std::isnan(right)) {
            return std::numeric_limits<double>::quiet_NaN();
        }
        return std::max(left, right);
    }

    double align_x(double width, BBox const& box, FractionAlignment alignment) {
        if (alignment == FractionAlignment::right) {
            return width - (box.w + box.R) * box.rscale;
        }
        if (alignment == FractionAlignment::left) {
            return box.L * box.rscale;
        }
        return (width - box.w * box.rscale) / 2;
    }

    double combined_child_width(BBox const& box) {
        return (box.L + box.w + box.R) * box.rscale;
    }

    double child_wrap_width(BBox const& box) {
        return box.w;
    }

    double pad_for(FractionInput const& input) {
        return input.with_delimiters ? 0.0 : input.font.nulldelimiterspace;
    }

    BBox empty_bbox() {
        BBox result;
        result.empty();
        return result;
    }
}

FractionResult measure_fraction(FractionInput const& input, FractionKind kind) {
    if (kind == FractionKind::bevelled) {
        const double delta = input.display ? 0.4 : 0.15;
        const double required_height = source_max(
            input.numerator.scale * (input.numerator.h + input.numerator.d),
            input.denominator.scale * (input.denominator.h + input.denominator.d)) + 2 * delta;
        const double numerator_y = input.numerator.scale *
            (input.numerator.d - input.numerator.h) / 2 + input.font.axis_height + delta;
        const double denominator_y = input.denominator.scale *
            (input.denominator.d - input.denominator.h) / 2 + input.font.axis_height - delta;
        return BevelRequest{required_height, delta, numerator_y, denominator_y,
            input.numerator, input.denominator, input.font, input.display};
    }

    FractionLayout result;
    result.kind = kind;
    result.bbox = empty_bbox();
    const double pad = pad_for(input);
    const double width = source_max(combined_child_width(input.numerator),
                                    combined_child_width(input.denominator));

    if (kind == FractionKind::atop || input.line_thickness == 0) {
        result.kind = FractionKind::atop;
        double u = input.display ? input.font.num1 : input.font.num3;
        double v = input.display ? input.font.denom1 : input.font.denom2;
        double q = u - input.numerator.d * input.numerator.scale -
            (input.denominator.h * input.denominator.scale - v);
        const double minimum_gap = (input.display ? 7.0 : 3.0) * input.font.rule_thickness;
        if (q < minimum_gap) {
            u += (minimum_gap - q) / 2;
            v += (minimum_gap - q) / 2;
            q = minimum_gap;
        }
        (void)q;
        result.bbox.combine(input.numerator, 0, u);
        result.bbox.combine(input.denominator, 0, -v);
        result.bbox.w += 2 * pad;
        result.bbox.clean();
        result.numerator_origin = {
            align_x(width, input.numerator, input.numerator_alignment) + pad, u};
        result.denominator_origin = {
            align_x(width, input.denominator, input.denominator_alignment) + pad, -v};
        double percent_width = -2 * pad;
        percent_width += result.bbox.w;
        result.content_width = percent_width;
        const double wrap_width = result.bbox.w - 0.0 - 2 * pad;
        result.numerator_wrap_width = wrap_width;
        result.denominator_wrap_width = wrap_width;
        return result;
    }

    const double axis = input.font.axis_height;
    const double thickness = input.line_thickness;
    const double T = (input.display ? 3.5 : 1.5) * thickness;
    const double u = (input.display ? input.font.num1 : input.font.num2) - axis - T;
    const double v = (input.display ? input.font.denom1 : input.font.denom2) + axis - T;
    const double numerator_y = axis + T + source_max(input.numerator.d * input.numerator.rscale, u);
    const double denominator_y = axis - T - source_max(input.denominator.h * input.denominator.rscale, v);
    result.bbox.combine(input.numerator, 0, numerator_y);
    result.bbox.combine(input.denominator, 0, denominator_y);
    result.bbox.w += 2 * pad + kRulePadding;
    result.bbox.clean();
    result.numerator_origin = {
        align_x(width, input.numerator, input.numerator_alignment) + kRuleHalfPadding + pad,
        numerator_y};
    result.denominator_origin = {
        align_x(width, input.denominator, input.denominator_alignment) + kRuleHalfPadding + pad,
        denominator_y};
    result.rule = FractionRule{pad, axis - thickness / 2, width + kRulePadding, thickness};
    double percent_width = -2 * pad;
    percent_width -= kRulePadding;
    percent_width += result.bbox.w;
    result.content_width = percent_width;
    const double wrap_width = result.bbox.w - kRulePadding - 2 * pad;
    result.numerator_wrap_width = wrap_width;
    result.denominator_wrap_width = wrap_width;
    return result;
}

FractionLayout finish_bevelled_fraction(BevelRequest const& request, BBox const& actual_slash_bbox) {
    FractionLayout result;
    result.kind = FractionKind::bevelled;
    result.bbox = empty_bbox();
    const double numerator_width = combined_child_width(request.numerator);
    const double slash_x = numerator_width - request.delta / 2;
    result.bbox.combine(request.numerator, 0, request.numerator_y);
    result.bbox.combine(actual_slash_bbox, result.bbox.w - request.delta / 2, 0);
    const double bbox_denominator_x = result.bbox.w - request.delta / 2;
    result.bbox.combine(request.denominator, bbox_denominator_x, request.denominator_y);
    result.bbox.clean();
    result.numerator_origin = {
        request.numerator.L * request.numerator.rscale, request.numerator_y};
    result.slash_origin = FractionPoint{slash_x, 0};
    result.denominator_origin = {
        numerator_width + actual_slash_bbox.w + request.denominator.L * request.denominator.rscale - request.delta,
        request.denominator_y};
    result.bbox_denominator_origin = FractionPoint{bbox_denominator_x, request.denominator_y};
    result.content_width = std::nullopt;
    result.numerator_wrap_width = child_wrap_width(request.numerator);
    result.denominator_wrap_width = child_wrap_width(request.denominator);
    return result;
}
}
