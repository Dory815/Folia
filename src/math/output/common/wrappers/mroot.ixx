// Degree-placement equations from Source CommonMroot. Kept independent of radical request state.
export module folia.math.output.common:mroot;
import std;
import folia.math.util.bbox;

export namespace folia::math::output::common {
struct RootDimensions {
    double x = 0;
    double height = 0;
    double dx = 0;
};

RootDimensions root_dimensions(BBox const& degree, BBox const& surd,
                               int surd_size, double whole_height);
double root_width(BBox const& degree);
}
