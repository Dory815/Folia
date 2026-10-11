// Root replay of actual pinned CJS reductions, exact binary64 except NaN payload.
#include "support/folia_test.hpp"
import folia.math.util.numeric;
namespace {
using namespace boost::ut;
struct NumericCase {
 std::vector<std::uint64_t> input;
 bool sum_nan;std::uint64_t sum_bits;
 bool max_nan;std::uint64_t max_bits;
};
#include "numeric_reductions_source.inc"
bool exact(double result,bool nan,std::uint64_t bits){return nan?std::isnan(result):std::bit_cast<std::uint64_t>(result)==bits;}
suite numeric_reductions=[] {
"math_port_numeric_reductions_source"_test=[] {
 for(const auto&c:source_cases){std::vector<double> input;for(auto bits:c.input)input.push_back(std::bit_cast<double>(bits));
  expect(exact(folia::math::numeric::sum(input),c.sum_nan,c.sum_bits));
  expect(exact(folia::math::numeric::max(input),c.max_nan,c.max_bits));
 }
};
};
}
