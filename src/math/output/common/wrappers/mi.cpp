module folia.math.output.common;
import :mi;

namespace folia::math::output::common {
void CommonMi::computeBBox(BBox &box, bool recompute) {
    (void)recompute;
    CommonWrapper::computeBBox(box, false);
    copySkewIC(box);
}
} // namespace folia::math::output::common
