/*************************************************************
 * Copyright (c) 2018-2026 The MathJax Consortium.
 * Native implementation of MathJax 4.1.3 util/numeric.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 * Source SHA256: a2004a9888f5c85f3cf81126e43d04dfe66b64c55e3b5f7d97366631809d7c15
 *************************************************************/
module folia.math.util.numeric;
import std;

namespace folia::math::numeric {

double sum(std::span<const double> values) {
    double result = 0.0;
    for (const double value : values) result = result + value;
    return result;
}

double max(std::span<const double> values) {
    double result = 0.0;
    for (const double value : values) {
        if (std::isnan(result) || std::isnan(value)) {
            result = std::numeric_limits<double>::quiet_NaN();
        } else if (value > result) {
            result = value;
        }
    }
    return result;
}

} // namespace folia::math::numeric
