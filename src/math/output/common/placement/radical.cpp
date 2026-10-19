module folia.math.output.common.placement.radical;
import std;
import folia.math.util.bbox;

namespace folia::math::output::common::placement {
namespace {
    double source_max(double left, double right) {
        if (std::isnan(left) || std::isnan(right)) {
            return std::numeric_limits<double>::quiet_NaN();
        }
        return std::max(left, right);
    }

    double radical_p(RadicalStretchRequest const& request) {
        return request.display ? request.font.x_height : request.font.rule_thickness;
    }

    double radical_q(
        RadicalStretchRequest const& request,
        BBox const& surd_bbox,
        double p) {
        const double rule = request.font.rule_thickness;
        const double surd_height = request.font.surd_height;
        if (surd_bbox.h + surd_bbox.d > request.required_surd_height) {
            return (surd_bbox.h + surd_bbox.d -
                (request.required_surd_height - rule - surd_height - p / 2)) / 2;
        }
        return surd_height + p / 4;
    }

    struct RootDimensions {
        double x = 0;
        double height = 0;
        double dx = 0;
    };

    RootDimensions root_dimensions(
        RadicalStretchRequest const& request,
        BBox const& surd_bbox,
        int surd_size,
        double whole_height) {
        if (!request.degree.has_value()) {
            return {};
        }

        const BBox& degree = request.degree.value();
        const double offset = (surd_size < 0 ? 0.5 : 0.6) * surd_bbox.w;
        const double width = source_max(degree.w, offset / degree.rscale);
        const double dx = source_max(0, width - degree.w);
        const double surd_total_height = surd_bbox.h + surd_bbox.d;
        const double b = (surd_size < 0 ? 1.9 : 0.55 * surd_total_height) -
            (surd_total_height - whole_height);
        const double height = b + source_max(0, degree.d * degree.rscale);
        const double x = width * degree.rscale - offset;
        return {x, height, dx};
    }
}

RadicalStretchRequest request_radical(RadicalInput const& input) {
    RadicalStretchRequest request;
    request.base = input.base;
    request.degree = input.degree;
    request.font = input.font;
    request.display = input.display;
    request.wrapper_bbox_scale = input.wrapper_bbox_scale;

    const double p = radical_p(request);
    request.required_surd_height = input.base.h + input.base.d +
        request.font.rule_thickness + request.font.surd_height + p / 4;
    request.stretch_height = request.required_surd_height - input.base.d;
    request.stretch_depth = input.base.d;
    request.stretch_exact = true;
    if (request.degree.has_value()) {
        const BBox& degree = request.degree.value();
        request.root_width = 0.4 + degree.w * degree.rscale;
    } else {
        request.root_width = 1.25;
    }
    return request;
}

RadicalLayout finish_radical(
    RadicalStretchRequest const& request,
    BBox const& actual_surd_bbox,
    int actual_surd_size) {
    RadicalLayout result;
    result.bbox.empty();
    result.p = radical_p(request);
    result.q = radical_q(request, actual_surd_bbox, result.p);
    result.required_surd_height = request.required_surd_height;

    // Source uses `new BBox(baseOuter)`, keeping only w, h, and d.
    BBox measured_base(BBoxData{
        .w = request.base.w,
        .h = request.base.h,
        .d = request.base.d,
    });
    result.measured_height = measured_base.h + result.q + request.font.rule_thickness;
    const RootDimensions measured_root = root_dimensions(
        request, actual_surd_bbox, actual_surd_size, result.measured_height);

    result.bbox.h = result.measured_height + request.font.surd_height;
    if (request.degree.has_value()) {
        result.bbox.combine(request.degree.value(), 0, measured_root.height);
    }
    result.bbox.combine(actual_surd_bbox, measured_root.x,
                        result.measured_height - actual_surd_bbox.h);
    result.bbox.combine(measured_base, measured_root.x + actual_surd_bbox.w, 0);
    result.bbox.clean();

    result.placed_surd_height = request.font.surd_height * request.wrapper_bbox_scale;
    result.placed_height = request.base.h + result.q + result.placed_surd_height;
    const RootDimensions placed_root = root_dimensions(
        request, actual_surd_bbox, actual_surd_size, result.placed_height);
    result.root_offset = placed_root.x;
    result.root_height = placed_root.height;
    result.root_dx = placed_root.dx;
    result.surd_origin = {placed_root.x, result.placed_height - actual_surd_bbox.h};
    result.base_origin = {placed_root.x + actual_surd_bbox.w, 0};
    if (request.degree.has_value()) {
        result.degree_origin = {
            placed_root.dx * request.degree->rscale,
            placed_root.height,
        };
    }
    result.overbar = {
        placed_root.x + actual_surd_bbox.w,
        result.placed_height - result.placed_surd_height,
        request.base.w,
        result.placed_surd_height,
    };
    return result;
}
}
