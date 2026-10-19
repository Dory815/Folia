#include "../support/folia_test.hpp"
import folia.math.util.lengths;

using namespace boost::ut;
using namespace folia::math::lengths;

suite lengths_tests = [] {
    "native_length_prefix_and_units"_test = [] {
        LengthContext c{2, 2, 20, .5, .125, 100};
        expect(length2em("", c) == 2.0);
        expect(length2em("3em", c) == 3.0);
        expect(length2em("2ex", c) == 1.0);
        expect(length2em("8mu", c) == 1.0);
        expect(length2em("20px", c) == .5);
        expect(length2em("1in", c) == 2.5);
        expect(length2em("2cm", c) == 2.0 * (100.0 / 2.54) / 20 / 2);
        expect(length2em("25%", c) == .5);
        expect(length2em("2", c) == 4.0);
        expect(length2em("nonsense", c) == 2.0);
        expect(length2em("1.5em trailing", c) == 1.5);
    };
    "ecmascript_whitespace_and_named_math_spaces"_test = [] {
        LengthContext c{3};
        expect(length2em("\xE3\x80\x80 2em", c) == 2.0);
        expect(length2em("thinmathspace", c) == 3.0 / 18.0);
        expect(length2em("negativeverythickmathspace", c) == -6.0 / 18.0);
        expect(length2em("small", c) == 0.70710678118654752440);
        expect(length2em("2em\xC2\xA0", c) == 2.0);
    };
    "typed_numeric_and_formatting"_test = [] {
        LengthContext c{2};
        expect(length2em(1.5, c) == 3.0);
        expect(length2em(-.5, c) == -1.0);
        expect(length2em(1e-6, c) == 2e-6);
        expect(length2em(1e20, c) == 2e20);
        expect(length2em(1e21, c) == 2.0);
        expect(format_decimal(1.25, 1) == "1.3");
        expect(format_decimal(-1.25, 1) == "-1.3");
        expect(format_decimal(1.0625, 3) == "1.063");
        expect(format_decimal(-1.0625, 3) == "-1.063");
        expect(format_decimal(1.005, 2) == "1.00");
        expect(format_decimal(-0.0, 2) == "0.00");
        expect(format_decimal(1e21, 2) == "1e+21");
        expect(format_decimal(std::numeric_limits<double>::infinity(), 2) == "Infinity");
        expect(format_decimal(-std::numeric_limits<double>::infinity(), 2) == "-Infinity");
        expect(format_decimal(std::numeric_limits<double>::quiet_NaN(), 2) == "NaN");
        bool precision_rejected = false;
        try {
            static_cast<void>(format_decimal(1.0, 101));
        } catch (...) {
            precision_rejected = true;
        }
        expect(precision_rejected);
        expect(percent(.125) == "12.5%");
        expect(percent(1.0) == "100%");
        expect(percent(.0125) == "1.3%");
        expect(percent(-.0125) == "-1.3%");
        expect(percent(-0.0) == "0%");
        expect(percent(1e19) == "1e+21%");
        expect(percent(std::numeric_limits<double>::infinity()) == "Infinity%");
        expect(percent(std::numeric_limits<double>::quiet_NaN()) == "NaN%");
        expect(em(1.25) == "1.25em");
        expect(em(1.0625) == "1.063em");
        expect(em(-1.0625) == "-1.063em");
        expect(em(.0009) == "0");
        expect(em(-0.0) == "0");
        expect(em(std::numeric_limits<double>::infinity()) == "Infinityem");
        expect(em(std::numeric_limits<double>::quiet_NaN()) == "NaNem");
        expect(px(1.25) == "20px");
        expect(px(.125, -100, 10) == "1.3px");
        expect(px(-.125, -100, 10) == "-1.3px");
        expect(px(-0.0) == "0");
        expect(px(std::numeric_limits<double>::infinity()) == "Infinitypx");
        expect(px(std::numeric_limits<double>::quiet_NaN()) == "NaNpx");
        expect(px(.001) == "0");
        expect(px(1, 20) == "20px");
    };
};
