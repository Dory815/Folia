export module folia.math.util.lengths;
import std;
import folia.math.util.constants;

export namespace folia::math::lengths {
struct LengthContext {
    double size_em = 0;
    double scale = 1;
    double em_pixels = 16;
    double ex_em = 0.431;
    double mu_em = 1.0 / 18.0;
    double pixels_per_inch = 96;
};

double length2em(std::string_view length, LengthContext const& context = {});
double length2em(double length, LengthContext const& context = {});
std::string format_decimal(double value, unsigned precision);
std::string percent(double value);
std::string em(double value);
std::string px(double value, double minimum_pixels = -1000000, double em_pixels = 16);
}
