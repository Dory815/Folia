// Numerical portion of Source CommonMsqrt/CommonMroot; wrapper orchestration remains unimplemented.
export module folia.math.output.common:msqrt;
import std;
import folia.math.util.bbox;
import :mroot;

export namespace folia::math::output::common {
struct RadicalFontParameters {
    double rule_thickness = 0;
    double surd_height = 0;
    double x_height = 0;
};

struct RadicalInput {
    BBox base;
    std::optional<BBox> degree;
    RadicalFontParameters font;
    bool display = false;
    double wrapper_bbox_scale = 1;
};

struct RadicalPoint { double x = 0; double y = 0; };
struct RadicalRect { double x = 0; double y = 0; double width = 0; double height = 0; };

struct RadicalStretchRequest {
    BBox base;
    std::optional<BBox> degree;
    RadicalFontParameters font;
    bool display = false;
    double wrapper_bbox_scale = 1;
    double required_surd_height = 0;
    double stretch_height = 0;
    double stretch_depth = 0;
    bool stretch_exact = true;
    double root_width = 0;
};

struct RadicalLayout {
    BBox bbox;
    double p = 0;
    double q = 0;
    double required_surd_height = 0;
    double measured_height = 0;
    double placed_height = 0;
    double placed_surd_height = 0;
    double root_offset = 0;
    double root_height = 0;
    double root_dx = 0;
    RadicalPoint surd_origin;
    RadicalPoint base_origin;
    std::optional<RadicalPoint> degree_origin;
    RadicalRect overbar;
};

RadicalStretchRequest request_radical(RadicalInput const& input);
RadicalLayout finish_radical(RadicalStretchRequest const& request,
                             BBox const& actual_surd_bbox, int actual_surd_size);
}
