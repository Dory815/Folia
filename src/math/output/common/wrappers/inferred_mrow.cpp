/*************************************************************
 * Candidate native port of MathJax 4.1.3 inferred mrow specialization.
 * See src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.output.common;
import :inferred_mrow;
import std;

namespace folia::math::output::common {
void CommonInferredMrow::getScale() {
    if (!parent)
        throw std::logic_error("inferred mrow requires parent wrapper");
    bbox.scale = parent->bbox.scale;
    bbox.rscale = 1;
}
} // namespace folia::math::output::common
