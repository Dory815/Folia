module folia.math.output.common;
import :mroot;
import std;
import folia.math.util.bbox;

namespace folia::math::output::common {
namespace {
double source_max(double left, double right) {
    if (std::isnan(left) || std::isnan(right))
        return std::numeric_limits<double>::quiet_NaN();
    return std::max(left, right);
}
}

RootDimensions root_dimensions(BBox const& degree, BBox const& surd,
                               int surd_size, double whole_height) {
    const double offset = (surd_size < 0 ? 0.5 : 0.6) * surd.w;
    const double width = source_max(degree.w, offset / degree.rscale);
    const double dx = source_max(0, width - degree.w);
    const double surd_total_height = surd.h + surd.d;
    const double b = (surd_size < 0 ? 1.9 : 0.55 * surd_total_height) -
        (surd_total_height - whole_height);
    const double height = b + source_max(0, degree.d * degree.rscale);
    const double x = width * degree.rscale - offset;
    return {x, height, dx};
}

double root_width(BBox const& degree) {
    return 0.4 + degree.w * degree.rscale;
}
}
